// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file Stereocode.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include <filesystem>
#include "CLI11.hpp"
#include "ClassModelCollection.hpp"

primitiveTypes PRIMITIVES;                        // Primitives type per language + any user supplied
int            METHODS_PER_CLASS_THRESHOLD = 21;  // Threshold for large class stereotype (from ICSM10)

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

    srcml_archive*              archive                  = srcml_archive_create();
    srcml_archive*              outputArchive            = srcml_archive_create();
    std::ofstream               reportFile;
    int                         error;
    
    error = srcml_archive_read_open_filename(archive, inputFile.c_str());   

    if (error) {
        std::cerr << "Error: File not found: " << inputFile << ", error == " << error << "\n";
        srcml_archive_free(archive);
        srcml_archive_free(outputArchive);
        return -1;
    }

    // Register namespace for output of stereotypes 
    error = srcml_archive_register_namespace(outputArchive, "st", "http://www.srcML.org/srcML/stereotype"); 
    if (error) {
        std::cerr << "Error registering namespace" << std::endl;
        srcml_archive_free(archive);
        srcml_archive_free(outputArchive);
        return -1;
    }
    
    error = srcml_archive_write_open_filename(outputArchive, outputFile.c_str());
    if (error) {
        std::cerr << "Error opening: " << outputFile << std::endl;
        srcml_archive_close(archive);
        srcml_archive_free(archive);
        srcml_archive_free(outputArchive);
        return -1;
    }

    // Optionally output a report (tab separated) path, class name, method, stereotype
    if (outputReport) 
        reportFile.open(inputFile + ".report.txt");
  

    std::vector<srcml_unit*> units;
    srcml_unit* unit = srcml_archive_read_unit(archive);

    while (unit){
        units.push_back(unit);
        unit = srcml_archive_read_unit(archive);
    }
    if (units.size() > 0){
        classModelCollection classObj(archive, units);
        classObj.outputWithStereotypes(archive, outputArchive, units);

        if (outputReport){
            classObj.outputReport(reportFile, inputFile);
            reportFile.close();
        }
    }
    
    for (size_t i = 0; i < units.size(); i++){
        srcml_unit_free(units[i]);
    }
    // Clean up
    srcml_archive_close(archive);
    srcml_archive_free(archive);
    srcml_archive_close(outputArchive);
    srcml_archive_free(outputArchive);   

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

    return 0;
}   
