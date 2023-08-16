// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file method.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "method.hpp"

methodModel::methodModel() : name(),  returnType(), parametersList(),
                             parametersNames(), parametersTypes(),
                             localVariablesNames(), localVariablesTypes(),
                             stereotype(), methodCalls(), functionCalls(),
                             constructorCalls(), method(), header(),
                             constMethod(false), isAttributeReturned(false), 
                             isAttributeUsed(false), isParamChanged(false), 
                             isEmpty(), attributesModified(0) {};

methodModel::methodModel(const std::string& xml, const std::string& s, bool f) : methodModel() {
    method = xml;
    header = s;
    constMethod = f;
};

void methodModel::setStereotype (const std::string& s) {
        stereotype.push_back(s);
}

std::string methodModel::getStereotype () const{
    std::string result = "";

    for (const std::string &value : stereotype)
        result += value + " ";

    if(result != "")
        return Rtrim(result);
        
    return result;
}

// For each function, find all return expressions and determine if they contain an attribute
// Return expressions collected are in the form of 'return a;' or 'return *a;' or 'return **a;' where a is an attribute
// Complex return expressions such as 'return a+b;' are ignored since they don't return an attribute, but a value calculated from the attribute(s) 
// Attribute is added to the list of attributes if it is inherited
//
bool methodModel::returnsAttribute(std::vector<AttributeInfo>& attribute, bool inherits) {
    std::vector<std::string> returnExpressions = findReturnExpressions(true);
    bool retAttribute = false;
    for (int j = 0; j < returnExpressions.size(); ++j) {
        std::string returnExpr = returnExpressions[j];
        if (returnExpr.find("**") == 0) returnExpr.erase(0, 2); // handles the case of '**a'
        if (returnExpr.find("*") == 0) returnExpr.erase(0, 1);  // handles the case of '*a'
        if (isAttribute(attribute, parametersNames, localVariablesNames, returnExpr)) {
            if(!retAttribute) retAttribute = true;
        }
        if (inherits)
            if (isInheritedAttribute(parametersNames, localVariablesNames, returnExpr)) {
                retAttribute = true;
                if (!isDuplicateAttribute(attribute, returnExpr)){}
                    attribute.push_back(AttributeInfo(returnExpr, returnType));
        }
    }   
    return retAttribute;
}

std::vector<std::string> methodModel::findReturnExpressions(bool getter) const {
    std::vector<std::string> returnExpressions;

    std::string xpath = "//src:return/src:expr[not(ancestor::src:catch)"; 
    if (getter) {
        xpath += " and (count(*)=1 and src:name) or (count(*)=2 and *[1][self::src:operator='*'] and *[2][self::src:name])";
        xpath += " or (count(*)=3 and *[1][self::src:operator='*'] and *[2][self::src:operator='*'] and *[3][self::src:name])]";
    }
    else {
        xpath += "]";
    }
    
    srcml_archive*          archive = nullptr;
    srcml_unit*             unit = nullptr;
    srcml_unit*             resultUnit = nullptr;
    srcml_transform_result* result = nullptr;

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive,method.c_str(), method.size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_results = srcml_transform_get_unit_size(result);

    for (int j = 0; j < number_of_results; ++j) {
        resultUnit = srcml_transform_get_unit(result, j);
        std::string expr = srcml_unit_get_srcml(resultUnit);
        if (expr.find("<expr><literal type=\"number\"") == 0) { // Used for inheritance
            expr = "#";
        } else {
            char *unparsed = nullptr;
            size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
            expr = unparsed;
            free(unparsed);
        }
        returnExpressions.push_back(expr);
    }
    srcml_unit_free(unit);
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);
    
    return returnExpressions;
}

std::vector<std::string> methodModel::findLocalVariables(const std::string& className, bool name) {
    std::vector<std::string> locals;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;
    std::string              decl  = ""; 


    std::string decl_stmt = "//src:decl_stmt[not(ancestor::src:catch)]";
    std::string control = "//src:control/src:init";
    if (name)
        decl = "/src:decl/src:name";
    else
        decl = "/src:decl/src:type";

    
    std::string xpath = decl_stmt + decl + " | " + control + decl;

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method.c_str(), method.size());
    unit = srcml_archive_read_unit(archive);

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    std::string prev = "";
    for (int j = 0; j < n; ++j) {
        srcml_unit* resultUnit = srcml_transform_get_unit(result, j);
        std::string nameOrType = srcml_unit_get_srcml(resultUnit);
  
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

        if (name){
            nameOrType = unparsed;
            size_t arr = nameOrType.find("[");
            if (arr != std::string::npos) {
                nameOrType = nameOrType.substr(0, arr);
                nameOrType = Rtrim(nameOrType);
            }
        }
        else {
            if (nameOrType == "<type ref=\"prev\"/>") {
                nameOrType = prev;           
            }
            else {  
                nameOrType = unparsed;
                prev = nameOrType;
            }
            
        }
        
        free(unparsed);
        locals.push_back(nameOrType);
    }
    srcml_unit_free(unit);
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return locals;
}

std::vector<std::string> methodModel::findParameters(const std::string& className, bool name) {
    std::vector<std::string> parameters;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;
    std::string              xpath = "";
    if (name)
        xpath = "//src:parameter_list/src:parameter/src:decl/src:name";
    else
        xpath = "//src:parameter_list/src:parameter/src:decl/src:type";
    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method.c_str(), method.size());
    unit = srcml_archive_read_unit(archive);

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);

        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

        std::string nameOrType = unparsed;
        free(unparsed);

        if (name){
            size_t arr = nameOrType.find("[");
            if (arr != std::string::npos) {
                nameOrType = nameOrType.substr(0, arr);
                nameOrType = Rtrim(nameOrType);
            }
        }

        parameters.push_back(nameOrType);

    }
    srcml_unit_free(unit);
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return parameters;
}

// Returns a list of call names that are not below throw or catch
// does not include calls following new operator: when callType is method
// does not include calls following the . or -> or new operators: when callType is function
// calls that follow the new keyword are constructor calls and do not count as a method or function call
//
std::vector<std::string> methodModel::findCalls(const std::string& callType, bool ignoreCalls) const {
    std::vector<std::string> ignorableCalls = {"assert"};
    std::vector<std::string> calls;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    
    std::string xpath = "//src:call[not(ancestor::src:throw) and not(ancestor::src:catch)";
    
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


    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method.c_str(), method.size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string call = unparsed;
        free(unparsed);

        call = trimWhitespace(call);

        // Ignore calls listed in ignorableCalls and primitive calls (int(), double()).
        if (ignoreCalls){
            bool found = false;
            for (std::string c : ignorableCalls){
                if (call.find(c) != std::string::npos){
                    found = true;
                    break;
                }               
            }
            if (!found && !isPrimitiveContainer(call)) {
                calls.push_back(call);
            }
        }
        else {
            calls.push_back(call);
        }

    }
    srcml_unit_free(unit);
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return calls;
}

int methodModel::findAssignOperatorAttribute(std::vector<AttributeInfo>& attribute, bool check_for_loop) {
    int changed = 0;

    for (int j = 0; j < ASSIGNMENT_OPERATOR.size(); ++j) {
        srcml_archive*           archive = nullptr;
        srcml_unit*              unit = nullptr;
        srcml_unit*              resultUnit = nullptr;
        srcml_transform_result*  result = nullptr;

        std::string xpath = "//src:operator[.='" + ASSIGNMENT_OPERATOR[j] + "'";
        xpath += " and not(ancestor::src:throw) and not(ancestor::src:catch)";
        if (check_for_loop) xpath += " and ancestor::src:for";
        xpath += "]/preceding-sibling::src:name";

        archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, method.c_str(), method.size());
        srcml_append_transform_xpath(archive, xpath.c_str());
        unit = srcml_archive_read_unit(archive);
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        for (int k = 0; k < n; ++k) {
            resultUnit = srcml_transform_get_unit(result, k);
            std::string name = srcml_unit_get_srcml(resultUnit);

            char * unparsed = nullptr;
            size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
            std::string var_name = unparsed;
            free(unparsed);
            
            if (isAttribute(attribute, parametersNames, localVariablesNames, var_name)) {
                 ++changed;
            } 
        }
        srcml_unit_free(unit);
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_archive_free(archive);
    }
    return changed;
}

int methodModel::findIncrementedAttribute(std::vector<AttributeInfo>& attribute, bool check_for_loop) {
    const std::vector<std::string> INC_OPS = {"++", "--"};
    const std::vector<std::string> LOCATION = {"following-sibling", "preceding-sibling"};
    int changed = 0;

    for (int j = 0; j < INC_OPS.size(); ++j) {          //for each operator (++ and --)
        for (int k = 0; k < LOCATION.size(); ++k) {     //check following and preceding
            srcml_archive*           archive = nullptr;
            srcml_unit*              unit = nullptr;
            srcml_unit*              resultUnit = nullptr;
            srcml_transform_result*  result = nullptr;

            std::string xpath = "//src:operator[.='";
            xpath += INC_OPS[j] + "' and not(ancestor::src:throw) and not(ancestor::src:catch)";
            if (check_for_loop) xpath += " and ancestor::src:for";
            xpath += "]/" + LOCATION[k] + "::src:name[1]";

            archive = srcml_archive_create();
            srcml_archive_read_open_memory(archive, method.c_str(), method.size());
            srcml_append_transform_xpath(archive, xpath.c_str());
            unit = srcml_archive_read_unit(archive);
            srcml_unit_apply_transforms(archive, unit, &result);
            int n = srcml_transform_get_unit_size(result);

            if (n != 0) {
                resultUnit = srcml_transform_get_unit(result, 0);
                std::string name = srcml_unit_get_srcml(resultUnit);
                char *unparsed = nullptr;
                size_t size = 0;
                srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
                std::string var_name = unparsed;
                free(unparsed);

                if (isAttribute(attribute, parametersNames, localVariablesNames, var_name)) {
                    ++changed;
                }                  
            }
            srcml_unit_free(unit);
            srcml_transform_free(result);
            srcml_clear_transforms(archive);
            srcml_archive_close(archive);
            srcml_archive_free(archive);
        }
    }
    return changed;
}

bool methodModel::variableChanged(const std::string& var_name) const {
    bool changed = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:name[.='" + var_name + "' and not(ancestor::src:throw) and not(ancestor::src:argument_list[@type='generic'])";
    xpath += " and not(ancestor::src:catch)]/following-sibling::*[1]";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method.c_str(), method.size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i) {
        resultUnit = srcml_transform_get_unit(result, i);
        std::string nextElement = srcml_unit_get_srcml(resultUnit);
        int equalSignCount = 0;
        for (int j = 0; j < nextElement.size(); ++j){
            if (nextElement[j] == '='){
                equalSignCount++;
            }
        }
        if (equalSignCount == 1){
            changed = true;
        }
    }
    srcml_unit_free(unit);
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return changed;
}

// Determines if a method is using an attribute
// Determines if a method is using a non-primitive attribute
// Attribute is added to the list of attributes if it is inherited
//     
bool methodModel::usesAttribute(std::vector<AttributeInfo>& attributes, bool inherit)  {
    bool found = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

   
    std::string xpath = "//src:expr//src:name[not(ancestor::src:throw) and not(ancestor::src:catch) and not(ancestor::src:argument_list[@type='generic'])";   
    xpath += " and preceding-sibling::*[2][self::src:name ='this'] or preceding-sibling::*[3][self::src:name ='this']";
    xpath += " or not(preceding-sibling::*[1][self::src:operator ='.']) and not(preceding-sibling::*[1][self::src:operator ='->'])";
    xpath += " and not(preceding-sibling::*[1][self::src:operator ='new']) and not(preceding::*[1][self::src:operator ='new'])";
    xpath += " and not(following-sibling::*[1][self::src:operator = '::']) and not(following::*[1][self::src:argument_list])";
    xpath += " and not(following::*[1][src:argument_list[@type='generic']]) and not(child::*[1][self::src:name])]";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method.c_str(), method.size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int j = 0; j < n; ++j) {
        resultUnit = srcml_transform_get_unit(result, j);
        std::string name = srcml_unit_get_srcml(resultUnit);
        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string possibleAttr = unparsed;
        free(unparsed);
 
        if (isAttribute(attributes, parametersNames, parametersTypes, possibleAttr)){
            if (!found){
                found = true;
                if(!inherit) break; // Break if it doesn't inherit since we are not adding new attributes to the list of attributes
            } 
            
        }        
        else if (inherit){
            if (isInheritedAttribute(parametersNames, localVariablesNames, possibleAttr)) {
                found = true;
                if (!isDuplicateAttribute(attributes, possibleAttr))
                    attributes.push_back(AttributeInfo(possibleAttr, "unknown"));   
            }   
        }
    
    }

    srcml_unit_free(unit);
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return found;
}
bool methodModel::usesNonPrimitiveAttributes(const std::vector<AttributeInfo>& nonPrimitiveAttributes, std::string className)  {
    bool found = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

   
    std::string xpath = "//src:expr//src:name[not(ancestor::src:throw) and not(ancestor::src:catch) and not(ancestor::src:argument_list[@type='generic'])";   
    xpath += " and preceding-sibling::*[2][self::src:name ='this'] or preceding-sibling::*[3][self::src:name ='this']";
    xpath += " or not(preceding-sibling::*[1][self::src:operator ='.']) and not(preceding-sibling::*[1][self::src:operator ='->'])";
    xpath += " and not(preceding-sibling::*[1][self::src:operator ='new']) and not(preceding::*[1][self::src:operator ='new'])";
    xpath += " and not(following-sibling::*[1][self::src:operator = '::']) and not(following::*[1][self::src:argument_list])";
    xpath += " and not(following::*[1][src:argument_list[@type='generic']]) and not(child::*[1][self::src:name])]";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method.c_str(), method.size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; i++) {
        resultUnit = srcml_transform_get_unit(result, i);
        std::string name = srcml_unit_get_srcml(resultUnit);
        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string possibleAttr = unparsed;
        free(unparsed);
 
        for (int j = 0; j < nonPrimitiveAttributes.size(); j++) {
            if (possibleAttr == nonPrimitiveAttributes[j].getName()) {
                bool isClassName = nonPrimitiveAttributes[j].getType().find(className) != std::string::npos;
                if (className == ""){ // Check for the use of non-primitive attributes that are not of the same class
                    if (!isClassName){
                        found = true;
                        break;
                    }
                }
                else { // Check for the use of non-primitive attributes that are of the same class
                    if (isClassName){
                        found = true;
                        break;
                    }
                }
            }
        }
        
    }

    srcml_unit_free(unit);
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return found;
}
bool methodModel::callsOnAttributes(std::vector<AttributeInfo>& attribute, std::string callType) {
    if (callType == "method"){
        for (int i = 0; i < methodCalls.size(); ++i) {
            size_t dot = methodCalls[i].find(".");
            size_t arrow = methodCalls[i].find("->");
            
            if (dot != std::string::npos) {
                std::string callingObject = methodCalls[i].substr(0, dot);
                if (isAttribute(attribute, parametersNames,  localVariablesNames, callingObject)) {
                    return true;
                } 
            }
            if (arrow != std::string::npos) {
                std::string callingObject = methodCalls[i].substr(0, arrow);
                if (isAttribute(attribute, parametersNames,  localVariablesNames, callingObject)) {
                    return true;
                } 
            }
            if (callsOnArguments(attribute, callType, i)) return true;
        }
    }
    if (callType == "function"){
        for (int i = 0; i < functionCalls.size(); ++i) {
            if (callsOnArguments(attribute, callType, i)) return true;
        }
    }
    return false;
}

bool methodModel::callsOnArguments(std::vector<AttributeInfo>& attribute, std::string callType, int i) {
    size_t argumentListOpen;
    size_t argumentListClose;
    std::string callingObject;

    if (callType == "method"){
        argumentListOpen = methodCalls[i].find("(");
        argumentListClose = methodCalls[i].find(")");
        callingObject = methodCalls[i].substr(argumentListOpen+1, argumentListClose - (argumentListOpen + 1));
    }
    else if (callType == "function"){
        argumentListOpen = functionCalls[i].find("(");
        argumentListClose = functionCalls[i].find(")");
        callingObject = functionCalls[i].substr(argumentListOpen+1, argumentListClose - (argumentListOpen + 1));
    }
    callingObject = trimWhitespace(callingObject);

    size_t start = 0;
    size_t end = callingObject.find(",");
    while (end != std::string::npos) {
        if (isAttribute(attribute, parametersNames,  localVariablesNames, callingObject.substr(start,end - start))){
            return true;
        }
        start = end + 1;
        end = callingObject.find(",", start);
    }
    if (isAttribute(attribute, parametersNames,  localVariablesNames, callingObject.substr(start,end - start))){
            return true;
    } 
    return false;
}

bool methodModel::isEmptyMethod() {
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:block_content[*[not(self::src:comment)]]";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method.c_str(), method.size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit_free(unit);
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return (n==0);
}


// Output a method for checking and testing
std::ostream& operator<<(std::ostream& out, const methodModel& m) {
    out << "Method: " << trimWhitespace(m.name) << " " << m.getConst();
    out << " returns: " << WStoBlank(m.getReturnType()) << std::endl;
    out << "   Stereotype: " << m.getStereotype() << std::endl;
    out << "   Parameters: ";
    for (int i=0; i< m.parametersNames.size(); ++i) {
        out << "(" << m.parametersTypes[i] << " : " << m.parametersNames[i] << ") ";
    }
    out << std::endl;
    out << "   Local Variables: ";
    for (int i=0; i< m.localVariablesNames.size(); ++i) {
        out << m.localVariablesNames[i] << "  ";
    }
    out << std::endl;
    return out;
}

