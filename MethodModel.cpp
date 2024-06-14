// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file MethodModel.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "MethodModel.hpp"

extern primitiveTypes    PRIMITIVES;                        
extern ignorableCalls    IGNORED_CALLS; 
extern XPathBuilder      XPATH_TRANSFORMATION;  

methodModel::methodModel(srcml_archive* archive, srcml_unit* unit, const std::string& methodXpath, 
                         const std::string& unitLang, const std::string& propertyReturnType, int unitNum) :
                         unitLanguage(unitLang),  xpath(methodXpath), unitNumber(unitNum) {

    char* typeUnparsed = nullptr;
    size_t typeSize = 0;
    srcml_unit_unparse_memory(unit, &typeUnparsed, &typeSize);
    temp = typeUnparsed;
    free(typeUnparsed);

    srcML = srcml_unit_get_srcml(unit);
    isConstructorDestructor(archive, unit);
    isStatic(archive, unit);
    findMethodName(archive, unit); 
    findParameterList(archive, unit);

    // Method could be inside a property (C# only), so return type is collected separately
    // returnType = "" if the unitLanguage is not C#
    returnType = propertyReturnType; 
    // Name signature needed for function call analysis
    std::string paramList = parameterList;
    std::string methName = name;
    removeBetweenComma(paramList, false);
    removeNamespace(methName, true, unitLanguage);
    nameSignature = methName + paramList;
    trimWhitespace(nameSignature);
}

void methodModel::findMethodData(std::unordered_map<std::string, variable>& attribute, 
                                 const std::unordered_map<std::string, methodModel>& classMethods, 
                                 const std::unordered_set<std::string>& inheritedClassMethods,
                                 const std::string& classNamePar) {
    classNameParsed = classNamePar;
    if (!constructorDestructorUsed) {                                
        srcml_archive* archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, srcML.c_str(), srcML.size());
        srcml_unit* unit = srcml_archive_read_unit(archive);

        findMethodReturnType(archive, unit); 
        findLocalVariableName(archive, unit);
        findLocalVariableType(archive, unit); 
        findParameterName(archive, unit);
        findParameterType(archive, unit);
        findReturnExpression(archive, unit);

        findCallName(archive, unit);
        findCallArgument(archive, unit);
        findNewAssign(archive, unit);
        isIgnorableCall(methodCall);
        isIgnorableCall(functionCall);

        isCallOnAttribute(attribute, classMethods, inheritedClassMethods);

        isAttributeOrParameterReturned(attribute, false);  
        isAttributeOrParamaterUsedInExpression(archive, unit, attribute);
        isAttributeOrParameterModified(archive, unit, attribute);

        isEmpty(archive, unit);

        if (unitLanguage == "C++") isConst(archive, unit);

        srcml_unit_free(unit);
        srcml_archive_close(archive);
        srcml_archive_free(archive); 
    }
}

void methodModel::findFreeFunctionData() {
    if (!constructorDestructorUsed) {                                
        srcml_archive* archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, srcML.c_str(), srcML.size());
        srcml_unit* unit = srcml_archive_read_unit(archive);

        findMethodReturnType(archive, unit); 
        findLocalVariableName(archive, unit);
        findLocalVariableType(archive, unit); 
        findParameterName(archive, unit);
        findParameterType(archive, unit);
        findReturnExpression(archive, unit);

        findCallName(archive, unit);
        findCallArgument(archive, unit);
        findNewAssign(archive, unit);
        isIgnorableCall(methodCall);
        isIgnorableCall(functionCall);

        isCallOnParameter();

        isAttributeOrParameterReturned(parameter, true); 
        isAttributeOrParamaterUsedInExpression(archive, unit, parameter);
        isAttributeOrParameterModified(archive, unit, parameter);
        isEmpty(archive, unit);

        srcml_unit_free(unit);
        srcml_archive_close(archive);
        srcml_archive_free(archive); 
    }

}

void methodModel::findFriendData() {
    srcml_archive* archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, srcML.c_str(), srcML.size());
    srcml_unit* unit = srcml_archive_read_unit(archive);

    findMethodReturnType(archive, unit); 
    findParameterName(archive, unit);
    findParameterType(archive, unit);
    if (unitLanguage == "C++")
        isConst(archive, unit);
        
    srcml_unit_free(unit);
    srcml_archive_close(archive);
    srcml_archive_free(archive); 
}

// Gets the method name
//
void methodModel::findMethodName(srcml_archive* archive, srcml_unit* unit) {
    if (constructorDestructorUsed)
        srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"constructor_destructor_name").c_str());
    else
        srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"method_name").c_str());

    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    // n > 0 instead of n == 1 because there is an issue with srcML and C# not closing the function block properly,
    // so it keeps reading other random names
    if (n > 0) { 
        srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        name = unparsed;
        free(unparsed);   
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Gets the method parameter list
//
void methodModel::findParameterList(srcml_archive* archive, srcml_unit* unit) {
    if (constructorDestructorUsed)
        srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"constructor_destructor_parameter_list").c_str());
    else
        srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"method_parameter_list").c_str());
   
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) { 
        srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        parameterList = unparsed;
        free(unparsed);   
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Gets the method return type 
//
void methodModel::findMethodReturnType(srcml_archive* archive, srcml_unit* unit) {
    if (returnType == "") { // If method was a property (C#), type is found in previous steps
        srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"method_return_type").c_str());
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        for (int i = 0; i < n; i++) {
            srcml_unit* resultUnit = srcml_transform_get_unit(result, i);
            std::string unparsed = srcml_unit_get_srcml(resultUnit);
            returnType += srcml_unit_get_srcml(resultUnit);
        }
        srcml_clear_transforms(archive);
        srcml_transform_free(result);
    }

    if (isNonPrimitiveType(returnType, nonPrimitiveReturnTypeExternal, unitLanguage, classNameParsed))
        nonPrimitiveReturnType = true; 
    
    returnTypeParsed = returnType;
    removeTypeTokens(returnTypeParsed, unitLanguage);
    trimWhitespace(returnTypeParsed);   
}

// Collects the names of local variables
//
void methodModel::findLocalVariableName(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"local_variable_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);  
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
    
        std::string localName = unparsed;
        variable v;

        // Chop off [] for arrays  
        if (unitLanguage == "C++") {
            size_t start_position = localName.find("[");
            if (start_position != std::string::npos){
                localName = localName.substr(0, start_position);
                Rtrim(localName);
            }
        }
        v.setName(localName);
        localOrdered.push_back(v);
        
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the types of local variables
//
void methodModel::findLocalVariableType(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"local_variable_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    std::string prev = "";
    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(resultUnit);
  
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

        if (type == "<type ref=\"prev\"/>") {
            type = prev;           
        }
        else {  
            type = unparsed;
            prev = type;
        }  
        localOrdered[i].setType(type);
        local.insert({localOrdered[i].getName(), localOrdered[i]});

        isNonPrimitiveType(type, nonPrimitiveLocalExternal, unitLanguage, classNameParsed);
  
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the names of parameters in each method
//
void methodModel::findParameterName(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"parameter_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

        std::string parameterName = unparsed;
        variable v;

        // Chop off [] for arrays
        if (unitLanguage == "C++") {  
            size_t start_position = parameterName.find("[");
            if (start_position != std::string::npos){
                parameterName = parameterName.substr(0, start_position);
                Rtrim(parameterName);
            }
        }
        v.setName(parameterName);
        v.setPos(i);
        
        parameterOrdered.push_back(v); 
        
        free(unparsed);
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the types of parameters
// In C++, parameters could have a type but no name (for backward compatibility),
// Only collect the type if there is a name
//
void methodModel::findParameterType(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"parameter_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
        
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string type = unparsed;
    
        parameterOrdered[i].setType(type);
        parameter.insert({parameterOrdered[i].getName(), parameterOrdered[i]});

        isNonPrimitiveType(type, nonPrimitiveParamaterExternal, unitLanguage, classNameParsed);
        
        free(unparsed);
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects all return expressions
//
void methodModel::findReturnExpression(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"return_expression").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string expr = unparsed;
        free(unparsed);
        
        std::string newOperator = expr.substr(0,3);
        if (newOperator == "new") 
            newReturned = true; 
        returnExpression.push_back(expr);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects names of calls including function, method, and constructor calls
//
void methodModel::findCallName(srcml_archive* archive, srcml_unit* unit) {   
    std::vector<std::string> callType = {"function", "method"};
    for (const std::string& c : callType) {
        if (c == "function") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"function_call_name").c_str());
        else if (c == "method") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"method_call_name").c_str());
        
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; ++i) {
            resultUnit = srcml_transform_get_unit(result, i);
            
            char * unparsed = nullptr;
            size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

            calls call;
            call.setName(unparsed);
            
            if (c == "function")  
                functionCall.push_back(call);
            else if (c == "method") 
                methodCall.push_back(call); 

            free(unparsed);                    
        }
        srcml_clear_transforms(archive);
        srcml_transform_free(result);
    }
}

// Collects arguments of calls including function, method, and constructor calls
//
void methodModel::findCallArgument(srcml_archive* archive, srcml_unit* unit) {   
    std::vector<std::string> callType = {"function", "method"};
    for (const std::string& c : callType) {
        if (c == "function") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"function_call_type").c_str());
        else if (c == "method") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"method_call_type").c_str());
        
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; ++i) {
            resultUnit = srcml_transform_get_unit(result, i);
            
            char * unparsed = nullptr;
            size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
            
            std::string arguList = unparsed;

            if (c == "function")  {
                functionCall[i].setArgumentList(arguList);
                
                removeBetweenComma(arguList, false);

                std::string funcCallName = functionCall[i].getName();
                removeNamespace(funcCallName, false, unitLanguage);

                std::string funcCallParsed = funcCallName + arguList;
                trimWhitespace(funcCallParsed);   

                functionCall[i].setSignature(funcCallParsed);
            }
                
            else if (c == "method") 
                methodCall[i].setArgumentList(arguList); 
 

            free(unparsed);


        }
        srcml_clear_transforms(archive);
        srcml_transform_free(result);
    }
}

// Finds all variables that are declared/initialized with the "new" operator
//
void methodModel::findNewAssign(srcml_archive* archive, srcml_unit* unit) {  
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"new_operator_assign").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string varName = unparsed;
        free(unparsed);
        trimWhitespace(varName);
        
        variablesCreatedWithNew.insert(varName);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Determines if method is empty
//
void methodModel::isEmpty(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"empty").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    empty = (n==0); 

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Determines if method is const
//
void methodModel::isConst(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"const").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) constMethod = true;
    
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Check if method is a constructor or a destructor
//
void methodModel::isConstructorDestructor(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"constructor_or_destructor").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) constructorDestructorUsed = true;

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Check if method is a constructor or a destructor
//
void methodModel::isStatic(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"static").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) staticMethod = true;

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// In C#, non-primitive parameters are passed by value and the value is a reference to the object,
//  this means that if you re-assign the parameters itself (e.g., a = value), then the original object won't change
//  So, C# need to use the ref, out, *(unsafe context), or [] to pass by reference and be able to re-assign the parameters
//  But, if you use a attribute inside the parameter (a.b = value) the change will persist and affect the original object
//  In Java, the (a.b = value) is the only way to change a parameter and keep the changes outside
// C++ can use *, [], or & to pass by reference
// No need to check for 'const' since this function is only called when there is a modification to the parameter
//
void methodModel::isParameterRefChanged(std::string para, bool propertyCheck) {
    std::string type = parameter[para].getType();
    if (unitLanguage == "C++" || unitLanguage == "C#"){
        // C# could use * in unsafe contexts
        bool referencePointer = type.find("*") != std::string::npos;      
        if (unitLanguage == "C++"){
            std::string parName = parameter[para].getName();
            bool reference = type.find("&") != std::string::npos;
    
            trimWhitespace(parName);
            bool referenceArray = parName.find("[]") != std::string::npos; 
            if (reference || referencePointer || referenceArray)                    
                parameterRefChanged = true;   
        }
        else if (unitLanguage == "C#"){
            bool nonPrimitive = !isPrimitiveType(type, unitLanguage);
            bool referenceOut = type.find("out") != std::string::npos ||
                                type.find("ref") != std::string::npos;

            trimWhitespace(type);
            bool referenceArray = type.find("[]") != std::string::npos; 
            if (referenceOut || referenceArray || referencePointer)                  
                parameterRefChanged = true;     
            else if (nonPrimitive && propertyCheck) {
                // For C# and Java, only check if a parameter's property is modified
                // For example, parameter.b = value --> check
                // parameter = value --> don't check
                parameterRefChanged = true;
            
            }
        }       
    }
    else if (unitLanguage == "Java"){
        bool nonPrimitive = !isPrimitiveType(type, unitLanguage);
        trimWhitespace(type);
        bool referenceArray = type.find("[]") != std::string::npos; 
        if (referenceArray || (nonPrimitive && propertyCheck))                
            parameterRefChanged = true;     
    }    
    if (parameterRefChanged)    
        parameterRefChangedPos.insert(parameter[para].getPos());  
}


// Determines if a return expression returns an attribute
//
void methodModel::isAttributeOrParameterReturned(std::unordered_map<std::string, variable>& attributeOrParameter, bool isParamaterCheck) {
    for (const std::string& expr : returnExpression) {
        if (isAttributeOrParameterUsed(attributeOrParameter, expr, true, false, isParamaterCheck)){
            // Simple return expression. 
            // For example, 'return a;' where 'a' is an attribute
            attributeReturned = true; 
        }
        else {
            // Complex return expression. 
            // For example, 'return a + value;' where 'a' is an attribute
            if (isParamaterCheck)
                parameterNotReturned = true;
            else
                attributeNotReturned = true;    
        }
    }
}

// Determines if an attribute is used in an expression
//
void methodModel::isAttributeOrParamaterUsedInExpression(srcml_archive* archive, srcml_unit* unit, 
                                                         std::unordered_map<std::string, variable>& attributeOrParameter)  {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"expression_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);
        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        
        isAttributeOrParameterUsed(attributeOrParameter, unparsed, false, false, false);

        free(unparsed);
        
    }        
    srcml_clear_transforms(archive);
    srcml_transform_free(result);    
}

// Finds if an attribute is changed or 
//  if a parameter that is passed by reference is changed
//
void methodModel::isAttributeOrParameterModified(srcml_archive* archive, srcml_unit* unit, 
                                       std::unordered_map<std::string, variable>& attributeOrParameter) { 
    int changed = 0;
    // An attribute that is changed multiple times should only be considered as 1 change
    std::unordered_set<std::string> checked; 

    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"expression_assignment").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int j = 0; j < n; ++j) {
        resultUnit = srcml_transform_get_unit(result, j);
        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string possibleAttribute = unparsed;
        free(unparsed);  

        if (isAttributeOrParameterUsed(attributeOrParameter, possibleAttribute, false, true, false) 
            && checked.find(possibleAttribute) == checked.end()) {
            changed++;  
            checked.insert(possibleAttribute);
        }
    }
    numOfAttributeModified = changed;

    srcml_clear_transforms(archive);
    srcml_transform_free(result);  
}

// Ignores calls from analysis
// For example, if call to ignore is 'foo', 
//  then some of the matched cases are foo<>() or bar::foo() or a->b.foo()
void methodModel::isIgnorableCall(std::vector<calls>& calls) {
    for (auto it = calls.begin(); it != calls.end();) {
        std::string callName = it->getName();

        size_t listOpen = callName.find("<");
        if (listOpen != std::string::npos)
            callName = callName.substr(0, listOpen);
            
        // Try to match the whole call
        if (IGNORED_CALLS.isIgnored(callName, unitLanguage)) { 
            it = calls.erase(it);
        }
        else {
            size_t split = callName.rfind("::");
            if (split != std::string::npos)
                callName = callName.substr(split + 2);
            else {
                split = callName.rfind("->");
                if (split != std::string::npos)
                    callName = callName.substr(split + 2);
                else {
                    split = callName.rfind(".");
                    if (split != std::string::npos)
                        callName = callName.substr(split + 1);
                }
            }
 
            if (IGNORED_CALLS.isIgnored(callName, unitLanguage)) 
                it = calls.erase(it);
            else ++it;                      
        }
    }
}


// Checks if there is a method call on an attribute
// For example, a.foo() where a is an attribute, else it is removed
// 
// Also, check if a function call is made to a method in the class, else it is removed
// For C#, Java --> calls in the form of foo(), base.foo(), super.foo(), this.base() are considered. 
//
// In C# or Java, a class name (only current class is considered) can be used with the dot operator to invoke static methods
// For example, className.staticMethodName();
// So these should be treated as function calls 
//
// this, base, and super can only be used to invoke non-static methods in the class (this) or parent class (base or super)
// For example, this.methodName();
// So these should be treated as function calls 
//
// Everything else is removed (stateless calls)
//
void methodModel::isCallOnAttribute(std::unordered_map<std::string, variable>& attribute, 
                                   const std::unordered_map<std::string, methodModel>& classMethods, 
                                   const std::unordered_set<std::string>& inheritedClassMethods) {    
    // Check on function calls (Should be done before checking on method calls)
    for (auto it = functionCall.begin(); it != functionCall.end();) {
        isAttributeUsedAsArgument(attribute, *it, true); // Checks if attribute is used as argument

        if (!matchSubstring(it->getSignature(), classNameParsed) && classMethods.find(it->getSignature()) == classMethods.end() && 
            inheritedClassMethods.find(it->getSignature()) == inheritedClassMethods.end()) {    
            it = functionCall.erase(it);
            ++numOfExternalFunctionCalls;    
        }
        else ++it;
        
    }  

    // Check on method calls 
    for (auto it = methodCall.begin(); it != methodCall.end();) {
        if (!isAttributeOrParameterUsed(attribute, it->getName(), false, false, false)) { // Could be a normal method call on an attribute
            bool isFunctionCall = false;
            if (unitLanguage != "C++") {
                isFunctionCall = matchSubstring(it->getName(), "base") || matchSubstring(it->getName(), "this") ||
                                      matchSubstring(it->getName(), "super") || matchSubstring(it->getName(), classNameParsed);
                if (isFunctionCall) {
                    isAttributeUsedAsArgument(attribute, *it, false); // Checks if attribute is used as argument
                    functionCall.push_back(*it); 
                    it = methodCall.erase(it);
                }
            }
            if (!isFunctionCall) {
                ++numOfExternalMethodCalls;
                it = methodCall.erase(it);  
            }                  
        }
        else ++it;
    }
}


void methodModel::isCallOnParameter() {   
    for (auto& m : functionCall) 
        isAttributeUsedAsArgument(parameter, m, false); 
    for (auto& m : methodCall) {
        isAttributeOrParameterUsed(parameter, m.getName(), false, false, false);
        isAttributeUsedAsArgument(parameter, m, false); 
    }
        
}


// Checks if a call uses an attribute as argument.
// For example, b(a) or b.d(a, c) where a and c are attributes
//
bool methodModel::isAttributeUsedAsArgument(std::unordered_map<std::string, variable>& attribute, calls& call, bool track) {
    std::string argumentList = call.getArgumentList();
    size_t open = argumentList.find("(");
    size_t close = argumentList.rfind(")");
    std::string callingObject = argumentList.substr(open + 1, close - (open + 1));

    size_t start = 0, end = 0;
    int pos = 0; // Used to track 
    std::string possibleAttribute;

    bool attributeUsedAsArg = false;

    while (end != std::string::npos) {
        end = callingObject.find(",", start);
        possibleAttribute = callingObject.substr(start, end - start);
        if (isAttributeOrParameterUsed(attribute, possibleAttribute, false, false, false)) {
            if (track) call.appendAttributePosition(pos);
            attributeUsedAsArg = true;
        }

        ++pos;      
        start = end + 1; 
    }

    return attributeUsedAsArg;
}

// Checks if an expression uses an attribute, local, or a parameter --> call them variable
// Possible cases: 
// C++: this->a; (*this).a; Foo::a; this->a.b; (*this).a.b; Foo::a.b; a.b; a
// Java: super.a; this.a; Foo.a; super.a.b; this.a.b; Foo.a.b; a.b; a
// C#: base.a; this.a; Foo.a; base.a.b; this.a.b; Foo.a.b; a.b; a
// Where 'a' is a variable and Foo is class itself if the variable is an attribute
// Can match with complex uses of variables (e.g., this->a.b.c or a[]->b or (*a).b.c)
//
bool methodModel::isAttributeOrParameterUsed(std::unordered_map<std::string, variable>& attribute, 
                                       const std::string& expression, bool returnCheck, 
                                       bool parameterModifiedCheck, bool isParamaterCheck) {
    
    std::string expr = expression; 
    trimWhitespace(expr);

    if (expr == "")
        return false;                                 

    // Remove brakcets. For example, a [3]
    size_t openingBracket = expr.find("["); 
    if (openingBracket != std::string::npos)
        expr = expr.substr(0, openingBracket); 
    
    // Removing () and {} on the outside of expression
    // Might remove } and ) for calls but that doesn't affect the analysis
    if (unitLanguage == "C++") {
        std::string thisStr = expr.substr(1, 5);
        while ((expr[0] == '{' || expr[0] == '(') && thisStr != "(*this)") {
            expr.erase(0, 1);
            thisStr = expr.substr(1, 5);
        }
        while (expr[expr.size() - 1] == '}' || expr[expr.size() - 1] == ')')
            expr.erase(expr.size() - 1, 1);
        
    }
    else {
        while (expr[0] == '(') 
            expr.erase(0, 1);   
               
        while (expr[expr.size() - 1] == ')')
            expr.erase(expr.size() - 1, 1);
    }

    // In C# the null-conditional operator is represented by ? and it allows you to check if an object 
    //  is null before accessing its members. For example, testString?.Length; 
    if (unitLanguage == "C#") {
        size_t nullConditionOperator = expr.find("?"); 
        while (nullConditionOperator != std::string::npos) {
            expr.erase(nullConditionOperator, 1); 
            nullConditionOperator = expr.find("?"); // For chaining
        }         
    }

    // Remove pointers. For example, *a
    if (unitLanguage != "Java") {
        while (expr[0] == '*') 
            expr.erase(0, 1);   
    }

    // ^ indicates that we should only match from the beginning
    // We only care about the first two variables. For example, in a.b.c() the a.b is sufficient to 
    //  determine what "a" is
    // base and super point to the parent class (not interfaces)

    bool isMatched = false;
    std::smatch match;
    std::string pattern;
    if (!returnCheck) {
        if (unitLanguage == "C++") 
            pattern = R"(^(?:\(\*this\)\.|this->|([^.->]*)(?:::|\.|->))([^.->]*))";
        else if (unitLanguage == "Java")
            pattern = R"(^(?:super|this|([^.]*))\.([^.]*))";
        else if (unitLanguage == "C#")
            pattern = R"(^(?:base|this|([^.->]*))(?:\.|->)([^.->]*))";
    }
    else {
        // $ Used to match end of line. For example, return this.a; matches but return this.a.b; doesn't
        if (unitLanguage == "C++") 
            pattern = R"(^(?:\(\*this\)\.|this->|([^.->]*)(?:::|\.|->))([^.->\(\){}]*)$)";
        else if (unitLanguage == "Java")
            pattern = R"(^(?:super|this|([^.]*))\.([^.\(\)]*)$)";
        else if (unitLanguage == "C#")
            pattern = R"(^(?:base|this|([^.->]*))(?:\.|->)([^.->\(\)]*)$)";
    }
    const std::regex regexPattern(pattern);
    isMatched = std::regex_search(expr, match, regexPattern);

    int count = isMatched ? 2 : 1;
    for (int i = 0; i < count; i++) {
        if (isMatched && i == 0) {    
            if (match[1] == "") {    
                expr = match[2]; // Case of base, super, and this  
            }   
            else 
                expr = match[1]; 
        } 
        else if (isMatched && match[1] != "") // Case of class or parent class
            expr = match[2];

        // Might be a local or a parameter (Checked first in case of overshadowing)
        if (local.find(expr) != local.end()) {
            // Needed for factory, so only simple return expressions that return a local are needed
            if (returnCheck) { 
                if (variablesCreatedWithNew.find(expr) != variablesCreatedWithNew.end())
                    variableCreatedWithNewReturned = true;
            }
            return false;
        }

        else if (parameter.find(expr) != parameter.end()) {
            parameterUsed = true;
            if (parameterModifiedCheck) isParameterRefChanged(expr, isMatched);
            if (returnCheck) {        
                if (variablesCreatedWithNew.find(expr) != variablesCreatedWithNew.end())
                    variableCreatedWithNewReturned = true;
            }
            if (isParamaterCheck)
                return true;
            else
                return false;
        }

        // Might be an attribute
        else if (attribute.find(expr) != attribute.end()) {
            attributeUsed = true;
            std::string type = attribute.at(expr).getType(); 
            isNonPrimitiveType(type, nonPrimitiveAttributeExternal, unitLanguage, classNameParsed);
            if (returnCheck) {
                if (variablesCreatedWithNew.find(expr) != variablesCreatedWithNew.end())
                    variableCreatedWithNewReturned = true;
            }
            return true;                                  
        }
    }
    if(isParamaterCheck && !returnCheck)
        globalOrStaticChanged = true;

    return false;  
}

void methodModel::setStereotype(const std::string& s) {
    stereotype.push_back(s);
}

std::string methodModel::getStereotype() const {
    std::string result;

    for (const std::string &value : stereotype)
        result += value + " ";

    if (result != "") 
        Rtrim(result);

    return result;
}

void methodModel::getDataFromCall(methodModel& methodCalled, calls& methodCallExpression) {
    numOfAttributeModifiedByCalls += methodCalled.getNumOfAttributeModified();

    std::set<int> intersection;
    std::set_intersection(methodCalled.parameterRefChangedPos.begin(), methodCalled.parameterRefChangedPos.end(), 
                          methodCallExpression.getAttributePosition().begin(), methodCallExpression.getAttributePosition().end(),
                          std::inserter(intersection, intersection.begin()));

    accessorMethodCallUsed = accessorMethodCallUsed || (methodCalled.IsAttributeUsed() && methodCalled.returnExpression.size() > 0);

    numOfAttributeModifiedByCalls += intersection.size();


}