// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file Stereocode.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>
#include <filesystem>

#include <srcml.h>
#include "CLI11.hpp"
#include "ClassInfo.hpp"
#include "PrimitiveTypes.hpp"

primitiveTypes PRIMITIVES;                        // Primitives type per language + any user supplied
int            METHODS_PER_CLASS_THRESHOLD = 21;  // Threshold for large class stereotype (from ICSM10)
bool           DEBUG = false;                     // Debug flag from CLI option

int main(int argc, char const *argv[]) {

    std::string inputFile      = "";
    std::string primitivesFile = "";
    std::string outputFile     = "";
    bool        outputReport   = false;
    bool        overWriteInput = false;
    bool        defaultOutput  = false;

    CLI::App app{"StereoCode: Determines method stereotypes"};
    
    app.add_option("input-archive",      inputFile,                   "File name of a srcML archive")->required();
    app.add_option("-o,--output-file",   outputFile,                  "File name of output - srcML archive with stereotypes");
    app.add_option("-p,--primitives",    primitivesFile,              "File name of user supplied primitive types [one per line]");
    app.add_flag  ("-r,--report",        outputReport,                "Output optional report file - *.report.txt (off by default)");
    app.add_flag  ("-v,--overwrite",     overWriteInput,              "Over write input file with stereotype output (off by default)");
    app.add_flag  ("-d,--debug",         DEBUG,                       "Turn on debug mode (off by default)");
    app.add_option("-c,--large-class",   METHODS_PER_CLASS_THRESHOLD, "The # of methods threshold for a large class stereotype (default=21)");

    CLI11_PARSE(app, argc, argv);

    if (primitivesFile != "") {         // Add user defined primitive types to initial set
        std::ifstream in(primitivesFile);
        if (in.is_open())
            in >> PRIMITIVES;
        else {
            std::cerr << "Error: Primitive types file not found: " << primitivesFile << std::endl;
            return -1;
        }
        in.close();
    }

    if (outputFile == "") {                                   // Skip if output file is specified                               
        std::string InputFileNoExt = inputFile.substr(0, inputFile.size() - 4);
        outputFile = InputFileNoExt + ".stereotypes.xml";     // Default output file name if output file name is not specified
        defaultOutput = true;
    }                       

    if (DEBUG) 
        std::cerr << std::endl << "Computing stereotypes for the following class: " << std::endl << std::endl;

    srcml_archive*              archive                  = srcml_archive_create();
    srcml_archive*              output_archive           = srcml_archive_create();
    srcml_unit*                 firstUnit                = nullptr;
    srcml_unit*                 secondUnit               = nullptr;
    srcml_unit*                 firstUnitTransformed     = nullptr;
    srcml_unit*                 secondUnitTransformed    = nullptr;
    srcml_transform_result*     resultFirst              = nullptr;
    srcml_transform_result*     resultSecond             = nullptr;
    std::ofstream               reportFile;
    classModel                  aClass;
    int                         error;
    bool                        hppFirst                 = false;
    bool                        languageWarn             = false;

    error = srcml_archive_read_open_filename(archive, inputFile.c_str());   

    if (error) {
        std::cerr << "Error: File not found: " << inputFile << ", error == " << error << "\n";
        srcml_archive_free(archive);
        srcml_archive_free(output_archive);
        return -1;
    }

    // Register namespace for output of stereotypes 
    error = srcml_archive_register_namespace(output_archive, "st", "http://www.srcML.org/srcML/stereotype"); 
    if (error) {
        std::cerr << "Error registering namespace" << std::endl;
        srcml_archive_free(archive);
        srcml_archive_free(output_archive);
        return -1;
    }
    
    error = srcml_archive_write_open_filename(output_archive, outputFile.c_str());
    if (error) {
        std::cerr << "Error opening: " << outputFile << std::endl;
        srcml_archive_close(archive);
        srcml_archive_free(archive);
        srcml_archive_free(output_archive);
        return -1;
    }

    // Optionally output a report (tab separated) path, class name, method, stereotype
    if (outputReport) 
        reportFile.open(inputFile + ".report.txt");

    // Currently, only C++ is supported
    // Input is expected to be an archive of .hpp and .cpp files (order doesn't matter) or a single .hpp file
    // A single class is expected in each unit

    firstUnit = srcml_archive_read_unit(archive);
    secondUnit = srcml_archive_read_unit(archive);

    while (firstUnit) {     
        if(!languageWarn){ // Issue a single warning whenever the unit/input is not C++
            std::string languageFirstUnit = srcml_unit_get_language(firstUnit);
            if (languageFirstUnit != "C++"){
                std::cerr << "Warning: stereocode is only designed for C++" << std::endl;
                languageWarn = true;
            }
            if (secondUnit){
                std::string languageSecondUnit = srcml_unit_get_language(secondUnit);
                if (languageSecondUnit != "C++"){
                    std::cerr << "Warning: stereocode is only designed for C++" << std::endl;
                    languageWarn = true;    
                }
            }   
        }     
        if (isHeaderFile(srcml_unit_get_filename(firstUnit))) 
            hppFirst = true;
        
        if (hppFirst)
            aClass  = classModel(archive, firstUnit, secondUnit);
        else
            aClass  = classModel(archive, secondUnit, firstUnit);
        
        aClass.ComputeMethodStereotype();                          // Analysis for method stereotypes
        aClass.ComputeClassStereotype();                           // Analysis for class stereotype
        
        // Add class & method stereotypes as attributes to both units
        if (hppFirst){ 
            firstUnitTransformed = aClass.outputUnitWithStereotypes(archive, firstUnit, &resultFirst, hppFirst); 
            if (aClass.getUnitTwoCount() != 0){ secondUnitTransformed = aClass.outputUnitWithStereotypes(archive, secondUnit, &resultSecond, !hppFirst ); } 
            srcml_archive_write_unit(output_archive, firstUnitTransformed); 
            srcml_archive_write_unit(output_archive, secondUnitTransformed);        
        }
        else { 
            firstUnitTransformed = aClass.outputUnitWithStereotypes(archive, secondUnit, &resultSecond, !hppFirst);
            if (aClass.getUnitOneCount() != 0){ secondUnitTransformed = aClass.outputUnitWithStereotypes(archive, firstUnit, &resultFirst, hppFirst); }
            srcml_archive_write_unit(output_archive, secondUnitTransformed); 
            srcml_archive_write_unit(output_archive, firstUnitTransformed);
        }
    
        if (outputReport)
            aClass.outputReport(reportFile, inputFile);
        if (DEBUG)
            std::cerr << aClass << std::endl;

        // Clean up
        if (hppFirst){ 
            if (aClass.getUnitTwoCount() != 0)
                srcml_transform_free(resultSecond);  // Will also clear "secondUnitTransformed" since it is contained in "result*"
            srcml_transform_free(resultFirst);       // Will also clear "firstUnitTransformed" since it is contained in "result*"
        }
        else {
            if (aClass.getUnitOneCount() != 0)
                srcml_transform_free(resultFirst);
            srcml_transform_free(resultSecond);
        }

        srcml_unit_free(firstUnit);
        srcml_unit_free(secondUnit);

        firstUnit = nullptr;
        secondUnit = nullptr;
        firstUnitTransformed = nullptr;
        secondUnitTransformed = nullptr;
        resultFirst = nullptr;
        resultSecond = nullptr;
           
        firstUnit = srcml_archive_read_unit(archive);    
        secondUnit = srcml_archive_read_unit(archive);

        hppFirst = false;  
    }

    // Clean up
    srcml_archive_close(archive);
    srcml_archive_free(archive);
    srcml_archive_close(output_archive);
    srcml_archive_free(output_archive);   

    if(overWriteInput){
        if(defaultOutput){
            std::filesystem::remove(inputFile);
            std::filesystem::rename(outputFile, inputFile);
        }
        else{
            std::filesystem::remove(inputFile);
            std::filesystem::copy_file(outputFile, inputFile);
        }
    }
    else if(defaultOutput)
        std::cout<<"Output file stored in: "<<outputFile<<std::endl;
    if (outputReport)
        reportFile.close();
    if (DEBUG)
        std::cerr << std::endl << "StereoCode completed." << std::endl;

    return 0;
}   
