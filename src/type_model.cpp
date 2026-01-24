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

extern thread_local helperFunctions    HELPERS;
extern              XPathGenerator     XPATH_GENERATOR;
extern thread_local primitives         PRIMITIVES;
extern thread_local srcml_unit*        typeUnit;
extern thread_local srcml_archive*     typeArchive;
extern thread_local srcml_unit*        methodUnit;
extern thread_local srcml_archive*     methodArchive;
extern thread_local srcml_unit*        propertyUnit;
extern thread_local srcml_archive*     propertyArchive;

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
    dataMembers.insert({variable.getName(), variable});
}

void typeModel::findData(const std::string& classXpath, int unitNumber) {
    xpath[unitNumber].push_back(classXpath);

    findStructureType();
    findParentName();
    
    std::vector<variableModel> dataMembersOrdered;
    findDataMemberName(dataMembersOrdered);
    findDataMemberType(dataMembersOrdered);
     
    findMethod(classXpath, unitNumber);

    if (unitLanguage == "C#") findProperty(classXpath, unitNumber); 
}

void typeModel::findStructureType() {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"structure_type").c_str());
    srcml_transform_result* result = nullptr;
    
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    for (int i = 0; i < n; i++) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, i);
        structureType += srcml_unit_get_srcml(resultUnit);
    }
    HELPERS.removeWhitespace(structureType);
    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result);
}


// Finds type name
//
void typeModel::findName() {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"type_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);

    if (srcml_transform_get_unit_size(result) == 1) {
        char *unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(srcml_transform_get_unit(result, 0), &unparsed, &size);
        std::string tempName = unparsed;
        name.push_back(tempName); 

        HELPERS.removeWhitespace(tempName);
        name.push_back(tempName);
        
        std::size_t listOpen = tempName.find("<");
        if (listOpen != std::string::npos) {
            std::string nameLeft = tempName.substr(0, listOpen);
            std::string nameRight = tempName.substr(listOpen, tempName.size() - listOpen);
            HELPERS.removeBetweenComma(nameRight, true);
            HELPERS.removeNamespace(nameLeft, unitLanguage, true);
            name.push_back(nameLeft + nameRight);
            name.push_back(nameLeft);
        }
        else {
            HELPERS.removeNamespace(tempName, unitLanguage, true);
            name.push_back(tempName);
            name.push_back(tempName); // Not a duplicate
        }     
        free(unparsed);     
    }

    // There might be a missing name (e.g., anonymous structs in C++)
    if (name.size() == 0) name = {"", "", "", ""}; 

    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result); 
}

// Finds parent classs
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
void typeModel::findParentName() { 
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"parent_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);

        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string parentName = unparsed;

        HELPERS.removeWhitespace(parentName);

        std::size_t listOpen = parentName.find("<");
        if (listOpen != std::string::npos) {
            std::string parClassNameLeft = parentName.substr(0, listOpen);
            std::string parClassNameRight = parentName.substr(listOpen, parentName.size() - listOpen);
            HELPERS.removeNamespace(parClassNameLeft, unitLanguage, true); 
            parents.insert(parClassNameLeft + parClassNameRight);
        }
        else {
            HELPERS.removeNamespace(parentName, unitLanguage, true);
            parents.insert(parentName);
        }
    
        free(unparsed);      
    }
    
    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result); 
}

// Finds data members names
// Only collect the name if there is a type
// C++:
//  This does not count unions without a name, so their data members will be collected even if nested without a class or a struct
//  Static data members  are ignored and treated as globals
// C#:
//  Auto-properties can be used to declare data members implicitly 
//   and regular properties are used to get or set data members (most of the time)
//  Therefore, both types of properties will be treated as data members as they can be used and called as normal data members  
//   where property name = data member name and where property type = data member type
//
void typeModel::findDataMemberName(std::vector<variableModel>& dataMembersOrdered) {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"data_member_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result,i);
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);     
        std::string dataMemberName = unparsed;

        variableModel v;

        // Chop off [] for arrays  
        if (unitLanguage == "C++")
            HELPERS.removeBracketSuffix(dataMemberName);
        
        v.setName(dataMemberName);

        dataMembersOrdered.push_back(v); 
        free(unparsed);

    }
    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result);
}

// Finds data members types
// Only collect the type if there is a name
//
void typeModel::findDataMemberType(std::vector<variableModel>& dataMembersOrdered) {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"data_member_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    std::string prev; 

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(resultUnit);
        
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
     
        if (type == "<type ref=\"prev\"/>") {
            type = prev;
        }
        else {  
            type = unparsed;
            prev = type;
        }

        dataMembersOrdered[i].setType(type);  
        dataMembers.insert({dataMembersOrdered[i].getName(), dataMembersOrdered[i]});
        bool nonPrimitiveDataMemberExternal = false;

        if (PRIMITIVES.isNonPrimitive(dataMembersOrdered[i], unitLanguage, name[3])) 
            if (nonPrimitiveDataMemberExternal) dataMembersOrdered[i].setNonPrimitiveExternal(true);
                          
        free(unparsed);
    }
    srcml_clear_transforms(typeArchive);
    srcml_transform_free(result);
}

// Finds methods defined inside the type
// C#:
//   Nested local functions within methods in C# are ignored 
void typeModel::findMethod(const std::string& classXpath, int unitNumber) {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"method").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        methodArchive = srcml_archive_clone(typeArchive);

        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_archive_write_open_memory(methodArchive, &unparsed, &size);
        srcml_archive_write_unit(methodArchive, resultUnit);
        srcml_archive_close(methodArchive);
        srcml_archive_free(methodArchive);
        
        methodArchive = srcml_archive_create();
        srcml_archive_read_open_memory(methodArchive, unparsed, size);
        methodUnit = srcml_archive_read_unit(methodArchive);
        
        std::string methodXpath = "(" + classXpath + XPATH_GENERATOR.getXPathList(unitLanguage, "method") + ")[" + std::to_string(i + 1) + "]";
        functionModel method = functionModel(methodXpath, unitLanguage, name[3], "", unitNumber, false);     
        methods.push_back(method);

        free(unparsed);
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
void typeModel::findProperty(const std::string& classXpath, int unitNumber) {
    srcml_append_transform_xpath(typeArchive, XPATH_GENERATOR.getXPathList(unitLanguage, "property").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(typeArchive, typeUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* resultUnit = nullptr;  
    
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        propertyArchive = srcml_archive_clone(typeArchive);

        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_archive_write_open_memory(propertyArchive, &unparsed, &size);
        srcml_archive_write_unit(propertyArchive, resultUnit);
        srcml_archive_close(propertyArchive);
        srcml_archive_free(propertyArchive);

        propertyArchive = srcml_archive_create();
        srcml_archive_read_open_memory(propertyArchive, unparsed, size);
        propertyUnit = srcml_archive_read_unit(propertyArchive);

        std::string propertyXpath = "(" + classXpath + XPATH_GENERATOR.getXPathList(unitLanguage,"property") + ")[" + std::to_string(i + 1) + "]";
        std::string propertyReturnType = findPropertyReturnType(); // No need to set pass the propertyXpath as the propertyArchive and propertyUnit are already set
        findMethodsInProperty(propertyXpath, propertyReturnType, unitNumber);

        free(unparsed);
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
void typeModel::findMethodsInProperty(const std::string& propertyXpath, const std::string& propertyReturnType, int unitNumber) {
    srcml_append_transform_xpath(propertyArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"property_method").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(propertyArchive, propertyUnit, &result);
    srcml_unit* resultUnit = nullptr;

    int n = srcml_transform_get_unit_size(result);
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        methodArchive = srcml_archive_create();
        srcml_archive_register_namespace(methodArchive, "pos", "http://www.srcML.org/srcML/position");
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_archive_write_open_memory(methodArchive, &unparsed, &size);
        srcml_archive_write_unit(methodArchive, resultUnit);
        srcml_archive_close(methodArchive);
        srcml_archive_free(methodArchive);

        methodArchive = srcml_archive_create();
        srcml_archive_read_open_memory(methodArchive, unparsed, size);
        methodUnit = srcml_archive_read_unit(methodArchive);

        std::string methodXpath = "(" + propertyXpath + XPATH_GENERATOR.getXPathList(unitLanguage,"property_method") + ")[" + std::to_string(i + 1) + "]";
        functionModel m = functionModel(methodXpath, unitLanguage, name[3], propertyReturnType, unitNumber, true);
        methods.push_back(m);

        free(unparsed);
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

// Gets the parent classs as a single string
//
std::string typeModel::getParentsString() const {
    std::string parentsString;
    for (const std::string & s : parents) {
        if (!parentsString.empty()) parentsString += " ";
        parentsString += s;
    }
    return parentsString;
}

// Merges another typeModel (e.g., partial class) into this one
//
void typeModel::mergeData(typeModel& other) {
    // Merge Methods
    methods.insert(methods.end(), std::make_move_iterator(other.getMethods().begin()), std::make_move_iterator(other.getMethods().end()));

    // Merge Parents
    const auto& otherParents = other.getParents();
    parents.insert(otherParents.begin(), otherParents.end());

    // Merge Method Signatures
    const auto& otherMethodSignatures = other.getMethodSignatures();
    methodSignatures.insert(otherMethodSignatures.begin(), otherMethodSignatures.end());

    // Merge Data Members
    auto& otherDataMembers = other.getDataMembers();
    dataMembers.insert(otherDataMembers.begin(), otherDataMembers.end());

    // Merge XPaths for each unit number
    const auto& otherXpath = other.getXpath();
    for (const auto& pair : otherXpath) {
        std::vector<std::string>& targetXpathVector = xpath[pair.first];
        targetXpathVector.insert(targetXpathVector.end(), pair.second.begin(), pair.second.end());
    }
}