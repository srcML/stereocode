// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file srcml_document.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef SRCML_DOCUMENT_HPP
#define SRCML_DOCUMENT_HPP

#include "element_path.hpp"
#include "query_engine.hpp"

#include <libxml/tree.h>

#include <memory>
#include <string>
#include <string_view>

// Frees a document with its nodes
//
struct documentDeleter {
    void operator()(xmlDoc* document) const { xmlFreeDoc(document); }
};

// A srcML unit parsed by libxml2, or a copy of one of its elements under a copy of the unit element, for the queries of its language
// A copy refers to the element of the parsed unit that it copies, so the parsed unit must outlive its copies
//
class srcmlDocument {
public:
                            srcmlDocument   (queryEngine&, queryLanguage, std::string_view);

    srcmlDocument           wrap            (xmlNode*)      const;
    nodeSet                 select          (query name)    const   { return engine->select(document.get(), language, name); }  // Nodes of a query
    std::string             fileName        ()              const;
    elementPath             path            ()              const;
private:
                            srcmlDocument   (queryEngine&, queryLanguage, xmlDoc*);

    queryEngine*            engine;             // Evaluates the queries of the document
    queryLanguage           language;           // Language of the queries
    std::unique_ptr<xmlDoc, documentDeleter> document;
    xmlNode*                element{nullptr};   // The unit element of a parsed unit, or the copied element
    xmlNode*                original{nullptr};  // The element of the parsed unit that the element of this document stands for
};

#endif
