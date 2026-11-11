// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file FunctionModel.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "function_model.hpp"
#include "helper_functions.hpp"
#include "interpolated_strings.hpp"
#include "language_tables.hpp"
#include "qualified_name.hpp"
#include "srcml_document.hpp"
#include "srcml_elements.hpp"

#include <algorithm>
#include <cctype>
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

    // Tells whether a C# parameter type carries the modifier in, which passes the argument as a read-only reference
    //
    bool hasInModifier(const xmlNode* type) {
        for (const xmlNode* specifier = firstChildElement(type, "specifier"); specifier; specifier = nextNamedSibling(specifier, "specifier"))
            if (sourceOf(specifier) == "in") return true;
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
        if (language == "C#" && hasReferenceModifier(type)) return true;
        if (language == "C#" && hasInModifier(type)) return false;

        const xmlNode* typeName = firstChildElement(type, "name");
        if (!typeName) return false;
        std::string name = sourceOf(typeName);
        HELPERS::removeWhitespace(name);
        if (declaredArray || name.find("[]") != std::string::npos) return true;

        name = withoutGenericArguments(name);
        HELPERS::removeNamespace(name, language, true);
        return !primitiveTypes.isPrimitive(name, language);
    }

    // Tells whether an operator element is the operator op
    //
    bool isOperator(const xmlNode* node, std::string_view op) {
        return isSrcElement(node, "operator") && sourceOf(node) == op;
    }

    // Tells whether an element is an operator that accesses a member, as . and -> do
    //
    bool isMemberAccess(const xmlNode* node) {
        return isOperator(node, ".") || isOperator(node, "->");
    }

    // Gets the first and the last name of the access chain that holds a written name, such as m_subs[1] and second of m_subs[1].second
    // srcML ends a name after an index, so the names of a chain such as f[i].x are siblings joined by access operators
    //
    std::pair<const xmlNode*, const xmlNode*> accessChain(const xmlNode* name) {
        const xmlNode* first = name;
        const xmlNode* last  = name;
        for (const xmlNode* op = xmlPreviousElementSibling(const_cast<xmlNode*>(first));
             isMemberAccess(op) && isSrcElement(xmlPreviousElementSibling(const_cast<xmlNode*>(op)), "name");
             op = xmlPreviousElementSibling(const_cast<xmlNode*>(first)))
            first = xmlPreviousElementSibling(const_cast<xmlNode*>(op));
        for (const xmlNode* op = xmlNextElementSibling(const_cast<xmlNode*>(last));
             isMemberAccess(op) && isSrcElement(xmlNextElementSibling(const_cast<xmlNode*>(op)), "name");
             op = xmlNextElementSibling(const_cast<xmlNode*>(last)))
            last = xmlNextElementSibling(const_cast<xmlNode*>(op));
        return {first, last};
    }

    // Gets the source code of the elements from the first to the last sibling of a chain
    //
    std::string chainSource(const xmlNode* first, const xmlNode* last) {
        std::string source = sourceOf(first);
        for (const xmlNode* node = first; node != last;) {
            node = xmlNextElementSibling(const_cast<xmlNode*>(node));
            source += sourceOf(node);
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

    // Tells whether an operator writes its operand, as =, +=, and ++ do, unlike == or <=
    //
    bool isAssignment(const xmlNode* node) {
        if (!isSrcElement(node, "operator")) return false;
        const std::string op = sourceOf(node);
        return op == "++" || op == "--" || (!op.empty() && op.back() == '=' && op != "==" && op != "!=" && op != "<=" && op != ">=");
    }

    // Tells whether an expression writes through the result of a call, as v.at(i) = x, v.data()[0] = x, v.front().y = x, and ++v.back() do
    //
    bool writesResult(const xmlNode* call) {
        const xmlNode* before = xmlPreviousElementSibling(const_cast<xmlNode*>(call));
        if (isOperator(before, "++") || isOperator(before, "--")) return true;

        // Indexes and members of the result, such as [0] or .y, lead to the place that the expression writes
        const xmlNode* after = xmlNextElementSibling(const_cast<xmlNode*>(call));
        while (isSrcElement(after, "index") || isSrcElement(after, "name") || isOperator(after, ".") || isOperator(after, "->"))
            after = xmlNextElementSibling(const_cast<xmlNode*>(after));
        return isAssignment(after);
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

    // Adds a call with its name, after the object that srcML marks apart from the call, if any, and the number of its arguments
    //
    void addCall(const xmlNode* callElement, const std::string& object, const std::string& unitLanguage, elementOrder& order, std::vector<callModel>& collected) {
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(callElement)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
            const xmlNode* argumentList = xmlNextElementSibling(const_cast<xmlNode*>(child));
            if (!isSrcElement(child, "name") || !isSrcElement(argumentList, "argument_list")) continue;

            callModel call;
            std::string callName = object + sourceOf(child);
            HELPERS::nameFilter(callName, call.getNameVector(), unitLanguage, false);
            const int arguments = countListItems(argumentList);
            call.setSignature(call.getName()[3] + HELPERS::signatureSlots(arguments));
            call.setArgumentCount(arguments);
            call.setPosition(order.position(child));
            call.setResultWritten(writesResult(callElement));
            collected.push_back(std::move(call));
        }
    }
}

// Collects the facts of a method or a free function, given the type and the name of the property that holds a C# accessor
//
functionModel::functionModel(const languageTables& tables_, const srcmlDocument& method, const std::optional<elementLocation>& location_, const std::string& unitLanguage_,
                             const std::string& typeNameParsed_, const std::string& propertyType, const std::string& propertyName_, int lineNumber_, bool isProperty_) :
                             tables{&tables_}, unitLanguage{unitLanguage_}, propertyName{propertyName_}, location{location_}, typeNameParsed{typeNameParsed_},
                             isProperty{isProperty_}, lineNumber{lineNumber_} {
    fileName = method.fileName(); // Get file name here since free functions do not have a type
    
    findName(method);
    if (name.empty()) return; // If there is no name, then it is not a valid method and we can skip the rest of the data collection

    findParameterList(method);
    findConstructorOrDestructorType(method);

    if (constructorOrDestructor.empty()) {
        if (unitLanguage == "C#" || unitLanguage == "Java") {
            findAttributesOrAnnotations(method);
        }
        if (unitLanguage == "C++") findConst(method);
        findSpecifiers(method);
        findReturnType(method, propertyType);
        findParameters(method, propertyType);

        // The positions of the elements tell which locals are in scope at each name
        elementOrder order;
        findLocalVariables(method, order);
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
        findExpressionAssignments(method, order);
        findNonCommentStatements(method);   
    }
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
void functionModel::findDataAfterCollection(const typeMembers& type, const globalVariables& globals) {
    if (constructorOrDestructor.empty()) {
        addStaticHolders(type.staticHolders);
        unknownParent = type.unknownParent;

        // Must only be called after findIgnorableCalls(). Will Filter the calls.
        findCallsOnFields(type.fields, type.methodNames, type.ownTypes, globals);

        // Must only be called after findNewAssignedVariables()
        findReturnedVariables(type.fields, globals, false);
        findVariablesInExpressions(type.fields, globals);
        findModifiedVariables(type.fields, type, globals, false);
    }
}

// Finds data for free functions, given the globals and the names of the C# structs and enums of the archive
//
void functionModel::findDataFreeFunctionAfterCollection(const globalVariables& globals, const std::unordered_set<std::string>& valueTypes) {
    if (constructorOrDestructor.empty()) {
        // A C# struct or enum of the archive passed by value is a copy of the value of the caller
        if (unitLanguage == "C#") {
            for (const auto& [parameterName, parameter] : parameters) {
                std::string typeName = withoutGenericArguments(parameter.getNamedType());
                HELPERS::removeNamespace(typeName, unitLanguage, true);
                if (referenceParameters.count(parameterName) == 0 && valueTypes.count(typeName) > 0) sharedParameters.erase(parameterName);
            }
        }

        // For free functions, we do not need to filter the calls like in findCallsOnFields()
        //  since all of the calls are external anyway
        for (const callModel& call : methodCalls) {
            const nameResolution receiver = resolveName(call.getName()[3], unitLanguage, typeNameParsed, false, scopesAt(call.getPosition(), parameters, globals));
            noteUse(receiver, call.getPosition(), parameters);
            if (tables->fieldQueries.isQuery(call.getName()[3], unitLanguage) && !call.isResultWritten()) continue;

            // A call other than a query method changes a parameter that shares the object of the caller, a global that is not immutable, or a static local
            const bool changesParameter = sharedParameters.count(receiver.variable) > 0 ||
                                          (sharedPointees.count(receiver.variable) > 0 && callsThroughPointer(call.getName()[1], receiver.variable));
            const bool changesGlobal    = receiver.kind == variableKind::global && !globals.isImmutable(unitLanguage, receiver.variable, unitNumber());
            if (receiver.kind == variableKind::parameter && changesParameter) parameterRefModified = true;
            else if (changesGlobal || isStaticLocal(receiver, call.getPosition())) globalOrStaticVariableModified = true;
        }

        findReturnedVariables(parameters, globals, true);
        findVariablesInExpressions(parameters, globals);
        findModifiedVariables(parameters, typeMembers{}, globals, true);
    }
}

// Finds attributes ( C# ) or annotations ( Java )
//
void functionModel::findAttributesOrAnnotations(const srcmlDocument& method) {
    const nodeSet result = method.select(query::methodAttributes);
    for (int i = 0; i < result.size(); i++) attributesOrAnnotations.push_back(result.src(i));
}

// Gets the method name
//
void functionModel::findName(const srcmlDocument& method) {
    const nodeSet result = method.select(query::methodName);

    if (result.size() > 0) { // srcML issue where it returns multiple results for the name
        std::string tempName = result.src(0); // Just pick the first
        HELPERS::nameFilter(tempName, name, unitLanguage, true);
    }
}

// Gets the method parameter list with the types of its parameters and the numbers of arguments that it takes
//
void functionModel::findParameterList(const srcmlDocument& method) {
    const nodeSet result = method.select(query::methodParameterList);
    if (result.size() > 0) {
        parametersList = result.src(0);
        parameterTypes = parameterTypesOf(result.node(0));
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
    if (isProperty) {
        returnType.setType(takesImplicitValue(name[0]) ? "void" : propertyType);
    }
    else {
        const nodeSet result = method.select(query::returnType);

        std::string returnTypeString;
        for (int i = 0; i < result.size(); i++) {
            returnTypeString += result.srcml(i); // srcml_unit_get_src() will not work since the result is already just source code due to text()
        }

        returnType.setType(returnTypeString);


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
            previousType   = sourceOf(type);
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
}

// Collects the parameters that have a name, each with its type
// In C++, parameters could have a type but no name (for backward compatibility)
// The set, init, add, and remove accessors of C# take the implicit parameter value, of the type of their property or event
//
void functionModel::findParameters(const srcmlDocument& method, const std::string& propertyType) {
    const nodeSet result = method.select(query::parameterDeclaration);

    for (int i = 0; i < result.size(); ++i) {
        const xmlNode* type = firstChildElement(result.node(i), "type");
        while (type && !isSrcElement(xmlNextElementSibling(const_cast<xmlNode*>(type)), "name")) type = nextNamedSibling(type, "type");
        if (!type) continue;

        // Chop off [] for arrays, as in int a[] of C++ or Java
        std::string parameterName = sourceOf(xmlNextElementSibling(const_cast<xmlNode*>(type)));
        const std::string declarator = parameterName;
        HELPERS::removeBracketSuffix(parameterName);
        if (parameterName != declarator) arrayParameters.insert(parameterName);
        if (unitLanguage == "C#" && hasReferenceModifier(type)) referenceParameters.insert(parameterName);
        if (sharesCallerObject(type, parameterName != declarator, unitLanguage, tables->primitiveTypes)) sharedParameters.insert(parameterName);
        if (unitLanguage == "C++" && pointsToSharedObject(type)) sharedPointees.insert(parameterName);

        variableModel parameter;
        parameter.setName(parameterName);
        parameter.setType(sourceOf(type));
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
    for (int i = 0; i < result.size(); ++i) returnExpressions.push_back({result.src(i), order.position(result.node(i))});

    newReturned = method.select(query::newReturnExpression).size() > 0;
}

// Collects the calls of each kind, constructor calls that use new included, with the name and the number of arguments of each call
// C++:
//   Constructor calls are a type of function calls (collected separately)
// C#:
//   A null-conditional call, as in o?.Dispose(), is a method call on the name before ?., as o.Dispose() is
//
void functionModel::findCalls(const srcmlDocument& method, elementOrder& order) {
    const std::pair<query, std::vector<callModel>*> callKinds[] = {
        {query::functionCall, &functionCalls}, {query::methodCall, &methodCalls}, {query::constructorCall, &newConstructorCalls}, {query::callOnResult, &callsOnResults}
    };
    for (const auto& [callQuery, collected] : callKinds) {
        const nodeSet result = method.select(callQuery);
        for (int i = 0; i < result.size(); ++i) addCall(result.node(i), "", unitLanguage, order, *collected);
    }

    if (unitLanguage == "C#") {
        const nodeSet result = method.select(query::nullConditionalCall);
        for (int i = 0; i < result.size(); ++i) {
            const xmlNode* ternary = result.node(i)->parent->parent->parent;
            addCall(result.node(i), sourceOf(firstChildElement(firstChildElement(firstChildElement(ternary, "condition"), "expr"), "name")) + ".", unitLanguage, order, methodCalls);
        }
    }
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

    if (result.size() > 0) { // srcML issue where it returns multiple results for the name
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

    const nodeSet result = method.select(query::expressionName);
    std::size_t nextString = 0;     // The first interpolated string that does not end before the name
    for (int i = 0; i < result.size(); i++) {
        const int position = order.position(result.node(i));
        while (nextString < interpolated.size() && interpolated[nextString].second <= position) ++nextString;
        if (nextString < interpolated.size() && interpolated[nextString].first <= position) continue;
        expressionNames.push_back({result.src(i), position});
    }
}

// Finds names of expressions
// All expressions with some assignment operator is collected. 
// Exception is made for C# where an assignment (e.g., Property = value) can be used in an initializer (e.g., var x = new Class { Property = value };), so only expressions with assignment that are not in an initializer are collected
// Each written name stands with its whole access chain, such as m_subs[1].second, whose first name tells the written variable
void functionModel::findExpressionAssignments(const srcmlDocument& method, elementOrder& order)  {
    const nodeSet result = method.select(query::expressionAssignment);
    for (int i = 0; i < result.size(); i++) {
        const auto [first, last] = accessChain(result.node(i));
        expressionAssignments.push_back({chainSource(first, last), order.position(first), isDereferenced(first, last)});
    }
}

// Finds if the return type, local types, and parameter types are non-primitive
//
void functionModel::findNonPrimitive() {
    // Return type
    if (tables->primitiveTypes.isNonPrimitive(returnType, unitLanguage, typeNameParsed, tables->typeSpecifiers)) {
        nonPrimitiveReturnType = true; 
        nonPrimitiveReturnTypeExternal = returnType.isNonPrimitiveExternal();
    }

    // Local types
    for (auto& local : locals) {               
        if (tables->primitiveTypes.isNonPrimitive(local.second.variable, unitLanguage, typeNameParsed, tables->typeSpecifiers) && local.second.variable.isNonPrimitiveExternal())
            nonPrimitiveLocalExternal = true;
    }

    // Parameter types
    for (auto& parameter : parameters) {
       if (tables->primitiveTypes.isNonPrimitive(parameter.second, unitLanguage, typeNameParsed, tables->typeSpecifiers) && parameter.second.isNonPrimitiveExternal())
            nonPrimitiveParamaterExternal = true;
    }
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
    return {
        [this, position](const std::string& variable) { return localAt(variable, position, true) != nullptr; },
        [this](const std::string& variable) { return parameters.find(variable) != parameters.end(); },
        [&fields](const std::string& variable) { return fields.find(variable) != fields.end(); },
        [this, position](const std::string& variable) { return localAt(variable, position, false) != nullptr; },
        [this, &globals, unit](const std::string& variable) {
            // A parent outside the archive may declare the name, unless the unit declares a global of the name
            const bool atNamespaceScope = unknownParent ? globals.declaredAtNamespaceScope(unit, variable) : globals.atNamespaceScope(unitLanguage, variable, unit);
            return atNamespaceScope ||
                   std::any_of(staticHolders.begin(), staticHolders.end(), [&](const std::string& holder) { return globals.holds(unitLanguage, holder, variable, unit); });
        },
        [this, &globals, unit](const std::string& holder, const std::string& variable) { return globals.holds(unitLanguage, holder, variable, unit); }
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
        fieldUsed = true;
        if (fields.at(resolution.variable).isNonPrimitiveExternal()) nonPrimitiveFieldExternal = true;
    }
    else if (resolution.kind == variableKind::global || isStaticLocal(resolution, position)) {
        globalUsed = true;
    }
}

// Tells whether a name refers to the object itself as a whole, as this, *this, and (*this) do, unlike this->a or this[i]
//
bool functionModel::refersToObject(const nameResolution& resolution) {
    return resolution.kind == variableKind::field && resolution.variable == "this" && !resolution.throughMember && !resolution.throughIndex;
}

// Records a variable created with new that a return expression returns
//
void functionModel::noteReturnedVariable(const nameResolution& resolution) {
    if (resolution.kind != variableKind::unknown && variablesCreatedWithNew.find(resolution.variable) != variablesCreatedWithNew.end())
        variableCreatedWithNewAndReturned = true;
}

// Tells whether a write to a parameter changes data of the caller
// A write changes the data of the caller through a C++ reference, a C# out or ref parameter, or a member, an element, or the target of the parameter,
//  such as p->b = v, a[i] = v, or *p = v. A write to the parameter itself, such as p = q or a = new int[n], changes only the copy of the function
// In C# and Java, the value of a non-primitive parameter is a reference to the object, so only a write to a member of the object changes the object
//
bool functionModel::changesCallerData(const std::string& para, const nameResolution& write) const {
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
                else if (!isParameterCheck && localAt(resolution.variable, written.position, false)->variable.isNonPrimitive() && writesThroughVariable(resolution))
                    nonPrimitiveLocalOrParameterModified = true;
                break;
            case variableKind::parameter: {
                const bool changesCaller = changesCallerData(resolution.variable, resolution);
                if (changesCaller) parameterRefModified = true;
                if (parameters.at(resolution.variable).isNonPrimitive() && (changesCaller || writesThroughVariable(resolution)))
                    nonPrimitiveLocalOrParameterModified = true;
                break;
            }
            case variableKind::field:
                if (isParameterCheck) break;
                // A write of the object itself, as *this = other, replaces every field
                if (refersToObject(resolution))                       objectReplaced = true;
                else if (checked.insert(resolution.variable).second) ++fieldsModifiedCount;
                break;
            case variableKind::global:
                if (isParameterCheck) globalOrStaticVariableModified = true;
                // The other class of a member that the write reaches makes a collaborator
                else if (resolution.otherQualifier && type.ownTypes.count(resolution.holder) == 0) nonPrimitiveFieldExternal = true;
                break;
            case variableKind::unknown: {
                // A free function may write a data member of a class outside the archive, as the definition of its method does
                if (resolution.variable.empty() || isParameterCheck) break;
                if (resolution.otherQualifier && type.ownTypes.count(resolution.variable) == 0) {
                    // The member of another object or class is no state of the object, while the other class makes a collaborator
                    nonPrimitiveFieldExternal = true;
                    break;
                }

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
void functionModel::findCallsOnFields(const std::unordered_map<std::string, variableModel>& allFields,
                                      const std::unordered_set<std::string>& allMethodNames, const std::unordered_set<std::string>& ownTypes,
                                      const globalVariables& globals) {
    // Check on function calls (Should be done before checking on method calls)
    // Notice we do not use parameters (i.e., signature) to check the function calls since some calls can use variable number of arguments with the 'params' keyword in C# or 'varargs' in Java and C++
    //  For example, foo(1, 2) can match void foo(params int[] numbers) in C# or void foo(int... numbers) in Java and C++
    // Or calls with default parameters in C++ where the arguments can be omitted. 
    //  For example, foo() can match void foo(int a=5) if the default value of a is provided
    // Therefore, we really only want to check that the function call name matches any of the overloaded methods (if any) and 
    //  this is sufficient to determine if the call is a method call or a static/free function call
    for (auto it = functionCalls.begin(); it != functionCalls.end();) {  
        if (allMethodNames.find(it->getName()[3]) == allMethodNames.end()) { 
            externalCalls.push_back(*it);
            ++externalFunctionCallsCount;
            it = functionCalls.erase(it);
        }
        else {
            ++it;
        }
    }  
    
    // A call on the result of a call or a cast is a call on another object
    for (const callModel& call : callsOnResults) {
        externalCalls.push_back(call);
        ++externalMethodCallsCount;
    }

    // Check on method calls
    for (auto it = methodCalls.begin(); it != methodCalls.end();) {
        if (unitLanguage == "Java" && callsMethodThroughSelf(it->getName()[3])) {
            functionCalls.push_back(*it);
            it = methodCalls.erase(it);
            continue;
        }

        const nameResolution resolution = resolveName(it->getName()[3], unitLanguage, typeNameParsed, false, scopesAt(it->getPosition(), allFields, globals));
        noteUse(resolution, it->getPosition(), allFields);

        // Could be a normal method call on a field
        if (resolution.kind != variableKind::field) {
            if (unitLanguage != "C++") {
                // Could be a function call
                std::string inheritKeyword = (unitLanguage == "C#") ? "base" : "super";
                if ((unitLanguage == "C#" || unitLanguage == "Java") && (HELPERS::isSubstringAtBeginning(it->getName()[3], "this") || HELPERS::isSubstringAtBeginning(it->getName()[3], inheritKeyword))) {
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
        else if (tables->fieldQueries.isQuery(it->getName()[3], unitLanguage) && !it->isResultWritten()) {
            it = methodCalls.erase(it);
        }
        // A call of a field, whose type tells the method that a call directly on the field calls, where -> reaches the object that a pointer field
        // points to, and otherwise calls the operator of the type of the field, as of an alias of a smart pointer
        else {
            const variableModel& field = allFields.at(resolution.variable);
            const std::string_view access = memberCallOperator(it->getName()[3], resolution.variable);
            const bool throughType = access == "->" && field.getPointeeType().empty();
            if (access == "->" && !throughType) it->setReceiverType(field.getPointeeType());
            else if (!access.empty())          it->setReceiverType(field.getNamedType());
            it->setReceiverUnit(field.getUnitNumber());
            it->setThroughPointer(throughType);
            ++it;
        }
    }
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

// Gets return type string
//
std::string functionModel::getReturnTypeParsed() const {
    std::string returnTypeString = returnType.getType();
    tables->typeSpecifiers.removeSpecifiers(returnTypeString, unitLanguage);  
    HELPERS::removeWhitespace(returnTypeString);
    return returnTypeString;
}


// Get return type string
//
std::string functionModel::getAttributesOrAnnotationsString() const {
    std::string attributesOrAnnotationsString;
    for (const std::string& a : attributesOrAnnotations) {
        if (!attributesOrAnnotationsString.empty()) attributesOrAnnotationsString += " ";
        attributesOrAnnotationsString += a;
    }
    return attributesOrAnnotationsString;
}

// Get return type string
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