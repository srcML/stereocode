// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file MethodModel.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "MethodModel.hpp"

methodModel::methodModel(srcml_archive* methodArchive, srcml_unit* methodUnit, const std::string& methodXpath, 
                         std::string unitLanguage, int unitNumber) {
    xpath = methodXpath;
    this->unitLanguage = unitLanguage;
    this->unitNumber = unitNumber;
    srcML = srcml_unit_get_srcml(methodUnit);
    findMethodName(methodArchive, methodUnit); 
};

void methodModel::findMethodData(const std::unordered_map<std::string, Variable>& attribute, 
                                 const std::unordered_set<std::string>& parentClassName, std::string className){
    srcml_archive* archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, srcML.c_str(), srcML.size());
    srcml_unit* unit = srcml_archive_read_unit(archive);

    findMethodReturnType(archive, unit, className); 
    findLocalVariableName(archive, unit);
    findLocalVariableType(archive, unit, className); 
    findParameterName(archive, unit);
    findParameterType(archive, unit, className);
    if (unitLanguage == "C++")
        isConst(archive, unit);

    for (const auto& c : parameterOrdered) 
        parameter.insert({c.getName(), c});

    for (const auto& c : localOrdered)
        local.insert({c.getName(), c});



    findReturnExpression(archive, unit);
    findCall(archive, unit, "function", true);
    findCall(archive, unit,"method", true);
    findCall(archive, unit, "constructor", true);

    isCallOnAttribute(attribute, "method", className, parentClassName);
    isCallOnAttribute(attribute, "function", className, parentClassName);
    isAttributeReturned(attribute, className, parentClassName);  
    isAttributeChanged(archive, unit, attribute, className, parentClassName);
    isAttributeUsed(archive, unit, attribute, className, parentClassName);
    isEmpty(archive, unit);

    srcml_unit_free(unit);
    srcml_archive_close(archive);
    srcml_archive_free(archive); 
}

// Gets the method name
//
void methodModel::findMethodName(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, "//src:function[not(ancestor::src:function)]/src:name");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

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

// Determines if method is const
//
void methodModel::isConst(srcml_archive* archive, srcml_unit* unit) {
    srcml_append_transform_xpath(archive, "//src:function/src:specifier[.='const']");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n == 1) constMethod = true;
    
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Gets the method return type 
//
void methodModel::findMethodReturnType(srcml_archive* archive, srcml_unit* unit, const std::string& className) {
    srcml_append_transform_xpath(archive, "//src:function[not(ancestor::src:function)]/src:type");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n > 0) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);
        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        returnType = unparsed;
        returnTypeSeparated = unparsed;
        removeSpecifiers(returnTypeSeparated, unitLanguage);
        removeContainers(returnTypeSeparated, unitLanguage);
        trimWhitespace(returnTypeSeparated);

        // Check if return type is non-primitive
        if (!nonPrimitiveReturnTypeExternal) {  
            if (!isPrimitiveType(returnTypeSeparated)){
                nonPrimitiveReturnType = true; 
                if (returnTypeSeparated.find(className) == std::string::npos)
                    nonPrimitiveReturnTypeExternal = true;
            }
        }
        free(unparsed);
    }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the names of local variables
//
void methodModel::findLocalVariableName(srcml_archive* archive, srcml_unit* unit) {
    std::string decl_stmt = "//src:decl_stmt[not(ancestor::src:catch)";
    decl_stmt += " and count(ancestor::src:function) = 1]";
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
        v.setName(localName);

        size_t arr = localName.find("[");
        if (arr != std::string::npos) {
            localName = localName.substr(0, arr);
            Rtrim(localName);
        } 
        v.setNameParsed(localName);

        localOrdered.push_back(v);
        
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the types of local variables
//
void methodModel::findLocalVariableType(srcml_archive* archive, srcml_unit* unit, const std::string& className) {
    std::string decl_stmt = "//src:decl_stmt[not(ancestor::src:catch) and not(ancestor::src:throw)";
    decl_stmt += " and count(ancestor::src:function) = 1]";
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

        // Check if method uses at least one non-primitive local
        if (!nonPrimitiveLocalExternal) {
            removeSpecifiers(type, unitLanguage);
            removeContainers(type, unitLanguage);
            trimWhitespace(type);
            localOrdered[i].setTypeParsed(type);
            if (!isPrimitiveType(type)){
                nonPrimitiveLocal = true; 
                localOrdered[i].setNonPrimitive(true);
                if (type.find(className) == std::string::npos) {
                    nonPrimitiveLocalExternal = true;   
                    localOrdered[i].setNonPrimitiveExternal(true);
                }                                  
            }
        }
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the names of parameters in each method
//
void methodModel::findParameterName(srcml_archive* archive, srcml_unit* unit) {
    std::string methodXpath = "//src:function[not(ancestor::src:function)]/src:parameter_list";
    methodXpath += "/src:parameter/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";

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
        std::string paraName = unparsed;

        Variable v;
        v.setName(paraName); 
    
        size_t arr = paraName.find("[");
        if (arr != std::string::npos) {
            paraName = paraName.substr(0, arr);
            Rtrim(paraName);
        }
        v.setNameParsed(paraName);

        parameterOrdered.push_back(v); 
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects the types of parameters
//
void methodModel::findParameterType(srcml_archive* archive, srcml_unit* unit, const std::string& className) {
    std::string methodXpath = "//src:function[not(ancestor::src:function)]/src:parameter_list";
    methodXpath += "/src:parameter/src:decl/src:type[following-sibling::*[1][self::src:name]]";
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

        removeSpecifiers(type, unitLanguage);
        trimWhitespace(type);

        // Check if method uses at least one non-primitive parameter
        if (!nonPrimitiveParamaterExternal){ 
            removeContainers(type, unitLanguage);
            parameterOrdered[i].setTypeParsed(type);
            if (!isPrimitiveType(type)){
                nonPrimitiveParamater = true;
                parameterOrdered[i].setNonPrimitive(true);
                if (type.find(className) == std::string::npos) {
                    nonPrimitiveParamaterExternal = true; 
                    parameterOrdered[i].setNonPrimitiveExternal(true);
                }                  
            }
        }
      
        free(unparsed);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Collects all return expressions
//
void methodModel::findReturnExpression(srcml_archive* archive, srcml_unit* unit) {
    std::string methodXpath = "//src:return[count(ancestor::src:function) = 1";
    methodXpath += " and not(ancestor::src:catch) and not(ancestor::src:throw)]/src:expr"; 

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
        
        returnExpression.push_back(expr);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Finds a list of calls that are not below throw or catch or in ignorableCalls
// does not include calls following new operator: when callType is method
// does not include calls following the . or -> or new operator: when callType is function
// calls that follow the new keyword are constructor calls and do not count as a method or function call
//
void methodModel::findCall(srcml_archive* archive, srcml_unit* unit, std::string callType, bool ignoreCalls) {
    std::unordered_set<std::string> ignorableCalls;
    if (unitLanguage == "C++")
        ignorableCalls = {"assert", "exit", "abort"};
    else if (unitLanguage == "C#")
        ignorableCalls = {"WriteLine", "Write", "ReadLine", "Read", "Trace", "Assert", "Exit"};
    else if (unitLanguage == "Java") 
        ignorableCalls = {"println", "print", "printf", "readLine", "read", "assert", "exit"};
    
    std::vector<std::string> calls;
    std::string methodXpath = "//src:call[not(ancestor::src:throw) and not(ancestor::src:catch)";
    methodXpath += " and count(ancestor::src:function) = 1";
    if (callType == "function") {    
        methodXpath += " and not(preceding-sibling::*[1][self::src:operator='new'])";
        methodXpath += " and not(src:name/src:operator='->') and not(src:name/src:operator='.')]";
    }
    else if (callType == "method") {
        methodXpath += " and not(preceding-sibling::*[1][self::src:operator='new'])";
        methodXpath += " and src:name/src:operator='->' or src:name/src:operator='.']";
    }  
    else if (callType == "constructor")
        methodXpath += " and preceding-sibling::*[1][self::src:operator='new']]";  

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
        std::string call = unparsed;
        free(unparsed);

        // Ignore calls listed in ignorableCalls.
        bool found = false;

        if (ignoreCalls && (ignorableCalls.find(call) != ignorableCalls.end()))
            found = true;
                                
        if (!found) {
            if (callType == "function")  
                functionCall.push_back(call);
            else if (callType == "method") 
                methodCall.push_back(call);
            else if (callType == "constructor")
                constructorCall.push_back(call);       
        }  
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

// Checks if an expression uses an attribute
// Possible cases: 
// C++: this->a; (*this).a; Foo::a; this->a.b; (*this).a.b; Foo::a.b; a.b; a
// Java: super.a; this.a; Foo.a; super.a.b; this.a.b; Foo.a.b; a.b; a
// C#: base.a; this.a; Foo.a; base.a.b; this.a.b; Foo.a.b; a.b; a
// Where 'a' is an attribute and Foo is either a parent class or current class
// Can match with complex uses of attributes (e.g., this->a.b.c)
//
bool methodModel::isAttribute(const std::unordered_map<std::string, Variable>& attribute, 
                                const std::string& expression, const std::string& className, 
                                const std::unordered_set<std::string>& parentClassName, bool call) {

    std::string pattern = "";
    if (call) {
        if (unitLanguage == "C++") 
            pattern = R"((?:\(\*this\)\.|this->|(\w+)(?:::|\.))(\w+)(?:\.|->\w+)?)";
        else if (unitLanguage == "Java")
            pattern = R"((?:super|this|(\w+))\.(\w+)(?:\.(?:\w+))?)";
        else if (unitLanguage == "C#")
            pattern = R"((?:base|this|(\w+))\.(\w+)(?:\.(?:\w+))?)";
    }
    else {
        // $ asserts the end of a line
        if (unitLanguage == "C++") 
            pattern = R"((?:\(\*this\)\.|this->|(\w+)(?:::|\.))(\w+)(?:\.|->\w+)?$)";
        else if (unitLanguage == "Java")
            pattern = R"((?:super|this|(\w+))\.(\w+)(?:\.|->\w+)?$)";
        else if (unitLanguage == "C#")
            pattern = R"((?:base|this|(\w+))\.(\w+)(?:\.|->\w+)?$)";
    }

    std::smatch match;
    const std::regex regexPattern(pattern);
    bool isMatched = std::regex_search(expression, match, regexPattern);

    bool inheritFromParent = false;
    bool attributeFound = false;

    std::string expr = expression;
    if (isMatched) { // Matches     
        expr = match[1]; // Case of class name or a parent class or the attribute itself
        if (match[1] != "") {
            expr.substr(0, expr.find("<"));
            trimWhitespace(expr);
            if (expr == className)
                expr = match[2];
            else {
                if (parentClassName.find(expr) != parentClassName.end()) {
                    inheritFromParent = true;   
                    expr = match[2];
                }             
            }
        }   
        else 
            expr = match[2]; // Case of base, super, and this  
    }

    // A local or a parameter might overshadow an attribute
    if (local.find(expr) != local.end()  || parameter.find(expr) != parameter.end())
        return false;

    // Check if 'a' is in the list of attributes
    else if (attribute.find(expr) != attribute.end()) {
        attributeUsed = true;
        attributeFound = true;
        if (attribute.at(expr).getNonPrimitive()) {
            nonPrimitiveAttribute = true;
            if (attribute.at(expr).getNonPrimitiveExternal()) 
                nonPrimitiveAttributeExternal = true;                  
        
        }              
    }
    else if (isMatched && inheritFromParent) { // Case where attribute is inherited.
        attributeUsed = true;
        attributeFound = true;
        // Assume the attribute is non-primitive and external
        nonPrimitiveAttribute = true; 
        nonPrimitiveAttributeExternal = true;
    }
    
    return attributeFound;
    
}

// For each function, find all return expressions and determine if they return an attribute
//
//  Assume "a" is an attribute. Some cases are:
//      return a;
//      return new int{3}; ignore
//      return new int[6]; ignore
//      return new int; ignore
//      return a+b; ignore
//      return **a;
//      return *a;
//      return this->a;
//      return (*this).a;
//      return a.b[4];
//      return a[2]; 
//      return a[2].b;
//      return a[1]->b[2].c;
//      return &a;
//      return 45; ignore
//      return base.a;
//      return this.a;
//      return super.a;
//      return Foo::a; Foo is class name or parent class name (C++)
//      return Foo.a; (C# and Java)
//
void methodModel::isAttributeReturned(const std::unordered_map<std::string, Variable>& attribute,
                                      const std::string& className, const std::unordered_set<std::string>& parentClassName) {
    for (size_t j = 0; j < returnExpression.size(); ++j) {
        std::string returnExpr = returnExpression[j];

        if (returnExpr.find("new ") != std::string::npos)
            newReturned = true;
        else {
            // Erase brackets only
            size_t openingBracket = returnExpr.find("["); 
            size_t closingBracket = returnExpr.find("]");
            while (openingBracket != std::string::npos && closingBracket != std::string::npos) {
                returnExpr.erase(openingBracket, closingBracket - openingBracket + 1); 
                openingBracket = returnExpr.find("[");
                closingBracket = returnExpr.find("]");
            }
            trimWhitespace(returnExpr);

            while (returnExpr[0] == '*' || returnExpr[0] == '&') 
              returnExpr.erase(0, 1);        

            bool result = isAttribute(attribute, returnExpr, className, parentClassName, false);
            if (result) attributeReturned = true;
            else {
                if (parameter.find(returnExpr) != parameter.end() ||
                    local.find(returnExpr) != local.end()) {
                    paraOrLocalReturned = true;
                }
            }
        }
    }
}

// Determines if a method is used in an expression
//
void methodModel::isAttributeUsed(srcml_archive* archive, srcml_unit* unit, const std::unordered_map<std::string, Variable>& attribute,
                                  const std::string& className, const std::unordered_set<std::string>& parentClassName)  {
    std::string methodXpath = "//src:expr[count(ancestor::src:function) = 1";
    methodXpath += " and not(ancestor::src:throw) and not(ancestor::src:catch)]/src:name";

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
        std::string possibleAttribute = unparsed;

        size_t arr = possibleAttribute.find("[");
        if (arr != std::string::npos) {
            possibleAttribute = possibleAttribute.substr(0, arr);
            Rtrim(possibleAttribute);
        }        
        
        isAttribute(attribute, possibleAttribute, className, parentClassName, false);

        free(unparsed);
        
    }        
    srcml_clear_transforms(archive);
    srcml_transform_free(result);    
}

// Determines if method is empty
//
void methodModel::isEmpty(srcml_archive* archive, srcml_unit* unit) {
    std::string methodXpath = "//src:function/src:block/src:block_content[*[not(self::src:comment)][1]";
    methodXpath += " and count(ancestor::src:function) = 1]";

    srcml_append_transform_xpath(archive, methodXpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    empty = (n==0); 

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
        Rtrim(result);

    return result;
}

// Finds if an attribute is modified or 
//  if a parameter pass by reference is changed
//
void methodModel::isAttributeChanged(srcml_archive* archive, srcml_unit* unit, 
                                    const std::unordered_map<std::string, Variable>& attribute,
                                    const std::string& className, const std::unordered_set<std::string>& parentClassName) { 
    int changed = 0;

    std::unordered_set<std::string> checked;

    std::string methodXpath = "//src:expr[count(ancestor::src:function) = 1]";
    methodXpath += "//src:name[not(ancestor::src:catch) and not(ancestor::src:throw)";
    methodXpath += " and following-sibling::*[1][self::src:operator='=' or self::src:operator='+='";
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

        size_t arr = possibleAttribute.find("[");
        if (arr != std::string::npos) {
            possibleAttribute = possibleAttribute.substr(0, arr);
            Rtrim(possibleAttribute);
        }     
        if (isAttribute(attribute, possibleAttribute, className, parentClassName, false) && checked.find(possibleAttribute) == checked.end()) {
            changed++;  
            checked.insert(possibleAttribute);
        }
      
        else if (parameter.find(possibleAttribute) != parameter.end()) {
            if (unitLanguage == "C++" || unitLanguage == "C#"){
                std::string type = parameter[possibleAttribute].getType();
                bool reference = type.find("&") != std::string::npos;
                bool referencePointer = type.find("*") != std::string::npos;      
                if (unitLanguage == "C++"){
                    std::string parName = parameter[possibleAttribute].getName();
                    trimWhitespace(parName);
                    bool referenceArray = parName.find("[]") != std::string::npos; 
                    bool constant = type.find("const") != std::string::npos;
                    if ((reference || referencePointer || referenceArray) && !constant)                    
                        parameterChanged = true;   
                }
                else if (unitLanguage == "C#"){
                    bool referenceOut = type.find("out") != std::string::npos ||
                                        type.find("ref") != std::string::npos;
                    trimWhitespace(type);
                    bool referenceArray = type.find("[]") != std::string::npos; 
                    bool nonPrimit = !parameter[possibleAttribute].getNonPrimitive();
                    if (reference || referencePointer || referenceArray || referenceOut || nonPrimit)                  
                        parameterChanged = true;     
                }       
            }
            else if (unitLanguage == "Java"){
                bool nonPrimit = !parameter[possibleAttribute].getNonPrimitive();
                if (nonPrimit)                  
                    parameterChanged = true;     
            }             
        }
    }
    numOfAttributeModified = changed;
    srcml_clear_transforms(archive);
    srcml_transform_free(result);  
}

// Checks if a method has a call on an attribute
// For example, a.b(c) where a and c are attributes
//
void methodModel::isCallOnAttribute(const std::unordered_map<std::string, Variable>& attribute, std::string callType,
                                    const std::string& className, const std::unordered_set<std::string>& parentClassName) {
    if (callType == "method"){
        for (size_t i = 0; i < methodCall.size(); ++i) {
            std::string callingObject = methodCall[i].substr(0, methodCall[i].find("("));
            if (isAttribute(attribute, callingObject, className, parentClassName, true)) 
                methodCallOnAttribute = true; 
            if (!methodCallOnAttribute) {
                if (isAttributeUsedAsArgument(attribute, methodCall[i], className, parentClassName)) 
                    methodCallOnAttribute = true; 
            }
        }
    }
    else if (callType == "function"){
        for (size_t i = 0; i < functionCall.size(); ++i) {
            if (isAttributeUsedAsArgument(attribute, functionCall[i], className, parentClassName)) 
                functionCallOnAttribute = true;
        }
    }
}

// Checks if a call uses an attribute as argument.
// For example, b(a) or b.d(a, c) where a and c are attributes
//
bool methodModel::isAttributeUsedAsArgument(const std::unordered_map<std::string, Variable>& attribute, std::string call,
                                            const std::string& className, const std::unordered_set<std::string>& parentClassName) {
    size_t argumentListOpen;
    size_t argumentListClose;
    std::string callingObject;

    argumentListOpen = call.find("(");
    argumentListClose = call.find(")");
    callingObject = call.substr(argumentListOpen+1, argumentListClose - (argumentListOpen + 1));
    
    trimWhitespace(callingObject);

    size_t start = 0;
    size_t end = callingObject.find(",");
    std::string possibleAttribute = "";
    while (end != std::string::npos) {
        possibleAttribute = callingObject.substr(start, end - start);
        if (isAttribute(attribute, possibleAttribute, className, parentClassName, true))
            return true;     
        start = end + 1;
        end = callingObject.find(",", start);
    }
    possibleAttribute = callingObject.substr(start, end - start);
    if (isAttribute(attribute, possibleAttribute, className, parentClassName, true))
        return true;  
        
    return false;
}
