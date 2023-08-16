// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassInfo.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "ClassInfo.hpp"

classModel::classModel(srcml_archive* archive, srcml_unit* firstUnit, srcml_unit* secondUnit) : classModel() {
    language = srcml_unit_get_language(firstUnit);
    PRIMITIVES.setLanguage(language);

    // Get class information
    findClassName(archive, firstUnit);
    findParentClassName(archive, firstUnit);
    findAttributeNames(archive, firstUnit);
    findAttributeTypes(archive, firstUnit);

    // Get the methods
    findMethods(archive, firstUnit, true);
    if (secondUnit) findMethods(archive, secondUnit, false); // Skip if there is no second unit

    // Get basic information on methods
    findMethodNames();
    findParameterLists();
    findMethodReturnTypes();
    findLocalVariablesNames();
    findLocalVariablesTypes();
    findParametersNames();
    findParametersTypes();
    findAllCalls();

    isAttributeReturned();
    isAttributeUsed();
    isParameterModified();
    isEmptyMethod();

    countChangedAttributes();
}

// Working on Class.
//
void classModel::findClassName(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, "//src:class/src:name");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result,i);

        char* name = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &name, &size);
        
        className = name;
        free(name);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

void classModel::findParentClassName(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, "//src:class/src:super_list/src:super/src:name");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        char* name = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &name, &size);

        parentClass.push_back(name);
        free(name);      
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Working on Class Attributes
//
void classModel::findAttributeNames(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, "//src:class//src:decl_stmt[not(ancestor::src:function)]/src:decl/src:name");
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
        if(start_position != std::string::npos){
            name = name.substr(0,start_position);
            name = Rtrim(name);
        }
    
        attribute.push_back(AttributeInfo(name)); 
     }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

void classModel::findAttributeTypes(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, "//src:class//src:decl_stmt[not(ancestor::src:function)]/src:decl/src:type");
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

        free(unparsed);
        attribute[i].setType(type);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Working on Methods
//
//  Calls constructor for methodModel and finds:
//      srcML for the method
//      header
//      const-ness
//
void classModel::findMethods(srcml_archive* archive, srcml_unit* unit, bool oneUnit) {
    srcml_append_transform_xpath(archive, "//src:function");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (oneUnit) {
        unitOneCount = n;       
    } 
    else {
        unitTwoCount = n;       
    }

    srcml_unit* resultUnit = nullptr;
    srcml_archive* temp = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        temp = srcml_archive_create();
        char* str = nullptr;
        size_t s = 0;
        srcml_archive_write_open_memory(temp, &str, &s);
        srcml_archive_write_unit(temp, resultUnit);
        srcml_archive_close(temp);
        srcml_archive_free(temp);
        std::string xml = str;
        free(str);

        // Checks if method is const
        bool isConstMethod = checkConst(xml);

        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string header = unparsed;
        free(unparsed); 
        header = header.substr(0, header.find("{"));
        header = Rtrim(header);
  
        method.push_back(methodModel(xml, header, isConstMethod));
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Gets the name of all methods
//
void classModel::findMethodNames() {
    for (int i = 0; i < method.size(); ++i) {
        srcml_archive*          archive = nullptr;
        srcml_unit*             unit = nullptr;
        srcml_unit*             resultUnit = nullptr;
        srcml_transform_result* result = nullptr;

        archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, method[i].getMethod().c_str(), method[i].getMethod().size()); 
        unit = srcml_archive_read_unit(archive);

        srcml_append_transform_xpath(archive, "//src:function/src:name");
        srcml_unit_apply_transforms(archive, unit, &result);
        resultUnit = srcml_transform_get_unit(result, 0);

        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string name = unparsed;
        free(unparsed);

        method[i].setName(name);

        srcml_unit_free(unit);
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_archive_free(archive);
    }
}

// Gets the parameter list of all methods
//
void classModel::findParameterLists() {
    for (int i = 0; i < method.size(); ++i) {
        srcml_archive*          archive = nullptr;
        srcml_unit*             unit = nullptr;
        srcml_unit*             resultUnit = nullptr;
        srcml_transform_result* result = nullptr;

        archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, method[i].getMethod().c_str(), method[i].getMethod().size());
        unit = srcml_archive_read_unit(archive);

        srcml_append_transform_xpath(archive, "//src:function/src:parameter_list");
        srcml_unit_apply_transforms(archive, unit, &result);
        resultUnit = srcml_transform_get_unit(result, 0);

        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string parameter_list = unparsed;
        free(unparsed);

        method[i].setParametersList(parameter_list);

        srcml_unit_free(unit);
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_archive_free(archive);
     }
}

// Gets the return types for each method
//
void classModel::findMethodReturnTypes() {
    for (int i = 0; i < method.size(); ++i) {
        srcml_archive*          archive = nullptr;
        srcml_unit*             unit = nullptr;
        srcml_unit*             resultUnit = nullptr;
        srcml_transform_result* result = nullptr;

        archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, method[i].getMethod().c_str(), method[i].getMethod().size());
        unit = srcml_archive_read_unit(archive);

        srcml_append_transform_xpath(archive, "//src:function/src:type");
        srcml_unit_apply_transforms(archive, unit, &result);
        resultUnit = srcml_transform_get_unit(result, 0);

        char * unparsed = nullptr;
        size_t size = 0;
        int error = srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string type = unparsed;
        free(unparsed);

        method[i].setReturnType(type);

        srcml_unit_free(unit);
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_archive_free(archive);
    }
}

// Collects the names of local variables in each method
//
void classModel::findLocalVariablesNames() {
    for (int i = 0; i < method.size(); ++i) {
        method[i].setLocalVariablesNames(method[i].findLocalVariables(className, true));
    }
}

// Collects the types of local variables in each method
//
void classModel::findLocalVariablesTypes() {
    for (int i = 0; i < method.size(); ++i) {
        method[i].setLocalVariablesTypes(method[i].findLocalVariables(className, false));
    }
}

// Collects the names of parameters in each method
//
void classModel::findParametersNames() {
    for (int i = 0; i < method.size(); ++i) {
        method[i].setParameterNames(method[i].findParameters(className, true)); 
    }
}

// Collects the types of parameters in each method
//
void classModel::findParametersTypes() {
    for (int i = 0; i < method.size(); ++i) {
        method[i].setParameterTypes(method[i].findParameters(className, false));   
    }
}

void classModel::findAllCalls(){
    for (int i = 0; i < method.size(); ++i) {
        method[i].setFunctionCalls(method[i].findCalls("function", true));
        method[i].setMethodCalls(method[i].findCalls("method", true));
        method[i].setConstructorCalls(method[i].findCalls("constructor", true));
    }
}

void classModel::isAttributeReturned() {
    for (int i = 0; i < method.size(); ++i) {
        method[i].setIsAttributeReturned(method[i].returnsAttribute(attribute, inherits()));
    }
}

void classModel::isAttributeUsed(){
    for (int i = 0; i < method.size(); ++i) {
        method[i].setIsAttributeUsed(method[i].usesAttribute(attribute, inherits()));
    }
}

// Checks whether a method modifies a parameter that is passed by reference and is not const
void classModel::isParameterModified(){
    for (int i = 0; i < method.size(); ++i) {
        std::vector<std::string> paramTypes = method[i].getParameterTypes();
        std::vector<std::string> paramNames = method[i].getParameterNames();

        for (int j = 0; j < paramTypes.size(); ++j){
            bool reference = paramTypes[j].find("&") != std::string::npos;
            bool constant = paramTypes[j].find("const") != std::string::npos;
            
            // If the parameter type contains an &, is not const
            if (reference && !constant) {
                // True if parameter is changed. For example, a = something where a is the parameter
                bool paramChanged = method[i].variableChanged(paramNames[j]); 
                    if (paramChanged) method[i].setIsParamChanged(true);            
            }
        }    
    }
 }

void classModel::isEmptyMethod(){
    for (int i = 0; i < method.size(); ++i) {
        method[i].setIsEmpty(method[i].isEmptyMethod()); // Check if method is empty
    }
}

// Need to count different attributes and not the same one twice.
// check with multiple changes with same operator(same var), different operators(same var), same operator(dif var)
// test ++attribute += 10;
//
void classModel::countChangedAttributes() {
    for (int i = 0; i < method.size(); ++i) {
        int changes = 0;
        changes += method[i].findIncrementedAttribute(attribute, false);     // No loop
        changes += method[i].findIncrementedAttribute(attribute, true);      // loop
        changes += method[i].findAssignOperatorAttribute(attribute, false);  // No loop
        changes += method[i].findAssignOperatorAttribute(attribute, true);   // loop

        method[i].setAttributesModified(changes);
    }
}

// Determining STEREOTYPES
//
// Compute class stereotype
//  Based on definition from Dragan, Collard, Maletic ICSM 2010
//
void classModel::ComputeClassStereotype() {
    set allMethods;
    set accessors, mutators, getters, setters;
    set collaborators, nonCollaborators, controllers, factory;
    set commands; 
    set degenerates;

    // Determine sets
    for (int i=0; i < method.size(); ++i) {
        std::string s = method[i].getStereotype();
        allMethods += i;
        if (s.find("get") != std::string::npos) {  
            accessors += i;
            getters += i;
        }
        if ((s.find("predicate") != std::string::npos) ||
            (s.find("property") != std::string::npos) ||
            (s.find("accessor") != std::string::npos) ) {
            accessors += i;
        }
        if ((s.find("set") != std::string::npos)) {
            mutators += i;
            setters += i;
        }
        if (s.find("command") != std::string::npos) {
            mutators += i;
        }
        if ((s.find("collaborator") != std::string::npos) ||
             (s.find("controller") != std::string::npos)) {
            collaborators += i;
        } 
        else {
            nonCollaborators += i;
        }
        if (s.find("controller") != std::string::npos) {
            controllers += i;
        }
        if (s.find("factory") != std::string::npos) {
            factory += i;
        }
        if (s.find("command") != std::string::npos) {
            commands += i;
        }
        if ( (s.find("empty") != std::string::npos) ||
             (s.find("stateless") != std::string::npos) ||
             (s.find("wrapper") != std::string::npos)) {
            degenerates += i;
        }
    }


    if (DEBUG) {  // Print out sets to check
        std::cerr << "Methods: " << allMethods.card() << " " << allMethods << std::endl;
        std::cerr << "Accessors: " << accessors.card() << " " << accessors << std::endl;
        std::cerr << "Getters: " << getters.card() << " " << getters << std::endl;
        std::cerr << "Mutators: " << mutators.card() << " " << mutators << std::endl;
        std::cerr << "Setters: " << setters.card() << " " << setters << std::endl;
        std::cerr << "Collaborators: " << collaborators.card() << " " << collaborators << std::endl;
        std::cerr << "Non Collaborators: " << nonCollaborators.card() << " " << nonCollaborators << std::endl;
        std::cerr << "Controllers: " << controllers.card() << " " << controllers << std::endl;
        std::cerr << "Factory: " << factory.card() << " " << factory << std::endl;
        std::cerr << "Commands: " << commands.card() << " " << commands << std::endl;
        std::cerr << "Degenerates: " << degenerates.card() << " " << degenerates << std::endl;
    }

    // Entity
    if ( ((accessors - getters) != set()) &&
         ((mutators - setters)  != set()) &&
         (controllers.card() == 0) ) {
        double ratio = double(collaborators.card()) / double(nonCollaborators.card());
        if (ratio > 1.9) {
            classStereotype.push_back("entity");
        }
    }
    // Minimal Entity
    if ((allMethods - (getters + setters + commands)) == set()) {
        double ratio = double(collaborators.card()) / double(nonCollaborators.card());
        if (ratio > 1.9) {
            classStereotype.push_back("minimal-entity");
        }
    }
    // Data Provider
    if ( (accessors.card() > 2 * mutators.card()) &&
         (accessors.card() > 2 * (controllers.card() + factory.card())) ) {
        classStereotype.push_back("data-provider");
    }
    // Commander
    if ( (mutators.card() > 2 * accessors.card()) &&
         (mutators.card() > 2 * (controllers.card() + factory.card())) ) {
        classStereotype.push_back("command");
    }
    // Boundary
    if ( (collaborators.card() > nonCollaborators.card()) &&
         (factory.card() < 0.5 * allMethods.card()) &&
         (controllers.card() < 0.33 * allMethods.card()) ) {
        classStereotype.push_back("boundary");
    }
    // Factory
    if (factory.card() > 0.66 * allMethods.card()) {
        classStereotype.push_back("factory");
    }
    // Controller
    if ( (controllers.card() + factory.card() > 0.66 * allMethods.card()) &&
         ((accessors.card() != 0) || (mutators.card() != 0)) ) {
        classStereotype.push_back("control");
    }
    // Pure Controller
    if ( (controllers.card() + factory.card() != 0) &&
         (accessors.card() + mutators.card() + collaborators.card() == 0) &&
         (controllers.card() != 0) ) {
        classStereotype.push_back("pure-control");
    }
    // Large Class
    {
        int accPlusMut = accessors.card() + mutators.card();
        int facPlusCon = controllers.card() + factory.card();
        int m = allMethods.card();
        if ( ((0.2 * m < accPlusMut) && (accPlusMut < 0.67 * m )) &&
             ((0.2 * m < facPlusCon) && (facPlusCon < 0.67 * m )) &&
             (factory.card() != 0) && (controllers.card() != 0) &&
             (accessors.card() != 0) && (mutators.card() != 0) ) {
            if (m > METHODS_PER_CLASS_THRESHOLD) { //Average methods/class + 1 std (system wide)
                classStereotype.push_back("large-class");
            }
        }
    }
    // Lazy Class
    if ( (getters.card() + setters.card() != 0) &&
         (degenerates.card() / double(allMethods.card()) > 0.33) &&
         ((allMethods.card() - (degenerates.card() + getters.card() + setters.card())) / double(allMethods.card())  <= 0.2) ) {
        classStereotype.push_back("lazy-class");
    }
    // Degenerate Class
    if (degenerates.card() / double(allMethods.card()) > 0.33)  {    
        classStereotype.push_back("degenerate");
    }
    // Data Class
    if (allMethods.card() - getters.card() - setters.card() == 0)  {
         classStereotype.push_back("data-class");
    }
    // Small Class
    if (allMethods.card() < 3)  {
        classStereotype.push_back("small-class");
    }
    // Final check if no stereotype was assigned
    if (classStereotype.size() == 0) classStereotype.push_back("unclassified");
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
//     return expression must be in the form 'return a;' or 'return *a;' or 'return **a;' where a is the attribute
//
void classModel::getter() {
    for (int i = 0; i < method.size(); ++i) {
        if (method[i].getIsAttributeReturned() && !method[i].getIsParamChanged()) {
            method[i].setStereotype("get");
        }
    }
}

// Stereotype predicate:
//     returns boolean
//     return expression is not a attribute
//     uses an attribute in an expression
//
void classModel::predicate() { 
    for (int i = 0; i < method.size(); ++i) {
        std::string returnType = separateTypeName(method[i].getReturnType());
        if (returnType == "bool" && !method[i].getIsAttributeReturned() && !method[i].getIsParamChanged() && method[i].getIsAttributeUsed()) {
            method[i].setStereotype("predicate"); 
        }
    }
}

// Stereotype property:
//     return type is not boolean or void
//     return expression is not a attribute
//     uses an attribute in an expression
//
void classModel::property() {
    for (int i = 0; i < method.size(); ++i) {
        std::string returnType = separateTypeName(method[i].getReturnType());
        if (returnType != "bool" && returnType != "void" && !method[i].getIsAttributeReturned() && !method[i].getIsParamChanged() && method[i].getIsAttributeUsed()) {
            method[i].setStereotype("property"); 
        }
    }
}

// Stereotype accessor:
//     contains at least 1 parameter that is: 
//     passed by non-const reference
//     and is assigned a value (one = in the expression).
//
void classModel::accessor() {
    for (int i = 0; i < method.size(); ++i) {
        if (method[i].getIsParamChanged())
            method[i].setStereotype("accessor");                
    }
}

// Stereotype set:
//     only 1 attribute has been changed
//
void classModel::setter() {
    for (int i = 0; i < method.size(); ++i) {
        std::string returnType = separateTypeName(method[i].getReturnType());
        if (method[i].getAttributesModified() == 1 && method[i].getMethodCalls().size() < 1 && method[i].getFunctionCalls().size() < 1) {
            method[i].setStereotype("set");
        }
    }
}

// stereotype command:
//    Case 1: at least one method or one function call that uses an attribute
//    Case 2: one or zero attributes are modified and more than one function and method calls
//    Case 3: more than 1 attribute is modified
//
void classModel::command() {
    for (int i = 0; i < method.size(); ++i) {
        std::vector<std::string> methodsCalls = method[i].getMethodCalls();
        std::vector<std::string> functionCalls = method[i].getFunctionCalls();

        bool hasMethodCallsOnAttributes = method[i].callsOnAttributes(attribute, "method");
        bool hasFunctionCallsOnAttributes = method[i].callsOnAttributes(attribute, "function");
        std::string returnType = separateTypeName(method[i].getReturnType());
      
        size_t numOfCalls = functionCalls.size() + methodsCalls.size();
        bool case1 = method[i].getAttributesModified() == 0 && (numOfCalls > 1 || (hasFunctionCallsOnAttributes || hasMethodCallsOnAttributes));
        bool case2 = method[i].getAttributesModified() == 1 && numOfCalls > 0;
        bool case3 = method[i].getAttributesModified() > 1;
        if (case1 || case2 || case3) {
            method[i].setStereotype("command");
        } 
    }
}

// Stereotype factory
//     factories must include a pointer to an object (non-primitive) in their return type
//     and their return expression must be a local variable, a parameter, an attribute, 
//     or a call to another object’s constructor using the keyword new. The constructor call doesn't
//     have to be in the return expression.
//
void classModel::factory() {
    for (int i = 0; i < method.size(); ++i) {
            bool                     returnsLocal       = false;
            bool                     returnsNew         = false;
            bool                     returnsParam       = false;

            std::vector<std::string> paramNames         = method[i].getParameterNames();
            std::vector<std::string> localNames         = method[i].getLocalVariablesNames();

            std::vector<std::string> returnExpressions  = method[i].findReturnExpressions(false);
            bool                     returnsPtr         = method[i].getReturnType().find("*") != std::string::npos;
            bool                     returnsObj         = !isPrimitiveContainer(method[i].getReturnType());


            for (int i = 0; i < returnExpressions.size(); ++i) {
                for (int j = 0; j < localNames.size(); ++j) {
                    if (returnExpressions[i] == localNames[j])
                        returnsLocal = true; 
                }
                for (int k = 0; k < paramNames.size(); ++k) {
                    if (returnExpressions[i] == paramNames[k])
                        returnsParam = true;
                }
                if (returnExpressions[i].find("new") != std::string::npos)
                    returnsNew = true;
            }
            std::vector<std::string> constructorCalls = method[i].getConstructorCalls();
            bool newCalls = false;
            
            if (constructorCalls.size() > 0) newCalls = true;

            bool returnEx  = (returnsLocal || returnsNew || returnsParam || method[i].getIsAttributeReturned());
            bool isFactory = returnsPtr && returnsObj && newCalls && returnEx;

            if (isFactory) {
                method[i].setStereotype("factory");
            }
    }
}

// Stereotype collaborator:
//     it must have a non-primitive type as a parameter, local variable or return type (not void)
//     or is using a non-primitive type attribute
//
// Stereotype controller:
//     it must have a non-primitive type as a parameter, local variable or return type (not void)
//     or is using a non-primitive type attribute
//
void classModel::collaboratorController() {
    std::vector<AttributeInfo> nonPrimitiveAttributes;
    for (int i = 0; i < attribute.size(); i++){     
        std::string type = attribute[i].getType();
        bool isPrimitive = isPrimitiveContainer(type);
        if (!isPrimitive) {
            nonPrimitiveAttributes.push_back(AttributeInfo(attribute[i].getName(), type));
        }
    }
    for (int i = 0; i < method.size(); ++i) {
        // Check if method uses an attribute of a non-primitive type
        bool usesAttrNotSameClass = method[i].usesNonPrimitiveAttributes(nonPrimitiveAttributes, "");
        bool usesAttrSameClass = method[i].usesNonPrimitiveAttributes(nonPrimitiveAttributes, className);
        
        // Check for non-primitive local variable types
        bool usesLocalNotSameClass = false;
        bool usesLocalSameClass = false;
        for(std::string type: method[i].getLocalVariablesTypes()){
            bool isPrimitive = isPrimitiveContainer(type);
            bool isClassName = type.find(className) != std::string::npos;
            if (!isPrimitive && !isClassName) {  
                usesLocalNotSameClass = true;
            }
            else if (!isPrimitive && isClassName){
                usesLocalSameClass = true;
            }
        }

        // Check for non-primitive parameter types
        bool usesParaNotSameClass = false;
        bool usesParaSameClass = false;
        for(std::string type: method[i].getParameterTypes()){
            bool isPrimitive = isPrimitiveContainer(type);
            bool isClassName = type.find(className) != std::string::npos;
            if (!isPrimitive && !isClassName) {  
                usesParaNotSameClass = true;
            }
            else if (!isPrimitive && isClassName){
                usesParaSameClass = true;
            }
        }

        // Check for a non-primitive return type
        // Only ignores return of type void. Returns such as void* are not ignored
        std::string methodReturnType = method[i].getReturnType();
        bool isClassName = methodReturnType.find(className) != std::string::npos;
        bool retObjNotSameClass = !isPrimitiveContainer(methodReturnType) && !isClassName && methodReturnType != "void";
        bool retObjSameClass = !isPrimitiveContainer(methodReturnType) && isClassName && methodReturnType != "void";

        if (usesAttrNotSameClass || usesLocalNotSameClass || usesParaNotSameClass || retObjNotSameClass
            && (!usesAttrSameClass && !usesLocalSameClass && !usesParaSameClass && !retObjSameClass)) {
                method[i].setStereotype("controller");
        }
        else if (usesAttrSameClass || usesLocalSameClass || usesParaSameClass || retObjSameClass ||
                    usesAttrNotSameClass || usesLocalNotSameClass || usesParaNotSameClass || retObjNotSameClass) {
                method[i].setStereotype("collaborator");
        }    
    }
}


// Stereotype empty
//
void classModel::empty() {
    for (int i = 0; i < method.size(); ++i) {
        if (method[i].getIsEmpty()) 
            method[i].setStereotype("empty");
    }
}

// Stereotype stateless (a.k.a incidental or pure stateless)
//     is not an empty method
//     has no calls of any type
//     does not use any attributes
//
void classModel::stateless() {
    for (int i = 0; i < method.size(); ++i) {
        if(!method[i].getIsEmpty()){
            bool usedAttr = method[i].getIsAttributeUsed();
            bool callsStateless = method[i].getMethodCalls().size() < 1 && method[i].getFunctionCalls().size() < 1  && method[i].getConstructorCalls().size() < 1;

            if (callsStateless && !usedAttr) {
                method[i].setStereotype("stateless");
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
    for (int i = 0; i < method.size(); ++i) {
        if(!method[i].getIsEmpty()){
            bool usedAttr = method[i].getIsAttributeUsed();
            bool callsWrapper = (method[i].getMethodCalls().size() +  method[i].getFunctionCalls().size() + method[i].getConstructorCalls().size()) == 1;

            if (callsWrapper && !usedAttr) {
                method[i].setStereotype("wrapper");
            }
        }
    }
}

std::string classModel::getClassStereotype () const{
    std::string result = "";

    for (const std::string &value : classStereotype)
        result += value + " ";

    if (result != "")
        return Rtrim(result);
        
    return result;
}

//  Copy unit and add in stereotype attribute on <class> and <function>
//  Example: <function st:stereotype="get"> ... </function>
//           <class st:stereotype="boundary"> ... ></class>
//
srcml_unit* classModel::outputUnitWithStereotypes(srcml_archive* archive, srcml_unit* unit, srcml_transform_result** result, bool oneUnit) {
    int n = unitOneCount;
    int offset = 0;
    if (!oneUnit) {
        n = unitTwoCount;
        offset = unitOneCount;
    }
    if (oneUnit) { // Add stereotype attribute to <class>
        std::string xpath = "//src:class";
        srcml_append_transform_xpath_attribute(archive, xpath.c_str(), "st",
                                               "http://www.srcML.org/srcML/stereotype",
                                               "stereotype", getClassStereotype().c_str());
    }

    for (int i = 0; i < n; ++i) { // Add stereotype attribute to each method/function
        std::string stereotype = method[i+offset].getStereotype();
        
        std::string xpath = "//src:function[string(src:name)='";
        xpath += method[i+offset].getName() + "' and string(src:type)='";
        xpath += method[i+offset].getReturnType() + "' and string(src:parameter_list)='";
        xpath += method[i+offset].getParametersList() + "']";
        srcml_append_transform_xpath_attribute(archive, xpath.c_str(), "st",
                                               "http://www.srcML.org/srcML/stereotype",
                                               "stereotype", stereotype.c_str());
    }

    srcml_unit_apply_transforms(archive, unit, result);
    unit = srcml_transform_get_unit(*result, 0); // "result" contains a list of units. "unit" is simply a pointer to the unit at the specified index.
    srcml_clear_transforms(archive);
    return unit;
}

// Outputs a report file for a class (tab separated)
// filepath || class name || method || stereotypes
//
void classModel::outputReport(std::ofstream& out, const std::string& input_file_path) {
    if (out.is_open()) {
        for (int i = 0; i < method.size(); ++i) {
            out << input_file_path << "\t" << className << "\t" << method[i].getHeader() << "\t" << method[i].getStereotype() << "\n";
        }
    }
}

// Output class information
//
std::ostream& operator<<(std::ostream& out, const classModel& c) {
    out << std::endl << "--------------------------------------" << std::endl;
    out << "Class: " << c.className << std::endl;
    out << "Class Stereotype: " << c.getClassStereotype() << std::endl;
    out << "# Methods: " << c.method.size() << std::endl;
    for (int i = 0; i < c.method.size(); ++i) {
        out <<  c.method[i] << std::endl;
    }
    out << "# Attributes: " << c.attribute.size() << std::endl;
    for (int i = 0; i < c.attribute.size(); ++i) {
        out << c.attribute[i] << std::endl;
    }
    out << "--------------------------------------" << std::endl;
    return out;
}
