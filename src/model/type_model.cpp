// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file typeModel.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/type_model.hpp"
#include "helper_functions.hpp"
#include "model/javascript_elements.hpp"
#include "model/python_elements.hpp"
#include "model/qualified_name.hpp"
#include "model/written_type.hpp"
#include "rules/language_tables.hpp"
#include "archive/node_text.hpp"
#include "archive/source_language.hpp"
#include "archive/srcml_document.hpp"
#include "archive/srcml_elements.hpp"

#include <algorithm>
#include <iterator>
#include <utility>

namespace {
    // Gets the name of a C# property with the parameter types of an indexer, such as Count or this(int), which tells it from the other properties of its type
    //
    std::string propertyNameOf(const xmlNode* property) {
        const xmlNode* nameElement = firstChildElement(property, "name");
        std::string name = nameElement ? sourceOf(nameElement) : "";
        HELPERS::removeWhitespace(name);
        if (const xmlNode* indexerParameters = firstChildElement(property, "parameter_list")) name += parameterTypesOf(indexerParameters);
        return name;
    }

    // Gets the name of the class that a type of a dynamic language names, such as Engine of Optional[Engine] or of Engine | undefined
    //
    std::string dynamicNamedType(const std::string& type, const std::string& unitLanguage) {
        return unitLanguage == "JavaScript" ? javascriptNamedType(type) : pythonNamedType(type);
    }

    // Adds the numbers of arguments that further methods take, each number once per name
    //
    void addArguments(argumentsByName& known, const argumentsByName& added) {
        for (const auto& [methodName, ranges] : added) {
            std::vector<argumentRange>& ofName = known[methodName];
            for (const argumentRange& range : ranges) {
                const bool held = std::any_of(ofName.begin(), ofName.end(), [&range](const argumentRange& other) {
                    return other.minimum == range.minimum && other.maximum == range.maximum && other.variadic == range.variadic;
                });
                if (!held) ofName.push_back(range);
            }
        }
    }
}

// The "this" keyword functions in most cases as "accessor" to the state of the type
// Therefore, it is added to the list of data members with the non-primitive type set to true since it always
//   refers to the type itself, which is a non-primitive type
// However, it is not a non-primitive of external type, so that is left as false
// The names of the types around a nested type qualify its name, such as Outer::Inner
//
typeModel::typeModel(const languageTables& tables_, const std::string& unitLanguage_, std::string qualifiedName) :
                     tables{&tables_}, unitLanguage{unitLanguage_} {
    HELPERS::nameFilter(qualifiedName, name, unitLanguage, true);

    variableModel variable;
    variable.setName("this");
    variable.setNonPrimitive(true);
    fields.insert({variable.getName(), variable});
}

// Finds other data for the type, which a scope of namespaces or a package holds
//
void typeModel::findData(const srcmlDocument& typeUnit, int unitNumber, std::vector<std::string> scope_) {
    locations.push_back({unitNumber, typeUnit.path()});
    partFileNames.push_back(typeUnit.fileName());
    scope = std::move(scope_);

    const std::string_view separator = unitLanguage == "C++" ? "::" : ".";
    for (const std::string& part : scope) scopedName += part + std::string(separator);
    for (const std::string& part : splitQualifiedName(name[1], unitLanguage)) scopedName += withoutGenericArguments(part) + std::string(separator);
    scopedName.erase(scopedName.size() - separator.size());

    if (unitLanguage == "C#") partial = typeUnit.select(query::partialType).size() > 0;

    findStructure(typeUnit);
    findParentNames(typeUnit);
    
    findFields(typeUnit, unitNumber);
    findStaticFields(typeUnit);

    findMethods(typeUnit, unitNumber);
    if (unitLanguage == "C#") findProperties(typeUnit, unitNumber);
    std::vector<std::pair<std::string, std::string>> methodSignaturesOrdered;
    findMethodDeclNames(typeUnit, methodSignaturesOrdered);
    findMethodDeclParameters(typeUnit, methodSignaturesOrdered);
    declMethodSignatures.insert(methodSignaturesOrdered.begin(), methodSignaturesOrdered.end());
    if (unitLanguage == "C++") findStaticMethodSignatures(typeUnit);

    // The object reaches a static or class method and a class in the body of its class of a dynamic language as it reaches a method
    const nodeSet staticMethods = typeUnit.select(query::staticMethodName);
    for (int i = 0; i < staticMethods.size(); ++i) staticMethodNames.insert(staticMethods.src(i));
    const nodeSet nestedTypes = typeUnit.select(query::nestedTypeName);
    for (int i = 0; i < nestedTypes.size(); ++i) nestedTypeNames.insert(nestedTypes.src(i));
    const nodeSet misread = typeUnit.select(query::misreadMethodName);
    for (int i = 0; i < misread.size(); ++i) misreadMethods.insert(withoutGenericArguments(misread.src(i)));

    // An alias of a pointer or a reference names no type that qualifies a member
    const nodeSet aliases = typeUnit.select(query::memberAliasName);
    for (int i = 0; i < aliases.size(); ++i) {
        const xmlNode* declaration = aliases.node(i)->parent;
        const xmlNode* type        = isSrcElement(declaration, "typedef") ? firstChildElement(declaration, "type")
                                                                          : xmlFirstElementChild(firstChildElement(declaration, "init"));
        if (!type || firstChildElement(type, "modifier") || isSrcElement(xmlPreviousElementSibling(aliases.node(i)), "modifier")) continue;
        const std::string target = withoutGenericArguments(namedTypeOf(type));
        if (!target.empty()) memberAliases.insert({aliases.src(i), splitQualifiedName(target, unitLanguage).back()});
    }
}

// Finds the signatures of the static methods that a C++ type declares, whose definitions outside the type are free functions
// The parameter types tell a static method from a non-static overload
//
void typeModel::findStaticMethodSignatures(const srcmlDocument& typeUnit) {
    const nodeSet result = typeUnit.select(query::staticDeclaration);
    for (int i = 0; i < result.size(); ++i) {
        std::string methodName = sourceOf(firstChildElement(result.node(i), "name"));
        HELPERS::removeWhitespace(methodName);
        staticMethodSignatures.insert({methodName, writtenTypesOf(firstChildElement(result.node(i), "parameter_list"))});
    }
}

// Gets the numbers of arguments that a method of the type takes, where the declaration in the type gives the default values
// that a C++ definition outside the type leaves out
//
argumentRange typeModel::argumentsOf(const functionModel& method) const {
    argumentRange range = method.getArguments();
    const auto declared = declaredArguments.find(method.getName()[3] + withoutQualifiers(method.getSignatureTypes()));
    if (declared != declaredArguments.end()) range.minimum = std::min(range.minimum, declared->second.minimum);
    return range;
}

// Gets the methods that the type declares without a method of the same name and parameter types among its methods, as abstract methods,
// by name
//
declaredMethods typeModel::undefinedMethods() const {
    declaredMethods undefined;
    for (const auto& [methodName, signature] : declMethodSignatures) {
        const std::string parameterTypes = withoutQualifiers(signature);
        const auto declared = declaredArguments.find(methodName + parameterTypes);
        if (declared == declaredArguments.end()) continue;

        const bool defined = std::any_of(methods.begin(), methods.end(), [&](const functionModel& method) {
            return !method.isInConstantBody() && method.getName()[3] == methodName && withoutQualifiers(method.getSignatureTypes()) == parameterTypes;
        });
        if (!defined) undefined[methodName].push_back({declared->second, parameterTypes});
    }
    return undefined;
}

// Finds method data after all types and globals are collected
//
void typeModel::findDataAfterCollection(const globalVariables& globals, const callableNames& archiveNames) {
    typeMembers members;
    members.fields.insert(fields.begin(), fields.end());
    members.fields.insert(inheritedFields.begin(), inheritedFields.end());
    members.staticFields.insert(staticFields.begin(), staticFields.end());
    members.staticFields.insert(inheritedStaticFields.begin(), inheritedStaticFields.end());

    // A data member of the type or of an ancestor, static or not, hides a data member of the same name of an enclosing instance
    for (const auto& [fieldName, field] : enclosingFields)
        if (members.staticFields.count(fieldName) == 0) members.fields.insert({fieldName, field});
    members.staticFields.insert(enclosingStaticFields.begin(), enclosingStaticFields.end());

    // The type of a data member of an ancestor or of an enclosing instance counts against this type, which may be that type itself
    for (auto& [fieldName, field] : members.fields) {
        if (fields.count(fieldName) > 0 || !field.isNonPrimitive()) continue;
        field.setNonPrimitiveExternal(false);
        tables->primitiveTypes.isNonPrimitive(field, unitLanguage, scopedName, tables->typeSpecifiers);
    }

    members.methodNames = methodNames();
    members.methodNames.insert(enclosingMethodNames.begin(), enclosingMethodNames.end());
    members.implicitAccessors = implicitAccessors();
    members.methodArguments = methodArguments;
    addArguments(members.methodArguments, inheritedMethodArguments);
    addArguments(members.methodArguments, enclosingMethodArguments);
    members.inheritedMethodArguments = inheritedMethodArguments;
    members.staticMethodNames.insert(staticMethodNames.begin(), staticMethodNames.end());
    members.staticMethodNames.insert(inheritedStaticMethodNames.begin(), inheritedStaticMethodNames.end());
    members.nestedTypeNames.insert(nestedTypeNames.begin(), nestedTypeNames.end());
    members.nestedTypeNames.insert(inheritedNestedTypeNames.begin(), inheritedNestedTypeNames.end());

    // A C++ call qualified with the name of the type or of a parent type, such as Base::draw(), calls a method of the object itself
    members.ownTypes.insert(name[3]);
    for (const auto& parent : parentNames)          members.ownTypes.insert(parent[3]);
    for (const auto& parent : inheritedParentNames) members.ownTypes.insert(parent[3]);
    for (const auto& [alias, target] : memberAliases)
        if (members.ownTypes.count(target) > 0) parentAliases.insert({alias, target});
    for (const auto& alias : parentAliases) members.ownTypes.insert(alias.first);
    if (!languageOf(unitLanguage).rootClass.empty()) members.ownTypes.insert(std::string(languageOf(unitLanguage).rootClass));
    members.unknownParent = unknownParent;
    members.holderFunctions = holderFunctions;
    members.outsideStaticImport = outsideStaticImport;

    // A method of a dynamic language reaches a variable of a class only through the object or the name of the class
    if (!languageOf(unitLanguage).dynamic) {
        members.staticHolders = members.ownTypes;
        members.staticHolders.insert(staticHolders.begin(), staticHolders.end());
    }

    for (auto& method : methods) {
        method.findDataAfterCollection(members, globals, archiveNames);
    }
}

// Collects the signatures of the methods, which tell the names that a call may name, and the numbers of arguments that the methods take
// A property of a dynamic language is a data member for the code that names it, so it gives no method name
//
void typeModel::buildMethodSignature() {
    const bool dynamic = languageOf(unitLanguage).dynamic;
    for (const auto& [methodName, declared] : undefinedMethods())
        for (const declaredMethod& method : declared) addArguments(methodArguments, {{methodName, {method.arguments}}});
    for (const functionModel& method : methods) {
        if (dynamic && !method.getPropertyName().empty()) continue;
        methodSignatures.insert(method.getNameSignature());
        addArguments(methodArguments, {{method.getName()[3], {argumentsOf(method)}}});
    }
}

// Tells whether the type itself declares or defines a method of a name, which is no static method
//
bool typeModel::namesOwnMethod(const std::string& methodName) const {
    const auto named = [&methodName](const std::pair<std::string, std::string>& signature) { return signature.first == methodName; };
    return std::any_of(methodSignatures.begin(), methodSignatures.end(), named) ||
           std::any_of(declMethodSignatures.begin(), declMethodSignatures.end(), named);
}

// Adds the types of static imports, whose static members the methods of the type reach without the names of the types
//
void typeModel::addStaticImports(const std::unordered_set<std::string>& imports) {
    staticImports.insert(imports.begin(), imports.end());
    staticHolders.insert(imports.begin(), imports.end());
}

// Sets the names of the functions that the own types and the static holders of the type hold, and whether a static import names a type outside the archive
//
void typeModel::setHolderFunctions(std::unordered_set<std::string> names, bool outsideImport) {
    holderFunctions     = std::move(names);
    outsideStaticImport = outsideImport;
}

// Gets the names of the methods that the type declares, defines, or inherits, after the methods of the parent types are known
//
std::unordered_set<std::string> typeModel::methodNames() const {
    std::unordered_set<std::string> names = componentNames;
    for (const auto* signatures : {&methodSignatures, &inheritedMethodSignatures, &declMethodSignatures, &inheritedDeclMethodSignatures})
        for (const auto& item : *signatures) names.insert(item.first);
    return names;
}

// Gets the components of a record whose accessors the record does not define, which the language gives the record as methods
// without parameters that return the component
//
std::unordered_set<std::string> typeModel::implicitAccessors() const {
    std::unordered_set<std::string> accessors = componentNames;
    for (const functionModel& method : methods) {
        const argumentRange arguments = argumentsOf(method);
        if (arguments.maximum == 0 && !arguments.variadic) accessors.erase(method.getName()[3]);
    }
    return accessors;
}

// Finds the objects that the methods of the type create by a call, given the names of the classes and the free functions of the archive
// in the language of the type and the globals of the archive
//
void typeModel::findCreatedObjects(const callableNames& archiveNames, const globalVariables& globals) {
    std::unordered_set<std::string> names = methodNames();
    names.insert(staticMethodNames.begin(), staticMethodNames.end());
    names.insert(inheritedStaticMethodNames.begin(), inheritedStaticMethodNames.end());

    std::unordered_set<std::string> variableNames = staticFields;
    variableNames.insert(inheritedStaticFields.begin(), inheritedStaticFields.end());
    for (const auto* members : {&fields, &inheritedFields})
        for (const auto& member : *members) variableNames.insert(member.first);

    for (functionModel& method : methods) method.findCreatedObjects({archiveNames, globals, names, variableNames});
}

// Adds the data members that the methods of a type of a dynamic language declare: a member that a method writes through the object,
// with the type that its first write with a known type gives it, and a property, with the return type that its method declares
// A write of a created object gives the class of the object, which the names of the classes and the free functions of the archive
// and its globals tell
// The name of a property is no name of a method, since no call names it
//
void typeModel::declareMethodMembers(const callableNames& archiveNames, const globalVariables& globals) {
    // The members of the parent types are not known yet, so only a method or a variable of the type itself tells what a call through the object names
    std::unordered_set<std::string> ownMethodNames = staticMethodNames;
    std::unordered_set<std::string> ownVariableNames = staticFields;
    for (const auto& field : fields) ownVariableNames.insert(field.first);
    for (const functionModel& method : methods) {
        if (method.getPropertyName().empty()) ownMethodNames.insert(method.getName()[3]);
        for (const memberWrite& write : method.getWrittenMembers()) ownVariableNames.insert(write.name);
    }

    const auto declare = [this](const std::string& memberName, const std::string& type) {
        variableModel& field = fields.try_emplace(memberName).first->second;
        if (!field.getType().empty() || (field.getName() == memberName && type.empty())) return;

        field.setName(memberName);
        field.setType(type);
        field.setNamedType(dynamicNamedType(type, unitLanguage));
        field.setUnitNumber(locations.front().unitNumber);
        tables->primitiveTypes.isNonPrimitive(field, unitLanguage, scopedName, tables->typeSpecifiers);
    };

    const creationNames creation{archiveNames, globals, ownMethodNames, ownVariableNames};
    for (const functionModel& method : methods) {
        if (!method.getPropertyName().empty()) declare(method.getPropertyName(), method.hasDeclaredReturnType() ? method.getReturnType().getType() : "");
        for (const memberWrite& write : method.getWrittenMembers())
            declare(write.name, !write.type.empty() ? write.type : method.createdClass(write.callName, creation).value_or(""));
    }
}

// Merges another part of a C# partial type into this one
//
void typeModel::mergeData(typeModel& other) {
    methods.insert(methods.end(), std::make_move_iterator(other.methods.begin()), std::make_move_iterator(other.methods.end()));
    parentNames.insert(parentNames.end(), other.parentNames.begin(), other.parentNames.end());
    for (std::size_t part : other.parentParts) parentParts.push_back(locations.size() + part);
    methodSignatures.insert(other.methodSignatures.begin(), other.methodSignatures.end());
    declMethodSignatures.insert(other.declMethodSignatures.begin(), other.declMethodSignatures.end());
    declaredArguments.insert(other.declaredArguments.begin(), other.declaredArguments.end());
    fields.insert(other.fields.begin(), other.fields.end());
    staticFields.insert(other.staticFields.begin(), other.staticFields.end());
    staticHolders.insert(other.staticHolders.begin(), other.staticHolders.end());
    staticImports.insert(other.staticImports.begin(), other.staticImports.end());
    misreadMethods.insert(other.misreadMethods.begin(), other.misreadMethods.end());
    locations.insert(locations.end(), other.locations.begin(), other.locations.end());
    partFileNames.insert(partFileNames.end(), other.partFileNames.begin(), other.partFileNames.end());
}

// Adds the data members, method signatures, and parents of a parent type, which already holds those of its own parents
//
void typeModel::appendInheritedDataMembers(const typeModel& parent) {
    inheritedFields.insert(parent.fields.begin(), parent.fields.end());
    inheritedFields.insert(parent.inheritedFields.begin(), parent.inheritedFields.end());
    inheritedStaticFields.insert(parent.staticFields.begin(), parent.staticFields.end());
    inheritedStaticFields.insert(parent.inheritedStaticFields.begin(), parent.inheritedStaticFields.end());
    inheritedStaticMethodNames.insert(parent.staticMethodNames.begin(), parent.staticMethodNames.end());
    inheritedStaticMethodNames.insert(parent.inheritedStaticMethodNames.begin(), parent.inheritedStaticMethodNames.end());
    inheritedNestedTypeNames.insert(parent.nestedTypeNames.begin(), parent.nestedTypeNames.end());
    inheritedNestedTypeNames.insert(parent.inheritedNestedTypeNames.begin(), parent.inheritedNestedTypeNames.end());
    memberAliases.insert(parent.memberAliases.begin(), parent.memberAliases.end());
    inheritedMethodSignatures.insert(parent.methodSignatures.begin(), parent.methodSignatures.end());
    inheritedMethodSignatures.insert(parent.inheritedMethodSignatures.begin(), parent.inheritedMethodSignatures.end());
    inheritedDeclMethodSignatures.insert(parent.declMethodSignatures.begin(), parent.declMethodSignatures.end());
    inheritedDeclMethodSignatures.insert(parent.inheritedDeclMethodSignatures.begin(), parent.inheritedDeclMethodSignatures.end());
    addArguments(inheritedMethodArguments, parent.methodArguments);
    addArguments(inheritedMethodArguments, parent.inheritedMethodArguments);
    for (const auto* names : {&parent.parentNames, &parent.inheritedParentNames}) {
        for (const std::vector<std::string>& parentName : *names)
            if (std::find(inheritedParentNames.begin(), inheritedParentNames.end(), parentName) == inheritedParentNames.end())
                inheritedParentNames.push_back(parentName);
    }
    unknownParent = unknownParent || parent.unknownParent;
}

// Adds the data members that an object of a Java inner class reaches through its enclosing instance: those of the enclosing type,
// of its known ancestors, and of its own enclosing instances, where a nearer data member hides a farther one of the same name
//
void typeModel::appendEnclosingDataMembers(const typeModel& enclosing) {
    enclosingFields.insert(enclosing.fields.begin(), enclosing.fields.end());
    enclosingFields.insert(enclosing.inheritedFields.begin(), enclosing.inheritedFields.end());
    enclosingStaticFields.insert(enclosing.staticFields.begin(), enclosing.staticFields.end());
    enclosingStaticFields.insert(enclosing.inheritedStaticFields.begin(), enclosing.inheritedStaticFields.end());
    for (const auto& [fieldName, field] : enclosing.enclosingFields)
        if (enclosingStaticFields.count(fieldName) == 0) enclosingFields.insert({fieldName, field});
    enclosingStaticFields.insert(enclosing.enclosingStaticFields.begin(), enclosing.enclosingStaticFields.end());

    // The object reaches the methods of its enclosing instances as well
    enclosingMethodNames = enclosing.methodNames();
    enclosingMethodNames.insert(enclosing.enclosingMethodNames.begin(), enclosing.enclosingMethodNames.end());
    for (const argumentsByName* known : {&enclosing.methodArguments, &enclosing.inheritedMethodArguments, &enclosing.enclosingMethodArguments})
        addArguments(enclosingMethodArguments, *known);
}

// Finds structure (e.g., class)
//
void typeModel::findStructure(const srcmlDocument& typeUnit) {
    const nodeSet result = typeUnit.select(query::structureType);

    std::string text;
    for (int i = 0; i < result.size(); i++) {
        text += result.srcml(i);
    }
    structure = HELPERS::typeStructure(text);
}

// Finds parent types
// C++:
//  Supports multiple inheritance and can use the public, private, and protected specifiers to control inheritance
//   It is private by default if nothing is specified for a class and public by default for a struct
//  Classes and structs can inherit from each other
//  Unions cannot inherit
//  Uses ":" for inheritance
// C#:
//  Inheritance is always public
//  Support single inheritance from other classes and multiple inheritance from interfaces
//  Interfaces can't inherit from classes or structs
//  Structs can't inherit from other structs or classes, but can inherit from interfaces
//  Uses ":" for inheritance
// Java:
//  Inheritance is always public
//  Support single inheritance from other classes and multiple inheritance from interfaces
//  Java interfaces can't inherit from classes
//  Uses 'extends' for class-to-class and interface-to-interface inheritance and 'implements' for class-to-interface inheritance
// 
void typeModel::findParentNames(const srcmlDocument& typeUnit) { 
    const nodeSet result = typeUnit.select(query::parentName);
    
    for (int i = 0; i < result.size(); i++) {
        std::string parentName = result.src(i);

        // A Python parent is looked up by its class, without the modules before it and the arguments in brackets after it, such as Base of pkg.Base[T],
        // and every class has the parent object
        if (unitLanguage == "Python") {
            HELPERS::removeWhitespace(parentName);
            HELPERS::removeBracketSuffix(parentName);
            parentName = splitQualifiedName(parentName, unitLanguage).back();
            if (parentName == languageOf(unitLanguage).rootClass) continue;
        }

        std::vector<std::string> parentNameVector;
        HELPERS::nameFilter(parentName, parentNameVector, unitLanguage, true);
        parentNames.push_back(std::move(parentNameVector));
        parentParts.push_back(0);
    }
}

// Finds the fields with their names and types, in the order of the type
// A declaration declares a field where its name follows its type
// C++:
//  This does not count unions without a name, so their fields will be collected even if nested within another type
//  Static fields are ignored and treated as globals
//  A declarator after the definition of a member type declares a field of that type, and the type of an anonymous member type has no name
// C#:
//  Auto-properties can be used to declare fields implicitly so they are treated as fields
//  Regular properties can be used to get or set fields
//   Therefore, regular properties will be treated like normal fields as they are used (most of the time) to get or set regular fields 
//    where property name = field name and where property type = field type
//   However, the assumption here is that their usage is assumed to be getting or setting a single field
//  Static fields or auto-properties are ignored and treated as globals
//  Const fields are ignored as they are implicitly static and thus treated as globals
//  Auto-properties cannot be const
// Java:
//  Static fields are ignored and treated as globals, as are the fields of interfaces, which are implicitly static
//  The components of a record are its fields
//
void typeModel::findFields(const srcmlDocument& typeUnit, int unitNumber) {
    const nodeSet result = typeUnit.select(query::fieldDeclaration);

    // The body of a Python class declares a data member by a name with an annotation or by a string of __slots__, and the body of a JavaScript
    // class by a declaration, by a name alone, as #count;, or by a parameter property of its constructor, with the type of its annotation or of
    // the object that its initializer creates
    if (languageOf(unitLanguage).dynamic) {
        for (int i = 0; i < result.size(); ++i) {
            const xmlNode* node = result.node(i);
            std::string fieldName;
            std::string type;
            if (unitLanguage == "Python") {
                const bool slot = isSrcElement(node, "literal");
                fieldName = result.src(i);
                if (slot) fieldName.erase(std::remove_if(fieldName.begin(), fieldName.end(), [](char c) { return c == '"' || c == '\''; }), fieldName.end());
                type = slot ? "" : pythonBoundValue(node).type;
            }
            else {
                const bool declaration = isSrcElement(node, "decl");
                fieldName = sourceOf(declaration ? firstChildElement(node, "name") : node);
                type      = declaration ? javascriptDeclaredType(node) : "";
            }

            variableModel field;
            field.setName(fieldName);
            field.setType(type);
            field.setNamedType(dynamicNamedType(type, unitLanguage));
            field.setUnitNumber(unitNumber);
            tables->primitiveTypes.isNonPrimitive(field, unitLanguage, scopedName, tables->typeSpecifiers);
            fields.insert({fieldName, std::move(field)});
        }
        return;
    }

    std::string previousType;
    std::string previousNamedType;
    std::string previousPointeeType;
    const xmlNode* previousTypeNode = nullptr;
    for (int i = 0; i < result.size(); ++i) {
        const xmlNode* declaration = result.node(i);
        const xmlNode* typeNode    = firstChildElement(declaration, "type");

        std::string fieldName = sourceOf(firstChildElement(declaration, "name"));
        HELPERS::removeBracketSuffix(fieldName); // Chop off [] for arrays, as in int a[] of C++ or Java

        std::string fieldType;
        std::string namedType;
        std::string pointeeType;
        // A data member that points to a function, as bool (*filter)(int);, has the type of the pointer
        if (isSrcElement(declaration, "function_decl")) {
            variableModel field;
            field.setName(fieldName);
            field.setType(functionPointerTypeOf(declaration));
            field.setUnitNumber(unitNumber);
            tables->primitiveTypes.isNonPrimitive(field, unitLanguage, scopedName, tables->typeSpecifiers);
            fields.insert({fieldName, std::move(field)});
            continue;
        }

        // A C# event without accessors, whose markup holds its type and its name as the two names before its initializer
        if (isSrcElement(declaration, "expr") || isSrcElement(declaration, "event")) {
            const xmlNode* eventType = typeNode;
            const xmlNode* eventName = nullptr;
            for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(declaration)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
                if (isSrcElement(child, "operator") && sourceOf(child) == "=") break;
                if (!isSrcElement(child, "name")) continue;
                if (!eventType) eventType = child;
                else            eventName = child;
            }
            if (!eventName) continue;

            std::string eventNamedType = sourceOf(eventType);
            HELPERS::removeWhitespace(eventNamedType);

            variableModel field;
            field.setName(sourceOf(eventName));
            field.setType(sourceOf(eventType));
            field.setNamedType(eventNamedType);
            field.setUnitNumber(unitNumber);
            field.setEvent(true);
            tables->primitiveTypes.isNonPrimitive(field, unitLanguage, scopedName, tables->typeSpecifiers);
            fields.insert({field.getName(), std::move(field)});
            continue;
        }

        const xmlNode* declaredType = nullptr;
        if (typeNode && refersToPreviousType(typeNode)) {
            fieldType   = previousType;
            namedType   = previousNamedType;
            pointeeType = previousPointeeType;
            declaredType = previousTypeNode;
        }
        else if (typeNode) {
            fieldType   = typeSourceOf(typeNode);
            namedType   = namedTypeOf(typeNode);
            pointeeType = pointeeTypeOf(typeNode);
            declaredType = typeNode;
        }
        else if (const xmlNode* memberTypeName = firstChildElement(declaration->parent, "name")) {
            fieldType = sourceOf(memberTypeName);
            namedType = fieldType;
            HELPERS::removeWhitespace(namedType);
        }
        previousType        = fieldType;
        previousNamedType   = namedType;
        previousPointeeType = pointeeType;
        previousTypeNode    = declaredType;

        variableModel field;
        field.setName(fieldName);
        field.setType(fieldType);
        field.setNamedType(namedType);
        field.setPointeeType(pointeeType);
        if (declaredType) {
            const elementType element = elementTypeOf(declaredType, firstChildElement(declaration, "name"));
            field.setElementType(element.name, element.depth);
        }
        field.setUnitNumber(unitNumber);
        tables->primitiveTypes.isNonPrimitive(field, unitLanguage, scopedName, tables->typeSpecifiers);
        if (isSrcElement(declaration->parent, "parameter")) componentNames.insert(fieldName);
        fields.insert({fieldName, std::move(field)});
    }
}

// Finds the names of the static fields, which are globals and thus no state of an object, as the static properties of C# are
//
void typeModel::findStaticFields(const srcmlDocument& typeUnit) {
    const nodeSet result = typeUnit.select(query::staticFieldName);
    for (int i = 0; i < result.size(); ++i) {
        std::string fieldName = result.src(i);
        HELPERS::removeBracketSuffix(fieldName); // Chop off [] for arrays, as in static int a[] of C++ or Java
        staticFields.insert(std::move(fieldName));
    }
}

// Finds the names of the methods that the type declares without a body, which calls of its methods and of its children reach
// C++ declares most methods in the class and defines them outside it, while C# and Java declare abstract and partial methods
// Static and friend declarations name free functions instead
//
void typeModel::findMethodDeclNames(const srcmlDocument& typeUnit, std::vector<std::pair<std::string, std::string>>& methodSignaturesOrdered) {
    const nodeSet result = typeUnit.select(query::functionDeclName);

    for (int i = 0; i < result.size(); ++i) {
        std::string methodName = result.src(i);
        std::vector<std::string> methodNameVector;
        HELPERS::nameFilter(methodName, methodNameVector, unitLanguage, true);
        methodSignaturesOrdered.push_back({methodNameVector[3], ""});
    }
}

// Finds the parameter lists of the method declarations to get the full signatures and the numbers of arguments that the declarations take
//
void typeModel::findMethodDeclParameters(const srcmlDocument& typeUnit, std::vector<std::pair<std::string, std::string>>& methodSignaturesOrdered) {
    const nodeSet result = typeUnit.select(query::functionDeclParameters);

    for (int i = 0; i < result.size(); ++i) {
        std::pair<std::string, std::string>& signature = methodSignaturesOrdered.at(i);
        signature.second = signatureTypesOf(result.node(i));
        declaredArguments.insert({signature.first + withoutQualifiers(signature.second), argumentRangeOf(result.node(i))});
    }
}


// Finds methods defined inside the type
// C#:
//   Nested local functions within methods in C# are ignored 
//   Attributes do not read/write to the state object. However, they can have expressions, decl_stmt, etc
//    Therefore, we ignore them when collecting data
//   Partial methods in partial classes may or may not have an implementation
//    If they do, we collect them as normal methods. Else, they are ignored since they are not implemented and thus cannot be called
// Java:
//   Annotations have the same story as attributes, so we ignore these statements inside them
//   
void typeModel::findMethods(const srcmlDocument& typeUnit, int unitNumber) {
    const nodeSet result = typeUnit.select(query::method);

    for (int i = 0; i < result.size(); ++i) {
        const std::string methodSrcml = result.srcml(i);
        const srcmlDocument methodUnit = typeUnit.wrap(result.node(i));

        const elementLocation location{unitNumber, methodUnit.path()};
        functionModel method = functionModel(*tables, methodUnit, location, unitLanguage, scopedName, "", "", HELPERS::extractFirstLineNumber(methodSrcml), false);
        if (!method.getName().empty()) {
            // srcML marks an enum constant with a body as a declaration with a block
            const xmlNode* owner = result.node(i)->parent;
            method.setInConstantBody(isSrcElement(owner, "block") && isSrcElement(owner->parent, "decl"));
            methods.push_back(std::move(method));
        }
    }
}

// Properties need to be collected separately since they hold the return type of the getters
// Properties cannot be nested in methods or in other properties
//
void typeModel::findProperties(const srcmlDocument& typeUnit, int unitNumber) {
    const nodeSet result = typeUnit.select(query::property);
    
    for (int i = 0; i < result.size(); ++i) {
        const srcmlDocument propertyUnit = typeUnit.wrap(result.node(i));

        std::string propertyReturnType = findPropertyReturnType(propertyUnit);
        findMethodsInProperty(propertyUnit, propertyReturnType, propertyNameOf(result.node(i)), unitNumber);
    }
}

// Finds the return type of a property
//
std::string typeModel::findPropertyReturnType(const srcmlDocument& propertyUnit) {
    const nodeSet result = propertyUnit.select(query::propertyType);

    // srcML v1.1 has parsing issues with certain properties, so we return an empty string if the return type is not found
    if (result.size() == 0) return "";
    return unparsedSourceOf(result.node(0));
}

// Finds the methods in a property
//
void typeModel::findMethodsInProperty(const srcmlDocument& propertyUnit, const std::string& propertyReturnType, const std::string& propertyName, int unitNumber) {
    const nodeSet result = propertyUnit.select(query::propertyMethod);

    for (int i = 0; i < result.size(); ++i) {
        const std::string methodSrcml = result.srcml(i);
        const srcmlDocument methodUnit = propertyUnit.wrap(result.node(i));

        const elementLocation location{unitNumber, methodUnit.path()};
        functionModel method = functionModel(*tables, methodUnit, location, unitLanguage, scopedName, propertyReturnType, propertyName, HELPERS::extractFirstLineNumber(methodSrcml), true);
        if (!method.getName().empty()) {
            methods.push_back(std::move(method));
        }
    }
}

// Gets the type stereotypes as a single string
//
std::string typeModel::getStereotypesString() const {
    std::string stereotypesString;
    for (const std::string & s : stereotypes) {
        if (!stereotypesString.empty()) stereotypesString += " ";
        stereotypesString += s;
    }
    return stereotypesString;
}

// Gets the parent types as a single string
//
std::string typeModel::getParentsString(bool isInherited) const {
    std::string parentsString;
    if (!isInherited) {
        for (const auto& s : parentNames) {
            if (!parentsString.empty()) parentsString += " ";
            parentsString += s[3];
        }
    }
    else {
        for (const auto& s : inheritedParentNames) {
            if (!parentsString.empty()) parentsString += " ";
            parentsString += s[3];
        }
    }
    return parentsString;
}



// Gets the inherited method signatures and inherited method declaration signatures as a single string
//
std::string typeModel::getInheritedMethodSignaturesString() const {
    std::string methodsignature;
    for (const std::pair<std::string, std::string> &s : inheritedMethodSignatures) {
        if (!methodsignature.empty()) methodsignature += " ";
        methodsignature += s.first + s.second;
    }
    for (const std::pair<std::string, std::string> &s : inheritedDeclMethodSignatures) {
        if (!methodsignature.empty()) methodsignature += " ";
        methodsignature += s.first + s.second;
    }
    return methodsignature;
}
