// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file typeModel.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "type_model.hpp"
#include "helper_functions.hpp"
#include "xpath_generator.hpp"
#include "primitives.hpp"

#include <srcml.h>

extern XPathGenerator                  XPATH_GENERATOR;
extern primitives                      PRIMITIVES;
extern srcml_unit*                     typeUnit;
extern srcml_archive*                  typeArchive;
extern srcml_unit*                     methodUnit;
extern srcml_archive*                  methodArchive;

srcml_unit*                            propertyUnit{nullptr};
srcml_archive*                         propertyArchive{nullptr};

// The "this" keyword functions in most cases as "accessor" to the state of the type
// Therefore, it is added to the list of data members with the non-primitive type set to true since it always
//   refers to the type itself, which is a non-primitive type
// However, it is not a non-primitive of external type, so that is left as false
//
typeModel::typeModel(const std::string& unitLanguage_) : unitLanguage{unitLanguage_} {
    findName();

    variableModel variable;
    variable.setName("this");
    variable.setNonPrimitive(true);
    fields.insert({variable.getName(), variable});
}

// Finds other data for the type
//
void typeModel::findData(const std::string& typeXpath, const std::string& header, int unitNumber) {
    xpath[unitNumber].push_back(typeXpath);

    findStructure();
    findParentNames();
    
    std::vector<variableModel> fieldsOrdered;
    findFieldNames(fieldsOrdered);
    findFieldTypes(fieldsOrdered);
     
    findMethod(typeXpath, header, unitNumber);
    if (unitLanguage == "C#" || unitLanguage == "Java") {
        findAttributesOrAnnotations();
        if (unitLanguage == "C#") findProperties(typeXpath, header, unitNumber);
    }
}

// Finds method data after all types are collected
//
void typeModel::findDataAfterCollection() {
    std::unordered_map<std::string, variableModel> allFields(fields.begin(), fields.end());
    allFields.insert(inheritedFields.begin(), inheritedFields.end());

    std::unordered_set<std::string> allMethodSignatures(methodSignatures.begin(), methodSignatures.end());
    allMethodSignatures.insert(inheritedMethodSignatures.begin(), inheritedMethodSignatures.end());

    for (auto& method : methods) method.findDataAfterCollection(allFields, allMethodSignatures);  
}

// Finds structure (e.g., class)
//
void typeModel::findStructure() {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"structure_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; i++) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, i);
        structure += srcml_unit_get_srcml(resultUnit);
    }
    helperFunctions::removeWhitespace(structure);

    // Structures in C++ get the semi-colon at the end, so we need to remove it ( class; )
    if (unitLanguage == "C++" && structure.back() ==';') structure.pop_back();

    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result);
}

// Finds attributes ( C# ) or annotations ( Java )
//
void typeModel::findAttributesOrAnnotations() {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"type_attributes_or_annotations").c_str());
    srcml_transform_result* result = nullptr;
    
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    for (int i = 0; i < n; i++) attributesOrAnnotations.push_back(srcml_unit_get_src(srcml_transform_get_unit(result, i)));

    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result);
}

// Finds type name
//
void typeModel::findName() {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"type_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    
    if (srcml_transform_get_unit_size(result) > 0) {
        std::string tempName = srcml_unit_get_src(srcml_transform_get_unit(result, 0));
        name.push_back(tempName); 

        helperFunctions::removeWhitespace(tempName);
        name.push_back(tempName);
        
        std::size_t listOpen = tempName.find("<");
        if (listOpen != std::string::npos) {
            std::string nameLeft = tempName.substr(0, listOpen);
            std::string nameRight = tempName.substr(listOpen, tempName.size() - listOpen);
            helperFunctions::removeBetweenComma(nameRight, true);
            helperFunctions::removeNamespace(nameLeft, unitLanguage, true);
            name.push_back(nameLeft + nameRight);
            name.push_back(nameLeft);
        }
        else {
            helperFunctions::removeNamespace(tempName, unitLanguage, true);
            name.push_back(tempName);
            name.push_back(tempName); // Not a duplicate
        }         
    }

    // There might be a missing name (e.g., anonymous structs in C++)
    if (name.size() == 0) name = {"", "", "", ""}; 

    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result); 
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
void typeModel::findParentNames() { 
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"parent_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; i++) {
        std::string parentName = srcml_unit_get_src(srcml_transform_get_unit(result, i));

        helperFunctions::removeWhitespace(parentName);

        std::size_t listOpen = parentName.find("<");
        if (listOpen != std::string::npos) {
            std::string left = parentName.substr(0, listOpen);
            std::string right = parentName.substr(listOpen, parentName.size() - listOpen);
            helperFunctions::removeNamespace(left, unitLanguage, true); 
            parentNames.insert(left + right);
        }
        else {
            helperFunctions::removeNamespace(parentName, unitLanguage, true);
            parentNames.insert(parentName);
        }    
    }
    
    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result); 
}

// Finds field names
// Only collect the name if there is a type
// C++:
//  This does not count unions without a name, so their fields will be collected even if nested within another type
//  Static fields are ignored and treated as globals
// C#:
//  Auto-properties can be used to declare fields implicitly so they are treated as fields
//  Regular properties can be used to get or set fields
//   Therefore, regular properties will be treated like normal fields as they are used (most of the time) to get or set regular fields 
//    where property name = field name and where property type = field type
//   However, the assumption here is that their usage is assumed to be getting or setting a single field
//
void typeModel::findFieldNames(std::vector<variableModel>& fieldsOrdered) {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"field_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; i++) {
        std::string dataMemberName = srcml_unit_get_src(srcml_transform_get_unit(result, i));

        variableModel v;

        // Chop off [] for arrays  
        if (unitLanguage == "C++")  helperFunctions::removeBracketSuffix(dataMemberName);
        
        v.setName(dataMemberName);

        fieldsOrdered.push_back(v); 
    }
    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result);
}

// Finds fields types
// Only collect the type if there is a name
//
void typeModel::findFieldTypes(std::vector<variableModel>& fieldsOrdered) {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"field_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    std::string prev; 
    for (int i = 0; i < n; ++i) {
        std::string type = srcml_unit_get_srcml(srcml_transform_get_unit(result, i));

        if (type == "<type ref=\"prev\"/>") type = prev;           
        
        else {  
            type = srcml_unit_get_src(srcml_transform_get_unit(result, i));
            prev = type;
        }  

        fieldsOrdered.at(i).setType(type);
        PRIMITIVES.isNonPrimitive(fieldsOrdered[i], unitLanguage, name[3]);

        fields.insert({fieldsOrdered[i].getName(), std::move(fieldsOrdered[i])});
    }
    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result);
}

// Finds methods defined inside the type
// C#:
//   Nested local functions within methods in C# are ignored 
//   Attributes do not read/write to the state object. However, they can have expressions, decl_stmt, etc.
//    Therefore, we ignore them when collecting data.
// Java:
//   Annotations have the same story as attributes, so we ignore these statements inside them
//   
void typeModel::findMethod(const std::string& classXpath, const std::string& header, int unitNumber) {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"method").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        std::string resultUnitSrcml = header + srcml_unit_get_srcml(resultUnit) + "</unit>";
        
        methodArchive = srcml_archive_create();
        srcml_archive_read_open_memory(methodArchive, resultUnitSrcml.c_str(), resultUnitSrcml.size());
        methodUnit = srcml_archive_read_unit(methodArchive);
        
        std::string methodXpath = "(" + classXpath + XPATH_GENERATOR.getXPathList(unitLanguage, "method") + ")[" + std::to_string(i + 1) + "]";
        functionModel method = functionModel(methodXpath, unitLanguage, name[3], "", unitNumber, false);     
        methods.push_back(method);

        srcml_unit_free(methodUnit);
        srcml_archive_close(methodArchive);
        srcml_archive_free(methodArchive); 
    }

    srcml_transform_free(result);
    srcml_clear_transforms(typeArchive);
}

// Properties need to be collected separately since they hold the return type of the getters
// Properties cannot be nested in methods or in other properties
//
void typeModel::findProperties(const std::string& classXpath, const std::string& header, int unitNumber) {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage, "property").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* resultUnit = nullptr;  
    
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        std::string resultUnitSrcml = header + srcml_unit_get_srcml(resultUnit) + "</unit>";

        propertyArchive = srcml_archive_create();
        srcml_archive_read_open_memory(propertyArchive, resultUnitSrcml.c_str(), resultUnitSrcml.size());
        propertyUnit = srcml_archive_read_unit(propertyArchive);

        std::string propertyXpath = "(" + classXpath + XPATH_GENERATOR.getXPathList(unitLanguage,"property") + ")[" + std::to_string(i + 1) + "]";
        std::string propertyReturnType = findPropertyReturnType(); // No need to set pass the propertyXpath as the propertyArchive and propertyUnit are already set
        findMethodsInProperty(propertyXpath, propertyReturnType, header, unitNumber);

        srcml_unit_free(propertyUnit);
        srcml_archive_close(propertyArchive);
        srcml_archive_free(propertyArchive); 
    }

    srcml_transform_free(result);
    srcml_clear_transforms(typeArchive);
}

// Finds the return type of a property
//
std::string typeModel::findPropertyReturnType() {
    srcml_append_transform_xpath(propertyArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"property_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(propertyArchive, propertyUnit, &result);

    srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);
    char* returnTypeUnparsed = nullptr;
    std::size_t typeSize = 0;
    srcml_unit_unparse_memory(resultUnit, &returnTypeUnparsed, &typeSize);

    srcml_clear_transforms(propertyArchive);

    // srcML v1.1 has parsing issues with certain properties, so we return an empty string if the return type is not found
    return (returnTypeUnparsed ? returnTypeUnparsed : "");
}

// Finds the methods in a property
//
void typeModel::findMethodsInProperty(const std::string& propertyXpath, const std::string& propertyReturnType, const std::string& header, int unitNumber) {
    srcml_append_transform_xpath(propertyArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"property_method").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(propertyArchive, propertyUnit, &result);
    srcml_unit* resultUnit = nullptr;

    int n = srcml_transform_get_unit_size(result);
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        std::string resultUnitSrcml = header + srcml_unit_get_srcml(resultUnit) + "</unit>";

        methodArchive = srcml_archive_create();
        srcml_archive_read_open_memory(methodArchive, resultUnitSrcml.c_str(), resultUnitSrcml.size());
        methodUnit = srcml_archive_read_unit(methodArchive);

        std::string methodXpath = "(" + propertyXpath + XPATH_GENERATOR.getXPathList(unitLanguage,"property_method") + ")[" + std::to_string(i + 1) + "]";
        functionModel m = functionModel(methodXpath, unitLanguage, name[3], propertyReturnType, unitNumber, true);
        methods.push_back(m);

        srcml_unit_free(methodUnit);
        srcml_archive_close(methodArchive);
        srcml_archive_free(methodArchive); 
    }
    
    srcml_transform_free(result);
    srcml_clear_transforms(propertyArchive);
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
std::string typeModel::getParentsString() const {
    std::string parentsString;
    for (const std::string & s : parentNames) {
        if (!parentsString.empty()) parentsString += " ";
        parentsString += s;
    }
    return parentsString;
}


// Gets the inherited parent types as a single string
//
std::string typeModel::getInheritedParentsString() const {
    std::string inheritedParentsString;
    for (const std::string & s : inheritedParentNames) {
        if (!inheritedParentsString.empty()) inheritedParentsString += " ";
        inheritedParentsString += s;
    }
    return inheritedParentsString;
}

// Gets the method signatures as a single string
//
std::string typeModel::getMethodSignaturesString() const {
    std::string methodsignature;
    for (const std::string & s : methodSignatures) {
        if (!methodsignature.empty()) methodsignature += " ";
        methodsignature += s;
    }
    return methodsignature;
}

// Gets the inherited method signatures as a single string
//
std::string typeModel::getInheritedMethodSignaturesString() const {
    std::string inheritedMethodsignature;
    for (const std::string & s : inheritedMethodSignatures) {
        if (!inheritedMethodsignature.empty()) inheritedMethodsignature += " ";
        inheritedMethodsignature += s;
    }
    return inheritedMethodsignature;
}

// Merges another typeModel (e.g., partial class) into this one
//
void typeModel::mergeData(typeModel& other) {
    // Merge Methods
    methods.insert(methods.end(), std::make_move_iterator(other.getMethods().begin()), std::make_move_iterator(other.getMethods().end()));

    // Merge Parents
    const auto& otherParents = other.getParentNames();
    parentNames.insert(otherParents.begin(), otherParents.end());

    // Merge Method Signatures
    const auto& otherMethodSignatures = other.getMethodSignatures();
    methodSignatures.insert(otherMethodSignatures.begin(), otherMethodSignatures.end());

    // Merge Data Members
    auto& otherDataMembers = other.getFields();
    fields.insert(otherDataMembers.begin(), otherDataMembers.end());

    // Merge XPaths for each unit number
    const auto& otherXpath = other.getXpath();
    for (const auto& pair : otherXpath) {
        std::vector<std::string>& targetXpathVector = xpath[pair.first];
        targetXpathVector.insert(targetXpathVector.end(), pair.second.begin(), pair.second.end());
    }
}