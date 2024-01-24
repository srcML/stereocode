// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModel.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "ClassModel.hpp"

classModel::classModel (srcml_archive* archive, srcml_unit* unit) {
    findClassName(archive, unit);
}

void classModel::findClassData(srcml_archive* archive, srcml_unit* unit, std::string classXpath,  std::string unitLang, int unitNumber){
    unitLanguage = unitLang;
    xpath[unitNumber].push_back(classXpath);
    
    if (unitLang == "C#") 
        findPartialClass(archive, unit);

    if (unitLanguage == "C++")
        findFriendFunctionDecl(archive, unit);      
        
    findParentClassName(archive, unit);

    int attributeOldSize = attribute.size(); // Needed for partial class analysis
    findAttributeName(archive, unit);
    findAttributeType(archive, unit, attributeOldSize);

    int nonPrivateAttributeOldSize = nonPrivateAttribute.size();
    findNonPrivateAttributeName(archive, unit);
    findNonPrivateAttributeType(archive, unit, nonPrivateAttributeOldSize);
    findMethod(archive, unit, classXpath, unitNumber);

    for (const auto& c : attributeOrdered)
        attribute.insert({c.getName(), c});
}

// Finds class name
//
void classModel::findClassName(srcml_archive* archive, srcml_unit* unit) {
    const std::string xpathS = "//src:class[not(ancestor::src:class)]/src:name";
    srcml_append_transform_xpath(archive, xpathS.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    // n > 0 instead of n == 1 avoids problems related to returning a second name
    // First value is always the name (if any)
    if (n > 0) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);
        char *name = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &name, &size);
        className = name;
        classNameParsed = className.substr(0, className.find("<"));
        free(name);    
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

// Finds partial class (C# only)
// The "partial" keyword in C# allows a class definition to be spread across multiple files or namespaces
//
void classModel::findPartialClass(srcml_archive* archive, srcml_unit* unit) {
    const std::string xpathS = "//src:class[not(ancestor::src:class)]/src:specifier[.='partial']";
    srcml_append_transform_xpath(archive, xpathS.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    
    partial = (n > 0);
    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

// Finds friend functions
//
void classModel::findFriendFunctionDecl(srcml_archive* archive, srcml_unit* unit) {
    std::string xpathS = "//src:function_decl[count(ancestor::src:class) = 1]//text()[not(ancestor::src:parameter)";
    xpathS += " or ancestor::src:type]";

    srcml_append_transform_xpath(archive, xpathS.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    std::string decl = "";
    for (int i = 0; i < n; i++) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, i);
        std::string declWord = srcml_unit_get_srcml(resultUnit);
        if (declWord != ";")
            decl += srcml_unit_get_srcml(resultUnit);
        else {
            trimWhitespace(decl);
            friendFunctionDecl.insert(decl);
            decl = "";
        }
    }
    
    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

// Finds parent classes
// C++ supports multiple inheritance
// Java and C# only support single inheritance from other classes,
//  and multiple inheritance from interfaces.
// C++ and C# uses ":" for inheritance
// Java uses "extends" to inherit from a class and "implements" to inherit from multiple interfaces
// 
void classModel::findParentClassName(srcml_archive* archive, srcml_unit* unit) { 
    std::string xpathS = "//src:class[not(ancestor::src:class)]/src:super_list[count(ancestor::src:class) = 1]/src:super/src:name";
    if (unitLanguage == "Java"){
        xpathS += "//src:class[not(ancestor::src:class)]/src:super_list/src:extends/src:super/src:name"; 
        xpathS += " | src:class[not(ancestor::src:class)]/src:super_list[count(ancestor::src:class) = 1]/src:implements/src:super/src:name";
    }

    srcml_append_transform_xpath(archive, xpathS.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);
        char* name = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &name, &size);
        parentClassName.insert(name);
        free(name);      
    }
    
    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

// Finds attribute names
// Only collect the name if there is a type
//
void classModel::findAttributeName(srcml_archive* archive, srcml_unit* unit) {
    std::string xpathS = "//src:decl_stmt[not(ancestor::src:function)";
    xpathS += " and count(ancestor::src:class) = 1]/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    // Auto-properties can be used to delcare fields implicitly where property name = field name 
    if (unitLanguage == "C#") {
        xpathS += " | //src:property[count(descendant::src:function) = 0]/src:name";
    }

    srcml_append_transform_xpath(archive, xpathS.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result,i);
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);     
        std::string name = unparsed;
        free(unparsed);

        Variable v;
        v.setName(name);

        // Chop off [] for arrays  
        size_t start_position = name.find("[");
        if (start_position != std::string::npos){
            name = name.substr(0, start_position);
            Rtrim(name);
        }

        v.setNameParsed(name);
        attributeOrdered.push_back(v);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds attribute types
// Only collect the type if there is a name
//
void classModel::findAttributeType(srcml_archive* archive, srcml_unit* unit, int attributeOldSize) {
    std::string xpathS = "//src:decl_stmt[not(ancestor::src:function)";
    xpathS += " and count(ancestor::src:class) = 1]/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    if (unitLanguage == "C#") {
        xpathS += " | //src:property[count(descendant::src:function) = 0]/src:type";
    }
    srcml_append_transform_xpath(archive, xpathS.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    std::string prev = "";

    for (int i = attributeOldSize; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(resultUnit);
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
     
        if (type == "<type ref=\"prev\"/>") {
            type = prev;
        }
        else {  
            type = unparsed;
            prev = type;
        }

        attributeOrdered[i].setType(type);
        removeSpecifiers(type, unitLanguage);
        removeContainers(type, unitLanguage);
        trimWhitespace(type);
        attributeOrdered[i].setTypeParsed(type);
        if (!isPrimitiveType(type)) {
            attributeOrdered[i].setNonPrimitive(true); // True if attribute is not a primitive type.
            if (type.find(classNameParsed) == std::string::npos) 
                attributeOrdered[i].setNonPrimitiveExternal(true);
        }                   

        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds non-private attribute names
// C# and Java have access specifiers for each type
// For C#, no specifier = private
// For Java, no specifier = accessible by derived classes within the same package. 
//  But, here, we are ignoring them.
//
void classModel::findNonPrivateAttributeName(srcml_archive* archive, srcml_unit* unit) {
    std::string xpathS = "//src:decl_stmt[not(ancestor::src:function)";
    if (unitLanguage == "C++")
        xpathS += " and count(ancestor::src:class) = 1 and not(ancestor::src:private)]/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    else{
        xpathS += " and count(ancestor::src:class) = 1]";
        xpathS += "/src:decl[./src:type/src:specifier[not(.='private')]]/src:name[preceding-sibling::*[1][self::src:type]]"; 
        if (unitLanguage == "C#") {
            xpathS += " | //src:property[count(descendant::src:function) = 0";
            xpathS += " and ./src:type/src:specifier[not(.='private')]]/src:name"; 
        }
    }

    srcml_append_transform_xpath(archive, xpathS.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result,i);
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);     
        std::string name = unparsed;
    
        Variable v;
        v.setName(name);

        // Chop off [] for arrays      
        size_t start_position = name.find("[");
        if (start_position != std::string::npos){
            name = name.substr(0, start_position);
            Rtrim(name);
        }
        v.setNameParsed(name);

        nonPrivateAttribute.push_back(v); 
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds non-private attribute types
//
void classModel::findNonPrivateAttributeType(srcml_archive* archive, srcml_unit* unit, int nonPrivateAttributeOldSize) {
    std::string xpathS = "//src:decl_stmt[not(ancestor::src:function)";
    if (unitLanguage == "C++")
        xpathS +=  " and count(ancestor::src:class) = 1 and not(ancestor::src:private)]/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    else {
        xpathS += " and count(ancestor::src:class) = 1]";
        xpathS += "/src:decl[./src:type/src:specifier[not(.='private')]]/src:type[following-sibling::*[1][self::src:name]]";  
        if (unitLanguage == "C#") {
            xpathS += " | //src:property[count(descendant::src:function) = 0";
            xpathS += " and ./src:type/src:specifier[not(.='private')]]/src:type"; 
        }
    }

    srcml_append_transform_xpath(archive, xpathS.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    std::string prev = "";
    for (int i = nonPrivateAttributeOldSize; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(resultUnit);
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
     
        if (type == "<type ref=\"prev\"/>") {
            type = prev;
        }
        else {  
            type = unparsed;
            prev = type;
        }

        nonPrivateAttribute[i].setType(type);
        removeSpecifiers(type, unitLanguage);
        removeContainers(type, unitLanguage);
        trimWhitespace(type);
        nonPrivateAttribute[i].setTypeParsed(type);
        if (!isPrimitiveType(type)) {
            nonPrivateAttribute[i].setNonPrimitive(true); 
            if (type.find(classNameParsed) == std::string::npos) 
                nonPrivateAttribute[i].setNonPrimitiveExternal(true);;  
        }
        
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds methods defined inside the class
// Nested local functions within methods in C# are ignored 
// C++ and Java don't have nested local functions
//
void classModel::findMethod(srcml_archive* archive, srcml_unit* unit, std::string classXpath, int unitNumber) {
    std::string xpathS = "//src:function[count(ancestor::src:class) = 1]";
    srcml_append_transform_xpath(archive, xpathS.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
        
    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        srcml_archive* methodArchive = srcml_archive_create();
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_archive_write_open_memory(methodArchive, &unparsed, &size);
        srcml_archive_write_unit(methodArchive, resultUnit);

        srcml_archive_close(methodArchive);
        srcml_archive_free(methodArchive);
     
        methodArchive = srcml_archive_create();
        srcml_archive_read_open_memory(methodArchive, unparsed, size);
        srcml_unit* methodUnit = srcml_archive_read_unit(methodArchive);

        std::string methodXpath = classXpath + "//src:function[count(ancestor::src:class) = 1][" + std::to_string(i+1) + "]";
        
        method.push_back(methodModel(methodArchive, methodUnit, methodXpath, unitLanguage, unitNumber)); 
        
        free(unparsed);       
        srcml_unit_free(methodUnit);
        srcml_archive_close(methodArchive);
        srcml_archive_free(methodArchive);   
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}


// Compute class stereotype
//  Based on definition from Dragan, Collard, Maletic ICSM 2010
//
void classModel::ComputeClassStereotype() {
    std::unordered_map<std::string, int> stereotypes = {
        {"get", 0},
        {"predicate", 0},
        {"property", 0},
        {"accessor", 0},
        {"non-const-get", 0},
        {"non-const-predicate", 0},
        {"non-const-property", 0},
        {"non-const-accessor", 0},
        {"set", 0},
        {"command", 0},
        {"non-void-command", 0},
        {"collaborator", 0},
        {"controller", 0},
        {"factory", 0},
        {"empty", 0},
        {"stateless", 0},
        {"wrapper", 0},
        {"unclassified", 0},
    };

    int nonCollaborators = 0;
    for (const auto& m : method) {      
        for (const std::string& s : m.getStereotypeList()){
            if (stereotypes.find(s) != stereotypes.end())
                stereotypes[s]++;
        }
        std::string methodStereotype = m.getStereotype();
        if (methodStereotype.find("collaborator") == std::string::npos &&
            methodStereotype.find("controller") == std::string::npos)
                nonCollaborators++;
    }

    int getters = stereotypes["get"] + stereotypes["non-const-get"];
    int accessors = getters + stereotypes["predicate"] + stereotypes["non-const-predicate"] +
                    stereotypes["property"] + stereotypes["non-const-property"] +
                    stereotypes["accessor"] + stereotypes["non-const-accessor"]; 

    int setters = stereotypes["set"];     
    int commands = stereotypes["command"] + stereotypes["non-void-command"];           
    int mutators = setters + commands;

    int controllers = stereotypes["controller"];
    int collaborator =  stereotypes["collaborator"]; 
    int collaborators = controllers + collaborator;
     
    int factory = stereotypes["factory"];

    int degenerates = stereotypes["empty"] + stereotypes["stateless"] + stereotypes["wrapper"]; 

    int allMethods = method.size();

    // Entity
    if (((accessors - getters) != 0) && ((mutators - setters)  != 0) ) {
        double ratio = double(collaborators) / double(nonCollaborators);
        if (ratio >= 2) 
            classStereotype.push_back("entity");   
    }

    // Minimal Entity
    if (((allMethods - (getters + setters + commands)) == 0) && (getters != 0) && (setters != 0) & (commands != 0)) {
        double ratio = double(collaborators) / double(nonCollaborators);
        if (ratio >= 2) 
            classStereotype.push_back("minimal-entity");
        
    }

    // Data Provider
    if ((accessors > 2 * mutators) && (accessors > 2 * (controllers + factory)) )
        classStereotype.push_back("data-provider");

    // Commander
    if ((mutators > 2 * accessors) && (mutators > 2 * (controllers + factory)))
        classStereotype.push_back("command");

    // Boundary
    if ((collaborators > nonCollaborators) && (factory < 0.5 * allMethods) && (controllers < 0.33 * allMethods))
        classStereotype.push_back("boundary");

    // Factory
    if (factory > 0.66 * allMethods)
        classStereotype.push_back("factory");
    
    // Controller
    if ((controllers + factory > 0.66 * allMethods) && ((accessors != 0) || (mutators != 0)))
        classStereotype.push_back("control");

    // Pure Controller
    if ((controllers + factory != 0) && ((accessors + mutators + collaborator) == 0) && (controllers != 0)) 
        classStereotype.push_back("pure-control");

    // Large Class
    {
        int accPlusMut = accessors + mutators;
        int facPlusCon = controllers + factory;
        if (((0.2 * allMethods < accPlusMut) && (accPlusMut < 0.67 * allMethods )) &&
            ((0.2 * allMethods < facPlusCon) && (facPlusCon < 0.67 * allMethods )) &&
            (factory != 0) && (controllers != 0) && (accessors != 0) && (mutators != 0) ) {
                if (allMethods > METHODS_PER_CLASS_THRESHOLD) { 
                    classStereotype.push_back("large-class");
            }
        }
    }

    // Lazy Class
    if ((getters + setters != 0) && (((degenerates / double(allMethods)) > 0.33)) &&
       (((allMethods - (degenerates + getters + setters)) / double(allMethods))  <= 0.2))
        classStereotype.push_back("lazy-class");
    
    // Degenerate Class
    if ((degenerates / double(allMethods)) > 0.33)  
        classStereotype.push_back("degenerate");
    
    // Data Class
    if ((allMethods - (getters + setters) == 0) && ((getters + setters) != 0))
         classStereotype.push_back("data-class");
    
    // Small Class
    if ((0 < allMethods) && (allMethods < 3))
        classStereotype.push_back("small-class");

    // Empty Class (Considered degenerate)
    if (allMethods == 0)
        classStereotype.push_back("empty");

    // Final check if no stereotype was assigned
    if (classStereotype.size() == 0) 
        classStereotype.push_back("unclassified");

    for (const auto& pair : xpath) {
        for (const auto& classXpath : pair.second) {
            xpathList[pair.first].push_back(std::make_pair(classXpath, getStereotype())); 
        }
    }       
}

//Compute method stereotypes
//
void classModel::ComputeMethodStereotype() {
    getter();
    predicate();
    property();
    accessor(); 
    setter();
    command();
    factory();
    collaboratorController();
    empty();
    stateless();
    wrapper();
    for (auto& m : method) { 
        if (m.getStereotypeList().size() == 0) 
             m.setStereotype("unclassified");
        int unitNumber = m.getUnitNumber();
        xpathList[unitNumber].push_back(std::make_pair(m.getXpath(), m.getStereotype()));
    }
}

// Stereotype get:
//     Contains at least 1 return statement that returns an attribute
//     Only simple return statements (e.g., return a) that return a 
//      single attribute are considered
//
void classModel::getter() {
    for (auto& m : method) {
        if (m.IsAttributeReturned() && !m.IsParameterChanged()) {
            if (!m.IsConstMethod() && unitLanguage == "C++")
                m.setStereotype("non-const-get");
            else
                m.setStereotype("get");
        }
    }
}

// Stereotype predicate:
//     Return type is Boolean
//     Return expression is not a attribute (e.g., return someAttributes;)
//     Uses an attribute in an expression
//     Parameters are not modified
//
void classModel::predicate() { 
    for (auto& m : method) {
        bool returnType = false;
        std::string returnTypeSeparated = m.getReturnTypeSeparated();

        if (unitLanguage == "C++")
            returnType = (returnTypeSeparated == "bool");
        else if (unitLanguage == "C#")
            returnType = (returnTypeSeparated == "bool") || 
                         (returnTypeSeparated == "Boolean");
        else
            returnType = (returnTypeSeparated == "boolean");

        if (returnType && !m.IsAttributeReturned() && 
            !m.IsParameterChanged() && m.IsAttributeUsed()) {
            if (!m.IsConstMethod() && unitLanguage == "C++")
                m.setStereotype("non-const-predicate");
            else
                m.setStereotype("predicate"); 
        }
    }
}

// Stereotype property:
//     Return type is not Boolean or void
//     Return expression is not a attribute
//     Uses an attribute in an expression
//     Parameters are not modified
//
void classModel::property() {
    for (auto& m : method) {
        bool returnType = false;
        std::string returnTypeSeparated = m.getReturnTypeSeparated();

        if (unitLanguage == "C++")
            returnType = (returnTypeSeparated != "bool" && returnTypeSeparated != "void" && returnTypeSeparated != "");
        else if (unitLanguage == "C#")
            returnType = (returnTypeSeparated != "bool" && returnTypeSeparated != "Boolean" &&
                          returnTypeSeparated != "void" && returnTypeSeparated != "Void" && returnTypeSeparated != "");
        else
            returnType = (returnTypeSeparated != "boolean" && returnTypeSeparated != "void" && 
                          returnTypeSeparated != "Void" && returnTypeSeparated != "");

        if (returnType && !m.IsAttributeReturned() && 
            !m.IsParameterChanged() && m.IsAttributeUsed()) {
            if (!m.IsConstMethod() && unitLanguage == "C++")
                m.setStereotype("non-const-property");
            else
                m.setStereotype("property");
        }
    }
}

// Stereotype accessor:
//     Contains at least 1 parameter that is 
//      passed by non-const reference and is assigned a value
//     Covers a more general case where the method could return one or more attributes 
//      using the parameters and the method return
//
void classModel::accessor() {
    for (auto& m : method) {
        if (m.IsParameterChanged()){
            if (!m.IsConstMethod() && unitLanguage == "C++")
                m.setStereotype("non-const-accessor");
            else
                m.setStereotype("accessor");       
        } 
    }
}

// Stereotype set:
//     Only 1 attribute is changed
//     Covers the case of mutable attributes
//
void classModel::setter() {
    for (auto& m : method) {
        if (m.getAttributeModified() == 1 && ((m.getMethodCall().size() + m.getFunctionCall().size()) <= 1)) {
            m.setStereotype("set");
        }
    }
}

// stereotype command:
//     Cases:
//         a) More than one attribute modified
//         b) 1 Attribute modified and the # of calls (method and function calls) is at least 2
//         c) 0 attributes modified and there is at least 1 method or function call on an attribute or the # of calls is at least 2
//
//     Method is not const (C++ only)
//     Case 3 applies when attributes are mutable and method is const (C++ only)
//
// stereotype non-void-command (C++ only):        
//     Same as command.
//     Method has a 'void' return type
//
void classModel::command() {
    for (auto& m : method) {
        bool hasMethodCallsOnAttribute = m.IsMethodCallOnAttribute();
        bool hasFunctionCallsOnAttribute = m.IsFunctionCallOnAttribute();
        std::string returnType = m.getReturnTypeSeparated();
      
        size_t numOfCalls = m.getFunctionCall().size() + m.getMethodCall().size();
        bool case1 = m.getAttributeModified() == 0 && (hasFunctionCallsOnAttribute || hasMethodCallsOnAttribute || numOfCalls > 1);
        bool case2 = m.getAttributeModified() == 1 && numOfCalls > 1;
        bool case3 = m.getAttributeModified() > 1;
        if (case1 || case2 || case3) {
            if (!m.IsConstMethod() || (case3 && m.IsConstMethod())){ // Handles case of mutable attributes (C++ only)
                if (returnType != "void")
                    m.setStereotype("non-void-command");  
                else
                    m.setStereotype("command");
            }
        } 
    }
}

// Stereotype factory
//     Factories must include a pointer to an object (non-primitive) in their return type
//      and their return expression must be a local variable, a parameter, an attribute, 
//      or a call to another object’s constructor using the keyword new
//
void classModel::factory() {
    for (auto& m : method) {
            bool     returnsNew          = m.IsNewReturned();          
            bool     returnsObj          = m.IsNonPrimitiveReturnType();
            bool     paraOrLocalReturned = m.IsParameterOrLocalReturned(); 
            bool     attributeReturned   = m.IsAttributeReturned();  

            bool newCalls = false;    
            if (m.getConstructorCall().size() > 0) newCalls = true;


            bool isFactory =  (returnsObj && (returnsNew || 
                              (newCalls && (paraOrLocalReturned || attributeReturned))));

            if (isFactory) m.setStereotype("factory");
            
    }
}

// Stereotype collaborator:
//     It must have a non-primitive type as a parameter, or as a local variable, or as a return type (not void)
//      or is using a non-primitive type attribute that is of a different type than the class.
//
// Stereotype controller:
//     Collaborates with other objects without changing the state of the current object
//     No attributes are modified
//     No method calls on attributes
//     No function calls
//
void classModel::collaboratorController() {
    for (auto& m : method) {
        if (!m.IsEmpty()){
            // Check if method uses an attribute of a non-primitive type
            bool NonPrimitiveAttribute = m.IsNonPrimitiveAttribute();
            bool NonPrimitiveAttributeExternal = m.IsNonPrimitiveAttributeExternal();
            
            // Check for non-primitive local variable types
            bool NonPrimitiveLocal = m.IsNonPrimitiveLocal();
            bool NonPrimitiveLocalExternal = m.IsNonPrimitiveLocalExternal();

            // Check for non-primitive parameter types
            bool NonPrimitiveParamater = m.IsNonPrimitiveParamater();
            bool NonPrimitiveParamaterExternal = m.IsNonPrimitiveParamaterExternal();

            // Check for a non-primitive return type
            // Only ignores return of type "void". Returns such as "void*"" are not ignored
            // This is why void is not added to the list of primitives (for C++ only) since void* can point to non-primitive
            bool returnNonPrimitive = m.IsNonPrimitiveReturnType();
            bool returnNonPrimitiveExternal = m.IsNonPrimitiveReturnTypeExternal();

            bool returnCheck = returnNonPrimitive && (m.getReturnTypeSeparated() != "void");
           
            if (NonPrimitiveAttribute || NonPrimitiveLocal || NonPrimitiveParamater || returnCheck) {
                 if (NonPrimitiveAttributeExternal || NonPrimitiveLocalExternal || NonPrimitiveParamaterExternal || returnNonPrimitiveExternal) {
                    if (m.getFunctionCall().size() == 0 && !m.IsMethodCallOnAttribute() && (m.getAttributeModified() == 0))
                        m.setStereotype("controller");     
                    else 
                        m.setStereotype("collaborator"); 
                 }
            }
        }
    }
}

// Stereotype empty
//  No statements except for comments
//
void classModel::empty() {
    for (auto& m : method) {
        if (m.IsEmpty()) 
            m.setStereotype("empty");
    }
}

// Stereotype stateless (a.k.a incidental or pure stateless)
//     is not an empty method
//     has no calls of any type
//     does not use any attributes
//
void classModel::stateless() {
    for (auto& m : method) {
        if(!m.IsEmpty()){
            bool usedAttr = m.IsAttributeUsed();
            bool noCalls = (m.getMethodCall().size() + m.getFunctionCall().size() + m.getConstructorCall().size()) == 0 ;
            if (noCalls && !usedAttr) {
                m.setStereotype("stateless");
            }
        }
    }
}

// Stereotype wrapper (a.k.a stateless)
//     is not empty
//     has exactly 1 call of any type (Can possibly change the object state indirectly)
//     does not use any attributes
//
void classModel::wrapper() {
    for (auto& m : method ) {
        if(!m.IsEmpty()){
            bool usedAttr = m.IsAttributeUsed();
            bool callsWrapper = (m.getMethodCall().size() + m.getFunctionCall().size() + m.getConstructorCall().size()) == 1;
            if (callsWrapper && !usedAttr)
                m.setStereotype("wrapper");         
        }
    }
}

std::string classModel::getStereotype () const {
    std::string result = "";

    for (const std::string &value : classStereotype)
        result += value + " ";

    Rtrim(result);
     
    return result;
}
