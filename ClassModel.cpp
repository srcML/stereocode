// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModel.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#include "ClassModel.hpp"

classModel::classModel () : classStereotype(), className(), parentClassName(), attribute(), 
                            nonPrivateAttribute(), method(), friendFunctionName(), xpath() {}

classModel::classModel (srcml_archive* archive, srcml_unit* unit, std::string xpathS, int unitNumber) : classModel() {
    xpath[unitNumber] = xpathS;
    findClassName(archive, unit);
}

void classModel::findClassInfo(srcml_archive* archive, srcml_unit* unit, int classOrder, int UnitNumber){
    if (PRIMITIVES.getLanguage() == "C++")
        findFriendFunction(archive, unit);
    findParentClassName(archive, unit);
    findAttributeName(archive, unit);
    findAttributeType(archive, unit);
    findNonPrivateAttributeName(archive, unit);
    findNonPrivateAttributeType(archive, unit);
    findMethod(archive, unit, classOrder, UnitNumber);
}

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
        friendFunctionName.push_back(name);

        free(name);    
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
}

void classModel::findClassName(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:class/src:name[count(ancestor::src:class) = 1]";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1){
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

void classModel::findParentClassName(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:super_list[count(ancestor::src:class) = 1]/src:super/src:name";
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

void classModel::findAttributeName(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:decl_stmt[not(ancestor::src:function)";
    xpath += " and count(ancestor::src:class) = 1]/src:decl/src:name";

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

void classModel::findAttributeType(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:decl_stmt[not(ancestor::src:function)";
    xpath += " and count(ancestor::src:class) = 1]/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    std::string prev = "";

    for (int i = 0; i < n; ++i) {
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
        type = trimWhitespace(removeSpecifiers(type));
        if (!isPrimitiveContainer(type))
            attribute[i].setNonPrimitive(true); // True if attribute is not a primitive type.
        
        free(unparsed);
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

void classModel::findNonPrivateAttributeName(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:decl_stmt[not(ancestor::src:function)";
    if (PRIMITIVES.getLanguage() == "C++")
        xpath += " and count(ancestor::src:class) = 1 and not(ancestor::src:private)]/src:decl/src:name";
    else
        xpath += " and count(ancestor::src:class) = 1  and *[1][src:type/src:specifier] and not(*[1][src:type/src:specifier='private'])]/src:decl/src:name";  
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

void classModel::findNonPrivateAttributeType(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:decl_stmt[not(ancestor::src:function)";
    if (PRIMITIVES.getLanguage() == "C++")
        xpath +=  " and count(ancestor::src:class) = 1 and not(ancestor::src:private)]/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    else
        xpath += " and count(ancestor::src:class) = 1 and *[1][src:type/src:specifier] and not(*[1][src:type/src:specifier='private'])]/src:decl/src:type";  
    
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    std::string prev = "";
    for (int i = 0; i < n; ++i) {
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
        type = trimWhitespace(removeSpecifiers(type));
        if (!isPrimitiveContainer(type))
            nonPrivateAttribute[i].setNonPrimitive(true); 
        
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// C# allow for local nested functions within methods.
//  For our analysis:
//      The outer method/function will ignore the definitions of the nested functions.
//      Any use of the nested function(s) will still be considered. For example, a call to
//          to a nested function within the outer method/function body is considered.
//      The nested functions are analyzed in the same way. 
//      But, any use of locals or parameters from the outer method/function(s) inside the nested functions is not considered.
//  C++ and Java don't have nested functions.
void classModel::findMethod(srcml_archive* archive, srcml_unit* unit, int classOrder, int unitNumber) {
    int methodOrder = 1;
    std::string xpath = "//src:function[count(ancestor::src:class) = 1]";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
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

        std::string fileName = srcml_unit_get_filename(resultUnit);
        std::string xpathM = "//src:unit[@filename='" + fileName + "']";
        xpathM += "//src:class[" + std::to_string(classOrder) + "]//src:function[" + std::to_string(methodOrder) + "]";

        methodModel m(temp, unitM, xpathM, unitNumber); 
        addMethod(temp, unitM, m, trimWhitespace(m.getMethodHeader()));

        free(unparsed);
        srcml_unit_free(unitM);
        srcml_archive_close(temp);
        srcml_archive_free(temp);
        methodOrder++;

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
        {"set", 0},
        {"command", 0},
        {"collaborator", 0},
        {"controller", 0},
        {"factory", 0},
        {"empty", 0},
        {"stateless", 0},
        {"wrapper", 0},
    };

    int nonCollaborators = 0;
    for (const auto& pair : method) {
        for (const std::string& s : pair.second.getStereotypeList()){
            if (stereotypes.find(s) != stereotypes.end()){
                if (s != "collaborator" && s != "controller")
                    nonCollaborators++;
                stereotypes[s]++;
            }
        }
    }
    int getters = stereotypes["get"];
    int accessors = getters + stereotypes["predicate"] + 
                    stereotypes["property"] + stereotypes["accessor"]; 

    int setters = stereotypes["set"];     
    int commands = stereotypes["command"] + stereotypes["non-void-command"];           
    int mutators = setters + commands;

    int controllers = stereotypes["controller"];
    int collaborators = controllers + stereotypes["collaborator"]; 
     

    int factory = stereotypes["factory"];

    int degenerates = stereotypes["empty"] + stereotypes["stateless"] + stereotypes["wrapper"]; 

    int allMethods = method.size();

    // Entity
    if (((accessors - getters) != 0) && ((mutators - setters)  != 0) && (controllers == 0)) {
        double ratio = double(collaborators) / double(nonCollaborators);
        if (ratio > 2) 
            classStereotype.push_back("entity");   
    }

    // Minimal Entity
    if (((allMethods - (getters + setters + commands)) == 0) && (getters != 0) && (setters != 0) & (commands != 0)) {
        double ratio = double(collaborators) / double(nonCollaborators);
        if (ratio > 2) {
            classStereotype.push_back("minimal-entity");
        }
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
    if ((controllers + factory != 0) && ((accessors + mutators + collaborators) == 0) && (controllers != 0)) 
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
    if ((allMethods - getters - setters == 0) && ((getters + setters) != 0))
         classStereotype.push_back("data-class");
    
    // Small Class
    if ((0 < allMethods) && (allMethods < 3))
        classStereotype.push_back("small-class");

    // Final check if no stereotype was assigned
    if (classStereotype.size() == 0) 
        classStereotype.push_back("unclassified");
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
}

// Stereotype get:
//     contains at least 1 return statement that returns an attribute
//     only simple return statements that return a single attribute are considered
//     For example, 'return a;', 'return (a);', 'return *a;', 'return a[index];', 'return this->a;',
//      'return className::a;', 'return {a};',  'return (*this).a;',
//
void classModel::getter() {
    for (auto& pair : method) {
        if (pair.second.getAttributeReturned() && !pair.second.getParameterChanged()) {
            if (!pair.second.getConstMethod() && PRIMITIVES.getLanguage() == "C++")
                pair.second.setStereotype("non-const");
            pair.second.setStereotype("get");
        }
    }
}

// Stereotype predicate:
//     returns boolean
//     return expression is not a attribute
//     uses an attribute in an expression
//
void classModel::predicate() { 
    for (auto& pair : method) {
        if (pair.second.getReturnTypeSeparated() == "bool" && !pair.second.getAttributeReturned() && 
            !pair.second.getParameterChanged() && pair.second.getAttributeUsed()) {
            if (!pair.second.getConstMethod() && PRIMITIVES.getLanguage() == "C++")
                pair.second.setStereotype("non-const");
            pair.second.setStereotype("predicate"); 
        }
    }
}

// Stereotype property:
//     return type is not boolean or void
//     return expression is not a attribute
//     uses an attribute in an expression
//
void classModel::property() {
    for (auto& pair : method) {
        std::string returnTypeSeparated = pair.second.getReturnTypeSeparated();
        if (returnTypeSeparated != "bool" && returnTypeSeparated != "void" && !pair.second.getAttributeReturned() && 
            !pair.second.getParameterChanged() && pair.second.getAttributeUsed()) {
            if (!pair.second.getConstMethod() && PRIMITIVES.getLanguage() == "C++")
                pair.second.setStereotype("non-const");
            pair.second.setStereotype("property");
        }
    }
}

// Stereotype accessor:
//     contains at least 1 parameter that is: 
//      passed by non-const reference and is assigned a value (one = in the expression)
//     covers a more general case where the method could return one or more attributes 
//      using the parameters and the method return.

void classModel::accessor() {
    for (auto& pair : method) {
        if (pair.second.getParameterChanged()){
            if (!pair.second.getConstMethod() && PRIMITIVES.getLanguage() == "C++")
                pair.second.setStereotype("non-const");
            pair.second.setStereotype("accessor");       
        } 
    }
}

// Stereotype set:
//     only 1 attribute is changed.
//     covers the case of mutable attributes.
//
void classModel::setter() {
    for (auto& pair : method) {
        if (pair.second.getAttributeModified() == 1 && ((pair.second.getMethodCalls().size() + pair.second.getFunctionCalls().size()) <= 1)) {
            pair.second.setStereotype("set");
        }
    }
}

// stereotype command:
//     Cases:
//         a) More than one attribute modified.
//         b) 1 Attribute modified and the # of calls (method and function calls) is at least 2.
//         c) 0 attributes modified and there is at least 1 method or function call on an attribute or the # of calls is at least 2.
//
//     Method is not const (C++ only)
//     Case 3 applies when attributes are mutable (C++ only)
//
// stereotype non-void-command (C++ only):        
//      Same as command.
//      Method has a 'void' return type
//
void classModel::command() {
    for (auto& pair : method) {
        std::vector<std::string> methodCalls = pair.second.getMethodCalls();
        std::vector<std::string> functionCalls = pair.second.getFunctionCalls();

        bool hasMethodCallsOnAttributes = pair.second.callOnAttribute(attribute, "method");
        bool hasFunctionCallsOnAttributes = pair.second.callOnAttribute(attribute, "function");
        std::string returnType = pair.second.getReturnTypeSeparated();
      
        size_t numOfCalls = functionCalls.size() + methodCalls.size();
        bool case1 = pair.second.getAttributeModified() == 0 && (hasFunctionCallsOnAttributes || hasMethodCallsOnAttributes || numOfCalls > 1);
        bool case2 = pair.second.getAttributeModified() == 1 && numOfCalls > 1;
        bool case3 = pair.second.getAttributeModified() > 1;
        if (case1 || case2 || case3) {
            if (!pair.second.getConstMethod() || (case3 && pair.second.getConstMethod())){ // Handles case of mutable attributes (C++ only)
                if (PRIMITIVES.getLanguage() == "C++")
                    if (returnType != "void") pair.second.setStereotype("non-void");  
                pair.second.setStereotype("command");
            }
        } 
    }
}

// Stereotype factory
//     factories must include a pointer to an object (non-primitive) in their return type
//      and their return expression must be a local variable, a parameter, an attribute, 
//      or a call to another object’s constructor using the keyword new. 
//
void classModel::factory() {
    for (auto& pair : method) {
            bool     returnsNew          = pair.second.getReturnsNew();          
            bool     returnsObj          = pair.second.getNonPrimitiveReturnType();
            bool     paraOrLocalReturned = pair.second.getParaOrLocalReturned(); 
            bool     attributeReturned   = pair.second.getAttributeReturned();  
            bool     returnsCall         = pair.second.getReturnsCall();

            bool newCalls = false;    
            if (pair.second.getConstructorCalls().size() > 0) newCalls = true;

            bool isFactory =  (returnsObj && ((returnsNew || 
                              (newCalls && (paraOrLocalReturned || attributeReturned))) ||
                              (returnsCall)));

            if (isFactory) pair.second.setStereotype("factory");
            
    }
}

// Stereotype collaborator:
//     it must have a non-primitive type as a parameter, or as a local variable, or as a return type (not void)
//     or is using a non-primitive type attribute.
//
// Stereotype controller:
//     All non-primitive types (if any) must be of a different type than the class.
//
void classModel::collaboratorController() {
    for (auto& pair : method) {
        if (!pair.second.getEmpty()){
            std::string classNameTemp = trimWhitespace(className.substr(0, className.find("<")));   

            // Check if method uses an attribute of a non-primitive type
            bool NonPrimitiveAttribute = pair.second.getNonPrimitiveAttribute();
            bool NonPrimitiveAttributeExternal = pair.second.getNonPrimitiveAttributeExternal();
            
            // Check for non-primitive local variable types
            bool NonPrimitiveLocal = pair.second.getNonPrimitiveLocal();
            bool NonPrimitiveLocalExternal = pair.second.getNonPrimitiveLocalExternal();


            // Check for non-primitive parameter types
            bool NonPrimitiveParamater = pair.second.getNonPrimitiveParamater();
            bool NonPrimitiveParamaterExternal = pair.second.getNonPrimitiveAttributeExternal();


            // Check for a non-primitive return type
            // Only ignores return of type void. Returns such as void* are not ignored
            // This is why void is not added to the list of primitives since void* can point to non-primitive
            bool returnNonPrimitive = pair.second.getNonPrimitiveReturnType();
            bool returnNonPrimitiveExternal = pair.second.getNonPrimitiveReturnTypeExternal();

            bool returnCheck = returnNonPrimitive && (pair.second.getReturnTypeSeparated() != "void");

            
            if (NonPrimitiveAttribute || NonPrimitiveLocal || NonPrimitiveParamater || returnCheck){
                if (NonPrimitiveAttributeExternal && NonPrimitiveLocalExternal && NonPrimitiveParamaterExternal && returnNonPrimitiveExternal)
                    pair.second.setStereotype("controller");     
                else 
                    pair.second.setStereotype("collaborator"); 
            }
        }
    }
}


// Stereotype empty
//
void classModel::empty() {
    for (auto& pair : method) {
        if (pair.second.getEmpty()) 
            pair.second.setStereotype("empty");
    }
}

// Stereotype stateless (a.k.a incidental or pure stateless)
//     is not an empty method
//     has no calls of any type
//     does not use any attributes
//
void classModel::stateless() {
    for (auto& pair : method) {
        if(!pair.second.getEmpty()){
            bool usedAttr = pair.second.getAttributeUsed();
            bool noCalls = (pair.second.getMethodCalls().size() + pair.second.getFunctionCalls().size() + pair.second.getConstructorCalls().size()) == 0 ;
            if (noCalls && !usedAttr) {
                pair.second.setStereotype("stateless");
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
    for (auto& pair : method ) {
        if(!pair.second.getEmpty()){
            bool usedAttr = pair.second.getAttributeUsed();
            bool callsWrapper = (pair.second.getMethodCalls().size() + pair.second.getFunctionCalls().size() + pair.second.getConstructorCalls().size()) == 1;
            if (callsWrapper && !usedAttr)
                pair.second.setStereotype("wrapper");         
        }
    }
}

std::string classModel::getClassStereotype () const{
    std::string result = "";

    for (const std::string &value : classStereotype)
        result += value + " ";

    if (result != "") return Rtrim(result);
        
    return result;
}
