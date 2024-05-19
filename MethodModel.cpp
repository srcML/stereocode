// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file MethodModel.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "MethodModel.hpp"
#include "IgnorableCalls.hpp"

extern ignorableCalls IGNORED_CALLS;
extern primitiveTypes PRIMITIVES;

methodModel::methodModel(srcml_archive* archive, srcml_unit* unit, const std::string& methodXpath, 
                         const std::string& unitLang, const std::string& propertyReturnType, int unitNum) :
                         unitLanguage(unitLang),  xpath(methodXpath), unitNumber(unitNum) {
  
    srcML = srcml_unit_get_srcml(unit);
    isConstructorDestructor(archive, unit);
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
    trimWhitespace(paramList);
    trimWhitespace(methName);
    nameSignature = methName + paramList;
};
void methodModel::findMethodData(std::unordered_map<std::string, Variable>& attribute, 
                                 const std::string& classNamePar)  {
    classNameParsed = classNamePar;
    if (!constructorDestructorUsed) {                                
        PRIMITIVES.setLanguage(unitLanguage); 
        IGNORED_CALLS.setLanguage(unitLanguage);
        
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
        
        if (unitLanguage != "C++") isFunctionCall(attribute);

        isAttributeReturned(attribute);  
        isAttributeUsedInExpression(archive, unit, attribute);
        isAttributeOrParameterModified(archive, unit, attribute);

        isEmpty(archive, unit);

        if (unitLanguage == "C++") isConst(archive, unit);

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
        srcml_append_transform_xpath(archive, "/src:unit/*[self::src:constructor or self::src:destructor]/src:name");
    else
        srcml_append_transform_xpath(archive, "/src:unit/src:function/src:name");

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


// Gets the method name
//
void methodModel::findParameterList(srcml_archive* archive, srcml_unit* unit) {
    if (constructorDestructorUsed)
        srcml_append_transform_xpath(archive, "/src:unit/*[self::src:constructor or self::src:destructor]/src:parameter_list");
    else
        srcml_append_transform_xpath(archive, "/src:unit/src:function/src:parameter_list");
        
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
    if (returnType != "") { // If method was a property, type is found in previous steps
        returnTypeParsed = returnType;
        removeSpecifiers(returnTypeParsed, unitLanguage);
        trimWhitespace(returnTypeParsed);
        if (isNonPrimitiveType(returnType, nonPrimitiveReturnTypeExternal, unitLanguage, classNameParsed))
            nonPrimitiveReturnType = true;      
    }
    else {
        // This skips the generics parameters in Java return types
        std::string methodXpath = "/src:unit/src:function/src:type//text()[not(ancestor::src:parameter_list)]";

        srcml_append_transform_xpath(archive, methodXpath.c_str());
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        for (int i = 0; i < n; i++) {
            srcml_unit* resultUnit = srcml_transform_get_unit(result, i);
            std::string unparsed = srcml_unit_get_srcml(resultUnit);
            returnType += srcml_unit_get_srcml(resultUnit);
        }
        if (isNonPrimitiveType(returnType, nonPrimitiveReturnTypeExternal, unitLanguage, classNameParsed))
            nonPrimitiveReturnType = true; 
        
        returnTypeParsed = returnType;
        removeSpecifiers(returnTypeParsed, unitLanguage);
        trimWhitespace(returnTypeParsed);
        
        srcml_clear_transforms(archive);
        srcml_transform_free(result);
    }
}

// Collects the names of local variables
//
void methodModel::findLocalVariableName(srcml_archive* archive, srcml_unit* unit) {
    std::string decl_stmt = "//src:decl_stmt[count(ancestor::src:function) = 1]";
    std::string control = "//src:control/src:init";
    std::string decl = "/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    std::string methodXpath = decl_stmt + decl + " | " + control + decl;

    srcml_append_transform_xpath(archive, methodXpath.c_str());
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
        Variable v;

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
    std::string decl_stmt = "//src:decl_stmt[count(ancestor::src:function) = 1]";
    std::string control = "//src:control/src:init";
    std::string decl = "/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    std::string methodXpath = decl_stmt + decl + " | " + control + decl;

    srcml_append_transform_xpath(archive, methodXpath.c_str());
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
    std::string methodXpath = "/src:unit/src:function/src:parameter_list/src:parameter/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";

    srcml_append_transform_xpath(archive, methodXpath.c_str());
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
        Variable v;

        // Chop off [] for arrays
        if (unitLanguage == "C++") {  
            size_t start_position = parameterName.find("[");
            if (start_position != std::string::npos){
                parameterName = parameterName.substr(0, start_position);
                Rtrim(parameterName);
            }
        }
        v.setName(parameterName);
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
    std::string methodXpath = "/src:unit/src:function/src:parameter_list/src:parameter/src:decl/src:type[following-sibling::*[1][self::src:name]]";

    srcml_append_transform_xpath(archive, methodXpath.c_str());
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
    std::string methodXpath = "//src:return[count(ancestor::src:function) = 1]/src:expr";

    srcml_append_transform_xpath(archive, methodXpath.c_str());
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
        std::string methodXpath = "//src:call[count(ancestor::src:function) = 1";
        if (c == "function") // Constructor calls are a type of function calls 
            methodXpath += " and not(src:name/src:operator='->') and not(src:name/src:operator='.')]";      
        else if (c == "method") 
            methodXpath += " and src:name/src:operator='->' or src:name/src:operator='.']";

        methodXpath += "/src:name[following-sibling::*[1][self::src:argument_list]]";

        srcml_append_transform_xpath(archive, methodXpath.c_str());
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; ++i) {
            resultUnit = srcml_transform_get_unit(result, i);
            
            char * unparsed = nullptr;
            size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

            if (c == "function")  
                functionCall.push_back({unparsed, ""});
            else if (c == "method") 
                methodCall.push_back({unparsed, ""}); 

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
        std::string methodXpath = "//src:call[count(ancestor::src:function) = 1";
        if (c == "function") 
            methodXpath += " and not(src:name/src:operator='->') and not(src:name/src:operator='.')]";      
        else if (c == "method") 
            methodXpath += " and src:name/src:operator='->' or src:name/src:operator='.']";

        methodXpath += "/src:argument_list[preceding-sibling::*[1][self::src:name]]";

        srcml_append_transform_xpath(archive, methodXpath.c_str());
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; ++i) {
            resultUnit = srcml_transform_get_unit(result, i);
            
            char * unparsed = nullptr;
            size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
            
            if (c == "function")  
                functionCall[i].second = unparsed;
            else if (c == "method") 
                methodCall[i].second = unparsed;  

            free(unparsed);
        }
        srcml_clear_transforms(archive);
        srcml_transform_free(result);
    }
}

// Finds all variables that are declared/initialized with the "new" operator
//
void methodModel::findNewAssign(srcml_archive* archive, srcml_unit* unit) {  
    std::string methodXpath = "//src:decl_stmt[count(ancestor::src:function) = 1]/src:decl[./src:init/src:expr/src:operator[.='new']]/src:name";
    methodXpath += " | //src:expr_stmt[count(ancestor::src:function) = 1]/src:expr[./src:operator[.='new']]/src:name";

    srcml_append_transform_xpath(archive, methodXpath.c_str());
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

//
//
void methodModel::findAccessorMethods() {  
    srcml_archive* archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, srcML.c_str(), srcML.size());
    srcml_unit* unit = srcml_archive_read_unit(archive);

    std::string methodXpath = "//src:call[preceding-sibling::*[1][self::src:operator='=' or self::src:operator='+='";
    methodXpath += " or self::src:operator='-=' or self::src:operator='*=' or self::src:operator='/='";
    methodXpath += " or self::src:operator='%=' or self::src:operator='>>=' or self::src:operator='<<='";
    methodXpath += " or self::src:operator='&=' or self::src:operator='^=' or self::src:operator='|='";
    methodXpath += " or self::src:operator='\\?\\?=' or self::src:operator='>>>=' or self::src:operator='++'"; 
    methodXpath += " or self::src:operator='--'] or ancestor::src:return or ancestor::src:init]/src:name[following-sibling::*[1][self::src:argument_list]]";

    srcml_append_transform_xpath(archive, methodXpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string possibleAccessorMethod = unparsed;
        free(unparsed);

        if(functionCallSet.find(possibleAccessorMethod) != functionCallSet.end()) {
            accessorMethodCallUsed = true;
            break;
        }  
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
    srcml_unit_free(unit);
    srcml_archive_close(archive);
    srcml_archive_free(archive); 
}



// Determines if method is empty
//
void methodModel::isEmpty(srcml_archive* archive, srcml_unit* unit) {
    std::string methodXpath = "/src:unit/src:function/src:block/src:block_content[*[not(self::src:comment)][1]]";

    srcml_append_transform_xpath(archive, methodXpath.c_str());
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
    std::string methodXpath = "/src:unit/src:function/src:specifier[.='const']";
    srcml_append_transform_xpath(archive, methodXpath.c_str());
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
    srcml_append_transform_xpath(archive, "/src:unit/*[self::src:constructor or self::src:destructor]");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) constructorDestructorUsed = true;

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
            bool nonPrimitive = !isPrimitiveType(type);
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
        bool nonPrimitive = !isPrimitiveType(type);
        trimWhitespace(type);
        bool referenceArray = type.find("[]") != std::string::npos; 
        if (referenceArray || (nonPrimitive && propertyCheck))                
            parameterRefChanged = true;     
    }             
}


// Determines if a return expression returns an attribute
//
void methodModel::isAttributeReturned(std::unordered_map<std::string, Variable>& attribute) {
    for (const std::string& expr : returnExpression) {
        if (isAttributeUsed(attribute, expr, true, false))
            // Simple return expression. 
            // For example, 'return a;' where 'a' is an attribute
            attributeReturned = true; 
        else
            // Complex return expression. 
            // For example, 'return a + value;' where 'a' is an attribute
            attributeNotReturned = true;     
    }
}

// Determines if an attribute is used in an expression
//
void methodModel::isAttributeUsedInExpression(srcml_archive* archive, srcml_unit* unit, std::unordered_map<std::string, Variable>& attribute)  {
    std::string methodXpath = "//src:expr[count(ancestor::src:function)]/src:name";
    methodXpath += " | //src:expr[count(ancestor::src:function)]/src:call/src:name";

    srcml_append_transform_xpath(archive, methodXpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;

    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);
        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        
        isAttributeUsed(attribute, unparsed, false, false);

        free(unparsed);
        
    }        
    srcml_clear_transforms(archive);
    srcml_transform_free(result);    
}

// Finds if an attribute is changed or 
//  if a parameter that is passed by reference is changed
//
void methodModel::isAttributeOrParameterModified(srcml_archive* archive, srcml_unit* unit, 
                                       std::unordered_map<std::string, Variable>& attribute) { 
    int changed = 0;
    // An attribute that is changed multiple times should only be considered as 1 change
    std::unordered_set<std::string> checked; 

    std::string methodXpath = "//src:expr[count(ancestor::src:function) = 1]/src:name[";
    methodXpath += "following-sibling::*[1][self::src:operator='=' or self::src:operator='+='";
    methodXpath += " or self::src:operator='-=' or self::src:operator='*=' or self::src:operator='/='";
    methodXpath += " or self::src:operator='%=' or self::src:operator='>>=' or self::src:operator='<<='";
    methodXpath += " or self::src:operator='&=' or self::src:operator='^=' or self::src:operator='|='";
    methodXpath += " or self::src:operator='\\?\\?=' or self::src:operator='>>>=' or self::src:operator='++'"; 
    methodXpath += " or self::src:operator='--'] or preceding-sibling::*[1][self::src:operator='++' or self::src:operator='--']]";

    srcml_append_transform_xpath(archive, methodXpath.c_str());
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

        if (isAttributeUsed(attribute, possibleAttribute, false, true) 
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
void methodModel::isIgnorableCall(std::vector<std::pair<std::string, std::string>>& calls) {
    for (auto it = calls.begin(); it != calls.end();) {
        std::string callName = it->first;

        size_t listOpen = callName.find("<");
        if (listOpen != std::string::npos)
            callName = callName.substr(0, listOpen);
            
        // Try to match the whole call
        if (IGNORED_CALLS.isCallIgnored(callName)) { 
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
 
            if (IGNORED_CALLS.isCallIgnored(callName)) 
                it = calls.erase(it);

            else ++it; 
                        
        }
    }
}

// In C# or Java, a class name and/or namespace can be used with the dot operator to invoke static methods
// For example, namespace.className.[data_member].staticMethodName();
// So these are removed
//
// 'this, base, and super' can only be used to invoke non-static methods in the class or parent class
// For example, this.methodName();
// So these are kept
//
// Calls on static data members and local variables are kept since static locals are collected
//
// Therefore, these should be treated as function calls
//
// Function call list should be left in the form of namespace::className::foo(), base.foo(), super.foo(), foo()
//
void methodModel::isFunctionCall(std::unordered_map<std::string, Variable>& attribute) {
    for (auto it = methodCall.begin(); it != methodCall.end();) {
        size_t dotOperator = it->first.find(".");
        if (dotOperator != std::string::npos) {
            // Keep call if it is to a variable
            if (!isAttributeUsed(attribute, it->first, false, false)) {
                std::string possibleMethodCall = it->first.substr(0, dotOperator);
                if (possibleMethodCall == "this" || possibleMethodCall == "base" || possibleMethodCall == "super") { 
                    functionCall.push_back(*it); 
                    it = methodCall.erase(it);
                }         
                else {
                    it = methodCall.erase(it); // Removes static calls  
                    ++numOfExternalFunctionCalls;
                }             
            }  
            else ++it;                          
        }
        else ++it;
    }              
}

// Checks if there is a method call on an attribute
// For example, a.foo() where a is an attribute, else it is removed
// 
// Also, check if a function call is made to a method in the class, else it is removed
// For C#, Java --> calls in the form of foo(), base.foo(), super.foo() are considered. 
//
void methodModel::isCallOnAttribute(std::unordered_map<std::string, Variable>& attribute, 
                                   const std::unordered_set<std::string>& classMethods, 
                                   const std::unordered_set<std::string>& inheritedClassMethods) {
    if (!constructorDestructorUsed) { 
        // Check on method calls
        for (auto it = methodCall.begin(); it != methodCall.end();) {
            if (!isAttributeUsed(attribute, it->first, false, true)) {
                ++numOfExternalMethodCalls;
                it = methodCall.erase(it);                          
            }
            else ++it;
        }

        // Check on function calls
        // We need to simply check the signature 
        for (auto it = functionCall.begin(); it != functionCall.end();) {
            std::string funcCallArgument = it->second;
            removeBetweenComma(funcCallArgument, false);

            std::string funcCallParsed = it->first + funcCallArgument;
            removeNamespace(funcCallParsed, true, unitLanguage);

            trimWhitespace(funcCallParsed);    
            if (funcCallParsed.find(classNameParsed) == std::string::npos) { 
                if (classMethods.find(funcCallParsed) == classMethods.end() && 
                    inheritedClassMethods.find(funcCallParsed) == inheritedClassMethods.end()) {        
                        it = functionCall.erase(it);
                        ++numOfExternalFunctionCalls;
                    }   
                else {
                    functionCallSet.insert(it->first); 
                    ++it;
                }
            }
            else 
                // Skip constructor calls to the same class
                // They are also not counted as external calls
                it = functionCall.erase(it);
            
        }
    }
}

// Checks if an expression uses an attribute, local, or a parameter --> call them variable
// Possible cases: 
// C++: this->a; (*this).a; Foo::a; this->a.b; (*this).a.b; Foo::a.b; a.b; a
// Java: super.a; this.a; Foo.a; super.a.b; this.a.b; Foo.a.b; a.b; a
// C#: base.a; this.a; Foo.a; base.a.b; this.a.b; Foo.a.b; a.b; a
// Where 'a' is a variable and Foo is either a parent class or class itself if the variable is an attribute
// Can match with complex uses of variables (e.g., this->a.b.c or a[]->b or (*a).b.c)
//
bool methodModel::isAttributeUsed(std::unordered_map<std::string, Variable>& attribute, 
                                       const std::string& expression, bool returnCheck, 
                                       bool parameterCheck) {

    std::string expr = expression; 
    trimWhitespace(expr);

    size_t openingBracket = expr.find("["); 
    if (openingBracket != std::string::npos)
        expr = expr.substr(0, openingBracket); 
    
    if (unitLanguage == "C++") {
        size_t checkThis = expr.find("(*this)");
        if (checkThis != std::string::npos) {
            std::string thisStr = expr.substr(1, 5);
            while (expr[0] == '{' || (expr[0] == '(' && thisStr != "(*this)")) {
                expr.erase(0, 1);
                thisStr = expr.substr(1, 5);
            }
            while (expr[expr.size() - 1] == '}' || expr[expr.size() - 1] == ')')
                expr.erase(expr.size() - 1, 1);
        }
    }
    else {
        while (expr[0] == '(' ) 
            expr.erase(0, 1);   
               
        while (expr[expr.size() - 1] == ')')
            expr.erase(expr.size() - 1, 1);
    }

    // In C# the null-conditional operator is represented by ?. and it allows you to check if an object 
    //  is null before accessing its members.
    // For example, int? stringLength = testString?.Length; 
    //  stringLength = length if testString is not null, otherwise stringLength becomes null
    //  The ? used with int (or any other value type) makes it nullable, meaning it can hold either a valid integer or null
    if (unitLanguage == "C#") {
        size_t nullConditionOperator = expr.find("?"); 
        while (nullConditionOperator != std::string::npos) {
            expr.erase(nullConditionOperator, 1); 
            nullConditionOperator = expr.find("?"); 
        }         
    }

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
    if (!returnCheck) {
        std::string pattern = "";
        if (unitLanguage == "C++") 
            pattern = R"(^(?:\(\*this\)\.|this->|([^.->]*)(?:::|\.|->))([^.->]*))";
        else if (unitLanguage == "Java")
            pattern = R"(^(?:super|this|([^.]*))\.([^.]*))";
        else if (unitLanguage == "C#")
            pattern = R"(^(?:base|this|([^.->]*))(?:\.|->)([^.->]*))";
    
        const std::regex regexPattern(pattern);
        isMatched = std::regex_search(expr, match, regexPattern);
    }

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
            if (parameterCheck)  isParameterRefChanged(expr, isMatched);
            if (returnCheck) {
                if (variablesCreatedWithNew.find(expr) != variablesCreatedWithNew.end())
                    variableCreatedWithNewReturned = true;
            }
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
