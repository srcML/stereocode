// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_comments.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "stereotype_comments.hpp"
#include "srcml_namespaces.hpp"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <algorithm>
#include <climits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
    using documentPtr = std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)>;

    const std::string_view commentStart = "// @stereotype";
    const std::string_view blanks       = " \t";

    // The text of the line of an element before the element, and the text node that holds the line break before the line
    //
    struct lineStart {
        xmlNode*        textNode{nullptr};      // Text node with the last line break before the element, null on the first line of the unit
        std::size_t     lineBreak{0};           // Position of the line break in the text node
        std::string     indentation;            // Blanks at the start of the line
        std::string     code;                   // Text between the indentation and the element
        bool            leadsDeclaration{true}; // The code can only lead the declaration of the element
    };

    // Gets the text of a text node
    //
    std::string_view textOf(const xmlNode* node) {
        return node->content ? std::string_view(reinterpret_cast<const char*>(node->content)) : std::string_view();
    }

    // Tells whether a node is a srcML element with a name
    //
    bool isSrcElement(const xmlNode* node, const char* name) {
        return node && node->type == XML_ELEMENT_NODE && node->ns && xmlStrEqual(node->ns->href, BAD_CAST srcNamespace) &&
               xmlStrEqual(node->name, BAD_CAST name);
    }

    // Tells whether a node is an element that contains another node
    //
    bool contains(const xmlNode* ancestor, const xmlNode* node) {
        for (const xmlNode* parent = node->parent; parent; parent = parent->parent)
            if (parent == ancestor) return true;
        return false;
    }

    // Gets the node after a node in document order within the unit, or null after the last node of the unit
    //
    xmlNode* nextInUnit(xmlNode* node, const xmlNode* unit) {
        if (node->type == XML_ELEMENT_NODE && node->children) return node->children;
        for (; node != unit; node = node->parent)
            if (node->next) return node->next;
        return nullptr;
    }

    // Gets the node before a node in document order within the unit, or null for the unit element
    //
    xmlNode* previousInUnit(xmlNode* node) {
        if (!node->parent || node->parent->type != XML_ELEMENT_NODE) return nullptr;
        if (!node->prev) return node->parent;
        node = node->prev;
        while (node->type == XML_ELEMENT_NODE && node->last) node = node->last;
        return node;
    }

    // Tells whether an element before an element on its line can only lead the declaration of the element
    //
    bool isLeadingElement(const xmlNode* node) {
        return isSrcElement(node, "comment") || isSrcElement(node, "specifier") || isSrcElement(node, "annotation") || isSrcElement(node, "attribute");
    }

    // Tells whether the text of code can only lead a declaration, unlike a brace, a semicolon, a parenthesis, or a label such as public:
    //
    bool leadsDeclaration(std::string_view code) {
        for (std::size_t position = 0; position < code.size(); ++position) {
            if (std::string_view("{};()=\"'").find(code[position]) != std::string_view::npos) return false;
            const bool scope = (position + 1 < code.size() && code[position + 1] == ':') || (position > 0 && code[position - 1] == ':');
            if (code[position] == ':' && !scope) return false;
        }
        return true;
    }

    // Tells whether a node before an element is or lies in an element that can only lead the declaration of the element
    //
    bool inLeadingElement(const xmlNode* node, const xmlNode* element) {
        for (const xmlNode* current = node; current && !contains(current, element); current = current->parent)
            if (isLeadingElement(current)) return true;
        return false;
    }

    // Finds the start of the line of an element and the code of the line before the element
    // Code that leads the declaration consists of specifiers, comments, and names that srcML leaves outside an element it fails to parse whole
    //
    lineStart findLineStart(xmlNode* element) {
        lineStart start;
        std::vector<std::string_view> texts;
        for (xmlNode* previous = previousInUnit(element); previous; previous = previousInUnit(previous)) {
            if (previous->type == XML_ELEMENT_NODE) {
                if (!contains(previous, element) && !inLeadingElement(previous, element)) start.leadsDeclaration = false;
                continue;
            }
            if (previous->type != XML_TEXT_NODE) continue;

            const std::string_view text = textOf(previous);
            const std::size_t lineBreak = text.rfind('\n');
            const std::string_view onLine = lineBreak == std::string_view::npos ? text : text.substr(lineBreak + 1);
            texts.push_back(onLine);
            if (!inLeadingElement(previous, element) && !leadsDeclaration(onLine)) start.leadsDeclaration = false;
            if (lineBreak == std::string_view::npos) continue;

            start.textNode  = previous;
            start.lineBreak = lineBreak;
            break;
        }

        std::string line;
        for (auto text = texts.rbegin(); text != texts.rend(); ++text) line.append(*text);
        const std::size_t code = std::min(line.find_first_not_of(blanks), line.size());
        start.indentation = line.substr(0, code);
        start.code        = line.substr(code);
        return start;
    }

    // Gets the stereotype comment on the line before a line start, such as the comment of an earlier run, or null without one
    //
    xmlNode* earlierComment(const lineStart& start) {
        if (!start.textNode) return nullptr;
        const std::string_view beforeLineBreak = textOf(start.textNode).substr(0, start.lineBreak);
        if (beforeLineBreak.find_first_not_of(blanks) != std::string_view::npos) return nullptr;

        xmlNode* comment = start.textNode->prev;
        if (!isSrcElement(comment, "comment") || !comment->children || comment->children != comment->last) return nullptr;
        const std::string_view text = textOf(comment->children);
        const bool isStereotypeComment = text.substr(0, commentStart.size()) == commentStart &&
                                         (text.size() == commentStart.size() || text[commentStart.size()] == ' ');
        return isStereotypeComment ? comment : nullptr;
    }

    // Gets a new text node, or throws when libxml2 cannot create it
    //
    xmlNode* newText(xmlDoc* document, const std::string& text) {
        xmlNode* node = xmlNewDocTextLen(document, BAD_CAST text.data(), static_cast<int>(text.size()));
        if (!node) throw std::runtime_error("cannot insert a stereotype comment into the srcML of a unit");
        return node;
    }

    // Gets a new line comment with a text, or throws when libxml2 cannot create it
    //
    xmlNode* newComment(xmlDoc* document, xmlNs* srcNs, const std::string& text) {
        xmlNode* comment = xmlNewDocRawNode(document, srcNs, BAD_CAST "comment", BAD_CAST text.c_str());
        if (comment && xmlNewProp(comment, BAD_CAST "type", BAD_CAST "line")) return comment;
        xmlFreeNode(comment);
        throw std::runtime_error("cannot insert a stereotype comment into the srcML of a unit");
    }

    // Adds a node before a node, or throws when libxml2 cannot add it
    //
    void addBefore(xmlNode* node, xmlNode* added) {
        if (!xmlAddPrevSibling(node, added)) {
            xmlFreeNode(added);
            throw std::runtime_error("cannot insert a stereotype comment into the srcML of a unit");
        }
    }

    // Adds a node after a node, or throws when libxml2 cannot add it
    //
    void addAfter(xmlNode* node, xmlNode* added) {
        if (!xmlAddNextSibling(node, added)) {
            xmlFreeNode(added);
            throw std::runtime_error("cannot insert a stereotype comment into the srcML of a unit");
        }
    }

    // Inserts the comment of a labeled element on a line of its own above the element, at the indentation of the line of the element
    // Specifiers and comments before the element stay with it, while other code before it, such as an earlier member, ends its line with the comment
    //
    void insertComment(xmlDoc* document, xmlNode* unit, xmlNode* element, const std::string& stereotypes) {
        const std::string text = std::string(commentStart) + " " + stereotypes;
        xmlNs* srcNs = xmlSearchNsByHref(document, element, BAD_CAST srcNamespace);
        if (!srcNs) throw std::runtime_error("a labeled element lies outside the srcML namespace");

        // The line break may lie inside an element before the element, such as a block comment, which must not receive the comment
        const lineStart start = findLineStart(element);
        const bool aboveLine = start.leadsDeclaration && (!start.textNode || contains(start.textNode->parent, element));
        if (!aboveLine) {
            if (!start.code.empty() && blanks.find(start.code.back()) == std::string_view::npos) addBefore(element, newText(document, " "));
            addBefore(element, newComment(document, srcNs, text));
            addBefore(element, newText(document, "\n" + start.indentation));
            return;
        }

        if (xmlNode* earlier = earlierComment(start)) {
            xmlNodeSetContentLen(earlier->children, BAD_CAST text.data(), static_cast<int>(text.size()));
            return;
        }

        if (!start.textNode) {
            xmlNode* firstNode = unit->children;
            if (!start.indentation.empty()) addBefore(firstNode, newText(document, start.indentation));
            addBefore(firstNode, newComment(document, srcNs, text));
            addBefore(firstNode, newText(document, "\n"));
            return;
        }

        const std::string_view lineText = textOf(start.textNode);
        const std::string beforeComment = std::string(lineText.substr(0, start.lineBreak + 1)) + start.indentation;
        const std::string afterComment  = "\n" + std::string(lineText.substr(start.lineBreak + 1));
        xmlNodeSetContentLen(start.textNode, BAD_CAST beforeComment.data(), static_cast<int>(beforeComment.size()));
        xmlNode* comment = newComment(document, srcNs, text);
        addAfter(start.textNode, comment);
        addAfter(comment, newText(document, afterComment));
    }
}

// Gets the srcML of a unit with a line comment "// @stereotype <labels>" before each element that carries the attribute st:stereotype
// in the namespace that the srcML declares, as srcml writes the comment for a source file that holds the comment
//
std::string withStereotypeComments(std::string_view srcml) {
    if (srcml.size() > INT_MAX) throw std::runtime_error("the srcML of a unit is too large");
    documentPtr document(xmlReadMemory(srcml.data(), static_cast<int>(srcml.size()), nullptr, nullptr, XML_PARSE_HUGE), xmlFreeDoc);
    xmlNode* unit = document ? xmlDocGetRootElement(document.get()) : nullptr;
    if (!unit) throw std::runtime_error("cannot parse the srcML of a unit for the stereotype comments");

    // The insertions change the tree, so the labeled elements are collected first
    std::vector<std::pair<xmlNode*, std::string>> labeled;
    for (xmlNode* node = unit; node; node = nextInUnit(node, unit)) {
        if (node->type != XML_ELEMENT_NODE) continue;
        xmlChar* stereotypes = xmlGetNsProp(node, BAD_CAST "stereotype", BAD_CAST stereotypeNamespace);
        if (stereotypes && *stereotypes) labeled.emplace_back(node, reinterpret_cast<const char*>(stereotypes));
        xmlFree(stereotypes);
    }
    for (const auto& [element, stereotypes] : labeled) insertComment(document.get(), unit, element, stereotypes);

    xmlChar* text = nullptr;
    int size = 0;
    xmlDocDumpMemoryEnc(document.get(), &text, &size, "UTF-8");
    if (!text) throw std::runtime_error("cannot write the srcML of a unit with stereotype comments");
    std::string commented(reinterpret_cast<const char*>(text), static_cast<std::size_t>(size));
    xmlFree(text);
    return commented;
}
