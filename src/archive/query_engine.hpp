// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file query_engine.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef QUERY_ENGINE_HPP
#define QUERY_ENGINE_HPP

#include "archive/node_text.hpp"
#include "archive/query_registry.hpp"

#include <libxml/xpath.h>

#include <array>
#include <memory>
#include <string>

// Frees the result of an XPath evaluation
//
struct xpathObjectDeleter {
    void operator()(xmlXPathObject* object) const { xmlXPathFreeObject(object); }
};

// Frees a compiled XPath
//
struct compiledXPathDeleter {
    void operator()(xmlXPathCompExpr* expression) const { xmlXPathFreeCompExpr(expression); }
};

// Frees an XPath context
//
struct xpathContextDeleter {
    void operator()(xmlXPathContext* context) const { xmlXPathFreeContext(context); }
};

// The nodes that a query selects, in document order, which stay valid while their document lives
//
class nodeSet {
public:
    explicit                nodeSet     (xmlXPathObject* result_) : result{result_}                {}

    int                     size        ()      const { return xmlXPathNodeSetGetLength(result->nodesetval); }  // Number of selected nodes
    xmlNode*                node        (int i) const { return xmlXPathNodeSetItem(result->nodesetval, i);   }  // Node i
    std::string             src         (int i) const { return sourceOf(node(i));                           }  // Source code of node i
    std::string             srcml       (int i) const { return srcmlOf(node(i));                            }  // srcML of node i
private:
    std::unique_ptr<xmlXPathObject, xpathObjectDeleter> result;
};

// Every query compiled once, with one XPath context to evaluate them
// libxml2 caches function lookups in a compiled XPath at its first evaluation, so each evaluating thread needs an engine of its own
//
class queryEngine {
public:
    explicit                queryEngine (const queryRegistry&);

    nodeSet                 select      (xmlDoc*, queryLanguage, query);
private:
    std::array<std::array<std::unique_ptr<xmlXPathCompExpr, compiledXPathDeleter>, queryCount>, languageCount> compiled;  // Null where a language has no such query
    std::unique_ptr<xmlXPathContext, xpathContextDeleter>                                                     context;
};

#endif
