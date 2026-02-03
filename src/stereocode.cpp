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
bool                               IS_VERBOSE              {false};
bool                               CSV_REPORT              {false};

int main (int argc, char const *argv[]) {
    std::string         inputFile;
    std::string         primitivesFile;
    std::string         ignorableCallsFile;
    std::string         specifiersFile;
    std::string         outputFile;

    bool                version{false};

    CLI::App app{"Stereocode: Determines function (method and free functions) and type (class, struct, interface, union, enum) stereotypes\nSupports C, C++, C#, and Java\n" };
    
    app.add_option("input-archive",            inputFile,                        "File name of srcML input archive")->required();
    app.add_option("-o,--output-file",         outputFile,                       "File name of srcML output archive annotated with stereotypes");
    app.add_option("-p,--primitive-file",      primitivesFile,                   "File name of user primitive types (one per line)");
    app.add_option("-g,--ignore-call-file",    ignorableCallsFile,               "File name of user calls to ignore (one per line)");
    app.add_option("-t,--type-specifier-file", specifiersFile,                   "File name of user specifiers to remove (one per line)");
    app.add_option("-l,--large-class",         METHODS_PER_TYPE_THRESHOLD,       "Method threshold for the large type stereotype (e.g., large-class) (default = 21)");
    app.add_flag  ("-f,--free-function",       FREE_FUNCTION,                    "Identify stereotypes for free functions (includes static methods) (C, C++, C#, and Java)");
    app.add_flag  ("-i,--interface",           INTERFACE,                        "Identify stereotypes for interfaces (C# and Java)");
    app.add_flag  ("-n,--union",               UNION,                            "Identify stereotypes for unions (C++)");
    app.add_flag  ("-m,--enum",                ENUM,                             "Identify stereotypes for enums (Java)");
    app.add_flag  ("-s,--struct",              STRUCT,                           "Identify stereotypes for structs (C, C++, C#, and Java)");
    app.add_flag  ("-z,--csv-report",          CSV_REPORT,                       "Output optional CSV file containing stereotypes");
    app.add_flag  ("-b,--verbose",             IS_VERBOSE,                       "Verbose output: primitives, ignorable calls, specifiers, and CSV with metadata");
    app.add_flag  ("-v,--version",             version,                          "Display version information");
    
    CLI11_PARSE(app, argc, argv);
    
    if (version) {
        std::cout << "Stereocode v1.0" << std::endl;
        return 0;
    }

    // Add user-defined primitive to initial set
    if (!primitivesFile.empty()) {         
        std::ifstream in(primitivesFile);
        if (in.is_open()) in >> PRIMITIVES;
        else std::cerr << "Error: Primitive types file not found: " << primitivesFile << '\n';
        in.close();
    }
    
    // Add user-defined ignorable calls to initial set
    if (!ignorableCallsFile.empty()) {         
        std::ifstream in(ignorableCallsFile);
        if (in.is_open()) in >> CALLS;
        else std::cerr << "Error: Ignorable calls file not found: " << ignorableCallsFile << '\n';
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
