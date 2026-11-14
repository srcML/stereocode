// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file FunctionModel.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/function_model.hpp"
#include "helper_functions.hpp"
#include "model/interpolated_strings.hpp"
#include "model/javascript_elements.hpp"
#include "model/python_elements.hpp"
#include "rules/language_tables.hpp"
#include "rules/return_types.hpp"
#include "rules/specifier_pattern.hpp"
#include "model/qualified_name.hpp"
#include "archive/source_language.hpp"
#include "archive/srcml_document.hpp"
#include "archive/srcml_elements.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <limits>
#include <string_view>
#include <utility>

namespace {
    // Tells whether a C# parameter type carries the modifier out or ref, which passes the argument by reference
    // srcML puts the modifier of a qualified type name inside the name
    //
    bool hasReferenceModifier(const xmlNode* element) {
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(element)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
            if (isSrcElement(child, "modifier")) {
                const std::string modifier = sourceOf(child);
                if (modifier == "out" || modifier == "ref") return true;
            }
            else if (isSrcElement(child, "name") && hasReferenceModifier(child)) {
                return true;
            }
        }
        return false;
    }

    // Tells whether the parameters of a constructor copy an object of its own type
    // The first parameter takes the type itself, by value or by reference but not as a pointer, and every other parameter has a default value,
    //  such as A(const A&), A(A const&, int k = 0), or A(const A<T>&), while A(A* parent) and A(const std::vector<A>&) are no copy constructors
    //
    bool copiesOwnType(const xmlNode* parameterList, const std::string& typeName, const std::string& language) {
        const xmlNode* first = firstChildElement(parameterList, "parameter");
        if (typeName.empty() || !first) return false;

        for (const xmlNode* other = nextNamedSibling(first, "parameter"); other; other = nextNamedSibling(other, "parameter"))
            if (!firstChildElement(firstChildElement(other, "decl"), "init")) return false;

        const xmlNode* type = firstChildElement(firstChildElement(first, "decl"), "type");
        if (!type) return false;

        std::string outerType;
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(type)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
            if (isSrcElement(child, "modifier") && sourceOf(child) == "*") return false;
            if (isSrcElement(child, "name")) outerType = sourceOf(child);
        }
        HELPERS::removeWhitespace(outerType);
        outerType = outerType.substr(0, outerType.find('<'));
        HELPERS::removeNamespace(outerType, language, true);
        return outerType == typeName;
    }

    // Tells whether a C# accessor takes the new value of its property or event as the implicit parameter value, as set, init, add, and remove do
    //
    bool takesImplicitValue(const std::string& accessorName) {
        return accessorName == "set" || accessorName == "init" || accessorName == "add" || accessorName == "remove";
    }

    // Tells whether a C++ type names the qualifier const as a word, as const A and A const do, unlike const_iterator
    //
    bool namesConst(const std::string& type) {
        const std::string_view keyword = "const";
        const auto isNamePart = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };
        for (std::size_t start = type.find(keyword); start != std::string::npos; start = type.find(keyword, start + 1)) {
            const std::size_t end = start + keyword.size();
            if ((start == 0 || !isNamePart(type[start - 1])) && (end == type.size() || !isNamePart(type[end]))) return true;
        }
        return false;
    }

    // Tells whether a C++ type is a std::shared_ptr or a std::weak_ptr to an object that is not const, which the copies of the pointer share
    //
    bool pointsToSharedObject(const xmlNode* type) {
        const xmlNode* name = firstChildElement(type, "name");
        const std::string pointer = name ? standardSmartPointer(name) : "";
        if (pointer != "shared_ptr" && pointer != "weak_ptr") return false;

        const xmlNode* object = smartPointerObject(name);
        for (const xmlNode* specifier = firstChildElement(object, "specifier"); specifier; specifier = nextNamedSibling(specifier, "specifier"))
            if (sourceOf(specifier) == "const") return false;
        return true;
    }

    // Gets the type of a variable that an expression declares without the name of a type: var for a name in the parentheses of
    // var (low, high), and none for a range variable of a query, as item of from item in items, which is a parameter of a lambda
    //
    std::string inferredTypeOf(const xmlNode* variable) {
        const xmlNode* clause = variable->parent->parent;
        for (const char* element : {"from", "join", "let", "into"})
            if (isSrcElement(clause, element)) return "";
        return "var";
    }

    // Tells whether a Java parameter takes its arguments as an array, as int... values does
    //
    bool isVariadicArray(const xmlNode* type, const std::string& language) {
        if (language != "Java") return false;
        for (const xmlNode* modifier = firstChildElement(type, "modifier"); modifier; modifier = nextNamedSibling(modifier, "modifier"))
            if (sourceOf(modifier) == "...") return true;
        return false;
    }

    // Tells whether a parameter, declared as an array after its name or not, shares an object with the caller that a call may change: a C++ reference,
    // pointer, or array to an object that is not const, a C# ref or out parameter, or a C# or Java type that is not primitive, as int[] and List<int> are
    //
    bool sharesCallerObject(const xmlNode* type, bool declaredArray, const std::string& language, const primitives& primitiveTypes) {
        if (language == "C++") {
            // const before the first & or * qualifies the object, as in const A& or A const*, while A* const qualifies the pointer
            const std::string declaration = withoutGenericArguments(sourceOf(type));
            const std::size_t indirection = declaration.find_first_of("&*");
            return (indirection != std::string::npos || declaredArray) && !namesConst(declaration.substr(0, indirection));
        }
        // The modifier in passes a read-only reference, which leaves the object of a class shared, while a struct counts as a copy
        if (language == "C#" && hasReferenceModifier(type)) return true;

        const xmlNode* typeName = firstChildElement(type, "name");
        if (!typeName) return false;
        std::string name = sourceOf(typeName);
        HELPERS::removeWhitespace(name);
        if (declaredArray || name.find("[]") != std::string::npos) return true;

        name = withoutGenericArguments(name);
        HELPERS::removeNamespace(name, language, true);
        return !primitiveTypes.isPrimitive(name, language);
    }

    // Tells whether an element is an operator that accesses a member, as . and -> do
    //
    bool isMemberAccess(const xmlNode* node) {
        return isOperator(node, ".") || isOperator(node, "->");
    }

    // Gets the name that ends at an element or that the indexes up to the element follow, or null
    // The C# operator ! after a value that is not null sets a name apart from its indexes and from the rest of its chain, as in buffer![0] and node!.Name
    //
    const xmlNode* indexedName(const xmlNode* last) {
        const xmlNode* node = last;
        if (isOperator(node, "!")) node = xmlPreviousElementSibling(const_cast<xmlNode*>(node));
        while (isSrcElement(node, "index")) {
            node = xmlPreviousElementSibling(const_cast<xmlNode*>(node));
            if (isOperator(node, "!")) node = xmlPreviousElementSibling(const_cast<xmlNode*>(node));
        }
        return isSrcElement(node, "name") ? node : nullptr;
    }

    // Gets the name before an access operator, or null for another element and for a name after another value, as the result of a call
    //
    const xmlNode* nameBefore(const xmlNode* access) {
        return isMemberAccess(access) ? indexedName(xmlPreviousElementSibling(const_cast<xmlNode*>(access))) : nullptr;
    }

    // Gets the first and the last name of the access chain that holds a written name, such as m_subs[1] and second of m_subs[1].second
    // srcML ends a name after an index, so the names of a chain such as f[i].x are siblings joined by access operators
    //
    std::pair<const xmlNode*, const xmlNode*> accessChain(const xmlNode* name) {
        const xmlNode* first = name;
        const xmlNode* last  = name;
        for (const xmlNode* before = nameBefore(xmlPreviousElementSibling(const_cast<xmlNode*>(first))); before;
             before = nameBefore(xmlPreviousElementSibling(const_cast<xmlNode*>(first))))
            first = before;
        for (const xmlNode* op = xmlNextElementSibling(const_cast<xmlNode*>(last));
             isMemberAccess(op) && isSrcElement(xmlNextElementSibling(const_cast<xmlNode*>(op)), "name");
             op = xmlNextElementSibling(const_cast<xmlNode*>(last)))
            last = xmlNextElementSibling(const_cast<xmlNode*>(op));
        return {first, last};
    }

    // Gets the source code of the elements from the first to the last sibling of a chain, without the C# operator ! after a value
    //
    std::string chainSource(const xmlNode* first, const xmlNode* last) {
        std::string source = sourceOf(first);
        for (const xmlNode* node = first; node != last;) {
            node = xmlNextElementSibling(const_cast<xmlNode*>(node));
            if (!isOperator(node, "!")) source += sourceOf(node);
        }
        return source;
    }

    // Tells whether an expression writes the target of a pointer, as *p = v does, given the first and the last name of the written chain
    // In *s++, the increment applies to the pointer
    //
    bool isDereferenced(const xmlNode* first, const xmlNode* last) {
        xmlNode* before = xmlPreviousElementSibling(const_cast<xmlNode*>(first));
        xmlNode* after  = xmlNextElementSibling(const_cast<xmlNode*>(last));
        if (!isOperator(before, "*") || isOperator(after, "++") || isOperator(after, "--")) return false;

        // A unary * starts the expression or follows another operator
        xmlNode* operand = xmlPreviousElementSibling(before);
        return !operand || isSrcElement(operand, "operator");
    }

    // Tells whether an operator writes its operand, as =, +=, ++, and ??= do, unlike == or <=
    // srcML marks the ??= of C# as the operator ?? before the operator =
    //
    bool isAssignment(const xmlNode* node) {
        if (!isSrcElement(node, "operator")) return false;
        const std::string op = sourceOf(node);
        if (op == "??") return isOperator(xmlNextElementSibling(const_cast<xmlNode*>(node)), "=");
        return op == "++" || op == "--" || (!op.empty() && op.back() == '=' && op != "==" && op != "!=" && op != "<=" && op != ">=");
    }

    // Tells whether an expression writes through the result of a call, as v.at(i) = x, v.data()[0] = x, v.front().y = x, ++v.back(), and v.Front()?.y = x do
    //
    bool writesResult(const xmlNode* call) {
        const xmlNode* before = xmlPreviousElementSibling(const_cast<xmlNode*>(call));
        if (isOperator(before, "++") || isOperator(before, "--")) return true;

        // Indexes and members of the result, such as [0] or .y, lead to the place that the expression writes, also after a C# ?.
        for (const xmlNode* last = call;;) {
            const xmlNode* after = xmlNextElementSibling(const_cast<xmlNode*>(last));
            if (!after) after = nullConditionalNext(last);
            if (!isSrcElement(after, "index") && !isSrcElement(after, "name") && !isOperator(after, ".") && !isOperator(after, "->") &&
                !(isOperator(after, "!") && !isAssignment(xmlNextElementSibling(const_cast<xmlNode*>(after))))) return isAssignment(after);
            last = after;
        }
    }

    // Tells whether the first name of an access chain goes on from the result of a call through members and indexes, as in f().a[i].b and f()[i].b
    //
    bool continuesCallResult(const xmlNode* first) {
        const xmlNode* before = xmlPreviousElementSibling(const_cast<xmlNode*>(first));
        if (!isMemberAccess(before)) return false;

        const xmlNode* result = xmlPreviousElementSibling(const_cast<xmlNode*>(before));
        while (isSrcElement(result, "index") || isOperator(result, "!")) result = xmlPreviousElementSibling(const_cast<xmlNode*>(result));
        return isSrcElement(result, "call");
    }

    // Tells whether a name is the name of an operator function, as operator() and operator<< are
    //
    bool namesOperatorFunction(std::string_view name) {
        constexpr std::string_view keyword = "operator";
        return name.size() > keyword.size() && name.substr(0, keyword.size()) == keyword &&
               !std::isalnum(static_cast<unsigned char>(name[keyword.size()])) && name[keyword.size()] != '_';
    }

    // Gets the operator through which a call names its method directly on a data member, . of m_engine.rpm, this->m_engine.rpm, and this.engine.rpm
    // or -> of m_spare->rpm, or an empty string for a call through another member or an element, as a.b.c and a[i].c are
    //
    std::string_view memberCallOperator(const std::string& callName, const std::string& member) {
        std::string_view name = callName;
        for (std::string_view self : {"this.", "this->", "(*this)."}) {
            if (name.substr(0, self.size()) != self) continue;
            name.remove_prefix(self.size());
            break;
        }
        if (name.substr(0, member.size()) != member) return {};
        name.remove_prefix(member.size());

        for (std::string_view access : {".", "->"}) {
            if (name.substr(0, access.size()) != access) continue;
            const std::string_view method = name.substr(access.size());
            if (namesOperatorFunction(method)) return access;
            return !method.empty() && method.find_first_of(".-:[(") == std::string_view::npos ? access : std::string_view{};
        }
        return {};
    }

    // Tells whether a Java call names a method of the object itself through this or super alone, as this.size() and super.size() do,
    // where a field of the same name plays no part, since Java keeps the names of fields and methods apart
    //
    bool callsMethodThroughSelf(const std::string& callName) {
        for (std::string_view self : {"this.", "super."})
            if (callName.compare(0, self.size(), self) == 0) return callName.find('.', self.size()) == std::string::npos;
        return false;
    }

    // Tells whether a Java call names a method through this or super after the name of a type: a method of an enclosing instance of an
    // inner class, as Ledger.this.add() does, or a default method of an interface, as Greeter.super.greet() does
    //
    bool callsMethodThroughQualifiedSelf(const std::string& callName) {
        for (std::string_view self : {".this.", ".super."}) {
            const std::size_t typeEnd = callName.find(self);
            if (typeEnd == std::string::npos || typeEnd == 0 || callName.find('.', typeEnd + self.size()) != std::string::npos) continue;
            return std::all_of(callName.begin(), callName.begin() + static_cast<std::ptrdiff_t>(typeEnd),
                               [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '$' || c == '.'; });
        }
        return false;
    }

    // Gets the position of the first element after a node in document order, where the node may be a text node between elements
    //
    int positionAfter(const xmlNode* node, elementOrder& order) {
        if (node->type == XML_ELEMENT_NODE) return order.end(node);
        if (const xmlNode* next = xmlNextElementSibling(const_cast<xmlNode*>(node))) return order.position(next);
        return order.end(node->parent);
    }

    // Tells whether a call goes through a pointer to its object, as w->reset() does for the variable w
    //
    bool callsThroughPointer(const std::string& callName, const std::string& variable) {
        return callName.compare(0, variable.size() + 2, variable + "->") == 0;
    }

    // Adds a call under the name that tells its receiver, with the number of its arguments, where the signature shows the name that the code writes
    //
    void addNamedCall(const xmlNode* callElement, const xmlNode* nameElement, const xmlNode* argumentList, std::string callName, std::string writtenName,
                      const std::string& unitLanguage, elementOrder& order, std::vector<callModel>& collected) {
        callModel call;
        HELPERS::nameFilter(callName, call.getNameVector(), unitLanguage, false);
        std::vector<std::string> written;
        HELPERS::nameFilter(writtenName, written, unitLanguage, false);

        const int arguments = countListItems(argumentList);
        call.setSignature(written[3] + HELPERS::signatureSlots(arguments));
        call.setArgumentCount(arguments);
        call.setPosition(order.position(nameElement));
        call.setResultWritten(writesResult(callElement));
        collected.push_back(std::move(call));
    }

    // Gets the names before a call that the C# operator ! sets apart from the call, with the access operator before the call and without
    // whitespace, as writer. of writer!.Flush() and nodes[0]. of nodes![0]!.Clear(), an empty text for such a call on another value, as
    // Open()!.Flush(), or nothing for a call without the operator before it
    //
    std::optional<std::string> nullForgivenObject(const xmlNode* callElement) {
        const xmlNode* access = xmlPreviousElementSibling(const_cast<xmlNode*>(callElement));
        if (!isMemberAccess(access)) return std::nullopt;

        bool forgiven = false;
        const xmlNode* first = nullptr;
        for (const xmlNode* link = access; isMemberAccess(link); link = xmlPreviousElementSibling(const_cast<xmlNode*>(first))) {
            for (const xmlNode* node = xmlPreviousElementSibling(const_cast<xmlNode*>(link)); isOperator(node, "!") || isSrcElement(node, "index");
                 node = xmlPreviousElementSibling(const_cast<xmlNode*>(node)))
                forgiven = forgiven || isOperator(node, "!");
            first = nameBefore(link);
            if (!first) return forgiven ? std::optional<std::string>("") : std::nullopt;
        }
        if (!forgiven) return std::nullopt;

        std::string object = chainSource(first, access);
        HELPERS::removeWhitespace(object);
        return object;
    }

    // Gets the type of the value that an async function returns through a result type of its language, as bool of Task<bool> and void
    // of Task, or nothing for another type
    //
    std::optional<std::string> awaitedType(std::string written, const std::vector<std::string_view>& results, const std::string& unitLanguage) {
        HELPERS::removeWhitespace(written);
        const std::string last = splitQualifiedName(written, unitLanguage).back();
        if (std::find(results.begin(), results.end(), withoutGenericArguments(last)) == results.end()) return std::nullopt;

        const int arity = genericArity(last);
        if (arity == 0) return "void";
        if (arity > 1) return std::nullopt;
        const std::string arguments = genericArguments(last);
        return arguments.substr(1, arguments.size() - 2);
    }

    // Gets the variable of a call on an element, with its indexes and the access operator before the call and without whitespace, as
    // slots[i]. of slots[i].clear(), or nothing for a call on the result of a call or on another value
    // srcML ends a name at its index, so the call after the access operator stands apart from the variable
    //
    std::string elementObjectOf(const xmlNode* callElement) {
        const xmlNode* access   = xmlPreviousElementSibling(const_cast<xmlNode*>(callElement));
        const xmlNode* variable = isOperator(access, ".") || isOperator(access, "->") ? xmlPreviousElementSibling(const_cast<xmlNode*>(access)) : nullptr;
        if (!isSrcElement(variable, "name") || !firstChildElement(variable, "index")) return "";

        const xmlNode* before = xmlPreviousElementSibling(const_cast<xmlNode*>(variable));
        if (isOperator(before, ".") || isOperator(before, "->")) return "";

        std::string object = sourceOf(variable) + sourceOf(access);
        HELPERS::removeWhitespace(object);
        return object;
    }

    // Gets the number of accesses by an index or by -> through which a call names its method on an element of a data member, as 2 of
    // grid[i][j].clear and of table[i]->clear, or 0 for a call that names its method otherwise
    //
    int elementAccesses(const std::string& callName, const std::string& member) {
        std::string_view name = callName;
        for (std::string_view self : {"this.", "this->", "(*this)."}) {
            if (name.substr(0, self.size()) != self) continue;
            name.remove_prefix(self.size());
            break;
        }
        if (name.substr(0, member.size()) != member) return 0;
        name.remove_prefix(member.size());

        int accesses = 0;
        while (!name.empty() && name.front() == '[') {
            std::size_t end = 0;
            for (int open = 0; end < name.size(); ++end) {
                if (name[end] == '[') ++open;
                else if (name[end] == ']' && --open == 0) break;
            }
            if (end == name.size()) return 0;
            name.remove_prefix(end + 1);
            ++accesses;
        }
        if (accesses == 0) return 0;

        const bool arrow = name.substr(0, 2) == "->";
        if (!arrow && name.substr(0, 1) != ".") return 0;
        const std::string_view method = name.substr(arrow ? 2 : 1);
        const bool named = !method.empty() && (namesOperatorFunction(method) || method.find_first_of(".-:[(") == std::string_view::npos);
        return named ? accesses + (arrow ? 1 : 0) : 0;
    }

    // Gets the local or anonymous type around an element of the body of a function, whose code runs on an object of that type, or null
    //
    const xmlNode* nestedTypeAround(const xmlNode* element) {
        for (const xmlNode* node = element->parent; node; node = node->parent)
            if (isTypeElement(node)) return node;
        return nullptr;
    }

    // Tells whether the body of a type declares a method or a data member of a name, where a C++ body holds its members in access sections
    //
    bool declaresMember(const xmlNode* type, const std::string& name, bool method) {
        const auto declares = [&](const xmlNode* member) {
            if (method) {
                const xmlNode* declared = isSrcElement(member, "function") || isSrcElement(member, "function_decl") ? firstChildElement(member, "name") : nullptr;
                return declared && sourceOf(declared) == name;
            }
            if (!isSrcElement(member, "decl_stmt")) return false;
            for (const xmlNode* declaration = firstChildElement(member, "decl"); declaration; declaration = nextNamedSibling(declaration, "decl")) {
                const xmlNode* declared = firstChildElement(declaration, "name");
                std::string declaredName = declared ? sourceOf(declared) : "";
                HELPERS::removeBracketSuffix(declaredName);
                if (declaredName == name) return true;
            }
            return false;
        };
        const xmlNode* body = firstChildElement(type, "block");
        for (const xmlNode* member = body ? xmlFirstElementChild(const_cast<xmlNode*>(body)) : nullptr; member; member = xmlNextElementSibling(const_cast<xmlNode*>(member))) {
            if (declares(member)) return true;
            if (!isSrcElement(member, "public") && !isSrcElement(member, "private") && !isSrcElement(member, "protected")) continue;
            for (const xmlNode* inSection = xmlFirstElementChild(const_cast<xmlNode*>(member)); inSection; inSection = xmlNextElementSibling(const_cast<xmlNode*>(inSection)))
                if (declares(inSection)) return true;
        }
        return false;
    }

    // Gets the length of the access to the object itself that starts a name, as of this. in this.count, or 0
    //
    std::size_t selfAccessLength(const std::string& name) {
        for (std::string_view self : {"this.", "super.", "base.", "this->"})
            if (name.compare(0, self.size(), self) == 0) return self.size();
        return 0;
    }

    // Gets a name in the body of a function as the function reads it, or nothing for a name that the function cannot reach
    // Inside a local or anonymous type, the object itself is the object of that type, whose declared data members are locals of the function
    //
    std::optional<std::string> nameOutsideNestedObject(const xmlNode* nameElement, std::string name) {
        const xmlNode* nested = nestedTypeAround(nameElement);
        if (!nested) return name;

        std::string written = name;
        HELPERS::removeWhitespace(written);
        if (written == "this" || written == "super" || written == "base") return std::nullopt;
        const std::size_t self = selfAccessLength(written);
        if (self == 0) return name;

        const std::string member = written.substr(self, written.find_first_of(".-[", self) - self);
        if (!declaresMember(nested, member, false)) return std::nullopt;
        return written.substr(self);
    }

    // Tells whether a call in a local or anonymous type calls a method on the object of that type: through the object itself, as
    // super.run() does, or without an object under the name of a method that the type defines
    //
    bool callsNestedObject(const xmlNode* callElement) {
        const xmlNode* nested = nestedTypeAround(callElement);
        const xmlNode* nameElement = nested ? firstChildElement(callElement, "name") : nullptr;
        if (!nameElement) return false;

        std::string name = sourceOf(nameElement);
        HELPERS::removeWhitespace(name);
        return selfAccessLength(name) > 0 || declaresMember(nested, name, true);
    }

    // Gets the object that srcML marks apart from a Java call with explicit type arguments, with its last access operator, as Flowable. of
    // Flowable.<T>error(e), an empty text where the type arguments follow the result of a call, as in a.b().<T>c(), or nothing for another call
    //
    std::optional<std::string> objectBeforeTypeArguments(const xmlNode* callElement) {
        const xmlNode* node = xmlPreviousElementSibling(const_cast<xmlNode*>(callElement));
        if (!isOperator(node, ">")) return std::nullopt;

        for (node = xmlPreviousElementSibling(const_cast<xmlNode*>(node)); node; node = xmlPreviousElementSibling(const_cast<xmlNode*>(node))) {
            if (isOperator(node, "<")) return isOperator(xmlPreviousElementSibling(const_cast<xmlNode*>(node)), ".") ? std::optional<std::string>("") : std::nullopt;
            if (!isSrcElement(node, "name")) return std::nullopt;
            if (isOperator(xmlLastElementChild(const_cast<xmlNode*>(node)), ".")) return sourceOf(node);
        }
        return std::nullopt;
    }

    // Adds a call with its name, after the object that srcML marks apart from the call, if any, and the number of its arguments
    // A call that srcML marks as a declaration, as the statement operator=(other); is, has its arguments in a parameter list
    //
    void addCall(const xmlNode* callElement, const std::string& object, const std::string& unitLanguage, elementOrder& order, std::vector<callModel>& collected) {
        // The creation of an anonymous class names the parent of the class before the arguments of the constructor
        if (isSrcElement(callElement, "class")) {
            const xmlNode* parent       = firstChildElement(callElement, "super");
            const xmlNode* name         = parent ? firstChildElement(parent, "name") : nullptr;
            const xmlNode* argumentList = parent ? xmlNextElementSibling(const_cast<xmlNode*>(parent)) : nullptr;
            if (name && isSrcElement(argumentList, "argument_list"))
                addNamedCall(callElement, name, argumentList, sourceOf(name), sourceOf(name), unitLanguage, order, collected);
            return;
        }

        // A creation that srcML marks without a call, as new Matrix or new(count), names the type after new, if any, and counts no arguments,
        // since no rule reads the arguments of a constructor call
        if (isSrcElement(callElement, "operator")) {
            const xmlNode* created = xmlNextElementSibling(const_cast<xmlNode*>(callElement));
            const std::string type = isSrcElement(created, "name") ? sourceOf(created) : "";
            addNamedCall(callElement, callElement, nullptr, type, type, unitLanguage, order, collected);
            return;
        }

        const char* const arguments = isSrcElement(callElement, "function_decl") ? "parameter_list" : "argument_list";
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(callElement)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
            const xmlNode* argumentList = xmlNextElementSibling(const_cast<xmlNode*>(child));
            if (!isSrcElement(child, "name") || !isSrcElement(argumentList, arguments)) continue;

            const std::string callName = unitLanguage == "JavaScript" ? withoutOptionalChaining(object + sourceOf(child)) : object + sourceOf(child);
            addNamedCall(callElement, child, argumentList, callName, callName, unitLanguage, order, collected);
        }
    }

    // Tells whether a text is a name of letters, digits, and underscores
    //
    bool isPlainName(const std::string& text) {
        return !text.empty() && std::all_of(text.begin(), text.end(), [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; });
    }

    // Tells whether a type holds the word signed or unsigned
    //
    bool namesSign(const std::string& type) {
        for (std::size_t start = type.find("signed"); start != std::string::npos; start = type.find("signed", start + 1)) {
            const std::size_t wordStart = start >= 2 && type.compare(start - 2, 2, "un") == 0 ? start - 2 : start;
            const std::size_t wordEnd   = start + 6;
            const auto isWordCharacter = [&type](std::size_t at) { return std::isalnum(static_cast<unsigned char>(type[at])) || type[at] == '_'; };
            if ((wordStart == 0 || !isWordCharacter(wordStart - 1)) && (wordEnd == type.size() || !isWordCharacter(wordEnd))) return true;
        }
        return false;
    }

    // Tells whether an element is a C++ shift operator
    //
    bool isShiftOperator(const xmlNode* node) {
        return isOperator(node, "<<") || isOperator(node, ">>");
    }

    // Makes the call of the operator function that an operator with one right operand stands for, as out.operator<< for << of out << value
    //
    callModel operatorCall(std::string name, int position, const std::string& unitLanguage) {
        callModel call;
        call.setSignature(name + HELPERS::signatureSlots(1));
        HELPERS::nameFilter(name, call.getNameVector(), unitLanguage, false);
        call.setArgumentCount(1);
        call.setPosition(position);
        return call;
    }

    // Tells whether the name of a type names a stream, as ostream, wistringstream, and QTextStream do
    //
    bool namesStream(const std::string& typeName) {
        constexpr std::string_view suffix = "stream";
        return typeName.size() >= suffix.size() &&
               std::equal(suffix.rbegin(), suffix.rend(), typeName.rbegin(), [](char expected, char c) { return std::tolower(static_cast<unsigned char>(c)) == expected; });
    }

    // Gets the last name of a name with qualifiers, such as Shape of shapes.Shape
    //
    std::string lastNameOf(const std::string& qualifiedName) {
        return qualifiedName.substr(qualifiedName.rfind('.') + 1);
    }
}

// Collects the facts of a method or a free function, given the type and the name of the property that holds a C# accessor
//
functionModel::functionModel(const languageTables& tables_, const srcmlDocument& method, const std::optional<elementLocation>& location_, const std::string& unitLanguage_,
                             const std::string& typeNameParsed_, const std::string& propertyType, const std::string& propertyName_, int lineNumber_, bool isProperty_) :
                             tables{&tables_}, unitLanguage{unitLanguage_}, propertyName{propertyName_}, location{location_}, typeNameParsed{typeNameParsed_},
                             ownTypeName{typeNameParsed_}, isProperty{isProperty_}, lineNumber{lineNumber_} {
    fileName = method.fileName(); // Get file name here since free functions do not have a type
    HELPERS::removeNamespace(typeNameParsed, unitLanguage, true);
    dynamic       = languageOf(unitLanguage).dynamic;
    moduleGlobals = languageOf(unitLanguage).modules;
    
    findName(method);
    if (name.empty()) return; // If there is no name, then it is not a valid method and we can skip the rest of the data collection

    if (unitLanguage == "Python") findBoundParameter(method);
    // this of a JavaScript static method names the class, as the parameter cls of a Python class method does
    else if (unitLanguage == "JavaScript" && !typeNameParsed.empty() && isStaticMethod(method.unitElement())) classParameter = "this";
    findParameterList(method);
    findConstructorOrDestructorType(method);

    // The positions of the elements tell which locals are in scope at each name
    elementOrder order;

    // The methods of a dynamic language declare a data member where they write it through the object, which a constructor mostly does
    if (dynamic) {
        findAttributesOrAnnotations(method);
        findPropertyName(method);
        findParameters(method, propertyType);
        findExpressionAssignments(method, order);
    }

    if (constructorOrDestructor.empty()) {
        if (unitLanguage == "C#" || unitLanguage == "Java") {
            findAttributesOrAnnotations(method);
        }
        if (unitLanguage == "C++") findConst(method);
        findSpecifiers(method);
        findReturnType(method, propertyType);
        if (!dynamic) findParameters(method, propertyType);

        findLocalVariables(method, order);
        findLocalFunctions(method);
        findNonPrimitive();
        findReturnExpression(method, order);
        findCalls(method, order);
        nameSignature = std::make_pair(name[3], signatureTypes);
        findIgnorableCalls(methodCalls);
        findIgnorableCalls(functionCalls);
        findIgnorableCalls(newConstructorCalls);
        findIgnorableCalls(callsOnResults);
        findNewAssignedVariables(method);
        findExpressionNames(method, order);
        if (!dynamic) findExpressionAssignments(method, order);
        findNonCommentStatements(method);
        if (unitLanguage == "JavaScript") inferReturnType();
    }
}

// Finds the first parameter of a Python method, which names the object as this does, or of a class method, which names the class
// Only a function in the body of a class has such a parameter
//
void functionModel::findBoundParameter(const srcmlDocument& method) {
    if (typeNameParsed.empty()) return;

    const nodeSet object = method.select(query::objectParameter);
    if (object.size() > 0) objectName = object.src(0);
    const nodeSet type = method.select(query::classParameter);
    if (type.size() > 0) classParameter = type.src(0);
}

// Finds the property that a Python method defines, by a decorator such as @property, @functools.cached_property, or @area.setter, or that
// a JavaScript accessor defines, as size of get size()
// The name of the property is a data member for the other methods, as a C# property is
//
void functionModel::findPropertyName(const srcmlDocument& method) {
    if (unitLanguage == "JavaScript") {
        if (isPropertyAccessor(method.unitElement())) propertyName = name[3];
        return;
    }
    if (objectName.empty()) return;

    for (std::string decorator : attributesOrAnnotations) {
        HELPERS::removeWhitespace(decorator);
        decorator = decorator.substr(0, decorator.find('('));
        const std::size_t qualifierEnd = decorator.rfind('.');
        const std::string last = decorator.substr(qualifierEnd == std::string::npos ? 1 : qualifierEnd + 1);

        const bool defines  = last == "property" || last == "cached_property" || last == "abstractproperty";
        const bool accessor = qualifierEnd != std::string::npos && (last == "setter" || last == "getter" || last == "deleter");
        if (defines || accessor) propertyName = name[3];
    }
}

// Gets a Python name or expression with this in place of the name of the object, and with the name of the class in place of the parameter
// that names the class in a class method and of the class of the object, such as this.total of self.total, Shape.count of cls.count,
// and Shape.count of self.__class__.count, as the other languages write them
//
std::string functionModel::withObjectNames(std::string text) const {
    if (objectName.empty() && classParameter.empty()) return text;

    const std::size_t start = text.find_first_not_of("( \t\r\n");
    if (start == std::string::npos) return text;

    const auto isNamePart = [&text](std::size_t position) {
        return position < text.size() && (std::isalnum(static_cast<unsigned char>(text[position])) || text[position] == '_');
    };
    const std::pair<std::string, std::string> replacements[] = {
        {objectName.empty() ? "" : objectName + ".__class__", typeNameParsed}, {objectName, "this"}, {classParameter, typeNameParsed}
    };
    for (const auto& [written, replacement] : replacements) {
        if (written.empty() || text.compare(start, written.size(), written) != 0 || isNamePart(start + written.size())) continue;
        return text.replace(start, written.size(), replacement);
    }
    return text;
}

// Gets a Python type with the name of the type of the method in place of Self
//
std::string functionModel::withOwnType(std::string type) const {
    constexpr std::string_view self = "Self";
    const auto isNamePart = [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; };
    for (std::size_t found = type.find(self); !typeNameParsed.empty() && found != std::string::npos; found = type.find(self, found + 1)) {
        const std::size_t end = found + self.size();
        if ((found > 0 && (isNamePart(type[found - 1]) || type[found - 1] == '.')) || (end < type.size() && isNamePart(type[end]))) continue;
        type.replace(found, self.size(), typeNameParsed);
    }
    return type;
}

// Records what an assignment of a Python function tells about its target, given the target as the model writes it, such as this.engine
// A call may create the object of the target, and a write of a member of the object declares the member, with the type of its annotation
// or of the parameter that it takes
//
void functionModel::noteBinding(const boundValue& value, const std::string& chain) {
    std::string target = chain;
    HELPERS::removeWhitespace(target);
    const std::string callName = withObjectNames(value.callName);
    if (!callName.empty()) assignedCalls.emplace_back(target, callName);

    constexpr std::string_view object = "this.";
    if (target.compare(0, object.size(), object) != 0 || !isPlainName(target.substr(object.size()))) return;

    memberWrite write{target.substr(object.size()), withOwnType(value.type), callName};
    const auto parameter = parameters.find(value.name);
    if (write.type.empty() && parameter != parameters.end()) write.type = parameter->second.getType();
    writtenMembers.push_back(std::move(write));
}

// Gets the class that a call of a Python function creates an object of, which is unknown for the class of another object, or nothing for
// another call, given the names that tell a class from a function, a method, and a variable
// A call creates an object where its name, alone or after the names of modules, is a class of the archive, or where it names the class
// of an object, as self.__class__(...)
// A name that the archive does not define creates an object where it follows the CapWords convention of class names, as OrderedDict
// A name that is called on a data member, a local, a parameter, or a global of the file names a method of another object, as Parse of
// self.parser.Parse(data) and SetFocus of user32.SetFocus(window), where user32 = ctypes.windll.user32 binds the global
// An element of a variable is an object as well, whose class no name tells, as in handlers[kind](event) and rows[0].Strip()
// A name that is called on the object names a method, which a parent outside the archive may define under any form of name, as Show of
// a wxPython window, unless the name is no method of the type and is a class of the archive or a variable of the type that follows
// the convention, as RequestHandlerClass of a server
//
std::optional<std::string> functionModel::createdClass(const std::string& callName, const creationNames& names) const {
    if (callName.empty()) return std::nullopt;
    const std::string last = lastNameOf(callName);
    if (last == "__class__") return callName == "this.__class__" ? typeNameParsed : std::string{};
    if (callName.find('[') != std::string::npos) return std::nullopt;

    const bool archiveClass = names.archive.classes.count(last) > 0;
    if (callsMethodThroughSelf(callName)) {
        const bool holdsClass = archiveClass || (names.variables.count(last) > 0 && namesPythonClass(last));
        return holdsClass && names.methods.count(last) == 0 ? std::optional{last} : std::nullopt;
    }

    const std::size_t firstEnd = callName.find('.');
    const std::string first    = callName.substr(0, firstEnd);
    const bool        onObject = firstEnd != std::string::npos &&
                                 (first == "this" || first == "super" || locals.count(first) > 0 || parameters.count(first) > 0 ||
                                  names.globals.declaredAtNamespaceScope(unitNumber(), first));
    if (onObject) return std::nullopt;
    return archiveClass || (names.archive.functions.count(last) == 0 && namesPythonClass(last)) ? std::optional{last} : std::nullopt;
}

// Finds the calls of a Python function that create an object, given the names that tell a class from a function, a method, and a variable
// The calls give the variables that hold a created object, the types of the locals that they create, and the return type of a function
// without an annotation: None without a returned value, bool where every returned value is Boolean by its form, the class of a returned
// object that the function creates, and otherwise Any
//
void functionModel::findCreatedObjects(const creationNames& names) {
    if (!constructorOrDestructor.empty()) return;
    const auto createdClass = [this, &names](const std::string& callName) { return this->createdClass(callName, names); };

    for (std::vector<callModel>* calls : {&functionCalls, &methodCalls}) {
        const auto creates = [&createdClass](const callModel& call) { return createdClass(call.getName()[3]).has_value(); };
        const auto kept = std::stable_partition(calls->begin(), calls->end(), [&creates](const callModel& call) { return !creates(call); });
        newConstructorCalls.insert(newConstructorCalls.end(), std::make_move_iterator(kept), std::make_move_iterator(calls->end()));
        calls->erase(kept, calls->end());
    }

    std::unordered_map<std::string, std::string> createdVariables;
    for (const auto& [variable, callName] : assignedCalls) {
        const std::optional<std::string> created = createdClass(callName);
        if (!created) continue;

        // A return of a data member names it without the object, as cache for this.cache = Cache()
        variablesCreatedWithNew.insert(withoutSelfAccess(variable));
        createdVariables.insert({withoutSelfAccess(variable), *created});
        const auto local = locals.find(variable);
        if (local != locals.end() && local->second.variable.getType().empty()) local->second.variable.setType(*created);
    }

    std::string returnedClass;
    bool        valueReturned = false;
    bool        allBoolean    = true;
    for (std::size_t i = 0; i < returnedValues.size(); ++i) {
        const returnedValue& value = returnedValues[i];
        if (value.none) continue;
        valueReturned = true;
        allBoolean    = allBoolean && value.boolean;

        std::string variable = returnExpressions[i].text;
        HELPERS::removeWhitespace(variable);
        const std::optional<std::string> created = createdClass(value.callName);
        const auto createdVariable = createdVariables.find(withoutSelfAccess(variable));
        if (created) newReturned = true;
        if (returnedClass.empty()) returnedClass = created ? *created : createdVariable != createdVariables.end() ? createdVariable->second : "";
    }
    if (!returnTypeDeclared) returnType.setType(!valueReturned ? "None" : allBoolean ? "bool" : !returnedClass.empty() ? returnedClass : "Any");

    findNonPrimitive();
}

// Infers the return type of a JavaScript function without an annotation: void without a returned value, boolean where every returned value
// is Boolean by its form, the class of a returned object that the function creates, by new or into a local, and otherwise any
//
void functionModel::inferReturnType() {
    if (returnTypeDeclared) return;

    std::string returnedClass;
    bool        valueReturned = false;
    bool        allBoolean    = true;
    for (std::size_t i = 0; i < returnedValues.size(); ++i) {
        const returnedValue& value = returnedValues[i];
        if (value.none) continue;
        valueReturned = true;
        allBoolean    = allBoolean && value.boolean;

        std::string variable = returnExpressions[i].text;
        HELPERS::removeWhitespace(variable);
        const scopedLocal* local = variablesCreatedWithNew.count(variable) > 0 ? localAt(variable, returnExpressions[i].position, false) : nullptr;
        if (returnedClass.empty()) returnedClass = !value.createdClass.empty() ? value.createdClass : local ? local->variable.getType() : "";
    }
    returnType.setType(!valueReturned ? "void" : allBoolean ? "boolean" : !returnedClass.empty() ? returnedClass : "any");
    findNonPrimitiveReturnType();
}

// Finds all specifiers of the method
//
void functionModel::findSpecifiers(const srcmlDocument& method) {
    const nodeSet result = method.select(query::methodSpecifiers);
    for (int i = 0; i < result.size(); ++i) {
        std::string specifier = result.src(i);
        if (std::find(specifiers.begin(), specifiers.end(), specifier) == specifiers.end()) specifiers.push_back(std::move(specifier));
    }
}

// Finds data after all type information and the globals of the archive are collected
//
void functionModel::findDataAfterCollection(const typeMembers& type, const globalVariables& globals, const callableNames& archive) {
    if (constructorOrDestructor.empty()) {
        addStaticHolders(type.staticHolders);
        unknownParent  = type.unknownParent;
        ownType        = &type;

        // Must only be called after findIgnorableCalls(). Will Filter the calls.
        findStreamCalls(type.fields, globals, archive);
        findCalledVariables(type.fields, globals, type.methodNames, archive);
        findCallsOnFields(type, globals);
        noteClassHolders(type.fields, globals);

        // Must only be called after findNewAssignedVariables()
        findReturnedVariables(type.fields, globals, false);
        findVariablesInExpressions(type.fields, globals);
        findModifiedVariables(type.fields, type, globals, false);
        ownType = nullptr;
    }
}

// Finds data for free functions, given the globals, the names of the C# structs and enums of the archive, and the names of its classes
//
void functionModel::findDataFreeFunctionAfterCollection(const globalVariables& globals, const std::unordered_set<std::string>& valueTypes,
                                                        const callableNames& archive) {
    if (constructorOrDestructor.empty()) {
        // A C# struct or enum of the archive passed by value is a copy of the value of the caller
        if (unitLanguage == "C#") {
            for (const auto& [parameterName, parameter] : parameters) {
                std::string typeName = withoutGenericArguments(parameter.getNamedType());
                HELPERS::removeNamespace(typeName, unitLanguage, true);
                if (referenceParameters.count(parameterName) == 0 && valueTypes.count(typeName) > 0) sharedParameters.erase(parameterName);
            }
        }

        findStreamCalls(parameters, globals, archive);
        findCalledVariables(parameters, globals, {}, archive);

        // For free functions, we do not need to filter the calls like in findCallsOnFields()
        //  since all of the calls are external anyway
        const auto noteCallOnVariable = [&](const callModel& call) {
            const nameResolution receiver = resolveName(call.getName()[3], unitLanguage, typeNameParsed, false, scopesAt(call.getPosition(), parameters, globals));
            noteUse(receiver, call.getPosition(), parameters);
            if (tables->fieldQueries.isQuery(call.getName()[3], call.getArgumentCount(), unitLanguage) && !call.isResultWritten()) return;

            // A call other than a query method changes a parameter that shares the object of the caller, a global that is not immutable, or a static local
            const bool changesParameter = sharedParameters.count(receiver.variable) > 0 ||
                                          (sharedPointees.count(receiver.variable) > 0 && callsThroughPointer(call.getName()[1], receiver.variable));
            const bool changesGlobal    = receiver.kind == variableKind::global && !globals.isImmutable(unitLanguage, receiver.variable, unitNumber());
            if (receiver.kind == variableKind::parameter && changesParameter) parameterRefModified = true;
            else if (changesGlobal || isStaticLocal(receiver, call.getPosition())) globalOrStaticVariableModified = true;
        };
        for (const callModel& call : methodCalls) noteCallOnVariable(call);

        // A call on an element of a variable is a call on the variable, as doors[0].close() is, as findCallsOnFields() tells for a data member
        for (const callModel& call : callsOnResults) {
            if (call.getElementObject().empty()) continue;
            callModel onElement = call;
            onElement.getNameVector().clear();
            std::string written = call.getElementObject() + call.getName()[0];
            HELPERS::nameFilter(written, onElement.getNameVector(), unitLanguage, false);
            noteCallOnVariable(onElement);
        }

        noteClassHolders(parameters, globals);

        findReturnedVariables(parameters, globals, true);
        findVariablesInExpressions(parameters, globals);
        findModifiedVariables(parameters, typeMembers{}, globals, true);
    }
}

// Keeps in the return type each name spelled as a macro that a C++ type or alias of the archive has, such as POINT of typedef Point POINT;,
// given the names of the types and aliases of the archive
//
void functionModel::confirmReturnTypeMacros(const std::unordered_set<std::string>& archiveTypeNames) {
    const auto namesType = [&archiveTypeNames](const std::string& macro) { return archiveTypeNames.count(macro) > 0; };
    const auto macrosEnd = std::remove_if(returnTypeMacros.begin(), returnTypeMacros.end(), namesType);
    if (macrosEnd == returnTypeMacros.end()) return;

    returnTypeMacros.erase(macrosEnd, returnTypeMacros.end());
    findNonPrimitiveReturnType();
}

// Finds attributes ( C# ) or annotations ( Java )
//
void functionModel::findAttributesOrAnnotations(const srcmlDocument& method) {
    const nodeSet result = method.select(query::methodAttributes);
    for (int i = 0; i < result.size(); i++) attributesOrAnnotations.push_back(result.src(i));
}

// Gets the method name
//
// A JavaScript constructor has no name element, nor has a method named get or set
//
void functionModel::findName(const srcmlDocument& method) {
    const nodeSet result = method.select(query::methodName);

    if (result.size() > 0) { // srcML issue where it returns multiple results for the name
        std::string tempName = result.src(0); // Just pick the first
        HELPERS::nameFilter(tempName, name, unitLanguage, true);
    }
    else if (unitLanguage == "JavaScript") {
        std::string unnamed = unnamedMethodName(method.unitElement());
        if (!unnamed.empty()) HELPERS::nameFilter(unnamed, name, unitLanguage, true);
    }
}

// Gets the method parameter list with the types of its parameters and the numbers of arguments that it takes
//
void functionModel::findParameterList(const srcmlDocument& method) {
    const nodeSet result = method.select(query::methodParameterList);
    if (result.size() > 0 && unitLanguage == "Python") {
        // A Python class has one method of a name, whatever its parameters, so every signature is alike
        parametersList = result.src(0);
        signatureTypes = "()";
        arguments      = pythonArgumentRange(result.node(0), !objectName.empty() || !classParameter.empty());
    }
    else if (result.size() > 0 && unitLanguage == "JavaScript") {
        // A JavaScript class has one method of a name as well
        parametersList = result.src(0);
        signatureTypes = "()";
        arguments      = javascriptArgumentRange(result.node(0));
    }
    else if (result.size() > 0) {
        parametersList = result.src(0);
        parameterTypes = writtenTypesOf(result.node(0));
        signatureTypes = signatureTypesOf(result.node(0));
        arguments      = argumentRangeOf(result.node(0));
    }
}

// Gets the method return type 
//
// Java:
//   The '+=' for 'returnTypeString' skips the generic parameter list in Java return types
//   For example, 'public static <T> void swap()' the <T> is included in <type>
//   However, it is a generic declaration for the parameters and not a type, so it needs to be ignored
// C#:
//   An accessor returns the type of its property, except set, init, add, and remove, which return void
//
void functionModel::findReturnType(const srcmlDocument& method, const std::string& propertyType) {
    if (unitLanguage == "Python") {
        // findCreatedObjects() infers the return type of a function without an annotation
        const nodeSet annotation = method.select(query::returnType);
        returnTypeDeclared = annotation.size() > 0;
        if (returnTypeDeclared) returnType.setType(withOwnType(pythonTypeOf(annotation.node(0))));
    }
    else if (unitLanguage == "JavaScript") {
        // inferReturnType() infers the return type of a function without an annotation, while an async function returns the value of its promise
        const nodeSet annotation = method.select(query::returnType);
        returnTypeDeclared = annotation.size() > 0;
        if (returnTypeDeclared) returnType.setType(javascriptTypeOf(annotation.node(0)));

        const nodeSet names = method.select(query::returnTypeName);
        const bool async = std::find(specifiers.begin(), specifiers.end(), "async") != specifiers.end();
        const std::optional<std::string> awaited = async && names.size() == 1 ? awaitedType(names.src(0), languageOf(unitLanguage).asyncResults, unitLanguage) : std::nullopt;
        if (awaited) returnType.setType(*awaited);
    }
    else if (isProperty) {
        returnType.setType(takesImplicitValue(name[0]) ? "void" : propertyType);
    }
    else {
        const nodeSet result = method.select(query::returnType);

        std::string returnTypeString;
        for (int i = 0; i < result.size(); i++) {
            returnTypeString += result.srcml(i); // srcml_unit_get_src() will not work since the result is already just source code due to text()
        }

        returnType.setType(returnTypeString);

        // An async function returns the value of its result type, as bool of async Task<bool>, or no value, as async Task does
        if (std::find(specifiers.begin(), specifiers.end(), "async") != specifiers.end()) {
            const nodeSet names = method.select(query::returnTypeName);
            const std::optional<std::string> awaited = names.size() == 1 ? awaitedType(names.src(0), languageOf(unitLanguage).asyncResults, unitLanguage) : std::nullopt;
            if (awaited) returnType.setType(*awaited);
        }

        // A C++ name after the first that is spelled as a macro, as SCI_METHOD of void SCI_METHOD Release(), leaves the type until
        // confirmReturnTypeMacros() finds a type of the archive with its name
        if (unitLanguage == "C++") {
            const nodeSet names = method.select(query::returnTypeName);
            for (int i = 1; i < names.size(); ++i)
                if (isSpelledAsMacro(names.src(i))) returnTypeMacros.push_back(names.src(i));
        }

        // Handle conversion operators (e.g. "public static implicit operator IntPtr(MyHandle handle)" in C# or "explicit operator bool() const" in C++)
        // The return type here is actually 'IntPtr', but the type tag will only contain "public static implicit",
        //  so 'returnType' will become empty after stripping specifiers
        // Therefore, we must extract the actual type 'IntPtr' from the name tag --> 'operator IntPtr'
        // A C++ conversion operator defined outside its class has no type tag and a qualified name, such as 'Ptr::operator bool'
        const std::string conversionType = HELPERS::conversionTypeOf(name[0]);
        if (unitLanguage != "Java" && !conversionType.empty()) {
            tables->typeSpecifiers.removeSpecifiers(returnTypeString, unitLanguage);
            HELPERS::removeWhitespace(returnTypeString);

            // If type is empty (meaning it was just specifiers), parse the name
            if (returnTypeString.empty()) returnType.setType(conversionType);
        }
    }
}

// Collects the local variables, each with the name and the type of its own declaration
// A declaration of several variables, such as int a, b; marks the type of each later variable as the type of the one before
// A parameter of a lambda may have no type, as in x -> x + 1, while p of struct Point { } p; takes the name of the type, if any
//
void functionModel::findLocalVariables(const srcmlDocument& method, elementOrder& order) {
    const nodeSet result = method.select(query::localDeclaration);

    // A Python name that the body binds is a local in the whole body, with the type of its annotation, unless a global statement names it
    if (unitLanguage == "Python") {
        for (int i = 0; i < result.size(); ++i) {
            const std::string localName = result.src(i);
            if (parameters.count(localName) > 0 || localName == objectName || localName == classParameter) continue;

            const std::string type = withOwnType(pythonBoundValue(result.node(i)).type);
            const auto known = locals.find(localName);
            if (known != locals.end()) {
                if (known->second.variable.getType().empty()) known->second.variable.setType(type);
                continue;
            }

            scopedLocal local;
            local.variable.setName(localName);
            local.variable.setType(type);
            local.declared = -1;
            local.scopeEnd = std::numeric_limits<int>::max();
            locals.insert({localName, local});
        }

        const nodeSet globalNames = method.select(query::globalName);
        for (int i = 0; i < globalNames.size(); ++i) locals.erase(globalNames.src(i));
        return;
    }

    // A JavaScript local is a declaration, with the type of its annotation or of the object that its initializer creates, or the name of a nested function
    if (unitLanguage == "JavaScript") {
        for (int i = 0; i < result.size(); ++i) {
            const xmlNode* node = result.node(i);
            const bool declaration = isSrcElement(node, "decl");

            scopedLocal local;
            local.variable.setName(sourceOf(declaration ? firstChildElement(node, "name") : node));
            local.variable.setType(declaration ? javascriptDeclaredType(node) : "");
            local.declared = order.position(node);
            local.scopeEnd = order.end(scopeOf(declaration ? node : node->parent));
            locals.insert({local.variable.getName(), local});
        }
        return;
    }

    std::string previousType;
    bool        previousStatic = false;
    for (int i = 0; i < result.size(); i++) {
        const xmlNode* nameElement = firstChildElement(result.node(i), "name");
        xmlNode* type = xmlPreviousElementSibling(const_cast<xmlNode*>(nameElement));
        if (!isSrcElement(type, "type")) type = nullptr;

        // Chop off [] for arrays, as in int a[] of C++ or Java
        std::string localName = sourceOf(nameElement);
        HELPERS::removeBracketSuffix(localName);

        const xmlNode* typeBody = isTypeElement(result.node(i)->parent) ? result.node(i)->parent : nullptr;
        if (type && !refersToPreviousType(type)) {
            previousType   = typeSourceOf(type);
            previousStatic = declaresStatic(type);
        }
        else if (!type && typeBody) {
            const xmlNode* typeName = firstChildElement(typeBody, "name");
            previousType   = typeName ? sourceOf(typeName) : "";
            previousStatic = declaresStatic(typeBody);
        }

        // A local is in scope from its declaration to the end of its block or statement
        scopedLocal local;
        local.variable.setName(localName);
        local.variable.setType(type || typeBody ? previousType : "");
        local.isStatic = (type || typeBody) && previousStatic;
        local.declared = order.position(result.node(i));
        local.scopeEnd = order.end(scopeOf(result.node(i)));
        locals.insert({localName, local});
    }

    // A variable that an expression declares, as found of other instanceof Gauge found, may stay in scope after its statement
    const nodeSet declared = method.select(query::expressionVariable);
    for (int i = 0; i < declared.size(); ++i) {
        const xmlNode* block = declared.node(i)->parent;
        while (block->parent && !isSrcElement(block, "block_content")) block = block->parent;

        // The declaration starts at the name of the type, as the declaration of another local does, or else at the expression of the variable
        const xmlNode* type = xmlPreviousElementSibling(const_cast<xmlNode*>(declared.node(i)));
        if (!isSrcElement(type, "name")) type = nullptr;
        scopedLocal local;
        local.variable.setName(declared.src(i));
        local.variable.setType(type ? sourceOf(type) : inferredTypeOf(declared.node(i)));
        local.declared = order.position(type ? type : declared.node(i)->parent);
        local.scopeEnd = order.end(block);
        locals.insert({local.variable.getName(), local});
    }
}

// Collects the parameters that have a name, each with its type
// In C++, parameters could have a type but no name (for backward compatibility)
// The set, init, add, and remove accessors of C# take the implicit parameter value, of the type of their property or event
//
void functionModel::findParameters(const srcmlDocument& method, const std::string& propertyType) {
    const nodeSet result = method.select(query::parameterDeclaration);

    // A Python parameter other than the object or the class shares its object with the caller, unless its annotation names a primitive type
    // or it gathers arguments, as *args and **kwargs do
    if (unitLanguage == "Python") {
        for (int i = !objectName.empty() || !classParameter.empty() ? 1 : 0; i < result.size(); ++i) {
            const xmlNode* annotation = firstChildElement(result.node(i), "annotation");

            variableModel parameter;
            parameter.setName(sourceOf(firstChildElement(result.node(i), "name")));
            parameter.setType(annotation ? withOwnType(pythonTypeOf(annotation)) : "");
            parameter.setNamedType(pythonNamedType(parameter.getType()));

            variableModel type = parameter;
            const bool primitive = !parameter.getType().empty() &&
                                   (parameter.getNamedType().empty() ? !tables->primitiveTypes.isNonPrimitive(type, unitLanguage, ownTypeName, tables->typeSpecifiers)
                                                                     : tables->primitiveTypes.isPrimitive(parameter.getNamedType(), unitLanguage));
            if (!primitive && !xmlHasProp(result.node(i), BAD_CAST "type")) sharedParameters.insert(parameter.getName());
            parameters.insert({parameter.getName(), parameter});
        }
        return;
    }

    // A JavaScript parameter shares its object with the caller, unless its annotation names a primitive type or it gathers the arguments,
    // as ...items does, while a destructuring pattern declares the names of its keys, as x and y of { x, y }
    if (unitLanguage == "JavaScript") {
        for (int i = 0; i < result.size(); ++i) {
            for (const xmlNode* declaration : patternDeclarations(result.node(i))) {
                variableModel parameter;
                parameter.setName(sourceOf(firstChildElement(declaration, "name")));
                const xmlNode* annotation = typeAnnotationOf(declaration);
                parameter.setType(annotation ? javascriptTypeOf(annotation) : "");
                parameter.setNamedType(javascriptNamedType(parameter.getType()));

                // An array, a tuple, and an object type hold objects of the caller, as number[] and { x: number } do, though their names are primitive
                const std::string& written = parameter.getType();
                const bool holdsObjects = written.find('[') != std::string::npos || written == "object";
                variableModel type = parameter;
                const bool primitive = !written.empty() && !holdsObjects &&
                                       !tables->primitiveTypes.isNonPrimitive(type, unitLanguage, ownTypeName, tables->typeSpecifiers);
                // A rest parameter or a rest element gathers the arguments into an array of its own
                const xmlNode* first = xmlFirstElementChild(const_cast<xmlNode*>(declaration));
                const bool gathers = (isSrcElement(first, "modifier") && sourceOf(first) == "...") || isOperator(first, "...");
                if (!primitive && !gathers) sharedParameters.insert(parameter.getName());
                parameters.insert({parameter.getName(), parameter});
            }
        }
        return;
    }

    for (int i = 0; i < result.size(); ++i) {
        // A parameter that points to a function shares no object with the caller
        if (isSrcElement(result.node(i), "function_decl")) {
            const xmlNode* declarator = firstChildElement(result.node(i), "name");
            if (!declarator) continue;

            variableModel parameter;
            parameter.setName(sourceOf(declarator));
            parameter.setType(functionPointerTypeOf(result.node(i)));
            parameters.insert({parameter.getName(), parameter});
            continue;
        }

        const xmlNode* type = firstChildElement(result.node(i), "type");
        while (type && !isSrcElement(xmlNextElementSibling(const_cast<xmlNode*>(type)), "name")) type = nextNamedSibling(type, "type");
        if (!type) continue;

        // Chop off [] for arrays, as in int a[] of C++ or Java
        std::string parameterName = sourceOf(xmlNextElementSibling(const_cast<xmlNode*>(type)));
        const std::string declarator = parameterName;
        HELPERS::removeBracketSuffix(parameterName);
        const bool declaredArray = parameterName != declarator || isVariadicArray(type, unitLanguage);
        if (declaredArray) arrayParameters.insert(parameterName);
        if (unitLanguage == "C#" && hasReferenceModifier(type)) referenceParameters.insert(parameterName);
        if (sharesCallerObject(type, declaredArray, unitLanguage, tables->primitiveTypes)) sharedParameters.insert(parameterName);
        if (unitLanguage == "C++" && pointsToSharedObject(type)) sharedPointees.insert(parameterName);

        variableModel parameter;
        parameter.setName(parameterName);
        parameter.setType(typeSourceOf(type));
        parameter.setNamedType(namedTypeOf(type));
        parameters.insert({parameterName, parameter});
    }

    if (isProperty && takesImplicitValue(name[0])) {
        variableModel value;
        value.setName("value");
        value.setType(propertyType);
        parameters.insert({"value", value});
    }
}

// Collects all return expressions, and whether one returns an object that it creates
//
void functionModel::findReturnExpression(const srcmlDocument& method, elementOrder& order) {
    const nodeSet result = method.select(query::returnExpression);
    for (int i = 0; i < result.size(); ++i) {
        returnExpressions.push_back({withObjectNames(result.src(i)), order.position(result.node(i))});
        if (unitLanguage == "Python") {
            const xmlNode* call = soleCallOf(result.node(i));
            returnedValues.push_back({isNoneLiteral(result.node(i)), isBooleanByForm(result.node(i)), call ? withObjectNames(pythonCallName(call)) : "", ""});
        }
        else if (unitLanguage == "JavaScript") {
            returnedValues.push_back({isUndefinedLiteral(result.node(i)), isJavaScriptBooleanByForm(result.node(i)), "", createdClassOf(result.node(i))});
        }
    }

    newReturned = method.select(query::newReturnExpression).size() > 0;
}

// Collects the calls of each kind, constructor calls that use new included, with the name and the number of arguments of each call
// C++:
//   Constructor calls are a type of function calls (collected separately)
// C#:
//   A call through a null-conditional chain of names, as in o?.Dispose() or o?.inner?.Dispose(), is a method call on the names, as o.inner.Dispose() is,
//   while a chain that starts at or passes through more than a name, as in Open()?.Dispose(), makes a call on a result
//
void functionModel::findCalls(const srcmlDocument& method, elementOrder& order) {
    const std::pair<query, std::vector<callModel>*> callKinds[] = {
        {query::functionCall, &functionCalls}, {query::methodCall, &methodCalls}, {query::constructorCall, &newConstructorCalls}, {query::callOnResult, &callsOnResults}
    };
    // A Python call names the object as the model does, where super().m() calls a method of a parent through the object, as super.m() of Java does
    // A call on an element of a variable, as rows[0].strip(), is a call on the variable, as in the other languages
    if (unitLanguage == "Python") {
        for (const auto& [callQuery, collected] : callKinds) {
            const nodeSet result = method.select(callQuery);
            for (int i = 0; i < result.size(); ++i) {
                std::string written = pythonCallName(result.node(i));
                if (written.empty()) continue;

                const bool afterAccess  = collected == &callsOnResults;
                const bool throughSuper = afterAccess && followsSuperCall(result.node(i));
                const std::string object = afterAccess && !throughSuper ? pythonIndexedObject(result.node(i)) : "";
                written.insert(0, object);
                const xmlNode* argumentList = firstChildElement(result.node(i), "argument_list");
                std::vector<callModel>& calls = throughSuper || !object.empty() ? methodCalls : *collected;
                addNamedCall(result.node(i), xmlFirstElementChild(result.node(i)), argumentList,
                             throughSuper ? "super." + written : withObjectNames(written), throughSuper ? "super()." + written : written,
                             unitLanguage, order, calls);
                const xmlNode* first = argumentList ? firstChildElement(argumentList, "argument") : nullptr;
                calls.back().setObjectFirst(first && !objectName.empty() && sourceOf(first) == objectName);
            }
        }
        return;
    }

    for (const auto& [callQuery, collected] : callKinds) {
        const nodeSet result = method.select(callQuery);
        for (int i = 0; i < result.size(); ++i) {
            // A call on the object of a local or anonymous type is a call on another object
            if ((collected == &functionCalls || collected == &methodCalls) && callsNestedObject(result.node(i))) {
                addCall(result.node(i), "", unitLanguage, order, callsOnResults);
                continue;
            }
            if (unitLanguage == "C#" && (collected == &methodCalls || collected == &callsOnResults)) {
                if (const std::optional<std::string> forgiven = nullForgivenObject(result.node(i))) {
                    addCall(result.node(i), *forgiven, unitLanguage, order, forgiven->empty() ? callsOnResults : methodCalls);
                    continue;
                }
            }
            const std::optional<std::string> object = collected == &functionCalls ? objectBeforeTypeArguments(result.node(i)) : std::nullopt;
            if (!object && collected == &callsOnResults) {
                // The variable of a call on an element may be a data member, which findCallsOnFields() tells
                const std::size_t known = callsOnResults.size();
                addCall(result.node(i), "", unitLanguage, order, callsOnResults);
                const std::string element = callsOnResults.size() > known ? elementObjectOf(result.node(i)) : "";
                for (std::size_t added = known; added < callsOnResults.size(); ++added) callsOnResults[added].setElementObject(element);
            }
            else if (!object)         addCall(result.node(i), "", unitLanguage, order, *collected);
            else if (object->empty()) addCall(result.node(i), "", unitLanguage, order, callsOnResults);
            else                      addCall(result.node(i), *object, unitLanguage, order, methodCalls);
        }
    }

    if (unitLanguage == "C#") {
        const nodeSet result = method.select(query::nullConditionalCall);
        for (int i = 0; i < result.size(); ++i) {
            const std::string object = nullConditionalObject(result.node(i));
            addCall(result.node(i), object, unitLanguage, order, object.empty() ? callsOnResults : methodCalls);
        }
    }
    if (unitLanguage == "C++") findOperatorCalls(method, order);

    // A call through this of a JavaScript static method calls a static method of the class, as this.create() does
    if (unitLanguage == "JavaScript" && !classParameter.empty()) {
        for (callModel& call : methodCalls) {
            std::string written = withObjectNames(call.getName()[0]);
            call.getNameVector().clear();
            HELPERS::nameFilter(written, call.getNameVector(), unitLanguage, false);
        }
    }
}

// Collects the calls that C++ operators make
// An operator after the object itself, as << of *this << value, calls the operator function of its name, which the type, a parent, or a free
// function defines. A shift operator after a variable, as << of out << value, calls an operator function only where the variable holds an
// object, which findStreamCalls() decides by the type of the variable
//
void functionModel::findOperatorCalls(const srcmlDocument& method, elementOrder& order) {
    const nodeSet onObject = method.select(query::selfOperator);
    for (int i = 0; i < onObject.size(); ++i)
        functionCalls.push_back(operatorCall("operator" + onObject.src(i), order.position(onObject.node(i)), unitLanguage));

    const nodeSet shifts = method.select(query::shiftOperator);
    for (int i = 0; i < shifts.size(); ++i) {
        // An operand inside a chain, as b of a << b << c, is no stream
        const auto [first, last] = accessChain(xmlPreviousElementSibling(const_cast<xmlNode*>(shifts.node(i))));
        if (isShiftOperator(xmlPreviousElementSibling(const_cast<xmlNode*>(first)))) continue;

        // A member or an element of a variable, as context.count[0], has no declared type in the model
        const std::string shifted = chainSource(first, last);
        if (!isPlainName(withoutSelfAccess(shifted))) continue;

        shiftOperation shift;
        shift.call = operatorCall(shifted + ".operator" + shifts.src(i), order.position(shifts.node(i)), unitLanguage);
        for (const xmlNode* op = shifts.node(i); isOperator(op, ">>");) {
            const xmlNode* target = xmlNextElementSibling(const_cast<xmlNode*>(op));
            if (!isSrcElement(target, "name")) break;

            const auto [targetFirst, targetLast] = accessChain(target);
            shift.extracted.push_back({chainSource(targetFirst, targetLast), order.position(targetFirst), false});
            op = xmlNextElementSibling(const_cast<xmlNode*>(targetLast));
        }
        shiftOperations.push_back(std::move(shift));
    }
}

// Collects the names of the local functions and of the locals that a lambda initializes, in a language in which the code calls a variable by its name
//
void functionModel::findLocalFunctions(const srcmlDocument& method) {
    if (languageOf(unitLanguage).callOperator.empty()) return;

    const nodeSet result = method.select(query::localFunction);
    for (int i = 0; i < result.size(); ++i) localFunctions.insert(result.src(i));
}

// Finds all variables that are declared or initialized with the 'new' operator
//
void functionModel::findNewAssignedVariables(const srcmlDocument& method) {
    const nodeSet result = method.select(query::newAssignedVariable);

    for (int i = 0; i < result.size(); ++i) {
        std::string varName = result.src(i);
        HELPERS::removeWhitespace(varName);
        
        // A return of the variable names it without this, such as cache for this.cache = new A();
        variablesCreatedWithNew.insert(withoutSelfAccess(varName));
    }
}

// Determines if method is empty
//
void functionModel::findNonCommentStatements(const srcmlDocument& method) {
    nonCommentStatementsCount = method.select(query::nonCommentStatements).size(); 
}

// Determines if method is const (C++ only)
//
void functionModel::findConst(const srcmlDocument& method) {
    if (method.select(query::constMethod).size() > 0) methodConst = true;
}

// Finds the type of constructor or destructor
//
void functionModel::findConstructorOrDestructorType(const srcmlDocument& method) {
    const nodeSet result = method.select(query::constructorOrDestructor);

    // Only a Python method is a constructor or a destructor, by its name, and Python has no copy constructor
    if (unitLanguage == "Python") {
        if (result.size() > 0 && !typeNameParsed.empty() && classParameter.empty()) constructorOrDestructor = name[3] == "__del__" ? "destructor" : "constructor";
    }
    // JavaScript has no destructor and no copy constructor
    else if (unitLanguage == "JavaScript") {
        if (result.size() > 0) constructorOrDestructor = "constructor";
    }
    else if (result.size() > 0) { // srcML issue where it returns multiple results for the name
        const nodeSet parameterList = method.select(query::methodParameterList);
        if      (xmlStrEqual(result.node(0)->name, BAD_CAST "destructor"))                         constructorOrDestructor = "destructor";
        else if (parameterList.size() > 0 && copiesOwnType(parameterList.node(0), typeNameParsed, unitLanguage)) constructorOrDestructor = "copy-constructor";
        else                                                                                       constructorOrDestructor = "constructor";
    }
}

// Finds names of expressions
// srcML marks a C# interpolated string as string literals, so the names of its holes come from its text and the names inside it are text
//
void functionModel::findExpressionNames(const srcmlDocument& method, elementOrder& order)  {
    std::vector<std::pair<int, int>> interpolated;  // Positions of the elements of each interpolated string, from its first to after its last
    if (unitLanguage == "C#") {
        const nodeSet strings = method.select(query::interpolatedString);
        for (int i = 0; i < strings.size(); ++i) {
            const int position = order.position(strings.node(i));
            if (!interpolated.empty() && position < interpolated.back().second) continue;

            // A string whose text ends before its closing quote has no known extent, so the names of srcML in it stay
            const interpolatedString string = readInterpolatedString(strings.node(i));
            if (string.closingNode) interpolated.emplace_back(position, positionAfter(string.closingNode, order));
            for (const std::string& holeName : string.names) expressionNames.push_back({holeName, position});
        }
    }

    // srcML marks a Python f-string and a JavaScript template literal as one literal, so the names of its holes come from its text
    if (unitLanguage == "Python" || unitLanguage == "JavaScript") {
        const nodeSet strings = method.select(query::interpolatedString);
        for (int i = 0; i < strings.size(); ++i) {
            const std::vector<std::string> holeNames = unitLanguage == "Python" ? formattedStringNames(strings.src(i)) : templateLiteralNames(strings.src(i));
            for (const std::string& holeName : holeNames) expressionNames.push_back({withObjectNames(holeName), order.position(strings.node(i))});
        }
    }

    const nodeSet result = method.select(query::expressionName);
    std::size_t nextString = 0;     // The first interpolated string that does not end before the name
    for (int i = 0; i < result.size(); i++) {
        const int position = order.position(result.node(i));
        while (nextString < interpolated.size() && interpolated[nextString].second <= position) ++nextString;
        if (nextString < interpolated.size() && interpolated[nextString].first <= position) continue;
        if (std::optional<std::string> reached = nameOutsideNestedObject(result.node(i), result.src(i)))
            expressionNames.push_back({withObjectNames(unitLanguage == "JavaScript" ? withoutOptionalChaining(*reached) : std::move(*reached)), position});
    }
}

// Finds names of expressions
// All expressions with some assignment operator is collected. 
// Exception is made for C# where an assignment (e.g., Property = value) can be used in an initializer (e.g., var x = new Class { Property = value };), so only expressions with assignment that are not in an initializer are collected
// Each written name stands with its whole access chain, such as m_subs[1].second, whose first name tells the written variable
// A chain that goes on from the result of a call, as a[i].b of f().a[i].b, names no variable, since the call notes the write through its result
void functionModel::findExpressionAssignments(const srcmlDocument& method, elementOrder& order)  {
    const nodeSet result = method.select(query::expressionAssignment);
    for (int i = 0; i < result.size(); i++) {
        // The C# operator ! sets a written name apart from its indexes, as in buffer![0] = value
        const xmlNode* written = indexedName(result.node(i));
        if (!written) continue;
        auto [first, last] = accessChain(written);
        if (written != result.node(i)) last = result.node(i);
        if (continuesCallResult(first)) continue;

        // A C# chain after ?. goes on from the chain before ?., as part?.b = v writes part.b, while Shared()?.b = v writes through a result
        std::string chain = chainSource(first, last);
        if (const xmlNode* start = unitLanguage == "C#" ? nullConditionalStart(first) : nullptr) {
            const std::string object = nullConditionalObject(start);
            if (object.empty()) continue;
            chain = object.substr(0, object.size() - 1) + chainSource(start, last);
        }
        std::optional<std::string> reached = nameOutsideNestedObject(first, std::move(chain));
        if (!reached) continue;
        chain = withObjectNames(std::move(*reached));
        if (unitLanguage == "Python" && first == last) noteBinding(pythonBoundValue(first), chain);
        if (unitLanguage == "JavaScript" && first == last) noteBinding(javascriptBoundValue(first), chain);
        expressionAssignments.push_back({chain, order.position(first), isDereferenced(first, last)});
    }
}

// Finds if the return type, local types, and parameter types are non-primitive
//
void functionModel::findNonPrimitive() {
    findNonPrimitiveReturnType();

    // Local types
    for (auto& local : locals) {               
        if (tables->primitiveTypes.isNonPrimitive(local.second.variable, unitLanguage, ownTypeName, tables->typeSpecifiers) && local.second.variable.isNonPrimitiveExternal())
            nonPrimitiveLocalExternal = true;
    }

    // Parameter types
    for (auto& parameter : parameters) {
       if (tables->primitiveTypes.isNonPrimitive(parameter.second, unitLanguage, ownTypeName, tables->typeSpecifiers) && parameter.second.isNonPrimitiveExternal())
            nonPrimitiveParamaterExternal = true;
    }
}

// Finds whether the return type without its macros is non-primitive and whether it is external
//
void functionModel::findNonPrimitiveReturnType() {
    variableModel type;
    type.setType(returnTypeWithoutMacros());
    nonPrimitiveReturnType         = tables->primitiveTypes.isNonPrimitive(type, unitLanguage, ownTypeName, tables->typeSpecifiers);
    nonPrimitiveReturnTypeExternal = type.isNonPrimitiveExternal();
}

// Gets the return type with a space in place of each macro, as a specifier leaves its place
//
std::string functionModel::returnTypeWithoutMacros() const {
    if (returnTypeMacros.empty()) return returnType.getType();
    return specifierPattern({}, returnTypeMacros).replace(returnType.getType());
}

// Finds variables in expressions
//
void functionModel::findVariablesInExpressions(const std::unordered_map<std::string, variableModel>& variables, const globalVariables& globals) {
    for (const bodyText& expr : expressionNames) {
        noteUse(resolveName(expr.text, unitLanguage, typeNameParsed, false, scopesAt(expr.position, variables, globals)), expr.position, variables);
    }
}

// Gets the locals in scope at a position, the parameters, the given fields, and the globals that the unit of the function reaches as the variables in scope
// A name without a qualifier reaches a global at namespace scope or a static field of a type that the function reaches without its name
//
variableScopes functionModel::scopesAt(int position, const std::unordered_map<std::string, variableModel>& fields, const globalVariables& globals) const {
    const int unit = unitNumber();

    // A method of a dynamic language reaches a data member only through the object, where every name that is no method is a data member
    const bool dynamicMethod = dynamic && ownType;
    // Only the file of a module reaches its globals without a qualifier
    const bool ownUnitOnly   = unknownParent || moduleGlobals;
    return {
        [this, position](const std::string& variable) { return localAt(variable, position, true) != nullptr; },
        [this](const std::string& variable) { return parameters.find(variable) != parameters.end(); },
        [&fields, dynamicMethod](const std::string& variable) { return dynamicMethod ? variable == "this" : fields.find(variable) != fields.end(); },
        [this, position](const std::string& variable) { return localAt(variable, position, false) != nullptr; },
        [this, &globals, unit, ownUnitOnly](const std::string& variable) {
            // A parent outside the archive may declare the name, unless the unit declares a global of the name
            const bool atNamespaceScope = ownUnitOnly ? globals.declaredAtNamespaceScope(unit, variable) : globals.atNamespaceScope(unitLanguage, variable, unit);
            return atNamespaceScope ||
                   std::any_of(staticHolders.begin(), staticHolders.end(), [&](const std::string& holder) { return globals.holds(unitLanguage, holder, variable, unit); });
        },
        [this, &globals, unit](const std::string& holder, const std::string& variable) { return globals.holds(unitLanguage, holder, variable, unit); },
        [this, &fields, dynamicMethod](const std::string& member) {
            return fields.find(member) != fields.end() ||
                   (dynamicMethod && isPlainName(member) && ownType->methodNames.count(member) == 0 && ownType->staticMethodNames.count(member) == 0 &&
                    ownType->nestedTypeNames.count(member) == 0);
        }
    };
}

// Gets the number of the unit that holds the function, which tells the globals with internal linkage that the function reaches
//
int functionModel::unitNumber() const {
    return location ? location->unitNumber : 0;
}

// Gets the local with a name that is declared before a position, the innermost one in scope if any, else the last one declared, or null
//
const scopedLocal* functionModel::localAt(const std::string& localName, int position, bool inScopeOnly) const {
    const scopedLocal* innermost = nullptr;
    const scopedLocal* last      = nullptr;
    const auto [first, end] = locals.equal_range(localName);
    for (auto it = first; it != end; ++it) {
        const scopedLocal& local = it->second;
        if (local.declared >= position) continue;
        if (position < local.scopeEnd && (!innermost || local.declared > innermost->declared)) innermost = &local;
        if (!last || local.declared > last->declared) last = &local;
    }
    if (innermost) return innermost;
    return !inScopeOnly && last ? last : nullptr;
}

// Tells whether a name at a position resolves to a static local
//
bool functionModel::isStaticLocal(const nameResolution& resolution, int position) const {
    return resolution.kind == variableKind::local && localAt(resolution.variable, position, false)->isStatic;
}

// Records the use of a parameter, a field, or a global or a static local by a name at a position
//
void functionModel::noteUse(const nameResolution& resolution, int position, const std::unordered_map<std::string, variableModel>& fields) {
    if (resolution.kind == variableKind::parameter) {
        parameterUsed = true;
    }
    else if (resolution.kind == variableKind::field) {
        // A data member of a dynamic language may have no declaration, and then no known type
        fieldUsed = true;
        const auto field = fields.find(resolution.variable);
        if (field != fields.end() && field->second.isNonPrimitiveExternal()) nonPrimitiveFieldExternal = true;
    }
    else if (resolution.kind == variableKind::global || isStaticLocal(resolution, position)) {
        globalUsed = true;
    }
}

// Records the use of each variable that holds the class of a created object in a language in which a call of a class creates an object,
// as the data member of self.HandlerClass() and the parameter of factory.Handler()
//
void functionModel::noteClassHolders(const std::unordered_map<std::string, variableModel>& variables, const globalVariables& globals) {
    if (!languageOf(unitLanguage).callsCreate) return;
    for (const callModel& call : newConstructorCalls) {
        const int position = call.getPosition();
        noteUse(resolveName(call.getName()[3], unitLanguage, typeNameParsed, false, scopesAt(position, variables, globals)), position, variables);
    }
}

// Tells whether a name refers to the object itself as a whole, as this, *this, and (*this) do, unlike this->a or this[i]
//
bool functionModel::refersToObject(const nameResolution& resolution) {
    return resolution.kind == variableKind::field && resolution.variable == "this" && !resolution.throughMember && !resolution.throughIndex;
}

// Records a variable created with new that a return expression returns, unlike an element of a created array, as made[0]
//
void functionModel::noteReturnedVariable(const nameResolution& resolution) {
    if (resolution.kind != variableKind::unknown && !resolution.throughIndex &&
        variablesCreatedWithNew.find(resolution.variable) != variablesCreatedWithNew.end())
        variableCreatedWithNewAndReturned = true;
}

// Tells whether a write to a parameter changes data of the caller
// A write changes the data of the caller through a C++ reference, a C# out or ref parameter, or a member, an element, or the target of the parameter,
//  such as p->b = v, a[i] = v, or *p = v. A write to the parameter itself, such as p = q or a = new int[n], changes only the copy of the function
// In C# and Java, the value of a non-primitive parameter is a reference to the object, so only a write to a member of the object changes the object
//
bool functionModel::changesCallerData(const std::string& para, const nameResolution& write) const {
    // A parameter of a dynamic language refers to the object of the caller, which a write through a member or an element changes
    if (dynamic) return sharedParameters.count(para) > 0 && writesThroughVariable(write);

    std::string type = parameters.at(para).getType();
    const bool nonPrimitive = !tables->primitiveTypes.isPrimitive(type, unitLanguage);
    HELPERS::removeWhitespace(type);

    const bool array   = type.find("[]") != std::string::npos || arrayParameters.find(para) != arrayParameters.end();
    const bool pointer = type.find('*') != std::string::npos || sharedPointees.count(para) > 0; // C# could use * in unsafe contexts, and a std::shared_ptr points

    if (unitLanguage == "C++") return type.find('&') != std::string::npos || ((pointer || array) && writesThroughVariable(write));
    if (unitLanguage == "C#" && referenceParameters.find(para) != referenceParameters.end()) return true;
    if (pointer || array) return writesThroughVariable(write);
    return nonPrimitive && write.throughMember;
}

// Determines if a return expression returns a field or a parameter
// Both simple returns (e.g., return dm;) and 
//   complex returns (e.g., return dm + 5; or return dm + 5;) are considered
//
void functionModel::findReturnedVariables(const std::unordered_map<std::string, variableModel>& variables, const globalVariables& globals, bool isParameterCheck) {
    for (const bodyText& expr : returnExpressions) {
        const nameResolution resolution = resolveName(expr.text, unitLanguage, typeNameParsed, true, scopesAt(expr.position, variables, globals));
        if (isParameterCheck) { 
            noteUse(resolution, expr.position, variables);
            noteReturnedVariable(resolution);
            if      (resolution.kind == variableKind::parameter) parameterReturned = true;
            else if (resolution.kind != variableKind::field)     parameterComplexReturn = true;
        }
        else if (refersToObject(resolution)) {
            // The 'this' is added as a field, however, the object itself is neither a simple nor a complex return
            objectReturned = true;
        }
        else {
            noteUse(resolution, expr.position, variables);
            noteReturnedVariable(resolution);
            if (resolution.kind == variableKind::field) simpleReturn = true;
            else                                        complexReturn = true;
        }
    }
}

// Finds if a field, local, or a parameter (normal and passed by reference) is modified
// Multiple modifications to the same field or parameter are only considered as 1 modification
// A write to a global changes a global for a free function and no state of the object for a method
// For a method, a name that no scope holds is a field when an unknown parent type may declare it
// A name other than the type or a parent that qualifies it, as Other does in Other.count = 1 or Other::count = 1, names a member of another object or class
//
void functionModel::findModifiedVariables(const std::unordered_map<std::string, variableModel>& variables, const typeMembers& type, const globalVariables& globals,
                                          bool isParameterCheck) {
    std::unordered_set<std::string> checked; 
    
    for (const bodyText& written : expressionAssignments) {
        nameResolution resolution = resolveName(written.text, unitLanguage, typeNameParsed, false, scopesAt(written.position, variables, globals));
        if (written.dereferenced) resolution.dereferenced = true;
        noteUse(resolution, written.position, variables);

        switch (resolution.kind) {
            case variableKind::local:
                if (isParameterCheck && isStaticLocal(resolution, written.position)) globalOrStaticVariableModified = true;
                // Only a write through the local changes an object, while a write to the local itself rebinds the local
                // In a dynamic language, a variable with a member is an object, whatever the code tells about its type
                else if (!isParameterCheck && writesThroughVariable(resolution) &&
                         (localAt(resolution.variable, written.position, false)->variable.isNonPrimitive() || (dynamic && resolution.throughMember)))
                    nonPrimitiveLocalOrParameterModified = true;
                break;
            case variableKind::parameter: {
                const bool changesCaller = changesCallerData(resolution.variable, resolution);
                if (changesCaller) parameterRefModified = true;
                if ((parameters.at(resolution.variable).isNonPrimitive() || (dynamic && resolution.throughMember)) &&
                    (changesCaller || writesThroughVariable(resolution)))
                    nonPrimitiveLocalOrParameterModified = true;
                break;
            }
            case variableKind::field:
                if (isParameterCheck) break;
                // A write of the object itself, as *this = other, replaces every field, while a dynamic language only binds the name of the object again
                if (refersToObject(resolution))                       objectReplaced = !dynamic;
                else if (checked.insert(resolution.variable).second) ++fieldsModifiedCount;
                break;
            case variableKind::global:
                if (isParameterCheck) globalOrStaticVariableModified = true;
                // The other class of a member that the write reaches makes a collaborator, unlike a namespace or a module
                else if (resolution.otherQualifier && type.ownTypes.count(resolution.holder) == 0 &&
                         globals.heldByType(unitLanguage, resolution.holder, resolution.variable, unitNumber()))
                    nonPrimitiveFieldExternal = true;
                break;
            case variableKind::unknown: {
                // A free function may write a data member of a class outside the archive, as the definition of its method does
                if (resolution.variable.empty() || isParameterCheck) break;
                if (resolution.otherQualifier && type.ownTypes.count(resolution.variable) == 0) {
                    // The member of another object or class is no state of the object, while the other class makes a collaborator
                    // In a language with modules, only the name of a class tells a class from a module, which is no class
                    if (!moduleGlobals || namesPythonClass(resolution.variable)) nonPrimitiveFieldExternal = true;
                    break;
                }
                // Only the object reaches a data member in a dynamic language, and the discard of a language is no variable
                if (dynamic || resolution.variable == languageOf(unitLanguage).discard) break;

                // A parent that qualifies a name, as Base does in Base::size, names a member that the object inherits, while a static field
                // or a global is no state of the object, unless an unknown parent may declare the name as a field
                const std::string& member = resolution.otherQualifier ? resolution.member : resolution.variable;
                if (type.fields.count(member) > 0 || (type.unknownParent && type.staticFields.count(member) == 0)) {
                    if (checked.insert(member).second) ++fieldsModifiedCount;
                    fieldUsed = true;
                }
                break;
            }
        }
    }
}

// Ignore calls from analysis
// For example, if call to ignore is 'foo', then some of the matched cases are 'foo<> or bar::foo or a->b.foo'
// However, usage of fields within these calls are not ignored (e.g., in arguments)
//
void functionModel::findIgnorableCalls(std::vector<callModel>& calls) {
    calls.erase(std::remove_if(calls.begin(), calls.end(), [this](const callModel& call) {
        return tables->ignorableCalls.isIgnored(call.getName()[3], unitLanguage);
    }), calls.end());
}



// Gets the declared type of the variable that a name resolves to, where the type of a global is not known
//
std::string functionModel::declaredTypeOf(const nameResolution& resolution, int position, const std::unordered_map<std::string, variableModel>& fields) const {
    if (resolution.kind == variableKind::local)     return localAt(resolution.variable, position, false)->variable.getType();
    if (resolution.kind == variableKind::parameter) return parameters.at(resolution.variable).getType();

    const auto field = resolution.kind == variableKind::field ? fields.find(resolution.variable) : fields.end();
    return field != fields.end() ? field->second.getType() : "";
}

// Counts each C++ shift operator after a variable whose type is a stream or a class of the archive as a call of the operator function on the
// variable, where each operand that >> extracts into is written, while a shift of a number stays an operator
//
void functionModel::findStreamCalls(const std::unordered_map<std::string, variableModel>& variables, const globalVariables& globals, const callableNames& archive) {
    for (shiftOperation& shift : shiftOperations) {
        const int position = shift.call.getPosition();
        const nameResolution object = resolveName(shift.call.getName()[3], unitLanguage, typeNameParsed, false, scopesAt(position, variables, globals));
        const std::vector<std::string> typeNames = tables->typeSpecifiers.typeNames(declaredTypeOf(object, position, variables), unitLanguage);
        if (typeNames.empty() || (!namesStream(typeNames.front()) && archive.classes.count(typeNames.front()) == 0)) continue;

        methodCalls.push_back(std::move(shift.call));
        expressionAssignments.insert(expressionAssignments.end(), shift.extracted.begin(), shift.extracted.end());
    }
    shiftOperations.clear();
}

// Counts each call of a variable as a call of the function that the variable holds, which reads the variable, whether the code calls the
// variable by its name, as handler() does, or by the call operator of its language, as handler.Invoke() does, given the data members or the
// parameters that a name may reach, the names of the methods of the type, and the names of the archive
// A variable whose type is a class of the archive holds an object, so the call operator after it names a method of that class, as
// job.Invoke() does, and the call of such a data member by its name names that operator, as m_compare.operator() for m_compare()
// A local function or a local that a lambda initializes holds code of the function itself, so its call is no call
// A call of any method on a C# event, as Closed.SafeInvoke(this, args), is such a call as well, since no call changes the delegate of an event
//
void functionModel::findCalledVariables(const std::unordered_map<std::string, variableModel>& variables, const globalVariables& globals,
                                        const std::unordered_set<std::string>& methodNames, const callableNames& archive) {
    const std::string callOperator{languageOf(unitLanguage).callOperator};
    if (callOperator.empty()) return;

    const auto resolve = [this, &variables, &globals](const callModel& call) {
        return resolveName(call.getName()[3], unitLanguage, typeNameParsed, false, scopesAt(call.getPosition(), variables, globals));
    };
    const auto holdsArchiveObject = [this, &variables, &archive](const nameResolution& variable, int position) {
        const std::vector<std::string> typeNames = tables->typeSpecifiers.typeNames(declaredTypeOf(variable, position, variables), unitLanguage);
        return !typeNames.empty() && archive.classes.count(typeNames.front()) > 0;
    };
    const auto withCallOperator = [this, &callOperator](callModel call) {
        std::string longForm = call.getName()[0] + "." + callOperator;
        call.getNameVector().clear();
        HELPERS::nameFilter(longForm, call.getNameVector(), unitLanguage, false);
        return call;
    };

    // The calls with an access operator, of which this->handler() names a data member through the object and handler.Invoke() names the call operator
    const std::string longFormEnd = "." + callOperator;
    const std::size_t written = methodCalls.size();
    std::vector<callModel> onVariables;
    for (std::size_t i = 0; i < written; ++i) {
        callModel& call = methodCalls[i];
        std::string called = withoutSelfAccess(call.getName()[3]);
        called.erase(std::remove(called.begin(), called.end(), '?'), called.end());

        const bool throughObject = called != call.getName()[3] && isPlainName(called) && variables.count(called) > 0 && methodNames.count(called) == 0;
        const bool longForm      = called.size() > longFormEnd.size() && called.compare(called.size() - longFormEnd.size(), longFormEnd.size(), longFormEnd) == 0 &&
                                   isPlainName(called.substr(0, called.size() - longFormEnd.size()));
        const auto event         = variables.find(called.substr(0, called.rfind('.')));
        const bool onEvent       = called.find('.') != std::string::npos && event != variables.end() && event->second.isEvent();
        nameResolution variable  = throughObject || longForm || onEvent ? resolve(call) : nameResolution{};
        if (onEvent && !longForm && variable.kind != variableKind::field) variable = {};
        if (variable.kind == variableKind::unknown) {
            onVariables.push_back(std::move(call));
        }
        else if (holdsArchiveObject(variable, call.getPosition())) {
            onVariables.push_back(longForm ? std::move(call) : withCallOperator(std::move(call)));
        }
        else {
            noteUse(variable, call.getPosition(), variables);
            functionCalls.push_back(std::move(call));
        }
    }
    methodCalls = std::move(onVariables);

    for (auto it = functionCalls.begin(); it != functionCalls.end();) {
        const std::string called = it->getName()[3];
        if (!isPlainName(called)) { ++it; continue; }
        if (localFunctions.count(called) > 0) { it = functionCalls.erase(it); continue; }

        const nameResolution variable = resolve(*it);
        const bool hidesMethods = variable.kind == variableKind::local || variable.kind == variableKind::parameter;
        if (variable.kind == variableKind::unknown || (!hidesMethods && methodNames.count(called) > 0)) { ++it; continue; }

        noteUse(variable, it->getPosition(), variables);
        if (variable.kind != variableKind::field || !holdsArchiveObject(variable, it->getPosition())) { ++it; continue; }

        methodCalls.push_back(withCallOperator(std::move(*it)));
        it = functionCalls.erase(it);
    }
}

// Function calls: --> foo() bar::foo()
//  Checks if a function call is made to a method in the type, else it is removed and considered external 
//  Static and free function calls are considered external calls
//
// Method Calls: --> bar.foo() where ( bar ) could be a field or a type name or a namespace 
//  Checks if there is a method call on a field
//  For example, a.foo() where ( a ) is a field, else it is removed and considered as external method call
//
//  In C# or Java, a type name can be used with the dot operator to invoke static methods
//  For example, typeName.staticMethodName();
//  These are removed and considered as external function calls
//
//  ( this ), ( base ), and ( super ) can only be used to invoke non-static methods in the current type (this) or parent type (base or super)
//  For example, this.methodName();
//  So these should be also treated as function calls and not method calls
//
void functionModel::findCallsOnFields(const typeMembers& type, const globalVariables& globals) {
    const std::unordered_map<std::string, variableModel>& allFields      = type.fields;
    const std::unordered_set<std::string>&                allMethodNames = type.methodNames;
    const std::unordered_set<std::string>&                ownTypes       = type.ownTypes;

    // Check on function calls (Should be done before checking on method calls)
    // Notice we do not use parameters (i.e., signature) to check the function calls since some calls can use variable number of arguments with the 'params' keyword in C# or 'varargs' in Java and C++
    //  For example, foo(1, 2) can match void foo(params int[] numbers) in C# or void foo(int... numbers) in Java and C++
    // Or calls with default parameters in C++ where the arguments can be omitted. 
    //  For example, foo() can match void foo(int a=5) if the default value of a is provided
    // Therefore, we really only want to check that the function call name matches any of the overloaded methods (if any) and 
    //  this is sufficient to determine if the call is a method call or a static/free function call
    // A method of a dynamic language calls a method of its type only through the object, so a call without an object calls a function
    for (auto it = functionCalls.begin(); it != functionCalls.end();) {  
        if (dynamic || (allMethodNames.find(it->getName()[3]) == allMethodNames.end() && !callsParentOutsideArchive(*it, type, globals))) {
            externalCalls.push_back(*it);
            ++externalFunctionCallsCount;
            it = functionCalls.erase(it);
        }
        else {
            ++it;
        }
    }  
    
    // A call on the result of a call or a cast is a call on another object, while a call on an element of a data member is a call on
    // the data member, as slots[i].clear()
    for (const callModel& call : callsOnResults) {
        if (!call.getElementObject().empty()) {
            callModel onElement = call;
            onElement.getNameVector().clear();
            std::string written = call.getElementObject() + call.getName()[0];
            HELPERS::nameFilter(written, onElement.getNameVector(), unitLanguage, false);
            const variableScopes scopes = scopesAt(call.getPosition(), allFields, globals);
            if (resolveName(onElement.getName()[3], unitLanguage, typeNameParsed, false, scopes).kind == variableKind::field) {
                methodCalls.push_back(std::move(onElement));
                continue;
            }
        }
        externalCalls.push_back(call);
        ++externalMethodCallsCount;
    }

    // Check on method calls
    for (auto it = methodCalls.begin(); it != methodCalls.end();) {
        // A Python method called under the name of the class or of a parent, with the object as its first argument, as Base.reset(self),
        // is called through the object, as super().reset() is
        const std::size_t holderEnd = it->passesObjectFirst() ? it->getName()[3].rfind('.') : std::string::npos;
        const std::string holder    = holderEnd == std::string::npos ? "" : lastNameOf(it->getName()[3].substr(0, holderEnd));
        if (ownTypes.count(holder) > 0) {
            std::string throughObject = (holder == typeNameParsed ? "this." : "super.") + it->getName()[3].substr(holderEnd + 1);
            it->getNameVector().clear();
            HELPERS::nameFilter(throughObject, it->getNameVector(), unitLanguage, false);
            it->setArgumentCount(it->getArgumentCount() - 1);
            it->setObjectFirst(false);
        }

        // In a dynamic language, a call through the object names a method, since findCalledVariables() takes the calls of data members
        if (((unitLanguage == "Java" || dynamic) && callsMethodThroughSelf(it->getName()[3])) ||
            (unitLanguage == "Java" && callsMethodThroughQualifiedSelf(it->getName()[3]))) {
            const std::string member = lastNameOf(it->getName()[3]);

            // A static or class method that the object reaches is a function, as the same method is under the name of the class
            if (type.staticMethodNames.count(member) > 0 && allMethodNames.count(member) == 0) {
                externalCalls.push_back(*it);
                ++externalFunctionCallsCount;
                it = methodCalls.erase(it);
                continue;
            }
            if (!dynamic || allMethodNames.count(member) > 0 || allFields.count(member) == 0) {
                functionCalls.push_back(*it);
                it = methodCalls.erase(it);
                continue;
            }
        }

        const nameResolution resolution = resolveName(it->getName()[3], unitLanguage, typeNameParsed, false, scopesAt(it->getPosition(), allFields, globals));
        noteUse(resolution, it->getPosition(), allFields);

        // Could be a normal method call on a field
        if (resolution.kind != variableKind::field) {
            if (unitLanguage != "C++") {
                // Could be a function call
                std::string inheritKeyword = (unitLanguage == "C#") ? "base" : "super";
                if (HELPERS::isSubstringAtBeginning(it->getName()[3], "this") || HELPERS::isSubstringAtBeginning(it->getName()[3], inheritKeyword)) {
                    functionCalls.push_back(*it);
                }

                // Could be a call on a local or a parameter
                else if (resolution.kind == variableKind::local || resolution.kind == variableKind::parameter) {
                    ++externalMethodCallsCount;
                }
                    
                    
                // It is a static call
                else {
                    externalCalls.push_back(*it);
                    ++externalFunctionCallsCount;
                }

                // Will need to be removed in any case
                it = methodCalls.erase(it);  
                
            }
            else {
                switch (cppCallReceiver(it->getName()[1], ownTypes)) {
                    case callReceiver::objectItself:
                        functionCalls.push_back(*it);
                        break;
                    case callReceiver::noObject:
                        externalCalls.push_back(*it);
                        ++externalFunctionCallsCount;
                        break;
                    case callReceiver::otherObject:
                        externalCalls.push_back(*it);
                        ++externalMethodCallsCount;
                        break;
                }
                it = methodCalls.erase(it);  
            }                  
        }
        // A call of a query method on a field, such as items.size(), reads the field, unless the expression writes through its result
        else if (tables->fieldQueries.isQuery(it->getName()[3], it->getArgumentCount(), unitLanguage) && !it->isResultWritten()) {
            it = methodCalls.erase(it);
        }
        // A call of a field, whose type tells the method that a call directly on the field calls, where -> reaches the object that a pointer field
        // points to, and otherwise calls the operator of the type of the field, as of an alias of a smart pointer
        else {
            static const variableModel undeclared;
            const auto declared = allFields.find(resolution.variable);
            const variableModel& field = declared != allFields.end() ? declared->second : undeclared;
            const std::string_view access = memberCallOperator(it->getName()[3], resolution.variable);
            const bool throughType = access == "->" && field.getPointeeType().empty();
            if (access == "->" && !throughType) it->setReceiverType(field.getPointeeType());
            else if (!access.empty())          it->setReceiverType(field.getNamedType());
            else if (field.getElementDepth() > 0 && elementAccesses(it->getName()[3], resolution.variable) == field.getElementDepth())
                it->setReceiverType(field.getElementType());
            it->setReceiverUnit(field.getUnitNumber());
            it->setThroughPointer(throughType);
            ++it;
        }
    }

    for (auto it = functionCalls.begin(); it != functionCalls.end();) {
        if (!readsThroughUnknownMethod(*it, type)) { ++it; continue; }
        fieldUsed = true;
        it = functionCalls.erase(it);
    }
}

// Tells whether a call without an object, whose name no method of the type or of its known ancestors has, names a method that the object
// inherits from a type outside the archive, which holds in a language without free functions for an accessor of the root class, and for
// any name in a type with a parent outside the archive, unless a static import outside the archive may hold a function of the name
// A static method, a method of a type that holds the type, a method that srcML misreads, and a variable that the code calls by its name are
// no such methods
//
bool functionModel::callsParentOutsideArchive(const callModel& call, const typeMembers& type, const globalVariables& globals) const {
    const std::string& called = call.getName()[3];
    if (!languageOf(unitLanguage).methodsOnly || !isPlainName(called) || type.holderFunctions.count(called) > 0) return false;
    if (tables->fieldQueries.isRootFunction(called, call.getArgumentCount(), unitLanguage)) return false;
    if (resolveName(called, unitLanguage, typeNameParsed, false, scopesAt(call.getPosition(), type.fields, globals)).kind != variableKind::unknown) return false;
    return tables->fieldQueries.isRootAccessor(called, call.getArgumentCount(), unitLanguage) || (type.unknownParent && !type.outsideStaticImport);
}

// Tells whether a call to the object itself reads the object through a method that no type of the archive defines or declares with the
// number of arguments of the call: the accessor that the language gives a component of a record, an accessor of the root class of the
// language, or a query method or a method called in a C++ const method that a parent outside the archive may define, unless the code
// writes through the result
//
bool functionModel::readsThroughUnknownMethod(const callModel& call, const typeMembers& type) const {
    const std::string& called = call.getName()[3];
    const std::size_t qualifierEnd = called.rfind("::");
    const std::string method = qualifierEnd == std::string::npos ? withoutSelfAccess(called) : called.substr(qualifierEnd + 2);
    if (call.isResultWritten() || (!isPlainName(method) && !namesOperatorFunction(method))) return false;

    // A C++ call that a parent qualifies, as Base::size(), starts at the parent, as a call through super or base does
    const bool throughParent = called.compare(0, 6, "super.") == 0 || called.compare(0, 5, "base.") == 0 ||
                               (qualifierEnd != std::string::npos && qualifyingType(called) != typeNameParsed);
    // In a language without overloads, a method of the name takes the call, whatever its arguments
    const argumentsByName& known = throughParent ? type.inheritedMethodArguments : type.methodArguments;
    if (languageOf(unitLanguage).overloads ? takes(known, method, call.getArgumentCount()) : known.count(method) > 0) return false;
    if (!throughParent && call.getArgumentCount() == 0 && type.implicitAccessors.count(method) > 0) return true;
    return tables->fieldQueries.isRootAccessor(method, call.getArgumentCount(), unitLanguage) ||
           (type.unknownParent && (methodConst || tables->fieldQueries.isQuery(method, call.getArgumentCount(), unitLanguage)));
}

// Gets stereotypes in one string
//
std::string functionModel::getStereotypesString() const {
    std::string stereotypesString;
    for (const std::string & s : stereotypes) {
        if (!stereotypesString.empty()) stereotypesString += " ";
        stereotypesString += s;
    }
    return stereotypesString;
}

// Gets specifiers in one string
//
std::string functionModel::getSpecifiersString() const {
    std::string specifierString;
    for (const std::string& s : specifiers) {
        if (!specifierString.empty()) specifierString += " ";
        specifierString += s;
    }
    return specifierString;
}

// Gets the return type without specifiers and whitespace, which the rules read as Boolean, void, or a value
// A container keeps its whole type, as List<Boolean>, since its elements leave it neither Boolean nor void
//
std::string functionModel::getReturnTypeParsed() const {
    const std::string declared = returnTypeWithoutMacros();
    std::string returnTypeString = declared;
    if (!tables->typeSpecifiers.namesContainer(declared, unitLanguage))
        tables->typeSpecifiers.removeSpecifiers(returnTypeString, unitLanguage);
    HELPERS::removeWhitespace(returnTypeString);

    // A C++ type of sign words alone, as unsigned, is an int, while signed and unsigned are specifiers of the other types
    if (returnTypeString.empty() && unitLanguage == "C++" && namesSign(declared)) return "int";
    return returnTypeString;
}

// Gets the attributes (C#) or the annotations (Java) of the function, separated by spaces, for the verbose report
//
std::string functionModel::getAttributesOrAnnotationsString() const {
    std::string attributesOrAnnotationsString;
    for (const std::string& a : attributesOrAnnotations) {
        if (!attributesOrAnnotationsString.empty()) attributesOrAnnotationsString += " ";
        attributesOrAnnotationsString += a;
    }
    return attributesOrAnnotationsString;
}

// Gets the signatures of the calls to other objects, or with all false of the calls to the methods of the type, separated by spaces,
// for the verbose report
//
std::string functionModel::getCallsString(bool all) const {
    std::string externalCallsString;
    if (all) {
        for (auto& f : externalCalls) {
            if (!externalCallsString.empty()) externalCallsString += " ";
            externalCallsString += f.getSignature();
        }
    }
    else {
        for (const auto& f : functionCalls) {
            if (!externalCallsString.empty()) externalCallsString += " ";
            externalCallsString += f.getSignature();
        }
    }
    return externalCallsString;
}