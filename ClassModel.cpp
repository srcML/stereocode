// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModel.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "ClassModel.hpp"

classModel::classModel () : className(), unitLanguage(), classStereotype(), parentClassName(), attribute(), 
                            nonPrivateAttribute(), method(), friendFunctionName(), xpath(), isPartial(false) {}

classModel::classModel (srcml_archive* archive, srcml_unit* unit, std::string unitLang) : classModel() {
    unitLanguage = unitLang;
    findClassName(archive, unit);
    if (unitLang == "C#")
        findPartialClass(archive, unit);
}

void classModel::findClassData(srcml_archive* archive, srcml_unit* unit, std::string classXpath, int unitNumber){
    xpath[unitNumber].push_back(classXpath);

    if (unitLanguage == "C++")
        findFriendFunction(archive, unit);      
    findParentClassName(archive, unit);

    int attributeOldSize = attribute.size();
    findAttributeName(archive, unit);
    findAttributeType(archive, unit, attributeOldSize);

    int nonPrivateAttributeOldSize = nonPrivateAttribute.size();
    findNonPrivateAttributeName(archive, unit);
    findNonPrivateAttributeType(archive, unit, nonPrivateAttributeOldSize);
    findMethod(archive, unit, classXpath, unitNumber);

}

// Finds class name
//
void classModel::findClassName(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:class/src:name[count(ancestor::src:class) = 1]";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    
    if (n > 0) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);
        char *name = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &name, &size);
        className = name;
        free(name);    
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

// Finds partial class (C# only)
// The "partial" keyword in C# allows a class definition to be spread across multiple files or namespaces
//
void classModel::findPartialClass(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:class/src:specifier[.='partial']";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    
    if (n > 0) isPartial = true;
    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

// Finds friend functions
//
void classModel::findFriendFunction(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:friend[count(ancestor::src:class) = 1]/src:function_decl/src:name";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; i++) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, i);
        char *name = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &name, &size);
        friendFunctionName.insert(name);

        free(name);    
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
    std::string xpath ="";
    if (unitLanguage == "Java"){
        xpath += "//src:super_list[count(ancestor::src:class) = 1]/src:extends/src:super/src:name"; 
        xpath += " | //src:super_list[count(ancestor::src:class) = 1]/src:implements/src:super/src:name";
    }
    else
        xpath += "//src:super_list[count(ancestor::src:class) = 1]/src:super/src:name";

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);
        char* name = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &name, &size);
        parentClassName.push_back(name);
        free(name);      
    }
    
    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

// Finds attribute names
// Only collect the name if there is a type
//
void classModel::findAttributeName(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:decl_stmt[not(ancestor::src:function)";
    xpath += " and count(ancestor::src:class) = 1]/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";

    srcml_append_transform_xpath(archive, xpath.c_str());
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

        // Chop off [] for arrays  
        size_t start_position = name.find("[");
        if (start_position != std::string::npos){
            name = name.substr(0, start_position);
            name = Rtrim(name);
        }

        attribute.push_back(AttributeInfo(name)); 
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds attribute types
// Only collect the type if there is a name
//
void classModel::findAttributeType(srcml_archive* archive, srcml_unit* unit, int attributeOldSize) {
    std::string xpath = "//src:decl_stmt[not(ancestor::src:function)";
    xpath += " and count(ancestor::src:class) = 1]/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    srcml_append_transform_xpath(archive, xpath.c_str());
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

        attribute[i].setType(type);
        type = trimWhitespace(removeContainers(removeSpecifiers(type)));
        if (!isPrimitiveType(type))
            attribute[i].setNonPrimitive(true); // True if attribute is not a primitive type.
        
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
    std::string xpath = "//src:decl_stmt[not(ancestor::src:function)";
    if (unitLanguage == "C++")
        xpath += " and count(ancestor::src:class) = 1 and not(ancestor::src:private)]/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    else{
        xpath += " and count(ancestor::src:class) = 1  and *[1][src:type/src:specifier]";
        xpath += " and not(*[1][src:type/src:specifier='private'])]/src:decl/src:name[preceding-sibling::*[1][self::src:type]]"; 
    }
 
    srcml_append_transform_xpath(archive, xpath.c_str());
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
    
        // Chop off [] for arrays      
        size_t start_position = name.find("[");
        if (start_position != std::string::npos){
            name = name.substr(0, start_position);
            name = Rtrim(name);
        }

        nonPrivateAttribute.push_back(AttributeInfo(name)); 
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds non-private attribute types
//
void classModel::findNonPrivateAttributeType(srcml_archive* archive, srcml_unit* unit, int nonPrivateAttributeOldSize) {
    std::string xpath = "//src:decl_stmt[not(ancestor::src:function)";
    if (unitLanguage == "C++")
        xpath +=  " and count(ancestor::src:class) = 1 and not(ancestor::src:private)]/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    else{
        xpath += " and count(ancestor::src:class) = 1 and *[1][src:type/src:specifier]"; 
        xpath += " and not(*[1][src:type/src:specifier='private'])]/src:decl/src:type[following-sibling::*[1][self::src:name]]";  
    }

    srcml_append_transform_xpath(archive, xpath.c_str());
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
        type = trimWhitespace(removeContainers(removeSpecifiers(type)));
        if (!isPrimitiveType(type))
            nonPrivateAttribute[i].setNonPrimitive(true); 
        
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
    std::string xpathM = "//src:function[count(ancestor::src:class) = 1]";
    srcml_append_transform_xpath(archive, xpathM.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
        
    srcml_unit* resultUnit = nullptr;
    if (parentClassName.size() > 0 && className == "Form1")
        {
            std::ofstream aa("a.txt");
            aa << srcml_unit_get_srcml(unit)<<std::endl;
        }
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        
        srcml_archive* temp = srcml_archive_create();
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_archive_write_open_memory(temp, &unparsed, &size);
        srcml_archive_write_unit(temp, resultUnit);
        srcml_archive_close(temp);
        srcml_archive_free(temp);

        temp = srcml_archive_create();
        srcml_archive_read_open_memory(temp, unparsed, size);
        srcml_unit* unitM = srcml_archive_read_unit(temp);

        methodModel m(temp, unitM); 

        std::string xpathS = classXpath + "//src:function[count(ancestor::src:class) = 1][" + std::to_string(i+1) + "]";

        addMethod(temp, unitM, m, xpathS, unitNumber);
        
        free(unparsed);
        srcml_unit_free(unitM);
        srcml_archive_close(temp);
        srcml_archive_free(temp);
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

    // Empty Class
    if (allMethods == 0)
        classStereotype.push_back("empty");

    // Final check if no stereotype was assigned
    if (classStereotype.size() == 0) 
        classStereotype.push_back("unclassified");

    for (const auto& pair : xpath) {
        for (const auto& classXpath : pair.second) {
            xpathList[pair.first].push_back(std::make_pair(classXpath, getClassStereotype())); 

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
    for (const auto& m : method) { 
        int unitNumber = m.getUnitNumber();
        xpathList[unitNumber].push_back(std::make_pair(m.getXpath(), m.getStereotype()));
    }
}

// Stereotype get:
//     Contains at least 1 return statement that returns an attribute
//     Only simple return statements that return a single attribute are considered
//      For example, 'return a;', 'return (a);', 'return *a;', 'return a[index];', 'return this->a;',
//       'return className::a;', 'return {a};',  'return (*this).a;'
//
void classModel::getter() {
    for (auto& m : method) {
        if (m.getAttributeReturned() && !m.getParameterChanged()) {
            if (!m.getConstMethod() && unitLanguage == "C++")
                m.setStereotype("non-const-get");
            else
                m.setStereotype("get");
        }
    }
}

// Stereotype predicate:
//     Return type is Boolean
//     Return expression is not a attribute
//     Uses an attribute in an expression
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

        if (returnType && !m.getAttributeReturned() && 
            !m.getParameterChanged() && m.getAttributeUsed()) {
            if (!m.getConstMethod() && unitLanguage == "C++")
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
//
void classModel::property() {
    for (auto& m : method) {
        bool returnType = false;
        std::string returnTypeSeparated = m.getReturnTypeSeparated();

        if (unitLanguage == "C++")
            returnType = (returnTypeSeparated != "bool" && returnTypeSeparated != "void");
        else if (unitLanguage == "C#")
            returnType = (returnTypeSeparated != "bool" && returnTypeSeparated != "Boolean" &&
                          returnTypeSeparated != "void" && returnTypeSeparated != "Void");
        else
            returnType = (returnTypeSeparated != "boolean" && returnTypeSeparated != "void" && returnTypeSeparated != "Void");

        if (returnType && !m.getAttributeReturned() && 
            !m.getParameterChanged() && m.getAttributeUsed()) {
            if (!m.getConstMethod() && unitLanguage == "C++")
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
        if (m.getParameterChanged()){
            if (!m.getConstMethod() && unitLanguage == "C++")
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
//      Same as command.
//      Method has a 'void' return type
//
void classModel::command() {
    for (auto& m : method) {
        std::vector<std::string> methodCalls = m.getMethodCall();
        std::vector<std::string> functionCalls = m.getFunctionCall();

        bool hasMethodCallsOnAttribute = m.getMethodCallOnAttribute();
        bool hasFunctionCallsOnAttribute = m.getFunctionCallOnAttribute();
        std::string returnType = m.getReturnTypeSeparated();
      
        size_t numOfCalls = functionCalls.size() + methodCalls.size();
        bool case1 = m.getAttributeModified() == 0 && (hasFunctionCallsOnAttribute || hasMethodCallsOnAttribute || numOfCalls > 1);
        bool case2 = m.getAttributeModified() == 1 && numOfCalls > 1;
        bool case3 = m.getAttributeModified() > 1;
        if (case1 || case2 || case3) {
            if (!m.getConstMethod() || (case3 && m.getConstMethod())){ // Handles case of mutable attributes (C++ only)
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
            bool     returnsNew          = m.getReturnsNew();          
            bool     returnsObj          = m.getNonPrimitiveReturnType();
            bool     paraOrLocalReturned = m.getParaOrLocalReturned(); 
            bool     attributeReturned   = m.getAttributeReturned();  
            bool     returnsCall         = m.getReturnsCall();

            bool newCalls = false;    
            if (m.getConstructorCall().size() > 0) newCalls = true;

            bool isFactory =  (returnsObj && ((returnsNew || 
                              (newCalls && (paraOrLocalReturned || attributeReturned))) ||
                              (returnsCall)));

            if (isFactory) m.setStereotype("factory");
            
    }
}

// Stereotype collaborator:
//     It must have a non-primitive type as a parameter, or as a local variable, or as a return type (not void)
//      or is using a non-primitive type attribute
//     All non-primitive types (if any) must be of a different type than the class
//
// Stereotype controller:
//     Collaborates with other objects without changing the state of the current object
//     No attributes are modified
//     No method calls on attributes
//     No function calls
//
void classModel::collaboratorController() {
    for (auto& m : method) {
        if (!m.getEmpty()){
            // Check if method uses an attribute of a non-primitive type
            bool NonPrimitiveAttribute = m.getNonPrimitiveAttribute();
            bool NonPrimitiveAttributeExternal = m.getNonPrimitiveAttributeExternal();
            
            // Check for non-primitive local variable types
            bool NonPrimitiveLocal = m.getNonPrimitiveLocal();
            bool NonPrimitiveLocalExternal = m.getNonPrimitiveLocalExternal();


            // Check for non-primitive parameter types
            bool NonPrimitiveParamater = m.getNonPrimitiveParamater();
            bool NonPrimitiveParamaterExternal = m.getNonPrimitiveAttributeExternal();

            // Check for a non-primitive return type
            // Only ignores return of type "void". Returns such as "void*"" are not ignored
            // This is why void is not added to the list of primitives (for C++ only) since void* can point to non-primitive
            bool returnNonPrimitive = m.getNonPrimitiveReturnType();
            bool returnNonPrimitiveExternal = m.getNonPrimitiveReturnTypeExternal();

            bool returnCheck = returnNonPrimitive && (m.getReturnTypeSeparated() != "void");
           
            if (NonPrimitiveAttribute || NonPrimitiveLocal || NonPrimitiveParamater || returnCheck) {
                 if (NonPrimitiveAttributeExternal && NonPrimitiveLocalExternal && NonPrimitiveParamaterExternal && returnNonPrimitiveExternal) {
                    if (m.getFunctionCall().size() == 0 && !m.getMethodCallOnAttribute() && (m.getAttributeModified() == 0))
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
        if (m.getEmpty()) 
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
        if(!m.getEmpty()){
            bool usedAttr = m.getAttributeUsed();
            bool noCalls = (m.getMethodCall().size() + m.getFunctionCall().size() + m.getConstructorCall().size()) == 0 ;
            if (noCalls && !usedAttr) {
                m.setStereotype("stateless");
            }
        }
    }
}

// Stereotype wrapper (a.k.a stateless)
//     is not empty
//     has exactly 1 call of any type
//     does not use any attributes
//
void classModel::wrapper() {
    for (auto& m : method ) {
        if(!m.getEmpty()){
            bool usedAttr = m.getAttributeUsed();
            bool callsWrapper = (m.getMethodCall().size() + m.getFunctionCall().size() + m.getConstructorCall().size()) == 1;
            if (callsWrapper && !usedAttr)
                m.setStereotype("wrapper");         
        }
        if (m.getStereotype() == "") // This here could breaks the disjointness of the rules. Should be outside if needed.
            m.setStereotype("unclassified");
    }
}

std::string classModel::getClassStereotype () const{
    std::string result = "";

    for (const std::string &value : classStereotype)
        result += value + " ";

    if (result != "") return Rtrim(result);
        
    return result;
}
