// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file MethodModel.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#include "MethodModel.hpp"
#include <fstream>

methodModel::methodModel() : methodName(), methodHeader(), returnType(), returnTypeSeparated(),
                             parameterList(), parameterName(), parameterType(),
                             localVariableName(), localVariableType(),
                             stereotype(), functionCall(), methodCall(),
                             constructorCall(), returnExpression(), xpath(), methodSrcML(),
                             constMethod(false), attributeReturned(false), paraOrLocalReturned(false),
                             returnsNew(false), returnsCall(false), attributeUsed(false), parameterChanged(false), 
                             empty(false),  nonPrimitiveAttribute(false), 
                             nonPrimitiveAttributeExternal(false), nonPrimitiveReturnType(false),
                             nonPrimitiveReturnTypeExternal(false),
                             nonPrimitiveLocal(false), nonPrimitiveLocalExternal(false),
                             nonPrimitiveParamater(false), nonPrimitiveParamaterExternal(false),
                             attributeModified(false) {};

methodModel::methodModel(srcml_archive* archive, srcml_unit* unit, std::string xpathS, int unitNumber) : methodModel() {
    xpath[unitNumber] = xpathS;

    methodSrcML = srcml_unit_get_srcml(unit);
    methodHeader = methodSrcML.substr(0, methodSrcML.find("("));

    findMethodName(archive, unit);     
    findLocalVariablesNames(archive, unit);
    findParametersNames(archive, unit); 
    findAllCalls(archive, unit);
    findReturnCalls(archive, unit);
    findReturnExpressions(archive, unit);
    isEmpty(archive, unit);
    if (PRIMITIVES.getLanguage() == "C++")
        isConst(archive, unit);
};

// Class-related analysis.
//
void methodModel::findMethodInfo(srcml_archive* archive, srcml_unit* unit,  std::vector<AttributeInfo>& attribute, std::string className){
    findMethodReturnType(archive, unit, className);
    findParametersTypes(archive, unit, className);
    findLocalVariablesTypes(archive, unit, className);
    isAttributeReturned(attribute, className); 
    isAttributeUsed(archive, unit, attribute, className); 
    countChangedAttribute(archive, unit, attribute);
}

// Gets the method name
//
void methodModel::findMethodName(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:function/src:name";
    xpath += "[count(ancestor::src:function) = 1]";

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1){
        srcml_unit* resultUnit = nullptr;
        resultUnit = srcml_transform_get_unit(result, 0);
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        methodName = unparsed;

        free(unparsed);   
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Gets the method return type 
//
void methodModel::findMethodReturnType(srcml_archive* archive, srcml_unit* unit, std::string className) {
    std::string xpath = "//src:function/src:type";
    xpath += "[count(ancestor::src:function) = 1]";

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    nonPrimitiveReturnTypeExternal = true;
    className = trimWhitespace(className.substr(0, className.find("<")));

    srcml_unit* resultUnit = nullptr;
    if (n == 1) {
        resultUnit = srcml_transform_get_unit(result, 0);
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        returnType = unparsed;
        returnTypeSeparated = trimWhitespace(removeSpecifiers(returnType));

        if (nonPrimitiveReturnTypeExternal){
            std::string type = trimWhitespace(removeSpecifiers(returnType));
            if (!isPrimitiveContainer(type)){
                if (!nonPrimitiveReturnType) nonPrimitiveReturnType = true; 
                if (type.find(className) != std::string::npos)
                    nonPrimitiveReturnTypeExternal = false;                     
            }
        }

        free(unparsed); 
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the names of local variables
//
void methodModel::findLocalVariablesNames(srcml_archive* archive, srcml_unit* unit) {
    std::string decl_stmt = "//src:decl_stmt[not(ancestor::src:catch)";
    decl_stmt += " and count(ancestor::src:function) = 1]";
    std::string control = "//src:control/src:init";
    std::string decl = "/src:decl/src:name";

    std::string xpath = decl_stmt + decl + " | " + control + decl;

    srcml_append_transform_xpath(archive, xpath.c_str());
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
       
        size_t arr = localName.find("[");
        if (arr != std::string::npos) {
            localName = localName.substr(0, arr);
            localName = Rtrim(localName);
        } 

        localVariableName.push_back(localName);

        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the types of local variables
//
void methodModel::findLocalVariablesTypes(srcml_archive* archive, srcml_unit* unit, std::string className) {
    std::string decl_stmt = "//src:decl_stmt[not(ancestor::src:catch)";
    decl_stmt += " and count(ancestor::src:function) = 1]";
    std::string control = "//src:control/src:init";
    std::string decl = "/src:decl/src:type[following-sibling::*[1][self::src:name]]";

    std::string xpath = decl_stmt + decl + " | " + control + decl;

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    nonPrimitiveLocalExternal = true;
    className = trimWhitespace(className.substr(0, className.find("<")));

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

        localVariableType.push_back(type);

        if (nonPrimitiveAttributeExternal){
            type = trimWhitespace(removeSpecifiers(type));
            if (!isPrimitiveContainer(type)){
                if (!nonPrimitiveLocal) nonPrimitiveLocal = true; 
                if (type.find(className) != std::string::npos)
                    nonPrimitiveAttributeExternal = false;                     
            }
        }

        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the names of parameters in each method
//
void methodModel::findParametersNames(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:function/src:parameter_list[count(ancestor::src:function) = 1]";
    xpath += "/src:parameter/src:decl/src:name";

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string paraName = unparsed;

        size_t arr = paraName.find("[");
        if (arr != std::string::npos) {
            paraName = paraName.substr(0, arr);
            paraName = Rtrim(paraName);
        }
    
        parameterName.push_back(paraName);

        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the types of parameters
//
void methodModel::findParametersTypes(srcml_archive* archive, srcml_unit* unit, std::string className) {
    // Parameters, locals, and attributes can sometime have only a type(no name, for backward compatibility, useless). 
    // Get the type only if the parameter has a name
    std::string xpath = "//src:function/src:parameter_list[count(ancestor::src:function) = 1]";
    xpath += "/src:parameter/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    nonPrimitiveParamaterExternal = true;
    className = trimWhitespace(className.substr(0, className.find("<")));

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string type = unparsed;

        parameterType.push_back(type);

        if (nonPrimitiveParamaterExternal){
            type = trimWhitespace(removeSpecifiers(type));
            if (!isPrimitiveContainer(type)){
                if (!nonPrimitiveParamater) nonPrimitiveParamater = true; 
                if (type.find(className) != std::string::npos)
                    nonPrimitiveParamaterExternal = false;                     
            }
        }
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

void methodModel::findReturnExpressions(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:return[count(ancestor::src:function) = 1 and not(ancestor::src:catch)]/src:expr"; 

    srcml_append_transform_xpath(archive, xpath.c_str());
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
        
        returnExpression.push_back(expr);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// For each function, find all return expressions and determine if they contain an attribute
// Complex return expressions such as 'return a+b;' are ignored since they don't return an attribute, but a value calculated from the attribute(s) 
//
//  Assume "a" is an attribute. Some cases are:
//      return a;
//      return new int{3}; ignore
//      return new int[6]; ignore
//      return new int; ignore
//      return (a);
//      return (a+b);
//      return {a};
//      return   * * a;
//      return *a;
//      return this->a;
//      return (*this).a;
//      return n.b[4]; ignore
//      return a[2];
//      return Foo::a;
//      return &a;
//      return 45; ignore
// 
void methodModel::isAttributeReturned(std::vector<AttributeInfo>& attributes, std::string className) {
    int attributeIndex = -1;
    int parameterIndex = -1;
    className = trimWhitespace(className.substr(0, className.find("<")));
    for (size_t j = 0; j < returnExpression.size(); ++j) {
        std::string returnExpr = returnExpression[j];  
        if (returnExpr.find("new ") == std::string::npos){ // Ignore expressions that contain the 'new' operator
            returnExpr = trimWhitespace(returnExpr);
            std::vector<std::string> specifiers = { "this->", "(*this).", "*"};
            for (const std::string& s : specifiers) {
                size_t pos = returnExpr.find(s);
                while (pos != std::string::npos) {
                    returnExpr.erase(pos, s.size());
                    pos = returnExpr.find(s);
                }
            }   

            if (returnExpr[0] == '(' || returnExpr[0] == '{'){ // (a) or {a}
                returnExpr.erase(0, 1);
                returnExpr.erase(returnExpr.find_last_of(")}"), 1);
            }

            size_t c = returnExpr.find(className + "::"); // className::a
            if (c != std::string::npos)
                returnExpr.erase(c, className.size() + 2);
            
            size_t a = returnExpr.find("["); // a[index]
            if (a != std::string::npos)
                returnExpr = returnExpr.substr(0, a);

            returnExpr = trimWhitespace(returnExpr);

            if (isAttribute(attributes, parameterName, localVariableName, returnExpr, false, attributeIndex, parameterIndex)) {
                if (!attributeReturned) attributeReturned  = true; // Returns an attribute
            }
            if (attributeIndex == -2)
                if (!paraOrLocalReturned) paraOrLocalReturned = true; // Returns a parameter or a local
        } 
    } 
}

void methodModel::findAllCalls(srcml_archive* archive, srcml_unit* unit) {
    functionCall = findCalls(archive, unit, "function", true);
    methodCall = findCalls(archive, unit,"method", true);
    constructorCall = findCalls(archive, unit, "constructor", true);
}

// Returns a list of calls that are not below throw or catch or in ignorableCalls
// does not include calls following new operator: when callType is method
// does not include calls following the . or -> or new operator: when callType is function
// calls that follow the new keyword are constructor calls and do not count as a method or function call
//
std::vector<std::string> methodModel::findCalls(srcml_archive* archive, srcml_unit* unit, const std::string& callType, bool ignoreCalls) const {
    std::vector<std::string> ignorableCalls;
    if (PRIMITIVES.getLanguage() == "C++")
        ignorableCalls = {"assert"};
    else {
        ignorableCalls = {"WriteLine", "Write", "ReadLine", "Write", "Read", "Trace", "Assert"};
    }
    std::vector<std::string> calls;
    std::string xpath = "//src:call[not(ancestor::src:throw) and not(ancestor::src:catch)";
    xpath += " and count(ancestor::src:function) = 1";
    if (callType == "function") {    
        xpath += " and not(preceding-sibling::*[1][self::src:operator='new'])";
        xpath += " and not(src:name/src:operator='->') and not(src:name/src:operator='.')]";
    }
    else if (callType == "method") {
        xpath += " and not(preceding-sibling::*[1][self::src:operator='new'])";
        xpath += " and src:name/src:operator='->' or src:name/src:operator='.']";
    }  
    else if(callType == "constructor")
        xpath += " and preceding-sibling::*[1][self::src:operator='new']]";  

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string call = unparsed;
        free(unparsed);

        call = trimWhitespace(call.substr(0, call.find("(")));

        // Ignore calls listed in ignorableCalls and primitive calls (int(), double()).
        bool found = false;
        if (ignoreCalls){     
            for (std::string c : ignorableCalls){
                if (call.find(c) != std::string::npos){
                    found = true;
                    break;
                }               
            }
        }
        
        if (!found)
            calls.push_back(call);     
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);

    return calls;
}

void methodModel::findReturnCalls(srcml_archive* archive, srcml_unit* unit) {
    int calls = 0;
    std::string xpath = "//src:return/src:expr/src:call[not(ancestor::src:throw) and not(ancestor::src:catch)";
    xpath += " and count(ancestor::src:function) = 1]";

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string call = unparsed;
        free(unparsed);

        if (call.find("new ") == std::string::npos)
            returnsNew = true;

        calls++;     
    }
    if (calls > 0) returnsCall = true;
    srcml_clear_transforms(archive);
    srcml_transform_free(result);

    
}

// Determines if a method is using an attribute
//     
void methodModel::isAttributeUsed(srcml_archive* archive, srcml_unit* unit,  std::vector<AttributeInfo>& attribute, std::string className)  {
    std::string xpath = "//src:expr[count(ancestor::src:function) = 1]";
    xpath += "//src:name[not(ancestor::src:throw) and not(ancestor::src:catch)";
    xpath += " and not(ancestor::src:argument_list[@type='generic']) and not(following::*[1][self::src:argument_list])";
    xpath += " and not(following::*[1][src:argument_list[@type='generic']])";
    xpath += " and not(preceding-sibling::*[1][self::src:operator ='.' or self::src:operator = '->'])";
    xpath += " and not(preceding::*[1][self::src:operator ='new']) and not(following-sibling::*[1][self::src:operator = '::'])";
    xpath += " and count(*)=0 and not(self::src:name='this') or preceding-sibling::*[2][self::src:name ='this'] or preceding-sibling::*[3][self::src:name ='this']]";

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit* resultUnit = nullptr;
    int attributeIndex = -1;
    int parameterIndex = -1;
    nonPrimitiveAttributeExternal = true;

    className = trimWhitespace(className.substr(0, className.find("<")));

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string possibleAttribute = unparsed;
        free(unparsed);
 
        // Check if method uses non-primitive attributes that are all of different type than class
        if (isAttribute(attribute, parameterName, parameterType, possibleAttribute, false, attributeIndex, parameterIndex)){
            if (!attributeUsed) attributeUsed = true;
            if (attribute[attributeIndex].getNonPrimitive()){
                nonPrimitiveAttribute = true;
                std::string type = attribute[attributeIndex].getType();
                if (type.find(className) != std::string::npos){
                    nonPrimitiveAttributeExternal = false;
                    break;
                }              
            } 
        }      
        attributeIndex = -1;
    }        
    srcml_clear_transforms(archive);
    srcml_transform_free(result);    
}

// Determines if method is empty
//
void methodModel::isEmpty(srcml_archive* archive, srcml_unit* unit) {
    std::string xpath = "//src:block_content[*[not(self::src:comment)][1]";
    xpath += " and count(ancestor::src:function) = 1]";

    srcml_append_transform_xpath(archive, xpath.c_str());
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
    std::string xpath = "//src:function/src:specifier[self::src:specifier='const']";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) constMethod = true;
  
    
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

void methodModel::setStereotype (const std::string& s) {
        stereotype.push_back(s);
}

std::string methodModel::getStereotype () const{
    std::string result = "";

    for (const std::string &value : stereotype)
        result += value + " ";

    if (result != "")
        return Rtrim(result);
        
    return result;
}

// An attributes changed multiple times is only counted as one change
//
void methodModel::countChangedAttribute(srcml_archive* archive, srcml_unit* unit, std::vector<AttributeInfo>& attribute) {
    std::vector<AttributeInfo> attributeCopy = attribute; 
    int changes = 0;
    changes += findIncrementedAttribute(archive, unit, attributeCopy);   
    changes += findAssignOperatorAttribute(archive, unit, attributeCopy);  
    attributeModified = changes;
}


int methodModel::findIncrementedAttribute(srcml_archive* archive, srcml_unit* unit, std::vector<AttributeInfo>& attribute) {
    const std::vector<std::string> INC_OPS = {"++", "--"};
    int changed = 0;
    int attributeIndex = -1;
    int parameterIndex = -1;
    for (size_t i = 0; i < INC_OPS.size(); i++) {  
        std::string xpath = "//src:expr[count(ancestor::src:function) = 1]";
        xpath += "//src:name[not(ancestor::src:catch)";
        xpath += " and following-sibling::*[1][self::src:operator='" + INC_OPS[i] + "']";
        xpath += "or preceding-sibling::*[1][self::src:operator='" + INC_OPS[i] + "']]";
        
        srcml_append_transform_xpath(archive, xpath.c_str());
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; ++i) {
            resultUnit = srcml_transform_get_unit(result, i);
            char *unparsed = nullptr;
            size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
            std::string possibleAttribute = unparsed;
            free(unparsed);
            size_t arr = possibleAttribute.find("[");

            if (arr != std::string::npos) {
                possibleAttribute = possibleAttribute.substr(0, arr);
                possibleAttribute = Rtrim(possibleAttribute);
            }              
            if (isAttribute(attribute, parameterName, localVariableName, possibleAttribute, true, attributeIndex, parameterIndex))
                ++changed;
        }
        srcml_clear_transforms(archive);
        srcml_transform_free(result); 
    }
    return changed;
}

int methodModel::findAssignOperatorAttribute(srcml_archive* archive, srcml_unit* unit, std::vector<AttributeInfo>& attribute) {
    int changed = 0;
    int attributeIndex = -1;
    int parameterIndex = -1;
    for (size_t i = 0; i < ASSIGNMENT_OPERATOR.size(); i++) {
        std::string xpath = "//src:expr[count(ancestor::src:function) = 1]";
        xpath += "//src:name[not(ancestor::src:catch) and not(ancestor::src:throw)";
        xpath += " and following-sibling::*[1][self::src:operator='" + ASSIGNMENT_OPERATOR[i] + "']]";
    
        srcml_append_transform_xpath(archive, xpath.c_str());
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
            size_t arr = possibleAttribute.find("[");
            if (arr != std::string::npos) {
                possibleAttribute = possibleAttribute.substr(0, arr);
                possibleAttribute = Rtrim(possibleAttribute);
            }        
            if (isAttribute(attribute, parameterName, localVariableName, possibleAttribute, true, attributeIndex, parameterIndex))
                ++changed;                 
            else if (ASSIGNMENT_OPERATOR[i] == "=" && parameterIndex != -1){
                std::string lang = PRIMITIVES.getLanguage();
                if (lang == "C++" || lang == "C#"){
                    bool reference = parameterType[parameterIndex].find("&") != std::string::npos;
                    bool referencePointer = parameterType[parameterIndex].find("*") != std::string::npos;
                    bool referenceArray = trimWhitespace(parameterType[parameterIndex]).find("[]") != std::string::npos;                  
                    if (lang == "C++"){
                        bool constant = parameterType[parameterIndex].find("const") != std::string::npos;
                        if ((reference || referencePointer || referenceArray) && !constant)                    
                            parameterChanged = true;   
                    }
                    else if (lang == "C#"){
                        bool referenceOut = parameterType[parameterIndex].find("out") != std::string::npos;
                        bool nonPrimit = !isPrimitiveContainer(trimWhitespace(removeSpecifiers(parameterType[parameterIndex])));
                        if (reference || referencePointer || referenceArray || referenceOut || nonPrimit)                  
                            parameterChanged = true;     
                    }       
                }
                else if (lang == "Java"){
                    bool nonPrimit = !isPrimitiveContainer(trimWhitespace(removeSpecifiers(parameterType[parameterIndex])));
                    if (nonPrimit)                  
                        parameterChanged = true;     
                }
                             
            }
           parameterIndex = -1;      
        }
      srcml_clear_transforms(archive);
      srcml_transform_free(result); 
    }
    return changed;
}


// Checks if a method has a call on an object. For example, a.b() where a is an attribute.
//
bool methodModel::callOnAttribute(std::vector<AttributeInfo>& attribute, std::string callType) {
    int attributeIndex = -1;
    int parameterIndex = -1;
    if (callType == "method"){
        for (size_t i = 0; i < methodCall.size(); ++i) {
            size_t dot = methodCall[i].find(".");
            size_t arrow = methodCall[i].find("->");        
            if (dot != std::string::npos) {
                std::string callingObject = methodCall[i].substr(0, dot);
                if (isAttribute(attribute, parameterName,  localVariableName, callingObject, false, attributeIndex, parameterIndex)) return true;         
            }
            if (arrow != std::string::npos) {
                std::string callingObject = methodCall[i].substr(0, arrow);
                if (isAttribute(attribute, parameterName,  localVariableName, callingObject,  false, attributeIndex, parameterIndex)) return true;
            }
            if (callOnArgument(attribute, callType, i)) return true;
        }
    }
    if (callType == "function"){
        for (size_t i = 0; i < functionCall.size(); ++i) {
            if (callOnArgument(attribute, callType, i)) return true;
        }
    }
    return false;
}

// Checks if call uses an attribute as argument. For example, Foo(a)
//
bool methodModel::callOnArgument(std::vector<AttributeInfo>& attribute, std::string callType, int i) {
    size_t argumentListOpen;
    size_t argumentListClose;
    std::string callingObject;
    int attributeIndex = -1;
    int parameterIndex = -1;
    if (callType == "method"){
        argumentListOpen = methodCall[i].find("(");
        argumentListClose = methodCall[i].find(")");
        callingObject = methodCall[i].substr(argumentListOpen+1, argumentListClose - (argumentListOpen + 1));
    }
    else if (callType == "function"){
        argumentListOpen = functionCall[i].find("(");
        argumentListClose = functionCall[i].find(")");
        callingObject = functionCall[i].substr(argumentListOpen+1, argumentListClose - (argumentListOpen + 1));
    }
    callingObject = trimWhitespace(callingObject);

    size_t start = 0;
    size_t end = callingObject.find(",");
    while (end != std::string::npos) {
        if (isAttribute(attribute, parameterName,  localVariableName, callingObject.substr(start, end - start), false, attributeIndex, parameterIndex)){
            return true;
        }
        start = end + 1;
        end = callingObject.find(",", start);
    }
    if (isAttribute(attribute, parameterName,  localVariableName, callingObject.substr(start, end - start), false, attributeIndex, parameterIndex)){
            return true;
    } 
    return false;
}
