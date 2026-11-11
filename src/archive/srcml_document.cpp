// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file srcml_document.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "archive/srcml_document.hpp"
#include "archive/srcml_elements.hpp"
#include "output/stereotype_attributes.hpp"

#include <libxml/parser.h>

#include <algorithm>
#include <climits>
#include <stdexcept>

namespace {
    // Frees a string of libxml2
    //
    struct xmlStringDeleter {
        void operator()(xmlChar* text) const { xmlFree(text); }
    };

    // Gets the position of an element among the element children of its parent, counted from 1
    //
    int childPosition(xmlNode* element) {
        int position = 1;
        for (xmlNode* sibling = xmlPreviousElementSibling(element); sibling; sibling = xmlPreviousElementSibling(sibling)) ++position;
        return position;
    }

    // Gets the positions on the way down from an element to one of its descendants, which are none for the element itself
    //
    elementPath pathBelow(const xmlNode* ancestor, xmlNode* descendant) {
        elementPath path;
        for (xmlNode* node = descendant; node != ancestor; node = node->parent) {
            if (!node || node->type != XML_ELEMENT_NODE) throw std::logic_error("a srcML element lies outside the element of its document");
            path.push_back(childPosition(node));
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
}

// Parses the srcML of a unit with the parser options of libsrcml
// The stereotypes of an earlier run take no part in the analysis, whose queries also read the srcML of elements
//
srcmlDocument::srcmlDocument(queryEngine& engine_, queryLanguage language_, std::string_view srcml) : engine{&engine_}, language{language_} {
    if (srcml.size() > INT_MAX) throw std::runtime_error("the srcML of a unit is too large");

    document.reset(xmlReadMemory(srcml.data(), static_cast<int>(srcml.size()), nullptr, nullptr, XML_PARSE_HUGE));
    if (!document || !xmlDocGetRootElement(document.get())) throw std::runtime_error("cannot parse the srcML of a unit");
    element = original = xmlDocGetRootElement(document.get());
    if (mayHoldStereotypes(srcml)) removeStereotypes(element);
}

// Takes a document
//
srcmlDocument::srcmlDocument(queryEngine& engine_, queryLanguage language_, xmlDoc* document_) : engine{&engine_}, language{language_}, document{document_} {
    if (!document) throw std::runtime_error("cannot create a srcML document");
}

// Copies an element of this document under a copy of the unit element into a document of its own,
// in which the queries of the element start at the unit, as their XPaths expect
//
srcmlDocument srcmlDocument::wrap(xmlNode* descendant) const {
    srcmlDocument wrapper(*engine, language, xmlNewDoc(BAD_CAST "1.0"));

    xmlNode* unit = xmlDocCopyNode(xmlDocGetRootElement(document.get()), wrapper.document.get(), 2);
    if (!unit) throw std::runtime_error("cannot copy a srcML unit element");
    xmlDocSetRootElement(wrapper.document.get(), unit);

    // With the unit as the parent, the copy refers to the namespaces of the unit and declares none of its own
    xmlNode* copy = nullptr;
    if (xmlDOMWrapCloneNode(nullptr, document.get(), descendant, &copy, wrapper.document.get(), unit, 1, 0) != 0 || !xmlAddChild(unit, copy)) {
        xmlFreeNode(copy);
        throw std::runtime_error("cannot copy a srcML element");
    }

    wrapper.element  = copy;
    wrapper.original = elementAt(original, pathBelow(element, descendant));
    if (!wrapper.original) throw std::logic_error("a copied srcML element has no original");
    return wrapper;
}

// Gets the file name of the unit, which is empty for code that srcML read from standard input or from --text
//
std::string srcmlDocument::fileName() const {
    const std::unique_ptr<xmlChar, xmlStringDeleter> name(xmlGetNoNsProp(xmlDocGetRootElement(document.get()), BAD_CAST "filename"));
    return name ? reinterpret_cast<const char*>(name.get()) : "";
}

// Gets the place in the parsed unit of the element that this document holds
//
elementPath srcmlDocument::path() const {
    return pathBelow(xmlDocGetRootElement(original->doc), original);
}
