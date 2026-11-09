// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file output_names.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef OUTPUT_NAMES_HPP
#define OUTPUT_NAMES_HPP

#include <string>

// Names of the CSV or text reports of a run
//
struct reportNames {
    std::string         types;                  // Report of the types and their methods
    std::string         freeFunctions;          // Report of the free functions
};

std::string     defaultOutputFile   (const std::string&);
reportNames     reportNamesFor      (const std::string&, const std::string&);

#endif
