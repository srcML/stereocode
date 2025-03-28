// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file utils.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "utils.hpp"

extern primitiveTypes                        PRIMITIVES;   
extern std::vector<std::string>              LANGUAGE;
extern typeModifiers                         TYPE_MODIFIERS;  

bool isNonPrimitiveType(const std::string& type, variable& var, 
                        const std::string& unitLanguage, const std::string& structureName) {
    std::string typeParsed = type;

    std::size_t listOpen = typeParsed.find("<");
    if (listOpen != std::string::npos) {
        std::string typeLeft = typeParsed.substr(0, listOpen);
        std::string typeRight = typeParsed.substr(listOpen, typeParsed.size() - listOpen);
        // removeNamespace() can mess up the string when there is a namespace inside <>
        // For example: Factory <hippodraw::DataRep> --> removeNamespace() --> DataRep>
        // This is why we need to separate them
        //
        removeNamespace(typeLeft, true, unitLanguage); // No generics and no comma separated list
        typeParsed = typeLeft + typeRight;
    }

    removeTypeModifiers(typeParsed, unitLanguage); // Can take full type as is
    trimWhitespace(typeParsed);  // Can take full type as is
    
    bool isNonPrimitive = false; 
    std::size_t start = 0;
    std::size_t end = typeParsed.find(",");
    std::string subType;
    while (end != std::string::npos) {
        subType = typeParsed.substr(start, end - start);   
        removeNamespace(subType, true, unitLanguage); 
        if (!isPrimitiveType(subType, unitLanguage)) {
            isNonPrimitive = true;
            if (subType != structureName)
                var.setNonPrimitiveExternal(true);
        }
        
        start = end + 1;
        end = typeParsed.find(",", start);
    }

    subType = typeParsed.substr(start, typeParsed.size() - start);
    removeNamespace(subType, true, unitLanguage);
    if (!isPrimitiveType(subType, unitLanguage)) {
        isNonPrimitive = true;
        if (subType != structureName)
            var.setNonPrimitiveExternal(true);
    }

    return isNonPrimitive;
}

// Checks if a type is primitive.  
//
bool isPrimitiveType(const std::string& type, const std::string& unitLanguage) {
    std::istringstream subType(type);
    std::string token;
    while (std::getline(subType, token, ','))
        if (!PRIMITIVES.isPrimitive(token, unitLanguage)) return false;
    return true;
}

bool matchSubstring(const std::string& text, const std::string& substring) {
    const std::string pattern = "\\b" + substring + "\\b"; 
    const std::regex regexPattern(pattern);
    std::smatch match;
    return std::regex_search(text, match, regexPattern);
}


// Removes specifiers from type name
//
void removeTypeModifiers(std::string& type, std::string unitLanguage) {
    std::regex regexPattern(TYPE_MODIFIERS.getTypeModifiers(unitLanguage));
    type = std::regex_replace(type, regexPattern, " ");
}

// Removes all whitespace from string
//
void trimWhitespace(std::string& s) {
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }), s.end());
}

// Trim blanks of the right of string
//
void Rtrim(std::string& s) {
    std::size_t lastNonSpace = s.find_last_not_of(' ');
    if (lastNonSpace != std::string::npos)
        s = s.substr(0, lastNonSpace + 1);   
}

// Removes namespaces from names
// all = false keeps the last :: or .
//
void removeNamespace(std::string& name, bool all, std::string_view unitLanguage) {
    std::size_t last, secondLast;
    if (unitLanguage == "C++") last = name.rfind("::");
    else last = name.rfind(".");
    if (last != std::string::npos) {
        if (all) {
            if (unitLanguage == "C++") name = name.substr(last + 2);
            else name = name.substr(last + 1);
        }
        else {
            if (unitLanguage == "C++") secondLast = name.rfind("::", last - 1);
            else secondLast = name.rfind(".", last - 1);
            if (secondLast != std::string::npos) {
                if (unitLanguage == "C++") name = name.substr(secondLast + 2); 
                else name = name.substr(secondLast + 1); 
            }                      
        }
    }
}

// Converts all whitespaces to blanks  ('\r' => ' ')
void WStoBlank(std::string& s) {
    std::replace_if(s.begin(),
                    s.end(),
                    [](char c) { return isspace(c); },
                    ' ');
}

// Removes all characters inside <> or () except for comma
// For example, myObject<int, std::pair<int, int>> becomes myObject<,>
//  and Foo(int, std::pair<int, int>, double) becomes Foo(,,)
//
void removeBetweenComma(std::string& s, bool isGeneric) {
    std::size_t opening;
    if (isGeneric)
      opening = s.find("<");
    else
      opening = s.find("(");

    if (opening != std::string::npos) {
        std::string name = s.substr(0, opening + 1);
        s = s.substr(opening + 1);
        
        // This could be nested inside () or <> for types
        // <[^>]*> --> starts at <, then matches everything except > and stops at > including the >
        std::string pattern = R"(<[^>]*>)";  
        std::regex regexPattern(pattern);
        s = std::regex_replace(s, regexPattern, "");  
        
        pattern = "";
        if (isGeneric)
          pattern = R"(([^,]*)(,|>))";
        else
          pattern = R"(([^,]*)(,|\)))";
        
        const std::regex regexPatternTwo (pattern);
        s = std::regex_replace(s, regexPatternTwo, "$2"); // $2 is used to replace the content with the second group
        s = name + s;
    }
}

// Workaround to get Stereocode to work with srcML 1.0.0
// It works by removing the item= attribute, which has an issue in srcML 1.0.0
//
void srcmlBackwardCompatibility(std::string& xmlText) {
    const std::vector<std::string> tags = {"><property", "><constructor", "><destructor", "><function"};

    std::size_t pos = std::string::npos;
    for (const auto& tag : tags) {
        pos = xmlText.find(tag);
        if (pos != std::string::npos) {
            break;
        }
    }

    if (pos != std::string::npos) { 
        std::string beforeFunction = xmlText.substr(0, pos);
        std::string afterFunction = xmlText.substr(pos);
        std::size_t item = beforeFunction.find("item=");
        if (item != std::string::npos) {}
            beforeFunction = beforeFunction.substr(0, item);

        beforeFunction.pop_back();
        xmlText = beforeFunction + afterFunction;
    }
}