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

