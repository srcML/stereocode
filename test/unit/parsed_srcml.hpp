// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file parsed_srcml.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef PARSED_SRCML_HPP
#define PARSED_SRCML_HPP

#include "archive/srcml_elements.hpp"

#include <libxml/parser.h>
#include <srcml.h>

#include <cstddef>
#include <memory>
#include <string>

// A parsed srcML unit that frees its document
//
using document = std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)>;

// Parses srcML into the children of a unit of a language
//
inline document parsedUnit(const std::string& language, const std::string& children) {
    const std::string srcml = R"(<unit xmlns="http://www.srcML.org/srcML/src" language=")" + language + R"(">)" + children + "</unit>";
    return document(xmlReadMemory(srcml.c_str(), static_cast<int>(srcml.size()), nullptr, nullptr, 0), &xmlFreeDoc);
}

// Parses the code of a language into a unit, with the markup of the installed srcML
//
inline document parsedCode(const std::string& language, const std::string& code) {
    srcml_archive* archive = srcml_archive_create();
    char*          buffer  = nullptr;
    std::size_t    size    = 0;
    srcml_archive_write_open_memory(archive, &buffer, &size);

    srcml_unit* unit = srcml_unit_create(archive);
    srcml_unit_set_language(unit, language.c_str());
    srcml_unit_parse_memory(unit, code.c_str(), code.size());
    const char*       markup = srcml_unit_get_srcml(unit);
    const std::string srcml  = markup ? markup : "";

    srcml_unit_free(unit);
    srcml_archive_close(archive);
    srcml_archive_free(archive);
    srcml_memory_free(buffer);
    return document(xmlReadMemory(srcml.c_str(), static_cast<int>(srcml.size()), nullptr, nullptr, 0), &xmlFreeDoc);
}

// Gets the first element with a name in a node, its later siblings, and their children, in document order
//
inline const xmlNode* firstElement(const xmlNode* node, const std::string& name) {
    for (; node; node = node->next) {
        if (isSrcElement(node, name)) return node;
        if (const xmlNode* found = firstElement(node->children, name)) return found;
    }
    return nullptr;
}

// Gets the first element with a name in a parsed unit
//
inline const xmlNode* firstElement(const document& unit, const std::string& name) {
    return firstElement(xmlDocGetRootElement(unit.get()), name);
}

#endif
