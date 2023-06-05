//
// Stereocode Main
//
// Creation: 2021
//
// Given a srcML archive, each method/function is annotated with a stereotype as an
//   attribute on <function stereotype="get"> ... </function>
//
// Input assumptions:
//  One class per file (i.e., <unit/> )
//  For C++ assumes archive with two units: foo.hpp foo.cpp
//

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>

#include <srcml.h>
#include <CLI11.hpp>
#include "ClassInfo.hpp"
#include "PrimitiveTypes.hpp"


//Globals
primitiveTypes PRIMITIVES;                        //Primitives type per language + any user supplied
int            METHODS_PER_CLASS_THRESHOLD = 21;  //Threshhold for large class stereotype (from ICSM10)
bool           DEBUG = false;                     //Debug flag from CLI option

int main(int argc, char const *argv[]) {
    std::string inputFile      = "";
    std::string primitivesFile = "";
    std::string outputFile     = "";
    bool        outputReport   = false;
    bool        overWriteInput = false;
    int         error          = 0;

    CLI::App app{"StereoCode: Determines method stereotypes"};

    app.add_option("input-archive",      inputFile,      "File name of a srcML archive of a class (for C++ it is the hpp and cpp pairs)")->required();
    app.add_option("-o,--output-file",   outputFile,     "File name of output - srcML archive with stereotypes");
    app.add_option("-p,--primitives",    primitivesFile, "File name of user supplied primitive types (one per line)");
    app.add_flag  ("-r,--report",        outputReport,   "Output optional report file - *.report.txt (off by default)");
    app.add_flag  ("-v,--overwrite",     overWriteInput, "Over write input file with stereotype output (off by default)");
    app.add_flag  ("-d,--debug",         DEBUG,          "Turn on debug mode");
    app.add_option("-c,--large-class",   METHODS_PER_CLASS_THRESHOLD, "The # of methods threshold for a large class stereotype (default=21)");

    CLI11_PARSE(app, argc, argv);

    if (primitivesFile != "") {        //Add user defined primitive types to initial set
        std::ifstream in(primitivesFile);
        if (in.is_open())
            in >> PRIMITIVES;
        else {
            std::cerr << "Error: Primitive types file not found: " << primitivesFile << std::endl;
            return -1;
        }
        in.close();
    }

    if (DEBUG) std::cerr << std::endl << "Computing stereotypes for the following class: " << std::endl << std::endl;

    srcml_archive* archive = srcml_archive_create();
    srcml_unit*    firstUnit = NULL;
    srcml_unit*    secondUnit = NULL;
    std::ofstream  reportFile;
    std::string    unitLanguage;
    std::string    unitFilename;
    classModel     aClass;
    bool           twoUnits = false;

    error = srcml_archive_read_open_filename(archive, inputFile.c_str());   //Read a srcML archive file
    if (error) {
        std::cerr << "Error: File not found: " << inputFile << ", error == " << error << "\n";
        return -1;
    }

    if (overWriteInput)
        outputFile = inputFile;                           //Overwrite input file
    else
        outputFile = inputFile + ".annotated.xml";       //Default output file name
    //Register namespace for output of stereotypes
    srcml_archive* output_archive = srcml_archive_create();
    error = srcml_archive_register_namespace(output_archive, "st", "http://www.srcML.org/srcML/stereotype");
    if (error) {
        std::cerr << "Error registering namespace" << std::endl;
        return -1;
    }
    error = srcml_archive_write_open_filename(output_archive, outputFile.c_str());
    if (error) {
        std::cerr << "Error opening: " << outputFile << std::endl;
        return -1;
    }
    if (outputReport) {        //Optionally output a report (tab separated) path, class name, method, stereotype
        reportFile.open(inputFile + ".report.txt");
    }

    //In the case of C++ there will normally be two units, a hpp, cpp pair. But may only have a hpp
    //In case of other languages just one unit
    firstUnit = srcml_archive_read_unit(archive); //.hpp, .java, .cs, etc
    
    while (firstUnit) {
        
        unitLanguage = srcml_unit_get_language(firstUnit);
        twoUnits = false;
        if (unitLanguage == "C++") {
            secondUnit = srcml_archive_read_unit(archive);  //.cpp - only C++ has two units (hpp is first)
            if (secondUnit != nullptr) {
                unitFilename = srcml_unit_get_filename(secondUnit);
            }
                
            if (!isHeaderFile(unitFilename))
                twoUnits = true;  //It is a .cpp file
        }
        if (twoUnits)
            aClass  = classModel(archive, firstUnit, secondUnit);  //Construct class and do initial anaylsis
        else
            aClass  = classModel(archive, firstUnit, 0);

        aClass.ComputeMethodStereotype();                                 //Analysis for method stereotypes
        aClass.ComputeClassStereotype();                                   //Analysis for class stereotype
        
        //Add class & method stereotypes as attributes to both units
        firstUnit = aClass.outputUnitWithStereotypes(archive, firstUnit, true);
        if (twoUnits) secondUnit = aClass.outputUnitWithStereotypes(archive, secondUnit, false);

        //Output srcML with stereotype attributes to some file
        srcml_archive_write_unit(output_archive, firstUnit);
        if (twoUnits) srcml_archive_write_unit(output_archive, secondUnit);

        if (outputReport) aClass.outputReport(reportFile, inputFile);
        if (DEBUG) std::cerr << aClass << std::endl;
 
        if (twoUnits) { 
            srcml_unit_free(firstUnit);
            // srcml_unit_free(secondUnit);  -----------------------> LINE REMOVED
            firstUnit = NULL;
            secondUnit = NULL;
            firstUnit = srcml_archive_read_unit(archive);
        } else if (secondUnit) {  //Second is a hpp
            srcml_unit_free(firstUnit);
            firstUnit = secondUnit;
            secondUnit = NULL;
        } else {                 //Only one unit read
            srcml_unit_free(firstUnit);
            firstUnit = NULL;
            secondUnit = NULL;
            firstUnit = srcml_archive_read_unit(archive);
        }

        aClass  = classModel();
    }

    //Clean up
    
    srcml_archive_close(archive);
    srcml_archive_free(archive);
    srcml_archive_close(output_archive);
    srcml_archive_free(output_archive);
    if (outputReport) reportFile.close();
    if (DEBUG) std::cerr << std::endl << "StereoCode completed." << std::endl;

    return 0;
}

