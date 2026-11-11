// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file query_engine.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "archive/query_engine.hpp"
#include "archive/srcml_namespaces.hpp"

#include <libxml/xpathInternals.h>

#include <stdexcept>

// Compiles the XPath of every query of every language and creates the context, which knows the prefix src
//
queryEngine::queryEngine(const queryRegistry& registry) : context{xmlXPathNewContext(nullptr)} {
    if (!context || xmlXPathRegisterNs(context.get(), BAD_CAST "src", BAD_CAST srcNamespace) != 0)
        throw std::runtime_error("cannot create an XPath context");

    for (std::size_t language = 0; language < languageCount; ++language) {
        for (std::size_t name = 0; name < queryCount; ++name) {
            const std::string& xpath = registry.xpath(static_cast<queryLanguage>(language), static_cast<query>(name));
            if (xpath.empty()) continue;

            compiled[language][name].reset(xmlXPathCompile(BAD_CAST xpath.c_str()));
            if (!compiled[language][name]) throw std::runtime_error("cannot compile the XPath " + xpath);
        }
    }
}

// Selects the nodes of a query in a document
//
nodeSet queryEngine::select(xmlDoc* document, queryLanguage language, query name) {
    xmlXPathCompExpr* expression = compiled[static_cast<std::size_t>(language)][static_cast<std::size_t>(name)].get();
    if (!expression) throw std::logic_error("the language has no query of this kind");

    // Every evaluation starts from the state of a new context, as in libsrcml, since an evaluation changes the context
    context->doc               = document;
    context->node              = nullptr;
    context->contextSize       = -1;
    context->proximityPosition = -1;

    xmlXPathObject* evaluated = xmlXPathCompiledEval(expression, context.get());
    nodeSet selected(evaluated);
    if (!evaluated || evaluated->type != XPATH_NODESET) throw std::runtime_error("cannot evaluate an XPath query");
    return selected;
}
