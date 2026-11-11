// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_attributes.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef STEREOTYPE_ATTRIBUTES_HPP
#define STEREOTYPE_ATTRIBUTES_HPP

#include "element_path.hpp"

#include <libxml/tree.h>

#include <string>
#include <string_view>
#include <vector>

// The st:stereotype attributes of the elements of a srcML unit, in place of those of an earlier run

// The stereotypes of one element of a unit
//
struct stereotypeAnnotation {
    elementPath         path;                   // Element that receives the st:stereotype attribute
    std::string         stereotypes;            // Value of the attribute
};

bool        mayHoldStereotypes          (std::string_view);
void        removeStereotypes           (xmlNode*);
std::string withStereotypeAttributes    (std::string_view, const std::vector<stereotypeAnnotation>&);

#endif
