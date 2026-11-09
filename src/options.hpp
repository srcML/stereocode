// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file options.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <optional>
#include <string>

// Settings of one run, read from the command line
//
struct options {
    std::string inputFile;                        // srcML input archive
    std::string outputFile;                       // srcML output archive annotated with stereotypes
    std::string primitivesFile;                   // User primitive types (one per line, in sections by language)
    std::string callsFile;                        // User calls to ignore (one per line, in sections by language)
    std::string specifiersFile;                   // User specifiers to ignore (one per line, in sections by language)
    std::string queryMethodsFile;                 // User query methods, whose calls on data members read them (one per line, in sections by language)
    std::optional<int> largeClassThreshold;       // Method threshold for the large type stereotype (e.g., large-class), computed from the run when not given
    unsigned    threads{1};                       // Worker threads that analyze and annotate the units
    bool        freeFunctions{false};             // Identify stereotypes for free functions
    bool        structs{false};                   // Identify stereotypes for structs
    bool        interfaces{false};                // Identify stereotypes for interfaces
    bool        unions{false};                    // Identify stereotypes for unions
    bool        enums{false};                     // Identify stereotypes for enums
    bool        csvReport{false};                 // Write the CSV report
    bool        txtReport{false};                 // Write the text report
    bool        comments{false};                  // Write a "// @stereotype" comment before each labeled element of the srcML output archive
    bool        verboseReport{false};             // Print the lists and write the verbose CSV report
    bool        disableSrcmlOutput{false};        // Skip the srcML output archive
};

#endif
