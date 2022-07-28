//
//  utils.cpp
//  
//
//  Created by jmaletic on 7/6/22.
//

#include "utils.hpp"

// Checks if name is global const format
// Example: GLOBAL_FROMAT - upper case with "_"
//
bool isGlobalConstFormat(const std::string& name) {
    bool result = true;
    for (int k = 0; k < name.size(); ++k) {
        if (!isupper(name[k]) && name[k] != '_') result = false;
    }
    return result;
}


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

// Checks if a name could be an inheritied attribute
//
// REQUIRES: parentClass.size() > 0 - the class inherits from another class
//
// TODO: Should we check for global const?  - AxisModelLinear.cxx bool FLT_EQUAL( double x, double y ) has a global.
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

    for (int k = 0; k < expr.size(); ++k) {  // expr is not inherited if it contains an operator
        if (expr[k] == '+' || expr[k] == '-' || expr[k] == '*' || expr[k] == '/'
            || expr[k] == '%' || expr[k] == '(' || expr[k] == '!' || expr[k] == '&'
            || expr[k] == '|' || expr[k] == '=' || expr[k] == '>' || expr[k] == '<'
            || expr[k] == '.' || expr[k] == '?' || expr[k] == ':' || expr[k] == '"'){
            is_inherited = false;
        }
    }

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
//  Examples: int, bool, char, double, cont int, inline int
//            vector<int>, map<int,int>, int[], int*, int&
bool isPrimitiveContainer(const std::string& str) {
    std::string s = removeSpecifiers(str);
    s = WStoBlank(s);
    //Remove std::vector, list, map, std::vector<list<x>>  std::map<int, int>
    size_t pos = 0;
    while ((pos = s.find("std::vector")) != std::string::npos) s.erase(pos, 11);
    while ((pos = s.find("std::list")) != std::string::npos) s.erase(pos, 9);
    while ((pos = s.find("std::set")) != std::string::npos) s.erase(pos, 8);
    while ((pos = s.find("std::map")) != std::string::npos) s.erase(pos, 8);
    while ((pos = s.find("vector")) != std::string::npos) s.erase(pos, 6);
    while ((pos = s.find("list")) != std::string::npos) s.erase(pos, 4);
    while ((pos = s.find("set")) != std::string::npos) s.erase(pos, 3);
    while ((pos = s.find("map")) != std::string::npos) s.erase(pos, 3);
    //Replace < > , with space
    std::replace_if(s.begin(), s.end(), [](char c) { return c == ',' || c == '<' || c == '>'; }, ' ');
    s = multiBlanksToBlank(Rtrim(Ltrim(s))); //Make one space between each name
    if (s.find(" ") != std::string::npos) {  //Multiple type names
        size_t start = 0;
        size_t end = s.find(" ");
        while (end != std::string::npos) {
            if (!PRIMITIVES.isPrimitive(s.substr(start, end - start))) return false;
            start = end + 1;
            end = s.find(" ", start);
        }
        return PRIMITIVES.isPrimitive(s.substr(start, end - start));
    }
    return PRIMITIVES.isPrimitive(s);  //One type name
}



//
// Removes WS, specifiers, *, & from type name
//
std::string separateTypeName(const std::string& type){
    std::string result = trimWhitespace(type);
    result = removeSpecifiers(result);

    return result;
}

//
// Removes specifiers, *, & from type name
//
std::string removeSpecifiers(const std::string& type) {
    std::string result = type;
    size_t pos = 0;
    pos = result.find("static");
    if (pos != std::string::npos) result.erase(pos, 6);
    pos = result.find("mutable");
    if (pos != std::string::npos) result.erase(pos, 7);
    pos = result.find("inline");
    if (pos != std::string::npos) result.erase(pos, 6);
    pos = result.find("virtual");
    if (pos != std::string::npos) result.erase(pos, 7);
    pos = result.find("const");
    if (pos != std::string::npos) result.erase(pos, 5);
    pos = result.find("friend");
    if (pos != std::string::npos) result.erase(pos, 6);
    pos = result.find("*");
    if (pos != std::string::npos) result.erase(pos, 1);
    pos = result.find("&&");
    if (pos != std::string::npos) result.erase(pos, 2);
    pos = result.find("&");
    if (pos != std::string::npos) result.erase(pos, 1);
    pos = result.find("[]");
    if (pos != std::string::npos) result.erase(pos, 2);

    return result;
}


//Removes all whitespace from string (' ', /t, /n)
std::string trimWhitespace(const std::string& s) {
    std::string result(s);
    result.erase(std::remove_if(result.begin(),
                                result.end(),
                                [](char c) { return isspace(c); } ),
                 result.end());
    return result;
}

//Trim blanks off the left of string
std::string Ltrim(const std::string& s) {
    std::string result(s);
    while (result[0] == ' ') result.erase(0, 1);
    return result;
}

//Trim blanks of the right of string
std::string Rtrim(const std::string& s) {
    std::string result(s);
    while (result[result.size()-1] == ' ') result.erase(result.size()-1, 1);
    return result;
}

//Normalize multiple blanks to one blank
std::string multiBlanksToBlank(const std::string& s) {
    std::string result(s);
    size_t pos = 0;
    while ((pos = result.find("  ")) != std::string::npos) result.erase(pos, 1);
    return result;
}

// Converts all whitespace to blanks  ('\r' => ' ')
std::string WStoBlank(const std::string& s) {
    std::string result(s);
    std::replace_if(result.begin(),
                    result.end(),
                    [](char c) { return isspace(c); },
                    ' ');
    return result;
}

