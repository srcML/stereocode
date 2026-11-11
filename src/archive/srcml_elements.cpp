// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file srcml_elements.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "archive/srcml_elements.hpp"
#include "archive/node_text.hpp"
#include "model/qualified_name.hpp"
#include "archive/srcml_namespaces.hpp"

#include <cctype>
#include <stdexcept>
#include <vector>

// Tells whether a node is the srcML element with a name
//
bool isSrcElement(const xmlNode* node, std::string_view name) {
    return node && node->type == XML_ELEMENT_NODE && node->ns && xmlStrEqual(node->ns->href, BAD_CAST srcNamespace) &&
           name == reinterpret_cast<const char*>(node->name);
}

// Tells whether an element is a type, such as a class or an enum
//
bool isTypeElement(const xmlNode* node) {
    return isSrcElement(node, "class") || isSrcElement(node, "struct") || isSrcElement(node, "union") ||
           isSrcElement(node, "interface") || isSrcElement(node, "enum");
}

// Gets the first child of an element that is the srcML element with a name, or null
//
xmlNode* firstChildElement(const xmlNode* element, std::string_view name) {
    for (xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(element)); child; child = xmlNextElementSibling(child))
        if (isSrcElement(child, name)) return child;
    return nullptr;
}

// Gets the next sibling of an element that is the srcML element with a name, or null
//
xmlNode* nextNamedSibling(const xmlNode* element, std::string_view name) {
    for (xmlNode* sibling = xmlNextElementSibling(const_cast<xmlNode*>(element)); sibling; sibling = xmlNextElementSibling(sibling))
        if (isSrcElement(sibling, name)) return sibling;
    return nullptr;
}

namespace {
    // Tells whether a name of a typedef names the type of the typedef itself, unlike a pointer, a reference, or an array,
    // as Point does and PointPtr does not in typedef struct { } *PointPtr, Point;
    //
    bool namesTypeItself(const xmlNode* name) {
        if (xmlFirstElementChild(const_cast<xmlNode*>(name))) return false;
        const xmlNode* before = name->prev;
        if (!before || before->type != XML_TEXT_NODE || !before->content) return true;
        const std::string_view text = reinterpret_cast<const char*>(before->content);
        return text.find_first_of("*&") == std::string_view::npos;
    }
}

// Gets the name element of a type, or the typedef name of a C++ class, struct, or union without one, as Point of typedef struct { } *PointPtr, Point;
//
xmlNode* typeNameElement(const xmlNode* type) {
    if (xmlNode* name = firstChildElement(type, "name")) return name;
    if (!isSrcElement(type, "class") && !isSrcElement(type, "struct") && !isSrcElement(type, "union")) return nullptr;

    const xmlNode* typedefType = type->parent;
    if (!isSrcElement(typedefType, "type") || !isSrcElement(typedefType->parent, "typedef")) return nullptr;
    for (xmlNode* name = nextNamedSibling(typedefType, "name"); name; name = nextNamedSibling(name, "name"))
        if (namesTypeItself(name)) return name;
    return nullptr;
}

// Tells whether a type stands for the type of the declaration before it, as the type of b in int a, b;
//
bool refersToPreviousType(const xmlNode* type) {
    xmlChar* reference = xmlGetNoNsProp(type, BAD_CAST "ref");
    const bool previous = reference && xmlStrEqual(reference, BAD_CAST "prev");
    xmlFree(reference);
    return previous;
}

// Tells whether an element, such as a type or a class, holds the specifier static
//
bool declaresStatic(const xmlNode* element) {
    for (const xmlNode* specifier = firstChildElement(element, "specifier"); specifier; specifier = nextNamedSibling(specifier, "specifier"))
        if (sourceOf(specifier) == "static") return true;
    return false;
}

// Gets the number of parameters of a parameter list or of arguments of an argument list
// The parameter list (void) of C declares no parameter
//
int countListItems(const xmlNode* list) {
    int count = 0;
    for (xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(list)); child; child = xmlNextElementSibling(child))
        if (isSrcElement(child, "parameter") || isSrcElement(child, "argument")) ++count;

    if (count == 1 && isSrcElement(list, "parameter_list")) {
        const xmlNode* declaration = firstChildElement(firstChildElement(list, "parameter"), "decl");
        const xmlNode* type = declaration ? firstChildElement(declaration, "type") : nullptr;
        if (type && !firstChildElement(declaration, "name") && sourceOf(type) == "void") return 0;
    }
    return count;
}

namespace {
    // Appends the source code of a node without whitespace to a text
    //
    void appendWithoutWhitespace(std::string& text, const xmlNode* node) {
        for (char c : sourceOf(node))
            if (!std::isspace(static_cast<unsigned char>(c))) text += c;
    }

    // Tells whether a node is the C# word ref, out, or in that passes an argument by reference, which tells overloads apart
    //
    bool isPassingWord(const xmlNode* node) {
        if (!isSrcElement(node, "modifier") && !isSrcElement(node, "specifier")) return false;
        const std::string word = sourceOf(node);
        return word == "ref" || word == "out" || word == "in";
    }

    // Appends the source code of a node of a type without whitespace, where a word that passes an argument by reference keeps a space after it,
    // also inside a qualified name, as in ref System.Drawing.Point
    //
    void appendTypeText(std::string& text, const xmlNode* node) {
        if (isPassingWord(node)) {
            text += sourceOf(node) + ' ';
            return;
        }
        if (node->type != XML_ELEMENT_NODE || !xmlFirstElementChild(const_cast<xmlNode*>(node))) {
            appendWithoutWhitespace(text, node);
            return;
        }
        for (const xmlNode* child = node->children; child; child = child->next) appendTypeText(text, child);
    }

    // Gets a type without whitespace and without its specifiers, attributes, and annotations, such as std::string& of const std::string&
    //
    std::string typeWithoutSpecifiers(const xmlNode* type) {
        std::string text;
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(type)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child)))
            if (isPassingWord(child) || (!isSrcElement(child, "specifier") && !isSrcElement(child, "attribute") && !isSrcElement(child, "annotation"))) appendTypeText(text, child);
        return text;
    }

    // Adds the source code of a node without whitespace to the text of a type
    //
    void addSource(writtenType& type, const xmlNode* node) {
        std::string text;
        appendWithoutWhitespace(text, node);
        addText(type, text);
    }

    // Tells whether a name holds template arguments, as std::vector<Count> and Grid<Count>::Row do
    //
    bool holdsArguments(const xmlNode* name) {
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(name)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child)))
            if (isSrcElement(child, "argument_list") || (isSrcElement(child, "name") && holdsArguments(child))) return true;
        return false;
    }

    void addTypeParts(writtenType&, const xmlNode*);

    // Adds a name with template arguments to a type as text, apart from the types of its arguments, as std::vector< and > around Count
    // of std::vector<Count>, since the name of a template is no alias
    //
    void addTemplateName(writtenType& type, const xmlNode* name) {
        for (const xmlNode* child = name->children; child; child = child->next) {
            if (isSrcElement(child, "argument")) addTypeParts(type, child);
            else if (isSrcElement(child, "name") || isSrcElement(child, "argument_list")) addTemplateName(type, child);
            else addSource(type, child);
        }
    }

    // Adds the nodes of a type or of a template argument to a type, where each name without template arguments is a part of its own
    //
    void addTypeParts(writtenType& type, const xmlNode* parent) {
        for (const xmlNode* child = parent->children; child; child = child->next) {
            if (isSrcElement(child, "expr") || isSrcElement(child, "type")) {
                addTypeParts(type, child);
            }
            else if (!isSrcElement(child, "name")) {
                addSource(type, child);
            }
            else if (holdsArguments(child)) {
                addTemplateName(type, child);
            }
            else {
                type.push_back({"", true});
                appendWithoutWhitespace(type.back().text, child);
            }
        }
    }

    // Gets the type of a parameter as the source code writes it, or the whole parameter without a type, as a function pointer
    //
    writtenType parameterTypeOf(const xmlNode* parameter) {
        const xmlNode* declaration = firstChildElement(parameter, "decl");
        const xmlNode* type = declaration ? firstChildElement(declaration, "type") : nullptr;
        if (type) return writtenTypeOf(type);

        writtenType written;
        addSource(written, parameter);
        return written;
    }

    // Gets the type of a parameter in a signature, without its name and specifiers, such as int[] of int a[] or int(*)(int) of the function pointer int (*f)(int)
    //
    std::string signatureTypeOf(const xmlNode* parameter) {
        if (const xmlNode* pointer = firstChildElement(parameter, "function_decl")) {
            std::string text = typeWithoutSpecifiers(firstChildElement(pointer, "type")) + "(";
            for (const xmlNode* modifier = firstChildElement(pointer, "modifier"); modifier; modifier = nextNamedSibling(modifier, "modifier"))
                appendWithoutWhitespace(text, modifier);
            return text + ")" + signatureTypesOf(firstChildElement(pointer, "parameter_list"));
        }

        const xmlNode* declaration = firstChildElement(parameter, "decl");
        const xmlNode* type = declaration ? firstChildElement(declaration, "type") : nullptr;
        if (!type) {
            std::string text;
            appendWithoutWhitespace(text, parameter);
            return text;
        }

        // The brackets after the name of a C++ or Java parameter, as in int a[], belong to its type
        std::string text = typeWithoutSpecifiers(type);
        for (const xmlNode* index = firstChildElement(firstChildElement(declaration, "name"), "index"); index; index = nextNamedSibling(index, "index"))
            appendWithoutWhitespace(text, index);
        return text;
    }

    // Gets the types of the parameters of a parameter list, each as a function of the parameter gives it
    //
    std::string listTypes(const xmlNode* list, std::string (*typeOf)(const xmlNode*)) {
        std::string types = "(";
        if (countListItems(list) > 0) {
            for (const xmlNode* parameter = firstChildElement(list, "parameter"); parameter; parameter = nextNamedSibling(parameter, "parameter")) {
                if (types.size() > 1) types += ',';
                types += typeOf(parameter);
            }
        }
        return types + ")";
    }

    // Tells whether a parameter takes any number of arguments, as ... of C and C++, a C++ parameter pack, params of C#, and int... of Java do
    //
    bool isVariadic(const xmlNode* parameter) {
        const xmlNode* type = firstChildElement(firstChildElement(parameter, "decl"), "type");
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(type)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
            const std::string text = sourceOf(child);
            if ((isSrcElement(child, "modifier") && text == "...") || (isSrcElement(child, "specifier") && text == "params")) return true;
        }
        return false;
    }
}

// Gets the types of the parameters of a parameter list without whitespace, such as (int,constchar*), which tell overloads apart
//
std::string parameterTypesOf(const xmlNode* list) {
    return typeListText(writtenTypesOf(list));
}

// Gets the types of the parameters of a parameter list as the source code writes them, where a C++ signature may write an alias
// in place of the type that the alias stands for
//
std::vector<writtenType> writtenTypesOf(const xmlNode* list) {
    std::vector<writtenType> types;
    if (countListItems(list) == 0) return types;
    for (const xmlNode* parameter = firstChildElement(list, "parameter"); parameter; parameter = nextNamedSibling(parameter, "parameter"))
        types.push_back(parameterTypeOf(parameter));
    return types;
}

// Gets a type with its specifiers as the source code writes it, where each name of a type without template arguments is a part of its own,
// such as const, Count, and & of const Count& or std::vector<, Count, and > of std::vector<Count>
//
writtenType writtenTypeOf(const xmlNode* type) {
    writtenType written;
    addTypeParts(written, type);
    return written;
}

// Gets the types of the parameters of a parameter list in a signature, without their names, specifiers, and whitespace, such as (std::string&,int)
// of (const std::string& s, int n = 0), where a declaration and a definition that place the specifiers apart, as in A const& and const A&, agree
//
std::string signatureTypesOf(const xmlNode* list) {
    return listTypes(list, signatureTypeOf);
}

// Gets the numbers of arguments that a parameter list takes, where a parameter with a default value may get no argument
//
argumentRange argumentRangeOf(const xmlNode* list) {
    argumentRange range;
    range.maximum = countListItems(list);
    if (range.maximum == 0) return range;

    for (const xmlNode* parameter = firstChildElement(list, "parameter"); parameter; parameter = nextNamedSibling(parameter, "parameter")) {
        if (isVariadic(parameter)) range.variadic = true;
        else if (!firstChildElement(firstChildElement(parameter, "decl"), "init")) ++range.minimum;
    }
    return range;
}

// Gets the name of a type that holds one name besides its specifiers and modifiers, such as Engine of const Engine* or ns::Box<int> of ns::Box<int>&,
// or nothing for a type of several names, such as unsigned int, or of an array, such as int[] of C# and Java
//
std::string namedTypeOf(const xmlNode* type) {
    const xmlNode* name = firstChildElement(type, "name");
    if (!name || nextNamedSibling(name, "name") || firstChildElement(name, "index")) return "";

    std::string text;
    appendWithoutWhitespace(text, name);
    return text;
}

// Gets the smart pointer of the C++ standard library that a name writes, with or without std::, such as shared_ptr of std::shared_ptr<Widget>,
// or an empty string for any other name
//
std::string standardSmartPointer(const xmlNode* name) {
    std::string text;
    appendWithoutWhitespace(text, name);
    const std::vector<std::string> parts = splitQualifiedName(withoutGenericArguments(text), "C++");
    const std::string& pointer = parts.back();
    const bool standard = parts.size() == 1 || (parts.size() == 2 && parts.front() == "std");
    return standard && (pointer == "unique_ptr" || pointer == "shared_ptr" || pointer == "weak_ptr") ? pointer : "";
}

// Gets the element of the type of the object that the name of a smart pointer points to, its first template argument,
// as const Engine of std::shared_ptr<const Engine>
//
xmlNode* smartPointerObject(const xmlNode* name) {
    xmlNode* argument = firstChildElement(firstChildElement(name, "argument_list"), "argument");
    return firstChildElement(argument, "expr") ? firstChildElement(argument, "expr") : argument;
}

namespace {
    // Tells whether a C++ type is a raw pointer, as Engine* is
    //
    bool isRawPointer(const xmlNode* type) {
        for (const xmlNode* modifier = firstChildElement(type, "modifier"); modifier; modifier = nextNamedSibling(modifier, "modifier"))
            if (sourceOf(modifier) == "*") return true;
        return false;
    }
}

// Gets the named type of the object that -> reaches through a C++ type that points to an object: a raw pointer, a std::unique_ptr, or a std::shared_ptr,
// as Engine of Engine* and of std::unique_ptr<const Engine>, or std::unique_ptr<Engine> of std::unique_ptr<Engine>*
// Any other type gives an empty string, since -> then calls the operator of the named type itself, as of an alias of a smart pointer
//
std::string pointeeTypeOf(const xmlNode* type) {
    const std::string named = namedTypeOf(type);
    if (named.empty() || isRawPointer(type)) return named;

    const xmlNode* name = firstChildElement(type, "name");
    const std::string pointer = standardSmartPointer(name);
    return pointer == "unique_ptr" || pointer == "shared_ptr" ? namedTypeOf(smartPointerObject(name)) : "";
}

// Gets the element whose end ends the scope of a declaration: the block, statement, handler, lambda, function, or type around it
// A parameter belongs to the lambda, function, declaration, or catch handler of its parameter list
// A variable of a for, while, if, switch, using, lock, or fixed statement or a resource of a try belongs to its statement
// A variable declared with a type that it defines, as in enum E { A } e; is a child of the type and belongs to the scope around the type
//
xmlNode* scopeOf(const xmlNode* declaration) {
    static const std::string_view scopes[] = {
        "block_content", "for", "foreach", "while", "if_stmt", "switch", "catch", "try", "using_stmt", "lock", "fixed",
        "lambda", "function", "class", "struct", "union", "interface", "enum"
    };
    xmlNode* node = declaration->parent;
    if (isSrcElement(node, "parameter") && isSrcElement(node->parent, "parameter_list") && node->parent->parent) return node->parent->parent;
    if (isTypeElement(node)) node = node->parent;

    for (; node && node->type == XML_ELEMENT_NODE && node->parent && node->parent->type == XML_ELEMENT_NODE; node = node->parent)
        for (std::string_view scope : scopes)
            if (isSrcElement(node, scope)) return node;
    return node;
}

namespace {
    // Gets the ternary that srcML makes of a C# ?. with an expression as its condition or its then, or null for an expression elsewhere
    // The ternary of a ?. has no else, while its condition holds the chain before ?. and its then holds the rest of the chain
    //
    const xmlNode* nullConditionalAccess(const xmlNode* expression) {
        const xmlNode* branch = expression->parent;
        if (!isSrcElement(branch, "condition") && !isSrcElement(branch, "then")) return nullptr;
        const xmlNode* ternary = branch->parent;
        return isSrcElement(ternary, "ternary") && !firstChildElement(ternary, "else") ? ternary : nullptr;
    }
}

// Gets the names of the C# null-conditional chain before an element after the . that opens a part of the chain, each with a dot after it, such as x.y.
// for Z() of x?.y?.Z() and x. for y.Z() of x?.y.Z(), or an empty string for a chain that starts at or passes through more than a name, as F()?.Z() does
//
std::string nullConditionalObject(const xmlNode* element) {
    std::string object = ".";
    for (const xmlNode* part = element->parent;;) {
        // The condition of a ?. that opens the then of another ?. continues the chain of the other, as .y of x?.y?.Z() continues x
        const xmlNode* access = nullConditionalAccess(part);
        while (access && isSrcElement(part->parent, "condition") && !xmlPreviousElementSibling(const_cast<xmlNode*>(access))) {
            part   = access->parent;
            access = nullConditionalAccess(part);
        }
        if (!access || !isSrcElement(part->parent, "then")) return "";

        const xmlNode* before = firstChildElement(firstChildElement(access, "condition"), "expr");
        const xmlNode* first  = xmlFirstElementChild(const_cast<xmlNode*>(before));
        const xmlNode* second = first ? xmlNextElementSibling(const_cast<xmlNode*>(first)) : nullptr;
        if (isSrcElement(first, "name") && !second) return sourceOf(first) + object;

        const bool continuesWithName = isSrcElement(first, "operator") && sourceOf(first) == "." && isSrcElement(second, "name") &&
                                       !xmlNextElementSibling(const_cast<xmlNode*>(second));
        if (!continuesWithName) return "";
        object = "." + sourceOf(second) + object;
        part   = before;
    }
}

// Gets the element that opens the then of a C# ?. before a name of the chain in the then, the . or the index after ?., as . of b in x?.b
// and [0] of b in x?[0].b, or null for a name outside such a chain
//
const xmlNode* nullConditionalStart(const xmlNode* name) {
    const xmlNode* start = name;
    for (const xmlNode* before = xmlPreviousElementSibling(const_cast<xmlNode*>(start));
         isSrcElement(before, "index") || (isSrcElement(before, "operator") && sourceOf(before) == ".");
         before = xmlPreviousElementSibling(const_cast<xmlNode*>(start)))
        start = before;

    const bool opensThen = start != name && !xmlPreviousElementSibling(const_cast<xmlNode*>(start)) && nullConditionalAccess(start->parent) &&
                           isSrcElement(start->parent->parent, "then");
    return opensThen ? start : nullptr;
}

// Gets the element that goes on with a C# null-conditional chain after the last element of the condition of a ?., the . or the index
// that opens its then, as . of .b in F()?.b, or null for an element elsewhere
//
const xmlNode* nullConditionalNext(const xmlNode* element) {
    const xmlNode* condition = element->parent;
    const xmlNode* access = nullConditionalAccess(condition);
    if (!access || !isSrcElement(condition->parent, "condition") || xmlNextElementSibling(const_cast<xmlNode*>(element))) return nullptr;

    // A ?. later in the chain, as the second of F()?.c?.b, makes the then a ?. whose condition starts with the rest of the chain
    const xmlNode* next = xmlFirstElementChild(firstChildElement(firstChildElement(access, "then"), "expr"));
    while (isSrcElement(next, "ternary")) {
        const xmlNode* innerCondition = firstChildElement(firstChildElement(next, "condition"), "expr");
        if (!innerCondition || !nullConditionalAccess(innerCondition)) break;
        next = xmlFirstElementChild(const_cast<xmlNode*>(innerCondition));
    }
    return next;
}

// Gets the element at the positions below an element, or null when the element has no such descendant
//
xmlNode* elementAt(xmlNode* ancestor, const elementPath& path) {
    xmlNode* node = ancestor;
    for (int position : path) {
        node = xmlFirstElementChild(node);
        for (int i = 1; node && i < position; ++i) node = xmlNextElementSibling(node);
        if (!node) return nullptr;
    }
    return node;
}

// Gets the position of an element in document order
//
int elementOrder::position(const xmlNode* element) {
    return range(element).first;
}

// Gets the position after the last descendant of an element
//
int elementOrder::end(const xmlNode* element) {
    return range(element).second;
}

// Gets the positions of an element, after numbering the elements of its document in a walk without recursion
//
const std::pair<int, int>& elementOrder::range(const xmlNode* element) {
    if (ranges.empty() && element && element->doc) {
        int next = 0;
        std::vector<const xmlNode*> path;
        for (const xmlNode* node = xmlDocGetRootElement(element->doc); node;) {
            ranges[node].first = next++;
            if (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(node))) {
                path.push_back(node);
                node = child;
                continue;
            }
            ranges[node].second = next;
            while (node && !xmlNextElementSibling(const_cast<xmlNode*>(node))) {
                if (path.empty()) {
                    node = nullptr;
                    break;
                }
                node = path.back();
                path.pop_back();
                ranges[node].second = next;
            }
            if (node) node = xmlNextElementSibling(const_cast<xmlNode*>(node));
        }
    }

    const auto found = ranges.find(element);
    if (found == ranges.end()) throw std::logic_error("a srcML element lies outside its document");
    return found->second;
}
