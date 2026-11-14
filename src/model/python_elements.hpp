// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file python_elements.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef PYTHON_ELEMENTS_HPP
#define PYTHON_ELEMENTS_HPP

#include "model/argument_range.hpp"
#include "model/bound_value.hpp"

#include <libxml/tree.h>

#include <string>
#include <vector>

// The srcML markup of Python, which has no declarations: an annotation gives the type of a name, an assignment binds a name,
// and a call of a class creates an object

std::string                 pythonTypeOf            (const xmlNode*);
std::string                 pythonNamedType         (const std::string&);
boundValue                  pythonBoundValue        (const xmlNode*);
argumentRange               pythonArgumentRange     (const xmlNode*, bool);
const xmlNode*              soleCallOf              (const xmlNode*);
std::string                 pythonCallName          (const xmlNode*);
bool                        followsSuperCall        (const xmlNode*);
std::string                 pythonIndexedObject     (const xmlNode*);
bool                        isBooleanByForm         (const xmlNode*);
bool                        isNoneLiteral           (const xmlNode*);
bool                        namesPythonClass        (const std::string&);
std::vector<std::string>    formattedStringNames    (const std::string&);
std::string                 moduleNameOf            (const std::string&);

#endif
