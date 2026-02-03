// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file FunctionModel.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "function_model.hpp"
#include "helper_functions.hpp"
#include "calls.hpp"
#include "xpath_generator.hpp"
#include "primitives.hpp"
#include "specifiers.hpp"

#include <regex>

extern              specifiers        SPECIFIERS;
extern              primitives        PRIMITIVES;
extern              calls             CALLS;
extern              XPathGenerator    XPATH_GENERATOR;

extern              srcml_unit*       methodUnit;
extern              srcml_archive*    methodArchive;

functionModel::functionModel(const std::string& xpath_, const std::string& unitLanguage_, const std::string& typeNameParsed_, 
                             const std::string& returnType_, int unitNumber_, bool isProperty_) :
                             unitLanguage{unitLanguage_}, xpath{xpath_}, typeNameParsed{typeNameParsed_},  
                             isProperty{isProperty_}, unitNumber{unitNumber_} {
    fileName = srcml_unit_get_filename(methodUnit); // Get file name here since free functions do not have a type

    returnType.setType(returnType_);
    
    if (unitLanguage == "C++") findConst();

    isConstructorOrDestructor();

    findName();          // Depends on isConstructorOrDestructor
    findParameterList(); // Depends on isConstructorOrDestructor

    if (!constructorOrDestructor.empty()) findConstructorOrDestructorType();

    findNameSignature();
   
    if (constructorOrDestructor.empty()) {
        if (unitLanguage == "C#" || unitLanguage == "Java") {
            findAttributesOrAnnotations();
            if (isProperty) findPropertyAttributes();
        }
        findSpecifiers();
        findReturnType(); 
        findParameterName();
        findParameterType();
        findLocalVariableName();
        findLocalVariableType(); 
        findNonPrimitive();
        findReturnExpression();
        findCallName();
        findCallArgument();
        findNewAssignedVariables();
        findIgnorableCalls(methodCalls);
        findIgnorableCalls(functionCalls);
        findIgnorableCalls(newConstructorCalls);
        findExpressionNames();
        findExpressionAssignments();
        findNonCommentStatements(); 
    }
}

// Finds all specifiers of the method
//
void functionModel::findSpecifiers() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"method_specifiers").c_str());
   
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, i);
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        specifiers.emplace(unparsed);
        free(unparsed);   
    }

    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Finds data after all type information is collected
//
void functionModel::findDataAfterCollection(const std::unordered_map<std::string, variableModel>& allFields, 
                                            const std::unordered_set<std::string>& allMethodSignatures) {   
    if (constructorOrDestructor.empty()) {
        // Must only be called after findIgnorableCalls()
        findCallsOnFields(allFields, allMethodSignatures);

        // Must only be called after findNewAssignedVariables()
        findReturnedVariables(allFields, false); 
        findVariablesInExpressions(allFields, false);
        findModifiedVariables(allFields, false);
    }
}

// Finds data for free functions
//
void functionModel::findDataFreeFunctionAfterCollection() {
    if (constructorOrDestructor.empty()) {
        // For free functions, we do not need to filter the calls like in findCallsOnFields()
        //  since all of the calls are external anyway
        findReturnedVariables(parameters, true); 
        findVariablesInExpressions(parameters, true);
        findModifiedVariables(parameters, true);
    }
}

// Counts the number of methods inside a property (C# only)
//
int functionModel::countMethodsInProperty() const {
    srcml_append_transform_xpath(methodArchive,  XPATH_GENERATOR.getXPathList(unitLanguage,"property_method").c_str());

    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);

    return n;
}


// Finds attributes ( C# ) or annotations ( Java )
//
void functionModel::findAttributesOrAnnotations() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"method_attributes_or_annotations").c_str());
    srcml_transform_result* result = nullptr;
    
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    for (int i = 0; i < n; i++) {
        char *unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(srcml_transform_get_unit(result, i), &unparsed, &size);
        attributesOrAnnotations.push_back(unparsed);
        free(unparsed);
    }
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Finds attributes ( C# ) for properties
//
void functionModel::findPropertyAttributes() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"property_attributes").c_str());
    srcml_transform_result* result = nullptr;
    
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    for (int i = 0; i < n; i++) {
        char *unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(srcml_transform_get_unit(result, i), &unparsed, &size);
        propertyAttributes.push_back(unparsed);
        free(unparsed);
    }
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Gets the method name
//
void functionModel::findName() {
    if (!constructorOrDestructor.empty())
        srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"constructor_destructor_name").c_str());
    else
        srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"method_name").c_str());

    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) { 
        srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        name = unparsed;
        free(unparsed);   
    }

    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Gets the method parameter list
//
void functionModel::findParameterList() {
    if (!constructorOrDestructor.empty())
        srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"constructor_destructor_parameter_list").c_str());
    else
        srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"method_parameter_list").c_str());
   
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) { 
        srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);
        char* unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        parametersList = unparsed;
        free(unparsed);   
    }
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Gets the method return type 
//
// Java:
//   The '+' for 'returnType' skips the generic parameter list in Java return types
//   For example, 'public static <T> void swap()' the <T> is included in <type>
//   However, it is a generic declaration and not a type, so it needs to be ignored
// C#:
//   Method could be inside a property (C# only), so the return type is collected separately
//
void functionModel::findReturnType() {
    if (!isProperty) { // If method was a property (C#), type is found in previous steps
        srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"method_return_type").c_str());
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
        int n = srcml_transform_get_unit_size(result);

        std::string returnTypeString;
        for (int i = 0; i < n; i++) {
            srcml_unit* resultUnit = srcml_transform_get_unit(result, i);
            returnTypeString += srcml_unit_get_srcml(resultUnit);
        }

        returnType.setType(returnTypeString);

        // Handle C# conversion operators (e.g. "implicit operator IntPtr")
        // The type tag often only has "public static implicit", so 'returnType' becomes empty after stripping specifiers
        // We must extract the actual type ("IntPtr") from the name ("operator IntPtr")
        if (unitLanguage == "C#" && name.rfind("operator", 0) == 0) {
            SPECIFIERS.removeSpecifiers(returnTypeString, unitLanguage);
            helperFunctions::removeWhitespace(returnTypeString);

            // If type is empty (meaning it was just specifiers), parse the name
            if (returnTypeString.empty()) {
                std::size_t spacePosition = name.find(' ');
                if (spacePosition != std::string::npos) {
                    std::string returnTypeName = name.substr(spacePosition + 1);
                    helperFunctions::removeWhitespace(returnTypeName);
                    returnType.setType(returnTypeName);
                }
            }
        }

        srcml_clear_transforms(methodArchive);
        srcml_transform_free(result);
    }
}

// Collects the names of local variables
//
void functionModel::findLocalVariableName() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"local_variable_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);  
        char * unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
    
        std::string localName = unparsed;

        // Chop off [] for arrays
        if (unitLanguage == "C++") 
            helperFunctions::removeBracketSuffix(localName); 

        localsOrdered.emplace_back(variableModel());
        localsOrdered.back().setName(localName);

        free(unparsed);
    }
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Collects the types of local variables
//
void functionModel::findLocalVariableType() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"local_variable_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    std::string prev;
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
        locals.insert({localsOrdered[i].getName(), localsOrdered[i]});

        free(unparsed);
    }
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Collects the names of parameters in each method
//
void functionModel::findParameterName() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"parameter_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        char * unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        
        std::string parameterName = unparsed;

        // Chop off [] for arrays
        if (unitLanguage == "C++") 
            helperFunctions::removeBracketSuffix(parameterName); 

        parametersOrdered.emplace_back(variableModel()); 
        parametersOrdered.back().setName(parameterName);

        free(unparsed);
    }

    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

 // Collects the types of parameters
 // In C++, parameters could have a type but no name (for backward compatibility)
 // Therefore, the type is only collected if there is a name
 //
void functionModel::findParameterType() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage, "parameter_type").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        char * unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string type = unparsed;
    
        parametersOrdered[i].setType(type);    
        parameters.insert({parametersOrdered[i].getName(), parametersOrdered[i]});

        free(unparsed);
    }
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Collects all return expressions
//
void functionModel::findReturnExpression() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage, "return_expression").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
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
       
        if (helperFunctions::isSubstringAtBeginning(expr, "new")) newReturned = true; 
    }
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Collects names of calls including function, method, and constructor calls
// C++:
//   Constructor calls are a type of function calls (collected separately)
//
void functionModel::findCallName() {
    std::vector<std::string> callType{ "function", "method", "constructor" };
    for (const std::string& c : callType) {
        if (c == "function") 
            srcml_append_transform_xpath(methodArchive, (XPATH_GENERATOR.getXPathList(unitLanguage,"function_call_name")).c_str());
        else if (c == "method") 
            srcml_append_transform_xpath(methodArchive, (XPATH_GENERATOR.getXPathList(unitLanguage,"method_call_name")).c_str());
        else if (c == "constructor") 
            srcml_append_transform_xpath(methodArchive, (XPATH_GENERATOR.getXPathList(unitLanguage,"constructor_call_name")).c_str());

        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
        int n = srcml_transform_get_unit_size(result);

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; ++i) {
            resultUnit = srcml_transform_get_unit(result, i);
            
            char* unparsed = nullptr;
            std::size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

            if (c == "function") {
               functionCalls.emplace_back(callModel());
               functionCalls.back().setName(unparsed);
            }
            else if (c == "method") {
               methodCalls.push_back(callModel());
               methodCalls.back().setName(unparsed);
            }
            else if (c == "constructor") {
               newConstructorCalls.push_back(callModel());
               newConstructorCalls.back().setName(unparsed);
            }

            free(unparsed);                    
        }
        srcml_clear_transforms(methodArchive);
        srcml_transform_free(result);
    }
}

// Collects arguments of calls including function, method, and constructor calls
//
void functionModel::findCallArgument() {   
    std::vector<std::string> callType{ "function", "method", "constructor" };
    for (const std::string& c : callType) {
        if (c == "function") 
            srcml_append_transform_xpath(methodArchive, (XPATH_GENERATOR.getXPathList(unitLanguage,"function_call_arglist")).c_str());
        else if (c == "method") 
            srcml_append_transform_xpath(methodArchive, (XPATH_GENERATOR.getXPathList(unitLanguage,"method_call_arglist")).c_str());
        else if (c == "constructor") 
            srcml_append_transform_xpath(methodArchive, (XPATH_GENERATOR.getXPathList(unitLanguage,"constructor_call_arglist")).c_str());
        
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
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
                helperFunctions::removeBetweenComma(arguList, false);
                std::string funcCallName = functionCalls[i].getName();
                helperFunctions::removeNamespace(funcCallName, unitLanguage, true);
                std::string funcCallParsed = funcCallName + arguList;
                helperFunctions::removeWhitespace(funcCallParsed);   
                functionCalls[i].setSignature(funcCallParsed);
            }             
            else if (c == "method") methodCalls[i].setArgumentList(arguList);                  
            else if (c == "constructor") newConstructorCalls[i].setArgumentList(arguList); 
                          
            free(unparsed);
        }
        srcml_clear_transforms(methodArchive);
        srcml_transform_free(result);
    }
}

// Finds all variables that are declared or initialized with the 'new' operator
//
void functionModel::findNewAssignedVariables() {
    srcml_append_transform_xpath(methodArchive, (XPATH_GENERATOR.getXPathList(unitLanguage,"new_operator_assign_decl_stmt") + " | " + XPATH_GENERATOR.getXPathList(unitLanguage,"new_operator_assign_expr_stmt")).c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        char *unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string varName = unparsed;
        free(unparsed);
        helperFunctions::removeWhitespace(varName);
        
        variablesCreatedWithNew.insert(varName);
    }
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Determines if method is empty
//
void functionModel::findNonCommentStatements() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"non_comment_statements").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    nonCommentStatementsCount = n; 

    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Determines if method is const (C++ only)
//
void functionModel::findConst() {
    std::string x = XPATH_GENERATOR.getXPathList(unitLanguage,"const");
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"const").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) methodConst = true;
    
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Check if method is a constructor or a destructor
//
void functionModel::isConstructorOrDestructor() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"constructor_or_destructor").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) constructorOrDestructor = true;

    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Finds the type of constructor or destructor
//
void functionModel::findConstructorOrDestructorType() {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"constructor_or_destructor").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) {
        std::string srcML = srcml_unit_get_srcml(srcml_transform_get_unit(result, 0));
        if      (srcML.find("<destructor>") != std::string::npos      )      constructorOrDestructor = "destructor"; 
        else if (parametersList.find(typeNameParsed) != std::string::npos)   constructorOrDestructor = "copy-constructor";
        else                                                                 constructorOrDestructor = "constructor";
    }

    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);
}

// Finds names of expressions
//
void functionModel::findExpressionNames()  {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"expression_name").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);
        char *unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

        expressionNames.insert(unparsed);

        free(unparsed);  
    }        
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);    
}

// Finds names of expressions
//
void functionModel::findExpressionAssignments()  {
    srcml_append_transform_xpath(methodArchive, XPATH_GENERATOR.getXPathList(unitLanguage,"expression_assignment").c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(methodArchive, methodUnit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);
        char *unparsed = nullptr;
        std::size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

        expressionAssignments.insert(unparsed);

        free(unparsed);  
    }        
    srcml_clear_transforms(methodArchive);
    srcml_transform_free(result);    
}

// Finds the signature of a function
//
void functionModel::findNameSignature() {
    std::string parametersList_ = parametersList;
    std::string name_ = name;

    helperFunctions::removeBetweenComma(parametersList_, false);
    helperFunctions::removeNamespace(name_, unitLanguage, true);

    nameSignature = name_ + parametersList_;

    helperFunctions::removeWhitespace(nameSignature);
}

// Finds if the return type, local types, and parameter types are non-primitive
//
void functionModel::findNonPrimitive() {
    // Return type
    if (PRIMITIVES.isNonPrimitive(returnType, unitLanguage, typeNameParsed)) {
        nonPrimitiveReturnType = true; 
        nonPrimitiveReturnTypeExternal = returnType.isNonPrimitiveExternal();
    }

    // Local types
    for (auto& local : locals) {               
        if (PRIMITIVES.isNonPrimitive(local.second, unitLanguage, typeNameParsed))
            nonPrimitiveLocalExternal = local.second.isNonPrimitiveExternal();
    }

    // Parameter types
    for (auto& parameter : parameters) {
       if (PRIMITIVES.isNonPrimitive(parameter.second, unitLanguage, typeNameParsed))
            nonPrimitiveParamaterExternal = parameter.second.isNonPrimitiveExternal();
    }
}

// Finds variables in expressions
//
void functionModel::findVariablesInExpressions(const std::unordered_map<std::string, variableModel>& variables, bool isParameterCheck) {
    for (const std::string& expr : expressionNames) {
        isVariableUsed(variables, nullptr, expr, false, false, false, isParameterCheck, false);
    }
}

// In C#, non-primitive parameters are passed by value and the value is a reference to the object,
//  this means that if you re-assign the parameters itself (e.g., a = value), then the original object won't change
//  So, C# need to use the ref, out, *(unsafe context), or [] to pass by reference and be able to re-assign the parameters
//  But, if you use a field inside the parameter (a.b = value) the change will persist and affect the original object
//  In Java, the (a.b = value) is the only way to change a parameter and keep the changes outside
// C++ can use *, [], or & to pass by reference
// No need to check for 'const' since this function is only called when there is a modification to the parameter
//
void functionModel::findModifiedRefParameter(std::string para, bool propertyCheck) {
    std::string type = parameters[para].getType();
    if (unitLanguage == "C++" || unitLanguage == "C#"){
        // C# could use * in unsafe contexts
        bool referencePointer = type.find("*") != std::string::npos;      
        if (unitLanguage == "C++"){
            std::string parName = parameters[para].getName();
            bool reference = type.find("&") != std::string::npos;
    
            helperFunctions::removeWhitespace(parName);
            bool referenceArray = parName.find("[]") != std::string::npos; 
            if (reference || referencePointer || referenceArray)                    
            parameterRefModified = true;   
        }
        else if (unitLanguage == "C#"){
            bool nonPrimitive = !PRIMITIVES.isPrimitive(type, unitLanguage);
            bool referenceOut = type.find("out") != std::string::npos ||
                                type.find("ref") != std::string::npos;

            helperFunctions::removeWhitespace(type);
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
        bool nonPrimitive = !PRIMITIVES.isPrimitive(type, unitLanguage);
        helperFunctions::removeWhitespace(type);
        bool referenceArray = type.find("[]") != std::string::npos; 
        if (referenceArray || (nonPrimitive && propertyCheck))                
        parameterRefModified = true;     
    }     
}

// Determines if a return expression returns a field or a parameter
// Both simple returns (e.g., return dm;) and 
//   complex returns (e.g., return dm + 5; or return dm + 5;) are considered
//
void functionModel::findReturnedVariables(const std::unordered_map<std::string, variableModel>& variables, bool isParameterCheck) {
    for (const std::string& expr : returnExpressions) {
        if (isParameterCheck) { 
            if (!isVariableUsed(variables, nullptr, expr, true, false, false, isParameterCheck, false))
                parameterComplexReturn = true; 
        }
        else {
            std::string checkThisPointer = expr;
            helperFunctions::removeLeadingAsterisks(checkThisPointer);
            if (checkThisPointer != "this") { // The 'this' is added as a field, however, we do not consider it as a simple nor complex return
                if (isVariableUsed(variables, nullptr, expr, true, false, false, false, false)) { // True if a field is found
                    simpleReturn = true; 
                }
                else {
                    complexReturn = true; 
                }
            }
        }
    }
}

// Finds if a field, local, or a parameter (normal and passed by reference) is modified
// Multiple modifications to the same field or parameter are only considered as 1 modification
//
void functionModel::findModifiedVariables(const std::unordered_map<std::string, variableModel>& variables, bool isParameterCheck) { 
    std::unordered_set<std::string> checked; 

    for (const std::string& expr : expressionAssignments) {
        std::size_t oldSize = checked.size();
        if (isParameterCheck)
            isVariableUsed(variables, nullptr, expr, false, true, false, true, false);
        else if (isVariableUsed(variables, &checked, expr, false, true, true, false, false)) {
            if (checked.size() > oldSize) { // 'checked' will not increase in size unless you add a new unique field to it 
                ++fieldsModifiedCount; // That way this only increase if a new field is changed
                oldSize = checked.size();
            }         
        }
    }
}

// Ignore calls from analysis
// For example, if call to ignore is 'foo', then some of the matched cases are foo<>() or bar::foo() or a->b.foo()
// However, usage of fields within these calls are not ignored (e.g., in arguments)
//
void functionModel::findIgnorableCalls(std::vector<callModel>& calls) {
    for (auto it = calls.begin(); it != calls.end();) {
        std::string callName = it->getName();

        std::size_t listOpen = callName.find("<");
        if (listOpen != std::string::npos)
            callName = callName.substr(0, listOpen);
            
        // Try to match the whole call
        if (CALLS.isIgnored(callName, unitLanguage)) { 
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
 
            if (CALLS.isIgnored(callName, unitLanguage)) 
                it = calls.erase(it);
            else ++it;                      
        }
    }
}

// Function calls: --> foo() bar::foo()
//  Checks if a function call is made to a method in the type, else it is removed and considered external 
//  Static and free function calls are considered external calls
//
// Method Calls: --> bar.foo() where ( bar ) could be a field or a type name or a namespace 
//  Checks if there is a method call on a field
//  For example, a.foo() where ( a ) is a field, else it is removed and considered as external method call
//
//  In C# or Java, a type name can be used with the dot operator to invoke static methods
//  For example, typeName.staticMethodName();
//  These are removed and considered as external function calls
//
//  ( this ), ( base ), and ( super ) can only be used to invoke non-static methods in the current type (this) or parent type (base or super)
//  For example, this.methodName();
//  So these should be also treated as function calls and not method calls
//
void functionModel::findCallsOnFields(const std::unordered_map<std::string, variableModel>& allFields,
                                           const std::unordered_set<std::string>& allMethodSignatures) {  
    // Check on function calls (Should be done before checking on method calls)
    for (auto it = functionCalls.begin(); it != functionCalls.end();) {  
        if (allMethodSignatures.find(it->getSignature()) == allMethodSignatures.end()) { 
            it = functionCalls.erase(it);
            ++externalFunctionCallsCount;
        }
        else ++it;
    }  
    
    // Check on method calls 
    for (auto it = methodCalls.begin(); it != methodCalls.end();) {
        // Could be a normal method call on a field
        if (!isVariableUsed(allFields, nullptr, it->getName(), false, false, false, false, false)) {
            if (unitLanguage != "C++") {
                // These should be function calls
                std::string inheritKeyword = (unitLanguage == "C#") ? "base" : "super";
                if ((unitLanguage == "C#" || unitLanguage == "Java") && 
                    (helperFunctions::isSubstringAtBeginning(it->getName(), "this") || 
                     helperFunctions::isSubstringAtBeginning(it->getName(), inheritKeyword))) functionCalls.push_back(*it);

                // Could be a call on a local or a parameter
                else if (isVariableUsed(allFields, nullptr, it->getName(), false, false, false, true, true)) 
                    ++externalMethodCallsCount;
                    
                // It is a static call
                else ++externalFunctionCallsCount;
                it = methodCalls.erase(it);  
                
            }
            else {
                ++externalMethodCallsCount;
                it = methodCalls.erase(it);  
            }                  
        }
        else ++it;   
    }
}

// Checks if an expression uses an field, local, or a parameter 
// Possible cases: 
// C++: this->a; (*this).a; Foo::a; this->a.b; (*this).a.b; Foo::a.b; a.b; a
// Java: super.a; this.a; Foo.a; super.a.b; this.a.b; Foo.a.b; a.b; a
// C#: base.a; this.a; Foo.a; base.a.b; this.a.b; Foo.a.b; a.b; a
// Where 'a' is a variableModel and Foo is type itself if the variableModel is an field
// Can match with complex uses of variables (e.g., this->a.b.c or a[]->b or (*a).b.c)
// 
bool functionModel::isVariableUsed(const std::unordered_map<std::string, variableModel>& variables, 
                                   std::unordered_set<std::string>* fieldsModified, 
                                   const std::string& expression, bool returnCheck, 
                                   bool parameterModifiedCheck,  bool localModifiedCheck,
                                   bool isParamaterCheck, bool isLocalCheck) {
                                    //else if (isVariableUsed(allFields, nullptr, it->getName(), false, false, false, true, true)) 
    std::string expr = expression; 
    helperFunctions::removeWhitespace(expr);
    helperFunctions::removeBracketSuffix(expr);
    
    // Removing () and {} on the outside of expression
    // Might remove } and ) for calls but that doesn't affect the analysis
    if (unitLanguage == "C++") {
        while (!expr.empty() && (expr.front() == '{' || expr.front() == '(')) { 
            if (expr.size() > 6 && expr.substr(1, 5) == "(*this)") break;
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
        std::size_t nullOp = expr.find("?"); 
        while (nullOp != std::string::npos) {
            expr.erase(nullOp, 1); 
            nullOp = expr.find("?"); // For chaining
        }         
    }

    // Remove pointers. For example, *a
    if (unitLanguage != "Java") helperFunctions::removeLeadingAsterisks(expr);     
    
    if (expr.empty()) return false;  

    // ^ indicates that we should only match from the beginning
    // We only care about the first two variables. For example, in a.b.c() the a.b is sufficient to determine what "a" is
    // Each regex has only two capturing or matching groups
    static const std::regex cppPattern(R"(^(?:\(\*this\)\.|this->|([^.->]*)(?:::|\.|->))([^.->]*))");
    static const std::regex javaPattern(R"(^(?:super|this|([^.]*))\.([^.]*))");
    static const std::regex csharpPattern(R"(^(?:base|this|([^.->]*))(?:\.|->)([^.->]*))");

    // $ Used to match end of line. For example, return this.a; matches but return this.a.b; doesn't
    static const std::regex cppReturnPattern(R"(^(?:\(\*this\)\.|this->|([^.->]*)(?:::|\.|->))([^.->\(\){}]*)$)");
    static const std::regex javaReturnPattern(R"(^(?:super|this|([^.]*))\.([^.\(\)]*)$)");
    static const std::regex csharpReturnPattern(R"(^(?:base|this|([^.->]*))(?:\.|->)([^.->\(\)]*)$)");

    const std::regex* currentRegex = nullptr;

    if (!returnCheck) {
        if (unitLanguage == "C++")       currentRegex = &cppPattern;
        else if (unitLanguage == "Java") currentRegex = &javaPattern;
        else if (unitLanguage == "C#")   currentRegex = &csharpPattern;
    } else {
        if (unitLanguage == "C++")       currentRegex = &cppReturnPattern;
        else if (unitLanguage == "Java") currentRegex = &javaReturnPattern;
        else if (unitLanguage == "C#")   currentRegex = &csharpReturnPattern;
    }

    std::smatch match;
    bool isMatched = std::regex_search(expr, match, *currentRegex);

    
    int count = isMatched ? 2 : 1;
    bool overShadow = true; // Needed in cases such as this.data = data where this.data is an field and data is a local or a parameter 

    // Regex uses the original ( expr ) to extract the matches. Therefore, we should not modify the original ( expr )
    // ( possibleVar ) is assigned ( expr ) initially as it could be the variableModel itself especially for returns (e.g., return a;)
    std::string possibleVariable = expr; 
    for (int i = 0; i < count; i++) {
        if (isMatched && i == 0) {    
            if (match[1] == "") { // We never catch 'base, super, or this' so if this condition is true, then it is one of them and the catch is a field
                possibleVariable = match[2]; 
                overShadow = false; // It is a field, so skip checking locals and parameters
            }   
            else if (!returnCheck) // For ( returnCheck ), this avoids conditions such as a.foo()
                possibleVariable = match[1]; // Perhaps variableModel itself (e.g., a or a.foo())        
        }
        // In C# or Java, a type name can be used to access static fields only
        // In C++, a type name can be used to access static and non-static fields
        // Parent type names can also be used to access fields in the child type, but we will ignore this case for now
        // Checking with type name also avoids problems with other typees or properties having the same names as the fields in the current type
        else if (isMatched && match[1] != "" && match[2] != "") {// Case of type name itself. The 'match[2] != ""' is to skip if it is just 'a''.
            std::string possibleTypeName = match[1];
            std::size_t listOpen = possibleTypeName.find("<");
            if (listOpen != std::string::npos) 
                possibleTypeName = possibleTypeName.substr(0, listOpen);
            if (typeNameParsed == possibleTypeName)
                possibleVariable = match[2];
        }

        if (overShadow) {
            // Checked first in case of overshadowing if variables = fields
            if (locals.find(possibleVariable) != locals.end()) {
                if (localModifiedCheck && locals.at(possibleVariable).isNonPrimitive()) 
                    nonPrimitiveLocalOrParameterModified = true;
                if (returnCheck) { 
                    if (variablesCreatedWithNew.find(possibleVariable) != variablesCreatedWithNew.end())
                        if (!variableCreatedWithNewAndReturned) variableCreatedWithNewAndReturned = true;
                }
                if (isLocalCheck) return true;
                else return false;
            }

            else if (parameters.find(possibleVariable) != parameters.end()) {
                parameterUsed = true;
                if (parameterModifiedCheck) {
                    if (parameters.at(possibleVariable).isNonPrimitive()) nonPrimitiveLocalOrParameterModified = true;
                    findModifiedRefParameter(possibleVariable, isMatched);
                }
                if (returnCheck) {        
                    if (variablesCreatedWithNew.find(possibleVariable) != variablesCreatedWithNew.end())
                        if (!variableCreatedWithNewAndReturned) variableCreatedWithNewAndReturned = true;
                }
                if (isParamaterCheck) return true;
                else return false;      
            }
        }
        
        // You only ever get here if variables = fields
        if (variables.find(possibleVariable) != variables.end()) {
            if (fieldsModified)
                if (fieldsModified->find(possibleVariable) == fieldsModified->end())
                    fieldsModified->insert(possibleVariable);
                
            fieldUsed = true;
            nonPrimitiveFieldExternal = variables.at(possibleVariable).isNonPrimitiveExternal(); 
            if (returnCheck) {    
                if (variablesCreatedWithNew.find(possibleVariable) != variablesCreatedWithNew.end())
                    if (!variableCreatedWithNewAndReturned) variableCreatedWithNewAndReturned = true;
            }            
            return true;                                  
        }
    }

    // If you get here, then whatever is modified is definitely not a field, local, or a parameter
    //   so, it is safe to assume that it is a global or a static
    if (parameterModifiedCheck) globalOrStaticVariableModified = true;

    return false;  
}

// Gets stereotypes in one string
//
std::string functionModel::getStereotypesString() const {
    std::string stereotypesString;
    for (const std::string & s : stereotypes) {
        if (!stereotypesString.empty()) stereotypesString += " ";
        stereotypesString += s;
    }
    return stereotypesString;
}

// Gets specifiers in one string
//
std::string functionModel::getSpecifiersString() const {
    std::string specifierString;
    for (const std::string& s : specifiers) {
        if (!specifierString.empty()) specifierString += " ";
        specifierString += s;
    }
    return specifierString;
}

// Gets return type string
//
std::string functionModel::getReturnTypeParsed() const {
    std::string returnTypeString = returnType.getType();
    SPECIFIERS.removeSpecifiers(returnTypeString, unitLanguage);  
    helperFunctions::removeWhitespace(returnTypeString);
    return returnTypeString;
}


// Get return type string
//
std::string functionModel::getAttributesOrAnnotationsString() const {
    std::string attributesOrAnnotationsString;
    for (const std::string& a : attributesOrAnnotations) {
        if (!attributesOrAnnotationsString.empty()) attributesOrAnnotationsString += " ";
        attributesOrAnnotationsString += a;
    }
    return attributesOrAnnotationsString;
}

// Get return type string
//
std::string functionModel::getFunctionCallsString() const {
    std::string functionCallsString;
    for (const auto& f : functionCalls) {
        if (!functionCallsString.empty()) functionCallsString += " ";
        functionCallsString += f.getSignature();
    }
    return functionCallsString;
}