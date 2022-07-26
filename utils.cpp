//
//  utils.cpp
//  
//
//  Created by jmaletic on 7/6/22.
//

#include "utils.hpp"


// Checks if method is const
bool checkConst(const std::string& srcml) {
    std::string function_srcml = trimWhitespace(srcml);
    size_t end = function_srcml.find("{");
    std::string function_srcml_header = function_srcml.substr(0, end);
    if (function_srcml_header.find("<specifier>const</specifier><block>") != std::string::npos){
        return true;
    } else if (function_srcml_header.find("</parameter_list><specifier>const</specifier>") != std::string::npos){
        return true;
    } else {
        return false;
    }
}

//
// Checks if a name could be an inheritied attribute
//
// REQUIRES: parentClass.size() > 0 - the class inherits from another class
//
bool isInheritedAttribute(const std::vector<std::string>& parameter_names,
                                      const std::vector<std::string>& local_var_names,
                                      const std::string& expr){
    bool is_inherited = true;
    // checks for literal return expression
    if (expr == "#") is_inherited = false;
    if (expr == "true" || expr == "false" || expr == "TRUE" || expr == "FALSE") is_inherited = false;
    // expr is a keyword that is not a attribute
    if (expr == "this" || expr == "cout" || expr == "endl") is_inherited = false;

    for (int k = 0; k < parameter_names.size(); ++k) { // expr is not inherited if it is a parameter name
        if(expr == parameter_names[k]) is_inherited = false;
    }

    for (int k = 0; k < local_var_names.size(); ++k) { // expr is not inherited if it is a local variable
        if (expr == local_var_names[k]) is_inherited = false;
    }

    for(int k = 0; k < expr.size(); ++k) {  // expr is not inherited if it contains an operator
        if (expr[k] == '+' || expr[k] == '-' || expr[k] == '*' || expr[k] == '/'
            || expr[k] == '%' || expr[k] == '(' || expr[k] == '!' || expr[k] == '&'
            || expr[k] == '|' || expr[k] == '=' || expr[k] == '>' || expr[k] == '<'
            || expr[k] == '.' || expr[k] == '?' || expr[k] == ':' || expr[k] == '"'){
            is_inherited = false;
        }
    }

    // expr is all uppercase letters
    // assumed to be global variable
    //if (upper_case(expr)) is_inherited = false;

    return is_inherited;
}


// dont count calls if
// there is a . or -> somewhere in the name
// or call is static and class name is the same
//
int countPureCalls(const std::vector<std::string>& all_calls)  {
    int result = all_calls.size();
    for (int i = 0; i < all_calls.size(); ++i){
        size_t colon = all_calls[i].find(":");
        size_t dot   = all_calls[i].find(".");
        size_t arrow = all_calls[i].find("->");
        if (dot != std::string::npos || arrow != std::string::npos) {
            --result;
        }
        else if (colon != std::string::npos) {
            std::string name = all_calls[i].substr(0, colon);
            --result;
        }
    }
    return result;
}

//
// Checks if a primitive type
//
bool isPrimitiveContainer(std::string return_type){
    return_type = separateTypeName(return_type); // trim whitespace, specifiers and modifiers

    // if the type is a vector or list, check if the element type is primitive
    if(return_type.find("vector") != std::string::npos || return_type.find("list") != std::string::npos){
        size_t start = return_type.find("<") + 1;
        size_t end = return_type.find(">");
        return_type = return_type.substr(start, end - start);
        // in the case of vector<vector<x>>
        if (return_type.find("vector") != std::string::npos || return_type.find("list") != std::string::npos){
            size_t start = return_type.find("<") + 1;
            return_type = return_type.substr(start);
        }
    }

    // if the type is a map check if the key and value are both primivite
    // assumes never get map<map<x,y>,z>
    if (return_type.find("map") != std::string::npos){
        size_t start = return_type.find("<") + 1;
        size_t split = return_type.find(",");
        size_t end = return_type.find(">");
        std::string key = return_type.substr(start, split-start);
        std::string value = return_type.substr(split + 1, end - split - 1);
        return(PRIMITIVES.isPrimitive(key) && PRIMITIVES.isPrimitive(value));
    }
    // else check if primitive(NOT container).
    return PRIMITIVES.isPrimitive(return_type);

}





//Removes all whitespace from string (' ', /t, /n)
std::string trimWhitespace(const std::string& s) {
    std::string result(s);
    result.erase(std::remove_if(result.begin(),
                                result.end(),
                                [](char c) { return (c == ' ' || c == '\t' || c == '\n' || c == '\r'); }),
                 result.end());
    return result;
}



// Replaces tabs and LR to a space
std::string LRtoSpace(const std::string& s) {
    std::string result(s);
    std::replace_if(result.begin(),
                    result.end(),
                    [](char c) { return c == '\t' || c == '\n'; },
                    ' ');
    return result;
}

//
// Removes specifiers, *, & from type name
//
std::string separateTypeName(const std::string& type){
    std::string result = trimWhitespace(type);
    size_t stat = result.find("static");
    if (stat != std::string::npos){
        result.erase(stat, 6);
    }
    size_t mut = result.find("mutable");
    if (mut != std::string::npos){
        result.erase(mut, 7);
    }
    size_t in = result.find("inline");
    if(in != std::string::npos){
        result.erase(in, 6);
    }
    size_t virt = result.find("virtual");
    if (virt != std::string::npos){
        result.erase(virt, 7);
    }

    size_t star = result.find("*");
    if (star != std::string::npos){
        result.erase(star, 1);
    }
    size_t amp = result.find("&");
    if (amp != std::string::npos){
        result.erase(amp, 1);
    }

    size_t con = result.find("const");
    if (con != std::string::npos){
        result.erase(con, 5);
    }
    return result;
}

