// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file python_elements.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/python_elements.hpp"
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
#include <utility>

namespace {
    // Tells whether a character can be part of a name
    //
    bool isNameCharacter(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    }

    // Tells whether a range of tokens ends on a call of a method of the built-in types that tests its object, as startswith of a string does,
    // where only the access operator joins the names, the calls, and the groups in parentheses of the range, as in name.strip().isdigit()
    //
    bool testsObject(const std::vector<const xmlNode*>& tokens, std::size_t begin, std::size_t end) {
        static const std::unordered_set<std::string> testMethods{"startswith", "endswith", "isalnum", "isalpha", "isascii", "isdecimal", "isdigit",
                "isidentifier", "islower", "isnumeric", "isprintable", "isspace", "istitle", "isupper", "isdisjoint", "issubset", "issuperset", "is_integer"};

        const xmlNode* last = tokens[end - 1];
        if (!isSrcElement(last, "call")) return false;

        const std::vector<std::string> operators = operatorsOutsideParentheses(tokens, begin, end);
        if (std::any_of(operators.begin(), operators.end(), [](const std::string& op) { return op != "."; })) return false;

        const std::string name     = pythonCallName(last);
        const std::size_t access   = name.rfind('.');
        const bool        onObject = access != std::string::npos || (end - begin >= 2 && isOperator(tokens[end - 2], "."));
        return onObject && testMethods.count(access == std::string::npos ? name : name.substr(access + 1)) > 0;
    }

    // Tells whether a range of the tokens of an expression is Boolean by its form
    //
    bool isBooleanRange(const std::vector<const xmlNode*>& tokens, std::size_t begin, std::size_t end) {
        static const std::unordered_set<std::string> comparisons{"==", "!=", "<", "<=", ">", ">=", "is", "is not", "in", "not in"};
        static const std::unordered_set<std::string> booleanCalls{"isinstance", "issubclass", "callable", "hasattr", "bool", "any", "all"};

        removeEnclosingParentheses(tokens, begin, end);
        if (begin >= end) return false;

        // The value before if stands before the ternary, which holds the condition and the value after else
        if (isSrcElement(tokens[end - 1], "ternary")) {
            const xmlNode* otherwise = firstChildElement(firstChildElement(tokens[end - 1], "else"), "expr");
            return otherwise && isBooleanRange(tokens, begin, end - 1) && isBooleanByForm(otherwise);
        }

        for (std::string_view connective : {"or", "and"}) {
            const auto operands = splitAt(tokens, begin, end, connective);
            if (operands.size() == 1) continue;
            return std::all_of(operands.begin(), operands.end(), [&tokens](const auto& operand) { return isBooleanRange(tokens, operand.first, operand.second); });
        }
        if (isOperator(tokens[begin], "not")) return true;

        const std::vector<std::string> operators = operatorsOutsideParentheses(tokens, begin, end);
        if (std::any_of(operators.begin(), operators.end(), [](const std::string& op) { return comparisons.count(op) > 0; })) return true;
        if (testsObject(tokens, begin, end)) return true;
        if (end - begin != 1) return false;

        const xmlNode* token = tokens[begin];
        if (isSrcElement(token, "literal")) {
            xmlChar* type = xmlGetProp(token, BAD_CAST "type");
            const bool boolean = type && xmlStrEqual(type, BAD_CAST "boolean");
            xmlFree(type);
            return boolean;
        }
        return isSrcElement(token, "call") && booleanCalls.count(pythonCallName(token)) > 0;
    }
}

// Gets the type that an annotation names, as the other languages write it: with the arguments of a generic type in angle brackets,
// without the quotes of a forward reference, and without the values of a Literal, such as Optional<Engine> of Optional["Engine"]
//
std::string pythonTypeOf(const xmlNode* annotation) {
    const xmlNode* expression = firstChildElement(annotation, "expr");
    if (!expression) return "";

    const std::string source = sourceOf(expression);
    constexpr std::string_view literal = "Literal";
    std::string type;
    for (std::size_t i = 0; i < source.size(); ++i) {
        const char c = source[i];
        if (c == '[' && type.size() >= literal.size() && type.compare(type.size() - literal.size(), literal.size(), literal) == 0) {
            int depth = 0;
            for (; i < source.size(); ++i) {
                if      (source[i] == '[') ++depth;
                else if (source[i] == ']' && --depth == 0) break;
            }
        }
        else if (c == '[') type += '<';
        else if (c == ']') type += '>';
        else if (c != '"' && c != '\'') type += c;
    }
    return type;
}

// Gets the name of the class that a type names, also as the argument of Optional or beside None, such as Engine of pkg.Engine,
// Optional<Engine>, or Engine | None, and Box of Box<int>, or nothing for a type of several names, such as int | str
// A module that qualifies the name is no part of the name of the class
//
std::string pythonNamedType(const std::string& type) {
    std::string name = type;
    HELPERS::removeWhitespace(name);

    constexpr std::string_view optional = "Optional<";
    if (name.compare(0, optional.size(), optional) == 0 && name.back() == '>') name = name.substr(optional.size(), name.size() - optional.size() - 1);
    for (std::string_view none : {"|None", "None|"}) {
        const std::size_t found = name.find(none);
        if (found != std::string::npos && (found == 0 || found + none.size() == name.size())) name.erase(found, none.size());
    }

    name = withoutGenericArguments(name);
    HELPERS::removeNamespace(name, "Python", true);
    return !name.empty() && std::all_of(name.begin(), name.end(), isNameCharacter) ? name : "";
}

// Reads what the assignment after a target name binds the name to, where only = alone binds a name, unlike += or a chain such as a = b = c
//
boundValue pythonBoundValue(const xmlNode* target) {
    boundValue value;
    const xmlNode* next = xmlNextElementSibling(const_cast<xmlNode*>(target));
    if (isSrcElement(next, "annotation")) {
        value.type = pythonTypeOf(next);
        next = xmlNextElementSibling(const_cast<xmlNode*>(next));
    }
    if (xmlPreviousElementSibling(const_cast<xmlNode*>(target)) || !isOperator(next, "=")) return value;

    const xmlNode* bound = xmlNextElementSibling(const_cast<xmlNode*>(next));
    if (!bound || xmlNextElementSibling(const_cast<xmlNode*>(bound))) return value;
    if      (isSrcElement(bound, "name")) value.name     = sourceOf(bound);
    else if (isSrcElement(bound, "call")) value.callName = pythonCallName(bound);
    return value;
}

// Gets the numbers of arguments that a parameter list takes, without the first parameter where it stands for the object or the class
// The markers / and * have no name, a parameter with a default value may get no argument, and *args and **kwargs take any number
//
argumentRange pythonArgumentRange(const xmlNode* list, bool firstBound) {
    argumentRange range;
    bool skip = firstBound;
    for (const xmlNode* parameter = firstChildElement(list, "parameter"); parameter; parameter = nextNamedSibling(parameter, "parameter")) {
        if (!firstChildElement(parameter, "name")) continue;
        if (std::exchange(skip, false)) continue;

        if (xmlHasProp(parameter, BAD_CAST "type")) {
            range.variadic = true;
            continue;
        }
        ++range.maximum;
        if (!firstChildElement(parameter, "init")) ++range.minimum;
    }
    return range;
}

// Gets the call that alone is the value of an expression, also in parentheses or after await, or null
//
const xmlNode* soleCallOf(const xmlNode* expression) {
    const std::vector<const xmlNode*> tokens = tokensOf(expression);
    std::size_t begin = 0;
    std::size_t end   = tokens.size();
    removeEnclosingParentheses(tokens, begin, end);
    if (begin < end && isOperator(tokens[begin], "await")) ++begin;
    return end - begin == 1 && isSrcElement(tokens[begin], "call") ? tokens[begin] : nullptr;
}

// Gets the name of a call without whitespace, where type(x)(...) calls the class of x, as x.__class__(...) does, or nothing for a call without a name
//
std::string pythonCallName(const xmlNode* call) {
    const xmlNode* first = xmlFirstElementChild(const_cast<xmlNode*>(call));
    std::string name;
    if (isSrcElement(first, "name")) {
        name = sourceOf(first);
    }
    else if (isSrcElement(first, "call") && pythonCallName(first) == "type") {
        const xmlNode* argument = firstChildElement(firstChildElement(first, "argument_list"), "argument");
        const xmlNode* object   = argument && !nextNamedSibling(argument, "argument") ? firstChildElement(argument, "expr") : nullptr;
        const std::vector<const xmlNode*> tokens = object ? tokensOf(object) : std::vector<const xmlNode*>{};
        if (tokens.size() == 1 && isSrcElement(tokens.front(), "name")) name = sourceOf(tokens.front()) + ".__class__";
    }
    HELPERS::removeWhitespace(name);
    return name;
}

// Tells whether a call names a method on the result of super(), as up of super().up() does
//
bool followsSuperCall(const xmlNode* call) {
    const xmlNode* access = xmlPreviousElementSibling(const_cast<xmlNode*>(call));
    const xmlNode* object = isOperator(access, ".") ? xmlPreviousElementSibling(const_cast<xmlNode*>(access)) : nullptr;
    return isSrcElement(object, "call") && pythonCallName(object) == "super";
}

// Gets the object of a call that follows the access operator after variables with an index, without whitespace and with the operator at its end,
// as rows[0]. of rows[0].strip() and a[0].b[1]. of a[0].b[1].c(), or nothing for a call on the result of a call or on a value
// srcML ends the name of a call at an index, so the names before the index stand apart from the call
//
std::string pythonIndexedObject(const xmlNode* call) {
    std::string object;
    for (const xmlNode* access = xmlPreviousElementSibling(const_cast<xmlNode*>(call)); isOperator(access, ".");) {
        const xmlNode* variable = xmlPreviousElementSibling(const_cast<xmlNode*>(access));
        if (!isSrcElement(variable, "name")) return "";

        object = sourceOf(variable) + "." + object;
        access = xmlPreviousElementSibling(const_cast<xmlNode*>(variable));
    }
    HELPERS::removeWhitespace(object);
    return object;
}

// Tells whether an expression is Boolean by its form: a Boolean literal, a negation, a comparison, a call of a function that tests its argument,
// such as isinstance, a call of a method of the built-in types that tests its object, such as startswith, a conjunction or a disjunction of
// such expressions, a conditional expression with two such results, or one of these in parentheses
//
bool isBooleanByForm(const xmlNode* expression) {
    const std::vector<const xmlNode*> tokens = tokensOf(expression);
    return isBooleanRange(tokens, 0, tokens.size());
}

// Tells whether an expression is the literal None, which gives the caller no value
//
bool isNoneLiteral(const xmlNode* expression) {
    const std::vector<const xmlNode*> tokens = tokensOf(expression);
    return tokens.size() == 1 && isSrcElement(tokens.front(), "literal") && sourceOf(tokens.front()) == "None";
}

// Tells whether a name follows the CapWords convention of class names, after its underscores in front: a capital letter and then
// a lowercase letter somewhere, as in Shape and HTTPServer, or only capital letters and digits, as in URL
//
bool namesPythonClass(const std::string& name) {
    const std::size_t start = name.find_first_not_of('_');
    if (start == std::string::npos || !std::isupper(static_cast<unsigned char>(name[start]))) return false;

    const auto isLower    = [](char c) { return std::islower(static_cast<unsigned char>(c)) != 0; };
    const auto isConstant = [](char c) { return std::isupper(static_cast<unsigned char>(c)) || std::isdigit(static_cast<unsigned char>(c)); };
    return std::any_of(name.begin() + static_cast<std::ptrdiff_t>(start), name.end(), isLower) ||
           std::all_of(name.begin() + static_cast<std::ptrdiff_t>(start), name.end(), isConstant);
}

// Gets the names that the holes of an f-string use, given the literal with its prefix and quotes, such as self.x and width of f"{self.x:>{width}}"
// A doubled brace is text, !r, !s, or !a converts the value of a hole, and a colon outside brackets starts its format, which may hold holes
//
std::vector<std::string> formattedStringNames(const std::string& literal) {
    static const std::unordered_set<std::string> keywords{"not", "and", "or", "if", "else", "in", "is", "for", "lambda", "await", "None", "True", "False"};
    std::vector<std::string> names;
    const std::size_t opening = literal.find_first_of("\"'");
    if (opening == std::string::npos) return names;

    const std::size_t quotes = literal.compare(opening, 3, std::string(3, literal[opening])) == 0 ? 3 : 1;
    const std::size_t textEnd = literal.size() >= opening + 2 * quotes ? literal.size() - quotes : literal.size();
    for (std::size_t i = opening + quotes; i < textEnd; ++i) {
        if (literal[i] != '{') continue;
        if (i + 1 < textEnd && literal[i + 1] == '{') {
            ++i;
            continue;
        }

        // The expression of the hole ends at its conversion, its format, or its closing brace
        int depth = 0;
        std::size_t end = i + 1;
        for (; end < textEnd; ++end) {
            const char c = literal[end];
            if (c == '"' || c == '\'') {
                end = quotedEnd(literal, end) - 1;
            }
            else if (c == '(' || c == '[' || c == '{') {
                ++depth;
            }
            else if (c == ')' || c == ']' || (c == '}' && depth > 0)) {
                --depth;
            }
            else if (depth == 0 && (c == '}' || c == ':' || (c == '!' && end + 1 < textEnd && literal[end + 1] != '='))) {
                break;
            }
        }
        addHoleNames(literal.substr(i + 1, end - i - 1), keywords, names);
        i = end;
    }
    return names;
}

// Gets the name of the module that a file defines, such as settings of app/settings.py, or the package of app/__init__.py
//
std::string moduleNameOf(const std::string& fileName) {
    std::string path = fileName;
    std::replace(path.begin(), path.end(), '\\', '/');
    const std::size_t extension = path.rfind('.');
    if (extension != std::string::npos && extension > path.rfind('/') + 1) path.erase(extension);

    constexpr std::string_view package = "/__init__";
    if (path.size() > package.size() && path.compare(path.size() - package.size(), package.size(), package) == 0) path.erase(path.size() - package.size());
    return path.substr(path.rfind('/') + 1);
}
