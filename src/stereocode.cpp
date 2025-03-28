// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereocode.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "StructureModelCollection.hpp"
#include "CLI11.hpp"

primitiveTypes                     PRIMITIVES;                                             // Primitive types per language + any user supplied
ignorableCalls                     IGNORED_CALLS;                                          // Calls to ignore + any user supplied
typeModifiers                      TYPE_MODIFIERS;                                         // Modifiers to remove from data types + any user supplied
int                                METHODS_PER_STRUCTURE_THRESHOLD = 21;                   // Threshold for large class stereotype (from ICSM10)
bool                               STRUCT                          = false;                // Identify and stereotype structs (C++ or C#)
bool                               INTERFACE                       = false;                // Identify and stereotype interfaces (C# or Java)
bool                               UNION                           = false;                // Identify and stereotype unions (C++)
bool                               ENUM                            = false;                // Identify and stereotype enums (Java)
bool                               IS_VERBOSE                      = false;                // Prints primitives, ignored calls, and type modifiers

std::unordered_map
     <int, std::unordered_map
     <std::string, std::string>>   XPATH_LIST;                         // Map key = unit number. Each map value is a pair of xpath and stereotype
std::vector<std::string>           LANGUAGE = {"C++", "C#", "Java"};   // Supported languages
XPathBuilder                       XPATH_TRANSFORMATION;               // List of xpaths used for transformations

int main (int argc, char const *argv[]) {

    std::string         inputFile;
    std::string         primitivesFile;
    std::string         ignoredCallsFile;
    std::string         typeModifiersFile;
    std::string         outputFile;
    bool                outputTxtReport    = false;
    bool                outputCsvReport    = false;
    bool                overWriteInput     = false;
    bool                reDocComment       = false;

    CLI::App app{"Stereocode: Determines method and class stereotypes\n"
                 "Supports C++, C#, and Java\n" };
    
    app.add_option("input-archive",           inputFile,                        "File name of a srcML input archive")->required();
    app.add_option("-o,--output-file",        outputFile,                       "File name of output - srcML archive with stereotypes");
    app.add_option("-p,--primitive-file",     primitivesFile,                   "File name of user supplied primitive types (one per line)");
    app.add_option("-g,--ignore-call-file",   ignoredCallsFile,                 "File name of user supplied calls to ignore (one per line)");
    app.add_option("-t,--type-modifier-file", typeModifiersFile,                "File name of user supplied data type modifiers to remove (one per line)");
    app.add_option("-l,--large-class",        METHODS_PER_STRUCTURE_THRESHOLD,  "Method threshold for the large-class stereotype (default = 21)");
    app.add_flag  ("-i,--interface",          INTERFACE,                        "Identify stereotypes for interfaces (C# and Java)");
    app.add_flag  ("-n,--union",              UNION,                            "Identify stereotypes for unions (C++)");
    app.add_flag  ("-m,--enum",               ENUM,                             "Identify stereotypes for enums (Java)");
    app.add_flag  ("-s,--struct",             STRUCT,                           "Identify stereotypes for structs (C# and Java)");
    app.add_flag  ("-e,--input-overwrite",    overWriteInput,                   "Overwrite input with stereotype information");
    app.add_flag  ("-x,--txt-report",         outputTxtReport,                  "Output optional TXT report file containing stereotype information");
    app.add_flag  ("-z,--csv-report",         outputCsvReport,                  "Output optional CSV report file containing stereotype information");
    app.add_flag  ("-c,--comment",            reDocComment,                     "Annotates stereotypes as a comment before method and structure definitions (/** @stereotype stereotype */)");
    app.add_flag  ("-v,--verbose",            IS_VERBOSE,                       "Outputs default primitives, ignored calls, type modifiers, and extra report files");
    
    CLI11_PARSE(app, argc, argv);
    
    // Add user-defined primitive types to initial set
    if (primitivesFile != "") {         
        std::ifstream in(primitivesFile);
        if (in.is_open())
            in >> PRIMITIVES;
        else {
            std::cerr << "Error: Primitive types file not found: " << primitivesFile << '\n';
            return -1;
        }
        in.close();
    }
    
    // Add user-defined ignored calls to initial set
    if (ignoredCallsFile != "") {         
        std::ifstream in(ignoredCallsFile);
        if (in.is_open())
            in >> IGNORED_CALLS;
        else {
            std::cerr << "Error: Ignorable calls file not found: " << ignoredCallsFile << '\n';
            return -1;
        }
        in.close();
    }

    // Add user-defined type tokens to initial set
    if (typeModifiersFile != "") {         
        std::ifstream in(typeModifiersFile);
        if (in.is_open())
            in >> TYPE_MODIFIERS;
        else {
            std::cerr << "Error: Type modifiers file not found: " << typeModifiersFile << '\n';
            return -1;
        }
        in.close();
    }

    srcml_archive* archive = srcml_archive_create();
    int error = srcml_archive_read_open_filename(archive, inputFile.c_str());   
    if (error) {
        std::cerr << "Error: File not found: " << inputFile << ", error == " << error << '\n';
        srcml_archive_free(archive);
        return -1;
    }

    // Default output file name if output a name is not specified by the user
    if (outputFile == "") {                                             
        std::string InputFileNoExt = inputFile.substr(0, inputFile.size() - 4);     
        outputFile = InputFileNoExt + ".stereotypes.xml";     
    }  

    srcml_archive* outputArchive = srcml_archive_create();
    error = srcml_archive_write_open_filename(outputArchive, outputFile.c_str());
    if (error) {
        std::cerr << "Error opening: " << outputFile << std::endl;
        srcml_archive_close(archive);
        srcml_archive_free(archive);
        srcml_archive_free(outputArchive);
        return -1;
    }
    
    // Register namespaces for output
    srcml_archive_register_namespace(outputArchive, "st", "http://www.srcML.org/srcML/stereotype"); 
    std::size_t size = srcml_archive_get_namespace_size(archive);
    for (std::size_t i = 0; i < size; i++) {
        if (strcmp(srcml_archive_get_namespace_prefix(archive, i), "pos")  == 0) {
            srcml_archive_register_namespace(outputArchive, "pos", "http://www.srcML.org/srcML/position");
            break;
        }
    }
    
    // Find stereotypes
    XPATH_TRANSFORMATION.generateXpath(); // Called here since it depends on globals initalized by user input
    structureModelCollection structureObj(archive, outputArchive, 
                                    inputFile, outputFile, outputTxtReport, outputCsvReport, reDocComment);

    if (overWriteInput) {
        std::filesystem::remove(inputFile);
        std::filesystem::rename(outputFile, inputFile);
    }
 
    return 0;
}   
