// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file srcml_elements.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef SRCML_ELEMENTS_HPP
#define SRCML_ELEMENTS_HPP

#include "argument_range.hpp"
#include "element_path.hpp"

#include <libxml/tree.h>

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

// Navigation among the srcML elements of a parsed unit

bool          isSrcElement            (const xmlNode*, std::string_view);
bool          isTypeElement           (const xmlNode*);
xmlNode*      firstChildElement       (const xmlNode*, std::string_view);
xmlNode*      nextNamedSibling        (const xmlNode*, std::string_view);
xmlNode*      typeNameElement         (const xmlNode*);
bool          refersToPreviousType    (const xmlNode*);
bool          declaresStatic          (const xmlNode*);
int           countListItems          (const xmlNode*);
std::string   parameterTypesOf        (const xmlNode*);
std::string   signatureTypesOf        (const xmlNode*);
argumentRange argumentRangeOf         (const xmlNode*);
std::string   namedTypeOf             (const xmlNode*);
std::string   standardSmartPointer    (const xmlNode*);
xmlNode*      smartPointerObject      (const xmlNode*);
std::string   pointeeTypeOf           (const xmlNode*);
xmlNode*      scopeOf                 (const xmlNode*);
xmlNode*      elementAt               (xmlNode*, const elementPath&);

// The positions of the elements of a document in document order, numbered at the first request
//
class elementOrder {
public:
    int                 position    (const xmlNode*);   // Position of an element
    int                 end         (const xmlNode*);   // Position after the last descendant of an element
private:
    const std::pair<int, int>& range(const xmlNode*);

    std::unordered_map<const xmlNode*, std::pair<int, int>> ranges;   // Position of each element and the position after its descendants
};

#endif
