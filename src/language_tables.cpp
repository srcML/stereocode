// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file language_tables.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "language_tables.hpp"
#include "user_lists.hpp"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    // Adds the entries of a user file to a table, where a file that cannot be read stops the run
    //
    template <typename table>
    void addUserEntries(const std::string& fileName, const char* description, table& entries) {
        if (fileName.empty()) return;

        std::ifstream in(fileName);
        if (in.is_open()) in >> entries;
        if (!in.is_open() || in.bad())
            throw std::runtime_error(std::string("cannot read the ") + description + " file " + fileName + ": " + std::strerror(errno));

        // A header in another case is most likely a typo of the header, while the reader keeps it as an entry for every language
        in.clear();
        in.seekg(0);
        for (const std::string& header : headersInOtherCase(in))
            std::cerr << "Warning: the line " << header << " of the " << description << " file " << fileName
                      << " is an entry, since a section header is written [C++], [C#], or [Java]\n";
    }
}

// Reads the user files before the defaults, since the specifier patterns include the user specifiers
//
languageTables::languageTables(const options& settings) : queries{settings} {
    addUserEntries(settings.primitivesFile, "primitive types", primitiveTypes);
    addUserEntries(settings.callsFile, "calls", ignorableCalls);
    addUserEntries(settings.specifiersFile, "specifiers", typeSpecifiers);
    addUserEntries(settings.queryMethodsFile, "query methods", fieldQueries);

    primitiveTypes.initializePrimitivesList();
    ignorableCalls.initializeIgnorableCalls();
    typeSpecifiers.initializeSpecifiersList();
    fieldQueries.initializeQueryMethods();
}
