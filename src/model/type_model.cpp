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
#include "rules/language_tables.hpp"
#include "archive/node_text.hpp"
#include "archive/srcml_document.hpp"
#include "archive/srcml_elements.hpp"

#include <algorithm>
#include <cctype>
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

    // Gets the types of a signature without the names that qualify them, such as (string&,int) of (std::string&,int),
    // since a declaration and a definition may qualify a type apart, as after using namespace std;
    //
    std::string withoutQualifiers(const std::string& types) {
        std::string bare;
        std::size_t nameStart = 0;
        for (std::size_t i = 0; i < types.size(); ++i) {
            if (types.compare(i, 2, "::") == 0) {
                bare.erase(nameStart);
                ++i;
                continue;
            }
            if (!std::isalnum(static_cast<unsigned char>(types[i])) && types[i] != '_') nameStart = bare.size() + 1;
            bare += types[i];
        }
        return bare;
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

// Finds method data after all types and globals are collected
//
void typeModel::findDataAfterCollection(const globalVariables& globals) {
    typeMembers members;
    members.fields.insert(fields.begin(), fields.end());
    members.fields.insert(inheritedFields.begin(), inheritedFields.end());
    members.staticFields.insert(staticFields.begin(), staticFields.end());
    members.staticFields.insert(inheritedStaticFields.begin(), inheritedStaticFields.end());

    // A data member of the type or of an ancestor, static or not, hides a data member of the same name of an enclosing instance
    for (const auto& [fieldName, field] : enclosingFields)
        if (members.staticFields.count(fieldName) == 0) members.fields.insert({fieldName, field});
    members.staticFields.insert(enclosingStaticFields.begin(), enclosingStaticFields.end());

    for (const auto* signatures : {&methodSignatures, &inheritedMethodSignatures, &declMethodSignatures, &inheritedDeclMethodSignatures})
        for (const auto& item : *signatures) members.methodNames.insert(item.first);

    // A C++ call qualified with the name of the type or of a parent type, such as Base::draw(), calls a method of the object itself
    members.ownTypes.insert(name[3]);
    for (const auto& parent : parentNames)          members.ownTypes.insert(parent[3]);
    for (const auto& parent : inheritedParentNames) members.ownTypes.insert(parent[3]);
    members.staticHolders = members.ownTypes;
    members.staticHolders.insert(staticHolders.begin(), staticHolders.end());
    members.unknownParent = unknownParent;

    for (auto& method : methods) {
        method.findDataAfterCollection(members, globals);
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
    inheritedMethodSignatures.insert(parent.methodSignatures.begin(), parent.methodSignatures.end());
    inheritedMethodSignatures.insert(parent.inheritedMethodSignatures.begin(), parent.inheritedMethodSignatures.end());
    inheritedDeclMethodSignatures.insert(parent.declMethodSignatures.begin(), parent.declMethodSignatures.end());
    inheritedDeclMethodSignatures.insert(parent.inheritedDeclMethodSignatures.begin(), parent.inheritedDeclMethodSignatures.end());
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

    std::string previousType;
    std::string previousNamedType;
    std::string previousPointeeType;
    for (int i = 0; i < result.size(); ++i) {
        const xmlNode* declaration = result.node(i);
        const xmlNode* typeNode    = firstChildElement(declaration, "type");

        std::string fieldName = sourceOf(firstChildElement(declaration, "name"));
        HELPERS::removeBracketSuffix(fieldName); // Chop off [] for arrays, as in int a[] of C++ or Java

        std::string fieldType;
        std::string namedType;
        std::string pointeeType;
        if (typeNode && refersToPreviousType(typeNode)) {
            fieldType   = previousType;
            namedType   = previousNamedType;
            pointeeType = previousPointeeType;
        }
        else if (typeNode) {
            fieldType   = sourceOf(typeNode);
            namedType   = namedTypeOf(typeNode);
            pointeeType = pointeeTypeOf(typeNode);
        }
        else if (const xmlNode* memberTypeName = firstChildElement(declaration->parent, "name")) {
            fieldType = sourceOf(memberTypeName);
            namedType = fieldType;
            HELPERS::removeWhitespace(namedType);
        }
        previousType        = fieldType;
        previousNamedType   = namedType;
        previousPointeeType = pointeeType;

        variableModel field;
        field.setName(fieldName);
        field.setType(fieldType);
        field.setNamedType(namedType);
        field.setPointeeType(pointeeType);
        field.setUnitNumber(unitNumber);
        tables->primitiveTypes.isNonPrimitive(field, unitLanguage, name[3], tables->typeSpecifiers);
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
        functionModel method = functionModel(*tables, methodUnit, location, unitLanguage, name[3], "", "", HELPERS::extractFirstLineNumber(methodSrcml), false);
        if (!method.getName().empty()) {
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
        functionModel method = functionModel(*tables, methodUnit, location, unitLanguage, name[3], propertyReturnType, propertyName, HELPERS::extractFirstLineNumber(methodSrcml), true);
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
