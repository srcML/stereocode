// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file node_text.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "node_text.hpp"
#include "srcml_namespaces.hpp"

#include <libxml/encoding.h>
#include <libxml/xmlIO.h>

#include <cstdlib>
#include <memory>
#include <stdexcept>

namespace {
    // Known defect: libsrcml reads the srcML of a result without XML_PARSE_HUGE, so its source code ends at the 258th level of elements
    constexpr int sourceDepthLimit = 257;

    // Appends the character for which a srcML escape element stands, which libsrcml reads from the first attribute
    //
    void appendEscape(const xmlNode* element, std::string& source) {
        const bool isEscape = xmlStrEqual(element->name, BAD_CAST "escape") && element->ns && xmlStrEqual(element->ns->href, BAD_CAST srcNamespace);
        if (!isEscape || !element->properties) return;

        const xmlNode* value = element->properties->children;
        const char* number = value && value->content ? reinterpret_cast<const char*>(value->content) : "";
        source += static_cast<char>(std::strtol(number, nullptr, 0));
    }

    // Appends the source code below an element at a depth (1 for the node of the result), and returns false where libsrcml stops
    //
    bool appendChildren(const xmlNode* element, int depth, std::string& source) {
        for (const xmlNode* child = element->children; child; child = child->next) {
            if (child->type == XML_TEXT_NODE || child->type == XML_CDATA_SECTION_NODE) {
                if (child->content) source += reinterpret_cast<const char*>(child->content);
            }
            else if (child->type == XML_ELEMENT_NODE) {
                if (depth >= sourceDepthLimit) return false;

                appendEscape(child, source);
                if (!appendChildren(child, depth + 1, source)) return false;
            }
        }
        return true;
    }

    // Appends what libxml2 writes to an output buffer to a string
    //
    int appendOutput(void* context, const char* buffer, int length) {
        try {
            static_cast<std::string*>(context)->append(buffer, static_cast<std::size_t>(length));
            return length;
        }
        catch (...) {
            return -1;
        }
    }
}

// Gets the source code of a node: its character data, with the characters of srcML escape elements
//
std::string sourceOf(const xmlNode* node) {
    std::string source;
    if (node->type == XML_ELEMENT_NODE) {
        appendEscape(node, source);
        appendChildren(node, 1, source);
    }
    else if (node->content) {
        source = reinterpret_cast<const char*>(node->content);
    }
    return source;
}

// Gets the srcML of a node: the text of a text node, or the XML of another node
//
std::string srcmlOf(xmlNode* node) {
    if (node->type == XML_TEXT_NODE) return node->content ? reinterpret_cast<const char*>(node->content) : "";

    std::string srcml;
    xmlOutputBuffer* output = xmlOutputBufferCreateIO(appendOutput, nullptr, &srcml, nullptr);
    if (!output) throw std::runtime_error("cannot write the srcML of an element");

    xmlNodeDumpOutput(output, node->doc, node, 0, 0, nullptr);
    if (xmlOutputBufferClose(output) < 0) throw std::runtime_error("cannot write the srcML of an element");
    return srcml;
}

// Gets the source code of a node as srcml_unit_unparse_memory writes it for a unit without an encoding, ignoring errors as libsrcml does
// Known defect: the text is ISO-8859-1, with character references for the characters outside ISO-8859-1
//
std::string unparsedSourceOf(const xmlNode* node) {
    const std::string source = sourceOf(node);

    std::unique_ptr<xmlBuffer, void (*)(xmlBuffer*)> buffer(xmlBufferCreate(), &xmlBufferFree);
    xmlOutputBuffer* output = buffer ? xmlOutputBufferCreateBuffer(buffer.get(), xmlFindCharEncodingHandler("ISO-8859-1")) : nullptr;
    if (!output) throw std::runtime_error("cannot convert the source code of an element");

    xmlOutputBufferWrite(output, static_cast<int>(source.size()), source.data());
    xmlOutputBufferClose(output);

    // libsrcml reads the converted text as a C string
    const xmlChar* converted = xmlBufferContent(buffer.get());
    return converted ? reinterpret_cast<const char*>(converted) : "";
}
