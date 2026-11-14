// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file javascript_elements.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef JAVASCRIPT_ELEMENTS_HPP
#define JAVASCRIPT_ELEMENTS_HPP

#include "model/argument_range.hpp"
#include "model/bound_value.hpp"

#include <libxml/tree.h>

#include <string>
#include <vector>

// The srcML markup of JavaScript and TypeScript: a TypeScript annotation follows its name as a ts:type element, a method reaches its object
// through this, and srcML marks true and false as names

const xmlNode*              typeAnnotationOf            (const xmlNode*);
std::string                 javascriptTypeOf            (const xmlNode*);
std::string                 javascriptDeclaredType      (const xmlNode*);
std::string                 javascriptNamedType         (const std::string&);
boundValue                  javascriptBoundValue        (const xmlNode*);
argumentRange               javascriptArgumentRange     (const xmlNode*);
std::string                 createdClassOf              (const xmlNode*);
bool                        isJavaScriptBooleanByForm   (const xmlNode*);
bool                        isUndefinedLiteral          (const xmlNode*);
bool                        isStaticMethod              (const xmlNode*);
bool                        isPropertyAccessor          (const xmlNode*);
std::string                 unnamedMethodName           (const xmlNode*);
std::vector<std::string>    templateLiteralNames        (const std::string&);
std::vector<const xmlNode*> patternDeclarations         (const xmlNode*);
std::string                 withoutOptionalChaining     (std::string);

#endif
