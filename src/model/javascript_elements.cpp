// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file javascript_elements.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/javascript_elements.hpp"
#include "helper_functions.hpp"
#include "model/expression_tokens.hpp"
#include "model/hole_names.hpp"
#include "model/qualified_name.hpp"
#include "archive/node_text.hpp"
#include "archive/srcml_elements.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>
#include <unordered_set>

namespace {
    // Gets the value of the attribute type of an element, such as get of a getter, or an empty text
    //
    std::string typeAttributeOf(const xmlNode* element) {
        xmlChar* value = xmlGetProp(element, BAD_CAST "type");
        std::string text = value ? reinterpret_cast<const char*>(value) : "";
        xmlFree(value);
        return text;
    }

    // Tells whether a token is the literal true or false, which srcML marks as a name in JavaScript and as a literal in TypeScript
    //
    bool isBooleanLiteral(const xmlNode* token) {
        const bool marked = isSrcElement(token, "name") || (isSrcElement(token, "literal") && typeAttributeOf(token) == "boolean");
        return marked && (sourceOf(token) == "true" || sourceOf(token) == "false");
    }

    // Gets a text without its whitespace
    //
    std::string withoutWhitespace(std::string text) {
        HELPERS::removeWhitespace(text);
        return text;
    }

    // Gets the name of a call without whitespace, or nothing for a call without a name
    //
    std::string callNameOf(const xmlNode* call) {
        const xmlNode* first = xmlFirstElementChild(const_cast<xmlNode*>(call));
        return isSrcElement(first, "name") ? withoutWhitespace(sourceOf(first)) : "";
    }

    // Gets the class of the object that a range of tokens creates as its whole value, also in parentheses or after await, or an empty text
    // srcML marks await as a name in a function and as an operator at the top level of a module
    //
    std::string createdClassIn(const std::vector<const xmlNode*>& tokens, std::size_t begin, std::size_t end) {
        removeEnclosingParentheses(tokens, begin, end);
        if (begin < end && (isOperator(tokens[begin], "await") || (isSrcElement(tokens[begin], "name") && sourceOf(tokens[begin]) == "await"))) ++begin;
        if (end - begin != 2 || !isOperator(tokens[begin], "new")) return "";

        const xmlNode* created = tokens[begin + 1];
        return isSrcElement(created, "call") ? callNameOf(created) : isSrcElement(created, "name") ? withoutWhitespace(sourceOf(created)) : "";
    }

    // Tells whether a range of tokens ends on a call of a method of the built-in types that tests its object, as startsWith of a string and has
    // of a map do, where only the access operators join the names and the calls of the range, as in name.trim().startsWith("a")
    // A call through this names a method of the class, whatever its name
    //
    bool testsObject(const std::vector<const xmlNode*>& tokens, std::size_t begin, std::size_t end) {
        static const std::unordered_set<std::string> testMethods{"startsWith", "endsWith", "includes", "has", "every", "some", "test", "hasOwnProperty",
                                                                 "isPrototypeOf"};

        const xmlNode* last = tokens[end - 1];
        if (!isSrcElement(last, "call")) return false;
        const std::vector<std::string> operators = operatorsOutsideParentheses(tokens, begin, end);
        if (std::any_of(operators.begin(), operators.end(), [](const std::string& op) { return op != "." && op != "?."; })) return false;

        const std::string name   = callNameOf(last);
        const std::size_t access = name.rfind('.');
        const bool onObject      = (access != std::string::npos && name.compare(0, access, "this") != 0) ||
                                   (end - begin >= 2 && isSrcElement(tokens[end - 2], "operator"));
        return onObject && testMethods.count(access == std::string::npos ? name : name.substr(access + 1)) > 0;
    }

    // Tells whether a range of the tokens of an expression is Boolean by its form
    //
    bool isBooleanRange(const std::vector<const xmlNode*>& tokens, std::size_t begin, std::size_t end) {
        static const std::unordered_set<std::string> comparisons{"===", "!==", "==", "!=", "<", "<=", ">", ">=", "instanceof", "in"};
        static const std::unordered_set<std::string> booleanCalls{"Boolean", "isNaN", "isFinite", "Array.isArray", "Number.isInteger", "Number.isSafeInteger",
                                                                  "Number.isFinite", "Number.isNaN", "Object.is", "Object.hasOwn", "Object.isFrozen",
                                                                  "Object.isSealed", "Object.isExtensible"};

        removeEnclosingParentheses(tokens, begin, end);
        if (begin >= end) return false;

        // The ternary holds its condition and its two results
        if (end - begin == 1 && isSrcElement(tokens[begin], "ternary")) {
            const xmlNode* then      = firstChildElement(firstChildElement(tokens[begin], "then"), "expr");
            const xmlNode* otherwise = firstChildElement(firstChildElement(tokens[begin], "else"), "expr");
            return then && otherwise && isJavaScriptBooleanByForm(then) && isJavaScriptBooleanByForm(otherwise);
        }

        for (std::string_view connective : {"||", "&&"}) {
            const tokenRanges operands = splitAt(tokens, begin, end, connective);
            if (operands.size() == 1) continue;
            return std::all_of(operands.begin(), operands.end(), [&tokens](const auto& operand) { return isBooleanRange(tokens, operand.first, operand.second); });
        }
        if (isOperator(tokens[begin], "!")) return true;

        const std::vector<std::string> operators = operatorsOutsideParentheses(tokens, begin, end);
        if (std::find(operators.begin(), operators.end(), "??") != operators.end()) return false;
        if (std::any_of(operators.begin(), operators.end(), [](const std::string& op) { return comparisons.count(op) > 0; })) return true;
        if (testsObject(tokens, begin, end)) return true;
        if (end - begin != 1) return false;

        return isBooleanLiteral(tokens[begin]) || (isSrcElement(tokens[begin], "call") && booleanCalls.count(callNameOf(tokens[begin])) > 0);
    }
}

// Gets the TypeScript annotation of a declaration or a function, the ts:type element after its name or its parameter list, or null
//
const xmlNode* typeAnnotationOf(const xmlNode* element) {
    for (xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(element)); child; child = xmlNextElementSibling(child))
        if (isTypeScriptElement(child, "type")) return child;
    return nullptr;
}

// Gets the type that a TypeScript annotation names, without the colon before it, where a function type, as (x: number) => void, holds code
// and an object type, as { x: number }, holds no object of a class
//
std::string javascriptTypeOf(const xmlNode* annotation) {
    std::string type = sourceOf(annotation);
    const std::size_t start = type.find_first_not_of(": \t\r\n");
    const std::size_t end   = type.find_last_not_of(" \t\r\n");
    type = start == std::string::npos ? "" : type.substr(start, end - start + 1);

    if (type.find("=>") != std::string::npos) return "Function";
    if (!type.empty() && type.front() == '{') return "object";
    return type;
}

// Gets the type of a declaration: the type of its annotation or else the class of the object that its initializer creates, as Engine of
// const engine = new Engine(4), or an empty text
//
std::string javascriptDeclaredType(const xmlNode* declaration) {
    if (const xmlNode* annotation = typeAnnotationOf(declaration)) return javascriptTypeOf(annotation);
    const xmlNode* initializer = firstChildElement(declaration, "init");
    const xmlNode* value = initializer ? firstChildElement(initializer, "expr") : nullptr;
    return value ? createdClassOf(value) : "";
}

// Gets the name of the class that a type names, also beside undefined or null, such as Engine of ns.Engine, Engine | undefined, or Box<number>,
// or nothing for a type of several names, as number | string, or of an array, as Engine[]
//
std::string javascriptNamedType(const std::string& type) {
    std::string name = type;
    HELPERS::removeWhitespace(name);
    for (std::string_view none : {"|undefined", "|null", "undefined|", "null|"}) {
        const std::size_t found = name.find(none);
        if (found != std::string::npos && (found == 0 || found + none.size() == name.size())) name.erase(found, none.size());
    }

    name = withoutGenericArguments(name);
    HELPERS::removeNamespace(name, "JavaScript", true);
    const auto isNamePart = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '$'; };
    return !name.empty() && std::all_of(name.begin(), name.end(), isNamePart) ? name : "";
}

// Reads what the assignment after a target name binds the name to, where only = alone binds a name, unlike += or a chain such as a = b = c:
// a name, as owner of this.owner = owner, or the class of a created object, as Engine of this.engine = new Engine(4)
//
boundValue javascriptBoundValue(const xmlNode* target) {
    boundValue value;
    const xmlNode* next = xmlNextElementSibling(const_cast<xmlNode*>(target));
    if (xmlPreviousElementSibling(const_cast<xmlNode*>(target)) || !isOperator(next, "=")) return value;

    const xmlNode* bound = xmlNextElementSibling(const_cast<xmlNode*>(next));
    if (isSrcElement(bound, "name") && !xmlNextElementSibling(const_cast<xmlNode*>(bound))) {
        value.name = sourceOf(bound);
        return value;
    }
    const std::vector<const xmlNode*> tokens = tokensOf(target->parent);
    const auto start = std::find(tokens.begin(), tokens.end(), bound);
    value.type = createdClassIn(tokens, static_cast<std::size_t>(start - tokens.begin()), tokens.size());
    return value;
}

// Gets the numbers of arguments that a parameter list takes, where a parameter with a default value or with ? may get no argument and
// a rest parameter, as ...items, takes any number
//
argumentRange javascriptArgumentRange(const xmlNode* list) {
    argumentRange range;
    for (const xmlNode* parameter = firstChildElement(list, "parameter"); parameter; parameter = nextNamedSibling(parameter, "parameter")) {
        const xmlNode* declaration = firstChildElement(parameter, "decl");
        if (!declaration) continue;

        const xmlNode* modifier = firstChildElement(declaration, "modifier");
        if (modifier && sourceOf(modifier) == "...") {
            range.variadic = true;
            continue;
        }
        bool optional = firstChildElement(declaration, "init") != nullptr;
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(declaration)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child)))
            optional = optional || (isTypeScriptElement(child, "modifier") && sourceOf(child) == "?");
        ++range.maximum;
        if (!optional) ++range.minimum;
    }
    return range;
}

// Gets the class of the object that an expression creates as its whole value, also in parentheses or after await, such as Engine of
// new Engine(4) and Map<string,Engine> of new Map<string, Engine>(), or an empty text
//
std::string createdClassOf(const xmlNode* expression) {
    const std::vector<const xmlNode*> tokens = tokensOf(expression);
    return createdClassIn(tokens, 0, tokens.size());
}

// Tells whether an expression is Boolean by its form: a Boolean literal, a negation, a comparison, a call of a function that tests its argument,
// such as Array.isArray, a call of a method of the built-in types that tests its object, such as startsWith, a conjunction or a disjunction of
// such expressions, a conditional expression with two such results, or one of these in parentheses
//
bool isJavaScriptBooleanByForm(const xmlNode* expression) {
    const std::vector<const xmlNode*> tokens = tokensOf(expression);
    return isBooleanRange(tokens, 0, tokens.size());
}

// Tells whether an expression is the literal undefined, which gives the caller no value
//
bool isUndefinedLiteral(const xmlNode* expression) {
    const std::vector<const xmlNode*> tokens = tokensOf(expression);
    return tokens.size() == 1 && sourceOf(tokens.front()) == "undefined";
}

// Tells whether a method of a class is static, where static comes first among the specifiers of JavaScript, after those of TypeScript
//
bool isStaticMethod(const xmlNode* function) {
    const xmlNode* specifier = firstChildElement(function, "specifier");
    return specifier && sourceOf(specifier) == "static";
}

// Tells whether a function is a get or set accessor of the objects of its class, which names a property, as size of get size() does
// srcML marks a method named get or set, as get(key), as an accessor without a name
//
bool isPropertyAccessor(const xmlNode* function) {
    const std::string kind = typeAttributeOf(function);
    return (kind == "get" || kind == "set") && firstChildElement(function, "name") && !isStaticMethod(function);
}

// Gets the name of a method without a name element: constructor for a constructor, and get or set for a method of that name,
// which srcML marks as an accessor without a name, or an empty text
//
std::string unnamedMethodName(const xmlNode* function) {
    if (isSrcElement(function, "constructor")) return "constructor";
    const std::string kind = typeAttributeOf(function);
    return isSrcElement(function, "function") && (kind == "get" || kind == "set") && !firstChildElement(function, "name") ? kind : "";
}

// Gets the names that the holes of a template literal use, given the literal with its backticks, such as this.count of `${this.count} items`
//
std::vector<std::string> templateLiteralNames(const std::string& literal) {
    static const std::unordered_set<std::string> keywords{"true", "false", "null", "undefined", "typeof", "instanceof", "new", "in", "of", "await",
                                                          "void", "delete", "function", "return", "async", "yield", "NaN", "Infinity"};
    std::vector<std::string> names;
    for (std::size_t i = 0; i + 1 < literal.size(); ++i) {
        if (literal[i] == '\\') {
            ++i;
            continue;
        }
        if (literal[i] != '$' || literal[i + 1] != '{') continue;

        // The code of the hole ends at the brace that closes it, outside its strings and its nested brackets
        int depth = 0;
        std::size_t end = i + 2;
        for (; end < literal.size(); ++end) {
            const char c = literal[end];
            if (c == '"' || c == '\'' || c == '`') end = quotedEnd(literal, end) - 1;
            else if (c == '(' || c == '[' || c == '{') ++depth;
            else if (c == ')' || c == ']') --depth;
            else if (c == '}' && depth == 0) break;
            else if (c == '}') --depth;
        }
        addHoleNames(literal.substr(i + 2, end - i - 2), keywords, names);
        i = end;
    }
    return names;
}

// Gets the declarations of the names that a declaration of a parameter declares: the declaration itself, or the declarations of the keys of
// its destructuring pattern, as x and y of { x, y }: Point, outside their default values
//
std::vector<const xmlNode*> patternDeclarations(const xmlNode* declaration) {
    // srcML marks a key that a pattern renames as a declaration whose initializer starts with a colon and holds the new name or a nested
    // pattern, as b of { a: b } and z of { inner: { z } }
    const xmlNode* initializer = isSrcElement(declaration, "decl") ? firstChildElement(declaration, "init") : nullptr;
    const xmlNode* renamed     = initializer && sourceOf(initializer).compare(0, 1, ":") == 0 ? firstChildElement(initializer, "expr") : nullptr;
    if (renamed) {
        const xmlNode* target = xmlFirstElementChild(const_cast<xmlNode*>(renamed));
        if (isSrcElement(target, "name")) return {renamed};
        return isSrcElement(target, "object") || isSrcElement(target, "array") ? patternDeclarations(target) : std::vector<const xmlNode*>{};
    }
    if (isSrcElement(declaration, "decl") && firstChildElement(declaration, "name")) return {declaration};

    // The pattern holds the declarations in its objects, arrays, and properties, while its annotation and its default value declare nothing
    // srcML marks each name of an array pattern of a parameter as an expression, as first and others of [first, ...others]
    std::vector<const xmlNode*> declarations;
    for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(declaration)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
        if (isSrcElement(child, "expr") && isSrcElement(declaration, "array")) {
            if (firstChildElement(child, "name")) declarations.push_back(child);
            continue;
        }
        if (!isSrcElement(child, "decl") && !isSrcElement(child, "object") && !isSrcElement(child, "array") && !isSrcElement(child, "property")) continue;
        const std::vector<const xmlNode*> inner = patternDeclarations(child);
        declarations.insert(declarations.end(), inner.begin(), inner.end());
    }
    return declarations;
}

// Gets a name with . in place of each ?., as this.cache.get of this.cache?.get, since both operators reach the same member
//
std::string withoutOptionalChaining(std::string name) {
    for (std::size_t found = name.find("?."); found != std::string::npos; found = name.find("?.", found)) name.erase(found, 1);
    return name;
}
