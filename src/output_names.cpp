// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file output_names.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "output_names.hpp"

namespace {
    // Removes a suffix from the end of a name, if the name ends with it
    //
    void removeSuffix(std::string& name, const std::string& suffix) {
        if (name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0)
            name.erase(name.size() - suffix.size());
    }
}

// Gets the srcML output archive of an input archive, such as code.stereotypes.xml for code.xml
//
std::string defaultOutputFile(const std::string& inputFile) {
    std::string stem = inputFile;
    removeSuffix(stem, ".xml");
    return stem + ".stereotypes.xml";
}

// Gets the reports with an extension beside a srcML output archive, such as code.stereotypes.csv and
// code.free_functions.stereotypes.csv for code.stereotypes.xml or code.xml and the extension .csv
//
reportNames reportNamesFor(const std::string& outputFile, const std::string& extension) {
    std::string stem = outputFile;
    removeSuffix(stem, ".xml");
    removeSuffix(stem, ".stereotypes");
    return {stem + ".stereotypes" + extension, stem + ".free_functions.stereotypes" + extension};
}
