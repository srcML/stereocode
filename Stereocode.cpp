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

std::vector<std::string> readFileNames(const std::string&);

primitiveTypes PRIMITIVES;  //Global set of primitives.


int main(int argc, char const *argv[])
{
    CLI::App app{"StereoCode: Determines method stereotypes"};

    std::string fileList = "";
    std::string inputFile = "";
    std::string primitivesFile = "";
    std::string outputFile = "";
    bool outputReport = false;
    bool overWriteInput = false;
    bool DEBUG = false;
    std::vector<std::string> inputFileList;

    app.add_option("-a,--archive",     inputFile,      "File name of a srcML archive of a class (for C++ it is the hpp and cpp units)");
    app.add_option("-o,--output-file", outputFile,     "File name of output - srcML archive with stereotypes");
    app.add_option("-l,--list-file",   fileList,       "File name that contains a list (one per line) of srcML archives");
    app.add_option("-p,--primitives",  primitivesFile, "File name of user supplied primitive types (one per line)");
    app.add_flag  ("-r,--report",      outputReport,   "Output optional report file - *.report.txt (off by default)");
    app.add_flag  ("-v,--overwrite",   overWriteInput, "Over write input file with stereotype output (off by default)");
    app.add_flag  ("-d,--debug",       DEBUG,          "Turn on debug mode");


    CLI11_PARSE(app, argc, argv);

    if (inputFile != "")                      //One class input archive
        inputFileList.push_back(inputFile);
    else if (fileList != "")                   //A list of archives
        inputFileList = readFileNames(fileList);
    else {
        std::cerr << "Error: No input given " << std::endl;
        return -1;
    }
    if (primitivesFile != "") {       //Add user defined primitive types to initial set
        std::ifstream in(primitivesFile);
        if (in.is_open())
            in >> PRIMITIVES;
        else {
            std::cerr << "Error: Primitive types file not found: " << primitivesFile << std::endl;
            return -1;
        }
        in.close();
    }

    if (DEBUG) std::cerr << "Computing stereotypes for the following class(es): " << std::endl;

    for (int i = 0; i < inputFileList.size(); ++i){
        int error;
        srcml_archive* archive = srcml_archive_create();
        error = srcml_archive_read_open_filename(archive, inputFileList[i].c_str());   //Read a srcML archive file
        if (error) {
            std::cerr << "Error: File not found: " << inputFileList[i] << ", error == " << error << "\n";
            return -1;
        }
        //In the case of C++ there will normally be two units, a hpp, cpp pair. But may only have a hpp
        //In case of other languages just one unit
        srcml_unit* firstUnit = srcml_archive_read_unit(archive);   //.hpp, .java, .cs, etc
        srcml_unit* secondUnit = srcml_archive_read_unit(archive);  //.cpp - only C++ has two units (hpp is first)

        classModel  aClass  = classModel(archive, firstUnit, secondUnit);  //Construct class and do initial anaylsis
        aClass.ComputeMethodStereotype();                                  //Analysis for method stereotypes
        aClass.ComputeClassStereotype();                                   //Analysis for class stereotype

        if (outputReport) {        //Optionally output a report (tab separated) path, class name, method, stereotype
            std::ofstream reportFile;
            reportFile.open(inputFileList[i] + ".report.txt");
            aClass.outputReport(reportFile, inputFileList[i]);
            reportFile.close();
        }

        firstUnit = aClass.outputUnitWithStereotypes(archive, firstUnit, true);
        if (aClass.getUnitTwoCount() != 0)
            secondUnit = aClass.outputUnitWithStereotypes(archive, secondUnit, false);

        //Namespace for output of stereotypes
        srcml_archive* output_archive = srcml_archive_create();
        error = srcml_archive_register_namespace(output_archive, "st", "http://www.srcML.org/srcML/stereotype");
        if (error) {
            std::cerr << "Error registering namespace" << std::endl;
            return -1;
        }
        //Output srcML with stereotype attributes to some file
        if (overWriteInput)
            outputFile = inputFileList[i];                    //Overwrite input file
        else if (fileList != "" || outputFile == "")          //Not specified, no overwrite
            outputFile = inputFileList[i] + ".annotated.xml"; //Default output file name
        error = srcml_archive_write_open_filename(output_archive, outputFile.c_str());
        if (error) {
            std::cerr << "Error opening: " << outputFile << std::endl;
            return -1;
        }
        srcml_archive_write_unit(output_archive, firstUnit);
        srcml_archive_write_unit(output_archive, secondUnit);

        if (DEBUG) std::cerr << aClass << std::endl;

        //Clean up
        srcml_unit_free(firstUnit);
        srcml_unit_free(secondUnit);
        srcml_archive_close(archive);
        srcml_archive_close(output_archive);
        srcml_archive_free(output_archive);
        srcml_archive_free(archive);
        aClass  = classModel();
        outputFile = "";
    }
    
    if (DEBUG) std::cerr << "StereoCode completed." << std::endl;
    return 0;
}


//
// Read a file of file names for input.
// Each file name is a srcML archive.
std::vector<std::string> readFileNames(const std::string & fileName){
    std::vector<std::string> list_of_archives;
    std::ifstream archives_file;
    archives_file.open(fileName);

    if (archives_file.is_open()){
        std::string name;
        while(std::getline(archives_file, name)){
            list_of_archives.push_back(name);
        }
    } else{
        std::cerr << "Error: File not found: " << fileName << "\n";
    }
    return list_of_archives;
}


