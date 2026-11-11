// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_attributes.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "output/stereotype_attributes.hpp"
#include "archive/srcml_elements.hpp"
#include "archive/srcml_namespaces.hpp"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <climits>
#include <memory>
#include <stdexcept>

namespace {
    using documentPtr = std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)>;
}

// Tells whether the srcML of a unit may hold st:stereotype attributes, which need a declaration of the stereotype namespace in the unit
//
bool mayHoldStereotypes(std::string_view unitSrcml) {
    return unitSrcml.find(stereotypeNamespace) != std::string_view::npos;
}

// Removes the st:stereotype attribute of an element and of each element below it
//
void removeStereotypes(xmlNode* element) {
    for (xmlAttr* attribute = element->properties; attribute;) {
        xmlAttr* next = attribute->next;
        if (attribute->ns && xmlStrEqual(attribute->ns->href, BAD_CAST stereotypeNamespace) && xmlStrEqual(attribute->name, BAD_CAST "stereotype"))
            xmlRemoveProp(attribute);
        attribute = next;
    }
    for (xmlNode* child = xmlFirstElementChild(element); child; child = xmlNextElementSibling(child)) removeStereotypes(child);
}

// Gets the srcML of a unit in which the element of each annotation carries its stereotypes and no other element carries any,
// as the input may hold the stereotypes of an earlier run
//
std::string withStereotypeAttributes(std::string_view unitSrcml, const std::vector<stereotypeAnnotation>& annotations) {
    if (unitSrcml.size() > INT_MAX) throw std::runtime_error("the srcML of a unit is too large");
    documentPtr document(xmlReadMemory(unitSrcml.data(), static_cast<int>(unitSrcml.size()), nullptr, nullptr, XML_PARSE_HUGE), xmlFreeDoc);
    xmlNode* unit = document ? xmlDocGetRootElement(document.get()) : nullptr;
    if (!unit) throw std::runtime_error("cannot parse the srcML of a unit for its stereotypes");

    removeStereotypes(unit);

    // The attributes take the prefix under which the unit already declares the namespace, if any
    xmlNs* stereotypeNs = xmlSearchNsByHref(document.get(), unit, BAD_CAST stereotypeNamespace);
    if (!stereotypeNs) stereotypeNs = xmlNewNs(unit, BAD_CAST stereotypeNamespace, BAD_CAST "st");
    if (!stereotypeNs) throw std::runtime_error("cannot declare the stereotype namespace in a unit");

    for (const stereotypeAnnotation& annotation : annotations) {
        xmlNode* element = elementAt(unit, annotation.path);
        if (!element || !xmlSetNsProp(element, stereotypeNs, BAD_CAST "stereotype", BAD_CAST annotation.stereotypes.c_str()))
            throw std::runtime_error("cannot add a stereotype attribute to a unit");
    }

    xmlChar* text = nullptr;
    int size = 0;
    xmlDocDumpMemoryEnc(document.get(), &text, &size, "UTF-8");
    if (!text) throw std::runtime_error("cannot write the srcML of a unit with its stereotypes");
    std::string labeled(reinterpret_cast<const char*>(text), static_cast<std::size_t>(size));
    xmlFree(text);
    return labeled;
}
