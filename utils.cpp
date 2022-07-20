//
//  utils.cpp
//  
//
//  Created by jmaletic on 7/6/22.
//

#include "utils.hpp"


// Checks if method is const
bool checkConst(std::string function_srcml) {
    trimWhitespace(function_srcml);
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
// TODO: Should return a string with no side effect. But this is efficent
//
void trimWhitespace(std::string& str) {
    str.erase(std::remove_if(str.begin(),
                             str.end(),
                             [](char c) { return (c == ' ' || c == '\t' || c == '\n'); }),
              str.end());
}

//
//
std::string separateTypeName(const std::string& type){
    std::string name = type;
    trimWhitespace(name);
    size_t stat = name.find("static");
    if (stat != std::string::npos){
        name.erase(stat, 6);
    }
    size_t mut = name.find("mutable");
    if (mut != std::string::npos){
        name.erase(mut, 7);
    }
    size_t in = name.find("inline");
    if(in != std::string::npos){
        name.erase(in, 6);
    }
    size_t virt = name.find("virtual");
    if (virt != std::string::npos){
        name.erase(virt, 7);
    }

    size_t star = name.find("*");
    if (star != std::string::npos){
        name.erase(star, 1);
    }
    size_t amp = name.find("&");
    if (amp != std::string::npos){
        name.erase(amp, 1);
    }

    size_t con = name.find("const");
    if (con != std::string::npos){
        name.erase(con, 5);
    }
    return name;
}

