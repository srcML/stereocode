// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file MethodModel.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
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
    srcML = srcml_unit_get_srcml(unit);

    findConstructorOrDestructor(archive, unit);
    findMethodName(archive, unit); 
    findParameterList(archive, unit);

    if (unitLanguage == "C++") findConst(archive, unit);

    // Method could be inside a property (C# only), so return type is collected separately
    // returnType = "" if the unitLanguage is not C#
    returnType = propertyReturnType; 
    
    // Name signature needed for call analysis
    std::string paramList = parametersList;
    std::string methName = name;
    removeBetweenComma(paramList, false);
    removeNamespace(methName, true, unitLanguage);
    nameSignature = methName + paramList;
    trimWhitespace(nameSignature);
}

void methodModel::findMethodData(std::unordered_map<std::string, variable>& fields, 
                                 const std::unordered_set<std::string>& classMethods, 
                                 const std::unordered_set<std::string>& inheritedclassMethods,
                                 const std::string& classNamePar) {
    classNameParsed = classNamePar;
    if (!constructorOrDestructor) {                                
        srcml_archive* archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, srcML.c_str(), srcML.size());
        srcml_unit* unit = srcml_archive_read_unit(archive);

        findMethodReturnType(archive, unit); 
        findParameterName(archive, unit);
        findParameterType(archive, unit);
    
        findLocalVariableName(archive, unit);
        findLocalVariableType(archive, unit); 
        findReturnExpression(archive, unit);

        findCallName(archive, unit);
        findCallArgument(archive, unit);
        findNewAssign(archive, unit);

        findIgnorableCalls(methodCalls);
        findIgnorableCalls(functionCalls);
        findIgnorableCalls(constructorCalls);

        // Must only be called after isIgnorableCall()
        findCallsOnField(fields, classMethods, inheritedclassMethods);
    
        // Must only be called after findNewAssign()
        findReturnedVariables(fields, false); 

        findVariablesInExpressions(archive, unit, fields, false);
        findModifiedVariables(archive, unit, fields, false);

        findNonCommentsStatements(archive, unit);

        srcml_unit_free(unit);
        srcml_archive_close(archive);
        srcml_archive_free(archive); 
    }
}

void methodModel::findFreeFunctionData() {
    if (!constructorOrDestructor) {
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

        findIgnorableCalls(methodCalls);
        findIgnorableCalls(functionCalls);

        findCallsOnParameters();

        findReturnedVariables(parameters, true); 
        findVariablesInExpressions(archive, unit, parameters, true);
        findModifiedVariables(archive, unit, parameters, true);
        findNonCommentsStatements(archive, unit);

        srcml_unit_free(unit);
        srcml_archive_close(archive);
        srcml_archive_free(archive); 
    }
}

// Gets the method name
//
void methodModel::findMethodName(srcml_archive* archive, srcml_unit* unit) {
    if (constructorOrDestructor)
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
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        name = unparsed;
        trimWhitespace(name);
        free(unparsed);   
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Gets the method parameter list
//
void methodModel::findParameterList(srcml_archive* archive, srcml_unit* unit) {
    if (constructorOrDestructor)
        srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"constructor_destructor_parameter_list").c_str());
    else
        srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"method_parameter_list").c_str());
   
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) { 
        srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        parametersList = unparsed;
        free(unparsed);   
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Gets the method return type 
// Java:
//  The + tor returnType skips the generics parameters in Java return types
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
    variable temp;
    if (isNonPrimitiveType(returnType, temp, unitLanguage, classNameParsed))
        nonPrimitiveReturnType = true; 
    nonPrimitiveReturnTypeExternal = temp.getNonPrimitiveExternal();

    returnTypeParsed = returnType;
    removeTypeModifiers(returnTypeParsed, unitLanguage);  

    trimWhitespace(returnType);
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
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
    
        std::string localName = unparsed;

        // Chop off [] for arrays  
        if (unitLanguage == "C++") {
            std::size_t start_position = localName.find("[");
            if (start_position != std::string::npos){
                localName = localName.substr(0, start_position);
                Rtrim(localName);
            }
        }
        
        localsOrdered.push_back(variable());
        localsOrdered.back().setName(localName);
        
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
        localsOrdered[i].setType(type);
        isNonPrimitiveType(type, localsOrdered[i], unitLanguage, classNameParsed);
        locals.insert({localsOrdered[i].getName(), localsOrdered[i]});
        nonPrimitiveLocalExternal = localsOrdered[i].getNonPrimitiveExternal();
  
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
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

        std::string parameterName = unparsed;

        // Chop off [] for arrays
        if (unitLanguage == "C++") {  
            std::size_t start_position = parameterName.find("[");
            if (start_position != std::string::npos){
                parameterName = parameterName.substr(0, start_position);
                Rtrim(parameterName);
            }
        }

        parametersOrdered.push_back(variable());
        parametersOrdered.back().setName(parameterName);
        parametersOrdered.back().setPos(i);
        
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
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string type = unparsed;
    
        parametersOrdered[i].setType(type);
        isNonPrimitiveType(type, parametersOrdered[i], unitLanguage, classNameParsed);
        parameters.insert({parametersOrdered[i].getName(), parametersOrdered[i]});
        nonPrimitiveParamaterExternal = parametersOrdered[i].getNonPrimitiveExternal();
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
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string expr = unparsed;
        free(unparsed);
        
        returnExpressions.push_back(expr);

        std::string newOperator = expr.substr(0,3);
        if (newOperator == "new") 
            newReturned = true; 
        
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects names of calls including function, method, and constructor calls
// C++:
//  Constructor calls are a type of function calls (collected separately)
void methodModel::findCallName(srcml_archive* archive, srcml_unit* unit) {   
    std::vector<std::string> callType = {"function", "method", "constructor"};
    for (const std::string& c : callType) {
        if (c == "function") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"function_call_name").c_str());
        else if (c == "method") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"method_call_name").c_str());
        else if (c == "constructor") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"constructor_call_name").c_str());

        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; ++i) {
            resultUnit = srcml_transform_get_unit(result, i);
            
            char* unparsed = nullptr;
            std::size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

            calls call;
            call.setName(unparsed);
            
            if (c == "function")  
                functionCalls.push_back(call);
            else if (c == "method") 
                methodCalls.push_back(call); 
            else if (c == "constructor") 
                constructorCalls.push_back(call); 

            free(unparsed);                    
        }
        srcml_clear_transforms(archive);
        srcml_transform_free(result);
    }
}

// Collects arguments of calls including function, method, and constructor calls
//
void methodModel::findCallArgument(srcml_archive* archive, srcml_unit* unit) {   
    std::vector<std::string> callType = {"function", "method", "constructor"};
    for (const std::string& c : callType) {
        if (c == "function") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"function_call_arglist").c_str());
        else if (c == "method") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"method_call_arglist").c_str());
        else if (c == "constructor") 
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"constructor_call_arglist").c_str());
        
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; ++i) {
            resultUnit = srcml_transform_get_unit(result, i);
            char * unparsed = nullptr;
            std::size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
            
            std::string arguList = unparsed;

            if (c == "function")  {
                functionCalls[i].setArgumentList(arguList);
                removeBetweenComma(arguList, false);
                std::string funcCallName = functionCalls[i].getName();
                removeNamespace(funcCallName, true, unitLanguage);
                std::string funcCallParsed = funcCallName + arguList;
                trimWhitespace(funcCallParsed);   
                functionCalls[i].setSignature(funcCallParsed);
            }
                
            else if (c == "method")  
                methodCalls[i].setArgumentList(arguList);
            
                
            else if (c == "constructor") 
                constructorCalls[i].setArgumentList(arguList); 
            
                
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
        std::size_t size = 0;
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
void methodModel::findNonCommentsStatements(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"non_comment_statements").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    numOfNonCommentsStatements = n; 

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}




// Determines if method is const
//
void methodModel::findConst(srcml_archive* archive, srcml_unit* unit) {
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
void methodModel::findConstructorOrDestructor(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"constructor_or_destructor").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) constructorOrDestructor = true;

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// In C#, non-primitive parameters are passed by value and the value is a reference to the object,
//  this means that if you re-assign the parameters itself (e.g., a = value), then the original object won't change
//  So, C# need to use the ref, out, *(unsafe context), or [] to pass by reference and be able to re-assign the parameters
//  But, if you use a field inside the parameter (a.b = value) the change will persist and affect the original object
//  In Java, the (a.b = value) is the only way to change a parameter and keep the changes outside
// C++ can use *, [], or & to pass by reference
// No need to check for 'const' since this function is only called when there is a modification to the parameter
//
void methodModel::findModifiedRefParameter(std::string para, bool propertyCheck) {
    std::string type = parameters[para].getType();
    if (unitLanguage == "C++" || unitLanguage == "C#"){
        // C# could use * in unsafe contexts
        bool referencePointer = type.find("*") != std::string::npos;      
        if (unitLanguage == "C++"){
            std::string parName = parameters[para].getName();
            bool reference = type.find("&") != std::string::npos;
    
            trimWhitespace(parName);
            bool referenceArray = parName.find("[]") != std::string::npos; 
            if (reference || referencePointer || referenceArray)                    
            parameterRefModified = true;   
        }
        else if (unitLanguage == "C#"){
            bool nonPrimitive = !isPrimitiveType(type, unitLanguage);
            bool referenceOut = type.find("out") != std::string::npos ||
                                type.find("ref") != std::string::npos;

            trimWhitespace(type);
            bool referenceArray = type.find("[]") != std::string::npos; 
            if (referenceOut || referenceArray || referencePointer)                  
            parameterRefModified = true;     
            else if (nonPrimitive && propertyCheck) {
                // For C# and Java, only check if a parameter's property is modified
                // For example, parameter.b = value --> check
                // parameter = value --> don't check
                parameterRefModified = true;
            
            }
        }       
    }
    else if (unitLanguage == "Java"){
        bool nonPrimitive = !isPrimitiveType(type, unitLanguage);
        trimWhitespace(type);
        bool referenceArray = type.find("[]") != std::string::npos; 
        if (referenceArray || (nonPrimitive && propertyCheck))                
        parameterRefModified = true;     
    }     
}

// Determines if a return expression returns a variable (parameter or an field)
// Both simple returns (e.g., return variable;) and 
//  complex returns (e.g., return variable + 5; or e.g., return 5 + 5;) are found
//
void methodModel::findReturnedVariables(std::unordered_map<std::string, variable>& variables, bool isParameterCheck) {
    for (const std::string& expr : returnExpressions) {
        if (isParameterCheck) {
            if (!isVariableUsed(variables, nullptr, expr, true, false, false, isParameterCheck, false))
                parameterNotReturned = true; // Complex return
        }
        else {
            if (isVariableUsed(variables, nullptr, expr, true, false, false, false, false)) 
                fieldReturned = true; // Simple return
            else
                complexReturn = true; // Complex return
        }
        
    }
}

// Determines if a variable (parameter or an field) is used in an expression
//
void methodModel::findVariablesInExpressions(srcml_archive* archive, srcml_unit* unit, 
                                                         std::unordered_map<std::string, variable>& variables, bool isParameterCheck)  {

    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"expression_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);
        char *unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        isVariableUsed(variables, nullptr, unparsed, false, false, false, isParameterCheck, false);
        
        free(unparsed);  
    }        
    srcml_clear_transforms(archive);
    srcml_transform_free(result);    
}

// Finds if an field is changed or 
//  if a parameter that is passed by reference is changed
// An field or a parameter that is changed multiple times should only be considered as 1 change
//
void methodModel::findModifiedVariables(srcml_archive* archive, srcml_unit* unit, 
                                     std::unordered_map<std::string, variable>& variables, bool isParameterCheck) { 
    std::unordered_set<std::string> checked; 

    srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"expression_assignment").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int j = 0; j < n; ++j) {
        resultUnit = srcml_transform_get_unit(result, j);
        char *unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string possibleVariable = unparsed;
        free(unparsed);  
        
        std::size_t oldSize = checked.size();
        if (name == "ReportError")
            std::cout <<"g";
        if (isParameterCheck)
            isVariableUsed(variables, nullptr, possibleVariable, false, true, false, true, false);
        else if (isVariableUsed(variables, &checked, possibleVariable, false, true, true, false, false)) {
            if (checked.size() > oldSize) {
                ++numOfFieldsModified;
                oldSize = checked.size();
            }         
        }
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);  
}

// Ignores calls from analysis
// Usage of fields within these calls are not ignored
// For example, if call to ignore is 'foo', 
//  then some of the matched cases are foo<>() or bar::foo() or a->b.foo()
//
void methodModel::findIgnorableCalls(std::vector<calls>& calls) {
    for (auto it = calls.begin(); it != calls.end();) {
        std::string callName = it->getName();

        std::size_t listOpen = callName.find("<");
        if (listOpen != std::string::npos)
            callName = callName.substr(0, listOpen);
            
        // Try to match the whole call
        if (IGNORED_CALLS.isIgnored(callName, unitLanguage)) { 
            it = calls.erase(it);
        }
        else {
            std::size_t split = callName.rfind("::");
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

// Function calls: --> foo() bar::foo()
//  Checks if a function call is made to a method in the class, else it is removed and considered external 
//  Static and free function calls are considered external calls
//
// Method Calls: --> bar.foo() where 'bar' could be a variable or a class name or a namespace 
//  Checks if there is a method call on an field
//  For example, a.foo() where a is an field, else it is removed and considered as external method call
//
//  In C# or Java, a class name can be used with the dot operator to invoke static methods
//  For example, className.staticMethodName();
//  These are removed and considered as external function calls
//
//  base, and super can only be used to invoke non-static methods in the current class (this) or parent class (base or super)
//  For example, this.methodName();
//  So these should be also treated as function calls and not method calls
//
void methodModel::findCallsOnField(std::unordered_map<std::string, variable>& fields, 
                                   const std::unordered_set<std::string>& classMethods, 
                                   const std::unordered_set<std::string>& inheritedclassMethods) {  
    // Check on function calls (Should be done before checking on method calls)
    for (auto it = functionCalls.begin(); it != functionCalls.end();) {  
        if (classMethods.find(it->getSignature()) == classMethods.end() && 
            inheritedclassMethods.find(it->getSignature()) == inheritedclassMethods.end()) { 
            it = functionCalls.erase(it);
            ++numOfExternalFunctionCalls;    
        }
        else ++it;
        
    }  

    // Check on method calls 
    for (auto it = methodCalls.begin(); it != methodCalls.end();) {
        // Could be a normal method call on an field
        if (!isVariableUsed(fields, nullptr, it->getName(), false, false, false, false, false)) { // Checks if call is on field
            if (unitLanguage != "C++") {
                // These should be function calls
                if (unitLanguage == "C#" && (matchSubstring(it->getName(), "this") || matchSubstring(it->getName(), "base")))
                    functionCalls.push_back(*it); 
                else if (unitLanguage == "Java" && (matchSubstring(it->getName(), "this") || matchSubstring(it->getName(), "super")))
                    functionCalls.push_back(*it); 

                // Could be a call on a local or a parameter
                else if (isVariableUsed(fields, nullptr, it->getName(), false, false, false, true, true)) 
                    ++numOfExternalMethodCalls;
                    
                // It is a static call
                else 
                    ++numOfExternalFunctionCalls;
                it = methodCalls.erase(it);  
                
            }
            else {
                ++numOfExternalMethodCalls;
                it = methodCalls.erase(it);  
            }                  
        }
        else ++it;   
    }
}

void methodModel::findCallsOnParameters() {   
    for (auto& m : methodCalls)
        isVariableUsed(parameters, nullptr, m.getName(), false, false, false, true, false);
}

// Checks if an expression uses an field, local, or a parameter --> call them variable
// Possible cases: 
// C++: this->a; (*this).a; Foo::a; this->a.b; (*this).a.b; Foo::a.b; a.b; a
// Java: super.a; this.a; Foo.a; super.a.b; this.a.b; Foo.a.b; a.b; a
// C#: base.a; this.a; Foo.a; base.a.b; this.a.b; Foo.a.b; a.b; a
// Where 'a' is a variable and Foo is class itself if the variable is an field
// Can match with complex uses of variables (e.g., this->a.b.c or a[]->b or (*a).b.c)
//
bool methodModel::isVariableUsed(std::unordered_map<std::string, variable>& variables, 
                                       std::unordered_set<std::string>* fieldsModified, 
                                       const std::string& expression, bool returnCheck, 
                                       bool parameterModifiedCheck,  bool localModifiedCheck,
                                       bool isParamaterCheck, bool isLocalCheck) {
    std::string expr = expression; 
    trimWhitespace(expr);
               
    // Remove brakcets. For example, a [3]
    std::size_t openingBracket = expr.find("["); 
    if (openingBracket != std::string::npos)
        expr = expr.substr(0, openingBracket); 
    
    // Removing () and {} on the outside of expression
    // Might remove } and ) for calls but that doesn't affect the analysis
    if (unitLanguage == "C++") {
        while (!expr.empty() && (expr.front() == '{' || expr.front() == '(')) { 
            if (expr.size() > 6 && expr.substr(1, 5) == "(*this)")
                break;
            expr.erase(0, 1);
        }
        while (!expr.empty() && (expr.back() == '}' || expr.back() == ')')) expr.pop_back();  
    }
    else {
        while (!expr.empty() && expr.front() == '(') expr.erase(0, 1);       
        while (!expr.empty() && expr.back() == ')') expr.pop_back();       
    }

    // In C# the null-coalescing  operator is represented by ? or ?? and it allows you to check if an object 
    //  is null before accessing its members or using its value. For example, testString?.Length; or userInput ?? "Default Name"; 
    if (unitLanguage == "C#") {
        std::size_t nullCoalescingOperator = expr.find("?"); 
        while (nullCoalescingOperator != std::string::npos) {
            expr.erase(nullCoalescingOperator, 1); 
            nullCoalescingOperator = expr.find("?"); // For chaining
        }         
    }

    // Remove pointers. For example, *a
    if (unitLanguage != "Java") 
        while (!expr.empty() && expr.front() == '*') expr.erase(0, 1);       
    
    if (expr.empty()) return false;  

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
    bool overShadow = true; // Needed in cases such as this.data = data where this.data is an field and data is a local or a parameter 

    std::string possibleVar = expr; // For some reason, this fixes an issues dealing with invalid read memory caused by regex_search()
    for (int i = 0; i < count; i++) {
        if (isMatched && i == 0) {    
            if (match[1] == "") {    
                possibleVar = match[2]; // Case of base, super, and this 
                overShadow = false; 
            }   
            // 
            else if (!returnCheck)
                possibleVar = match[1]; // Perhaps variable itself (e.g., a or a.foo())        
        } 
        // In C# or Java, a class name can be used to access static fields only
        // In C++, a class name can be used to access static and non-static fields
        // Parent class names can also be used to access fields in the child class, but we will ignore this case for now
        // Checking with class name also avoids problems with other classs or properties having the same names as the fields in the current class
        else if (isMatched && match[1] != "") {// Case of class name itself
            std::string possibleClassName = match[1];
            std::size_t listOpen = possibleClassName.find("<");
            if (listOpen != std::string::npos) 
                possibleClassName = possibleClassName.substr(0, listOpen);
            if (classNameParsed == possibleClassName)
                possibleVar = match[2];
        }

        if (overShadow) {
            // Checked first in case of overshadowing if variables = fields
            // Case of variables = locals
            if (locals.find(possibleVar) != locals.end()) {
                if (localModifiedCheck && locals.at(possibleVar).getNonPrimitive()) 
                    nonPrimitiveLocalOrParameterModified = true;
                if (returnCheck) { 
                    if (variablesCreatedWithNew.find(possibleVar) != variablesCreatedWithNew.end())
                        if (!fieldsCreatedAndReturnedWithNew) fieldsCreatedAndReturnedWithNew = true;
                }
                if (isLocalCheck) return true;
                else return false;
            }

            // Case of variables = parameters
            else if (parameters.find(possibleVar) != parameters.end()) {
                parameterUsed = true;
                if (parameterModifiedCheck) {
                    if (parameters.at(possibleVar).getNonPrimitive()) nonPrimitiveLocalOrParameterModified = true;
                    findModifiedRefParameter(possibleVar, isMatched);
                }
                if (returnCheck) {        
                    if (variablesCreatedWithNew.find(possibleVar) != variablesCreatedWithNew.end())
                        if (!fieldsCreatedAndReturnedWithNew) fieldsCreatedAndReturnedWithNew = true;
                }
                if (isParamaterCheck) return true;
                else return false;      
            }
        }

        // Case of variables = fields
        if (variables.find(possibleVar) != variables.end()) {
            if (fieldsModified)
                if (fieldsModified->find(possibleVar) == fieldsModified->end())
                    fieldsModified->insert(possibleVar);
            
            fieldUsed = true;
            if (possibleVar != "this") { 
                nonPrimitiveFieldExternal = variables.at(possibleVar).getNonPrimitiveExternal(); 
                if (returnCheck) {
                    fieldReturned = true; // Simple return     
                    if (variablesCreatedWithNew.find(possibleVar) != variablesCreatedWithNew.end())
                        if (!fieldsCreatedAndReturnedWithNew) fieldsCreatedAndReturnedWithNew = true;
                }
            }
            return true;                                  
        }
    }

    if(parameterModifiedCheck)
        globalOrStaticModified = true;

    return false;  
}



std::string methodModel::getStereotype() const {
    std::string result;

    for (const std::string &value : stereotype)
        result += value + " ";

    if (result != "") 
        Rtrim(result);

    return result;
}
