// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModel.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "ClassModel.hpp"

extern XPathBuilder                  XPATH_TRANSFORMATION;  

classModel::classModel(srcml_archive* archive, srcml_unit* unit, const std::string& unitLang) {
    unitLanguage = unitLang;
    findClassName(archive, unit);  
}

void classModel::findClassData(srcml_archive* archive, srcml_unit* unit, const std::string& classXpath, int unitNumber) {
    xpath[unitNumber].push_back(classXpath);
    if (unitLanguage == "C++") findClassType(archive, unit); // Needed for findParentClassName()
    findParentClassName(archive, unit); // Requires class type for C++
    
    std::vector<variable> fieldOrdered;
    int numOfCurrentFields = fieldOrdered.size(); // Used for partial classs
    findFieldName(archive, unit, fieldOrdered);
    findFieldType(archive, unit, fieldOrdered, numOfCurrentFields);
    
    // The "this" keyword by itself is assumed to be an "accessor" to the state of the class
    // It is also not a non-primitive
    variable v;
    v.setName("this");
    fields.insert({v.getName(), v});
    
    std::vector<variable> nonPrivateFieldOrdered; 
    int numOfCurrentNonPrivateFields = nonPrivateFieldOrdered.size();
    findNonPrivateFieldName(archive, unit, nonPrivateFieldOrdered);
    findNonPrivateFieldType(archive, unit, nonPrivateFieldOrdered, numOfCurrentNonPrivateFields);
    findMethod(archive, unit, classXpath, unitNumber);

    if (unitLanguage == "C#") findMethodInProperty(archive, unit, classXpath, unitNumber); 
}


// Finds class name
//
void classModel::findClassName(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"class_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);

    if (srcml_transform_get_unit_size(result) == 1) {
        char *unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(srcml_transform_get_unit(result, 0), &unparsed, &size);
        
        std::string tempName = unparsed;
        name.push_back(tempName); 

        trimWhitespace(tempName);
        name.push_back(tempName);
        
        std::size_t listOpen = tempName.find("<");
        if (listOpen != std::string::npos) {
            std::string nameLeft = tempName.substr(0, listOpen);
            std::string nameRight = tempName.substr(listOpen, tempName.size() - listOpen);
            removeBetweenComma(nameRight, true);
            removeNamespace(nameLeft, true, unitLanguage);
            name.push_back(nameLeft + nameRight);
            name.push_back(nameLeft);
        }
        else {
            removeNamespace(tempName, true, unitLanguage);
            name.push_back(tempName);
            name.push_back(tempName); // Not a duplicate
        }
        
        free(unparsed);     
    }

    // There might be a missing name (e.g., anonymous structs in C++)
    if (name.size() == 0) name = {"", "", "", ""}; 

    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

// Determines the class type (class, interface, or struct)
//
void classModel::findClassType(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"class_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);

    if (srcml_transform_get_unit_size(result) == 1) {
        classType = srcml_unit_get_srcml(srcml_transform_get_unit(result, 0));
        trimWhitespace(classType);
    }
    
    srcml_clear_transforms(archive);
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
void classModel::findParentClassName(srcml_archive* archive, srcml_unit* unit) { 
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"parent_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);

        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string parentName = unparsed;

        std::string inheritanceSpecifier;
        if (unitLanguage == "C++") {
            std::string temp = srcml_unit_get_srcml(resultUnit);
            if (temp.find("<specifier>public</specifier>") != std::string::npos) {
                inheritanceSpecifier = "public";
                parentName.erase(0, inheritanceSpecifier.size());  
            }             
            else if (temp.find("<specifier>protected</specifier>") != std::string::npos) {
                inheritanceSpecifier = "protected";
                parentName.erase(0, inheritanceSpecifier.size());  
            }  
            else if (temp.find("<specifier>private</specifier>") != std::string::npos) {
                inheritanceSpecifier = "private";
                parentName.erase(0, inheritanceSpecifier.size());  
            }             
            else if (classType == "class")
                inheritanceSpecifier = "private";
            else
                inheritanceSpecifier = "public";         
            
        }
        trimWhitespace(parentName);

        std::size_t listOpen = parentName.find("<");
        if (listOpen != std::string::npos) {
            std::string parClassNameLeft = parentName.substr(0, listOpen);
            std::string parClassNameRight = parentName.substr(listOpen, parentName.size() - listOpen);
            removeNamespace(parClassNameLeft, true, unitLanguage); 
            parentClassName.insert({parClassNameLeft + parClassNameRight, inheritanceSpecifier});
        }
        else {
            removeNamespace(parentName, true, unitLanguage);
            parentClassName.insert({parentName, inheritanceSpecifier});
        }
    
        free(unparsed);      
    }
    
    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

// Finds field names
// Only collect the name if there is a type
// C++:
//  This does not count unions without a name, so their fields will be collected even if nested without a class or a struct
//  Static fields are ignored and treated as globals
// C#:
//  Auto-properties can be used to declare data members implicitly 
//   and regular properties are used to get or set data members (most of the time)
//  Therefore, both types of properties will be treated as data members as they can be used and called as normal data members  
//   where property name = data member name and where property type = data member type 
void classModel::findFieldName(srcml_archive* archive, srcml_unit* unit, std::vector<variable>& fieldOrdered) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"field_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result,i);
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);     
        std::string fieldName = unparsed;

        variable v;

        // Chop off [] for arrays  
        if (unitLanguage == "C++") {
            std::size_t start_position = fieldName.find("[");
            if (start_position != std::string::npos){
                fieldName = fieldName.substr(0, start_position);
                Rtrim(fieldName);
            }
        }

        v.setName(fieldName);

        fieldOrdered.push_back(v); 
        free(unparsed);

    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds field types
// Only collect the type if there is a name
//
void classModel::findFieldType(srcml_archive* archive, srcml_unit* unit, std::vector<variable>& fieldOrdered, int numOfCurrentFields) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"field_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
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

        fieldOrdered[numOfCurrentFields + i].setType(type);  
        fields.insert({fieldOrdered[numOfCurrentFields + i].getName(), fieldOrdered[numOfCurrentFields + i]});
        bool nonPrimitiveFieldExternal = false;

        isNonPrimitiveType(type, fieldOrdered[numOfCurrentFields + i], unitLanguage, name[3]);

        if (nonPrimitiveFieldExternal)
            fieldOrdered[numOfCurrentFields + i].setNonPrimitiveExternal(true);
                          
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds non-private field names
// For C++, no access specifier = private for a class and public for a struct 
// For C#, no access specifier = private for a class and public for a struct
//  Interfaces can't have fields, only properties, which are always public even if no specifier is used
// For Java, no access specifier = accessible by derived classes within the
//  same package (package-private, case ignored for now) and always public static for an interface even if no specifier is used
//
void classModel::findNonPrivateFieldName(srcml_archive* archive, srcml_unit* unit, std::vector<variable>& nonPrivateFieldOrdered) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"non_private_field_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result,i);
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size); 
        std::string fieldName = unparsed;

        variable v;
        // Chop off [] for arrays  
        if (unitLanguage == "C++") {
            std::size_t start_position = fieldName.find("[");
            if (start_position != std::string::npos){
                fieldName = fieldName.substr(0, start_position);
                Rtrim(fieldName);
            }
        }
        
        v.setName(fieldName);

        nonPrivateFieldOrdered.push_back(v); 
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds non-private field types
//
void classModel::findNonPrivateFieldType(srcml_archive* archive, srcml_unit* unit, std::vector<variable>& nonPrivateFieldOrdered, 
                                             int numOfNonPrivateCurrentFields) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"non_private_field_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
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

        nonPrivateFieldOrdered[numOfNonPrivateCurrentFields + i].setType(type);
        nonPrivateInheritedFields.insert({nonPrivateFieldOrdered[numOfNonPrivateCurrentFields + i].getName(), 
                                                nonPrivateFieldOrdered[numOfNonPrivateCurrentFields + i]});

        bool nonPrimitiveFieldExternal = false;
        isNonPrimitiveType(type, nonPrivateFieldOrdered[numOfNonPrivateCurrentFields + i], unitLanguage, name[3]);

        if (nonPrimitiveFieldExternal)
            nonPrivateFieldOrdered[numOfNonPrivateCurrentFields + i].setNonPrimitiveExternal(true);
        
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds methods defined inside the class
// C#:
//  Nested local functions within methods in C# are ignored 
void classModel::findMethod(srcml_archive* archive, srcml_unit* unit, const std::string& classXpath, int unitNumber) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"method").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);    
    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        srcml_archive* methodArchive = srcml_archive_create();
        srcml_archive_register_namespace(methodArchive, "pos", "http://www.srcML.org/srcML/position");
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_archive_write_open_memory(methodArchive, &unparsed, &size);
        srcml_archive_write_unit(methodArchive, resultUnit);
        srcml_archive_close(methodArchive);
        srcml_archive_free(methodArchive);

        // Workaround, remove when srcML v1.1 is released
        std::string methString = unparsed;
        srcmlBackwardCompatibility(methString);

        methodArchive = srcml_archive_create();
        srcml_archive_read_open_memory(methodArchive, methString.c_str(), methString.size()); // Workaround, remove when srcML v1.1 is released
        //srcml_archive_read_open_memory(propertyArchive, unparsed, size); // Uncomment when srcML v1.1 is released
        srcml_unit* methodUnit = srcml_archive_read_unit(methodArchive);
        
        std::string methodXpath = "(" + classXpath + XPATH_TRANSFORMATION.getXpath(unitLanguage,"method") + ")[" + std::to_string(i + 1) + "]";
        methodModel m = methodModel(methodArchive, methodUnit, methodXpath, unitLanguage, "", unitNumber);
        
        methods.push_back(m); 
        
        free(unparsed);       
        srcml_unit_free(methodUnit);
        srcml_archive_close(methodArchive);
        srcml_archive_free(methodArchive); 
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Properties need to be collected separately since they hold the return type of the getters
// Properties can't be nested in methods or in other properties
//
void classModel::findMethodInProperty(srcml_archive* archive, srcml_unit* unit, const std::string& classXpath, int unitNumber) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"property").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);    

    for (int i = 0; i < n; ++i) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, i);

        srcml_archive* propertyArchive = srcml_archive_create();
        srcml_archive_register_namespace(propertyArchive, "pos", "http://www.srcML.org/srcML/position");
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_archive_write_open_memory(propertyArchive, &unparsed, &size);
        srcml_archive_write_unit(propertyArchive, resultUnit);
        srcml_archive_close(propertyArchive);
        srcml_archive_free(propertyArchive);
        
        // Workaround, remove when srcML v1.1 is released
        std::string propertyString = unparsed;
        srcmlBackwardCompatibility(propertyString);

        propertyArchive = srcml_archive_create();
        srcml_archive_read_open_memory(propertyArchive, propertyString.c_str(), propertyString.size()); // Workaround, remove when srcML v1.1 is released
        //srcml_archive_read_open_memory(propertyArchive, unparsed, size); // Uncomment when srcML v1.1 is released
        srcml_unit* propertyUnit = srcml_archive_read_unit(propertyArchive);

        srcml_append_transform_xpath(propertyArchive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"property_type").c_str());
        srcml_transform_result* propertyResult = nullptr;
        srcml_unit_apply_transforms(propertyArchive, propertyUnit, &propertyResult);
        int nType = srcml_transform_get_unit_size(propertyResult);   
        if (nType > 0) {
            srcml_unit* typeUnit = srcml_transform_get_unit(propertyResult, 0);
            char* typeUnparsed = nullptr;
            std::size_t typeSize = 0;
            srcml_unit_unparse_memory(typeUnit, &typeUnparsed, &typeSize);

            srcml_transform_free(propertyResult);
            propertyResult = nullptr;
            srcml_clear_transforms(propertyArchive);

            srcml_append_transform_xpath(propertyArchive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"property_method").c_str());
            srcml_unit_apply_transforms(propertyArchive, propertyUnit, &propertyResult);
            int nMethod = srcml_transform_get_unit_size(propertyResult); 
            for (int j = 0; j < nMethod; j++) {
                srcml_unit* methodResultUnit = srcml_transform_get_unit(propertyResult, j);

                srcml_archive* methodArchive = srcml_archive_create();
                srcml_archive_register_namespace(methodArchive, "pos", "http://www.srcML.org/srcML/position");
                char* methodUnparsed = nullptr;
                std::size_t methodSize = 0;
                srcml_archive_write_open_memory(methodArchive, &methodUnparsed, &methodSize);
                srcml_archive_write_unit(methodArchive, methodResultUnit);
                srcml_archive_close(methodArchive);
                srcml_archive_free(methodArchive);
            
                methodArchive = srcml_archive_create();
                srcml_archive_read_open_memory(methodArchive, methodUnparsed, methodSize);
                srcml_unit* methodUnit = srcml_archive_read_unit(methodArchive);

                std::string methodXpath = "((" + classXpath + XPATH_TRANSFORMATION.getXpath(unitLanguage,"property") + ")[" + std::to_string(i + 1) + "]";
                methodXpath += "//src:function)[" + std::to_string(j + 1) + "]";

                methodModel m = methodModel(methodArchive, methodUnit, methodXpath, unitLanguage, typeUnparsed, unitNumber);

                methods.push_back(m); 

                free(methodUnparsed);       
                srcml_unit_free(methodUnit);
                srcml_archive_close(methodArchive);
                srcml_archive_free(methodArchive); 
            }
            free(typeUnparsed);
        }

        free(unparsed);       
        srcml_unit_free(propertyUnit);
        srcml_transform_free(propertyResult);
        srcml_clear_transforms(propertyArchive);
        srcml_archive_close(propertyArchive);
        srcml_archive_free(propertyArchive); 
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}


std::string classModel::getStereotype () const {
    std::string result;

    for (const std::string &value : stereotype)
        result += value + " ";

    Rtrim(result);
     
    return result;
}
