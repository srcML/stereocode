// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file joined_names.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef JOINED_NAMES_HPP
#define JOINED_NAMES_HPP

#include "rules/stereotype_labels.hpp"

#include <string>

// Joins the names of stereotypes with spaces, as stereocode writes them
//
inline std::string joinedNames(const stereotypeList& labels) {
    std::string names;
    for (stereotype label : labels) {
        if (!names.empty()) names += " ";
        names += stereotypeName(label);
    }
    return names;
}

#endif
