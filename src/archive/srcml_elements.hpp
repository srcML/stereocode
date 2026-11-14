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

#include "model/argument_range.hpp"
#include "archive/element_path.hpp"
#include "model/written_type.hpp"

#include <libxml/tree.h>

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// The type of the elements of a variable, as Slot of Slot[], of Slot* and of std::vector<Slot>, with the number of accesses by an index
// or by -> that lead from the variable to an object of that type
//
struct elementType {
    std::string name;
    int         depth{0};
};

// Navigation among the srcML elements of a parsed unit

bool          isSrcElement            (const xmlNode*, std::string_view);
bool          isTypeScriptElement     (const xmlNode*, std::string_view);
bool          isTypeElement           (const xmlNode*);
bool          isOperator              (const xmlNode*, std::string_view);
xmlNode*      firstChildElement       (const xmlNode*, std::string_view);
xmlNode*      nextNamedSibling        (const xmlNode*, std::string_view);
xmlNode*      typeNameElement         (const xmlNode*);
std::string   typeSourceOf            (const xmlNode*);
bool          refersToPreviousType    (const xmlNode*);
bool          declaresStatic          (const xmlNode*);
int           countListItems          (const xmlNode*);
std::string   parameterTypesOf        (const xmlNode*);
std::vector<writtenType> writtenTypesOf(const xmlNode*);
writtenType   writtenTypeOf           (const xmlNode*);
std::string   signatureTypesOf        (const xmlNode*);
std::string   functionPointerTypeOf   (const xmlNode*);
argumentRange argumentRangeOf         (const xmlNode*);
std::string   namedTypeOf             (const xmlNode*);
std::string   standardSmartPointer    (const xmlNode*);
xmlNode*      smartPointerObject      (const xmlNode*);
std::string   pointeeTypeOf           (const xmlNode*);
elementType   elementTypeOf           (const xmlNode*, const xmlNode*);
xmlNode*      scopeOf                 (const xmlNode*);
std::string   nullConditionalObject   (const xmlNode*);
const xmlNode* nullConditionalStart   (const xmlNode*);
const xmlNode* nullConditionalNext    (const xmlNode*);
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
