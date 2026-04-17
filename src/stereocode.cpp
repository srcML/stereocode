// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereocode.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "CLI11.hpp"
#include "stereotypes_analyzer.hpp"
#include "primitives.hpp"
#include "calls.hpp"
#include "specifiers.hpp"
#include "xpath_generator.hpp"

primitives                         PRIMITIVES;
calls                              CALLS;
specifiers                         SPECIFIERS;

int                                METHODS_PER_TYPE_THRESHOLD = 21;

bool                               FREE_FUNCTION           {false};
bool                               STRUCT                  {false};
bool                               INTERFACE               {false};
bool                               UNION                   {false};
bool                               ENUM                    {false};
bool                               VERBOSE_REPORT          {false};
bool                               CSV_REPORT              {false};
bool                               DISABLE_SRCML_OUTPUT    {false};

int main (int argc, char const *argv[]) {
    std::string         inputFile;
    std::string         primitivesFile;
    std::string         callsFile;
    std::string         specifiersFile;
    std::string         outputFile;

    bool                version{false};

    CLI::App app{"Stereocode: Determines function (method and free functions) and type (class, struct, interface, union, enum) stereotypes\nSupports C, C++, C#, and Java\n" };
    
    app.add_option("input-archive",            inputFile,                        "File name of srcML input archive")->required();
    app.add_option("-o,--output-file",         outputFile,                       "File name of srcML output archive annotated with stereotypes");
    app.add_option("-p,--primitive-file",      primitivesFile,                   "File name of user primitive types to consider (one per line) (Default set exists)");
    app.add_option("-c,--call-file",           callsFile,                        "File name of user calls to ignore (one per line) (Default set exists)");
    app.add_option("-s,--specifier-file",      specifiersFile,                   "File name of user specifiers to ignore (one per line) (Default set exists)");
    app.add_option("-l,--large-class",         METHODS_PER_TYPE_THRESHOLD,       "Method threshold for the large type stereotype (e.g., large-class) (default = 21)");
    app.add_flag  ("-f,--free-function",       FREE_FUNCTION,                    "Identify stereotypes for free functions (includes static methods) (C, C++, C#, and Java)");
    app.add_flag  ("-i,--interface",           INTERFACE,                        "Identify stereotypes for interfaces (C# and Java)");
    app.add_flag  ("-u,--union",               UNION,                            "Identify stereotypes for unions (C++)");
    app.add_flag  ("-e,--enum",                ENUM,                             "Identify stereotypes for enums (Java)");
    app.add_flag  ("-t,--struct",              STRUCT,                           "Identify stereotypes for structs (C, C++, C#, and Java)");
    app.add_flag  ("-r,--csv-report",          CSV_REPORT,                       "Output optional CSV file containing stereotypes");
    app.add_flag  ("-b,--verbose",             VERBOSE_REPORT,                   "Verbose output: primitives, ignorable calls, specifiers, and optional CSV containing stereotypes and extra metadata (overrides --csv-report flag)");
    app.add_flag  ("-d,--srcmlOutput",         DISABLE_SRCML_OUTPUT,             "Disables srcML output archive (Requires --csv-report or --verbose flag to be set for CSV output)");
    app.add_flag  ("-v,--version",             version,                          "Display version information");
    
    CLI11_PARSE(app, argc, argv);
    
    if (version) {
        std::cout << "Stereocode v1.0" << std::endl;
        return 0;
    }

    // Validate input file extension
    if (inputFile.size() < 4 || inputFile.substr(inputFile.size() - 4) != ".xml") {
        std::cerr << "Error: Input archive must have a valid .xml extension.\n";
        return 1;
    }

    // Ensure that if srcML output is disabled, a CSV/Verbose report is requested
    if (DISABLE_SRCML_OUTPUT && !CSV_REPORT && !VERBOSE_REPORT) {
        std::cerr << "Error: The --srcmlOutput (-d) flag disables the srcML output archive. "
                  << "You must specify either --csv-report (-r) or --verbose (-b) to generate output.\n";
        return 1;
    }

    // Add user-defined primitive to initial set
    if (!primitivesFile.empty()) {         
        std::ifstream in(primitivesFile);
        if (in.is_open()) in >> PRIMITIVES;
        else std::cerr << "Error: Primitive types file not found: " << primitivesFile << '\n';
        in.close();
    }
    
    // Add user-defined calls to initial set
    if (!callsFile.empty()) {         
        std::ifstream in(callsFile);
        if (in.is_open()) in >> CALLS;
        else std::cerr << "Error: Calls file not found: " << callsFile << '\n';
        in.close();
    }

    // Add user-defined specifiers to initial set
    if (!specifiersFile.empty()) {         
        std::ifstream in(specifiersFile);
        if (in.is_open()) in >> SPECIFIERS;
        else std::cerr << "Error: Specifiers file not found: " << specifiersFile << '\n'; 
        in.close();
    }

    // Create default output file name if an output name is not specified by the user
    if (outputFile.empty()) {                                             
        std::string InputFileNoExt = inputFile.substr(0, inputFile.size() - 4);     
        outputFile = InputFileNoExt + ".stereotypes.xml";     
    }  

    // Compute stereotypes
    stereotypesAnalyzer analyzer(inputFile, outputFile);
 
    return 0;
}
