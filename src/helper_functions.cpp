// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file helper_functions.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#include "helper_functions.hpp"
#include "primitives.hpp"
#include "specifiers.hpp"

#include <regex>
#include <cctype>

thread_local extern primitives PRIMITIVES;   
thread_local extern specifiers SPECIFIERS;  

// Helper to escape CSV fields for CSV parsers
// Outer quotes handle commas, newlines, carriage returns, etc.
// Inner quotes handle inner quotes
//
std::string helperFunctions::escapeCSV(const std::string& data) {
    std::string result = "\""; // Outer
    for (char c : data) {
        if (c == '\"') result += "\"\""; // Inner (Original + extra for escaping)
        else result += c;
    }
    result += "\""; // Outer
    return result;
}

// This function checks whether a given 'substring' appears in the beginning of 'text' as a whole word
// For example:
//   text = "hello, there" and substring = "hello" will match
//   text = "hello, there" and substring = "there" will not match
//   text = "hellothere" and substring = "hello" will not match
// A word boundary character is a character that is not from these [A-Z, a-z, 0-9,  _]
//
bool helperFunctions::isSubstringAtBeginning(const std::string& text, const std::string& substring) {
    // 1. Fail fast if text is shorter than substring
    if (text.size() < substring.size()) return false;

    // 2. Check prefix match (0 = match)
    if (text.compare(0, substring.size(), substring) != 0) return false;

    // 3. Check Word Boundary
    // We check the char *after* the substring
    // If text == substring, this hits the null terminator '\0', which is safe and passes the check
    char nextChar = text[substring.size()];
    return !isalnum(nextChar) && nextChar != '_';
}

// Function that removes everything starting at '[' and then trims right whitespace
//
void helperFunctions::removeBracketSuffix(std::string& text) {
    std::size_t startPosition = text.find("[");
    if (startPosition != std::string::npos) {
        text = text.substr(0, startPosition);
        removeWhitespace(text);
    }
}

// Function that removes the leading asterisks
//
void helperFunctions::removeLeadingAsterisks(std::string& text) {
    while (!text.empty() && text.front() == '*') 
        text.erase(0, 1);   
}


// Removes namespaces by finding the last :: or . and removing everything after it
// if 'removeAll = false', then it keeps the last :: or .
//
void helperFunctions::removeNamespace(std::string& name, std::string_view unitLanguage, bool removeAll) {
    std::size_t last, secondLast;
    if (unitLanguage == "C++") last = name.rfind("::");
    else last = name.rfind(".");
    if (last != std::string::npos) {
        if (removeAll) {
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

// Removes all characters inside <> or () except for comma
// For example, myObject<int, std::pair<int, int>> becomes myObject<,>
//  and Foo(int, std::pair<int, int>, double) becomes Foo(,,)
//
void helperFunctions::removeBetweenComma(std::string& s, bool isGeneric) {
    std::size_t opening;
    if (isGeneric) opening = s.find("<");
    else opening = s.find("(");

    if (opening != std::string::npos) {
        std::string name = s.substr(0, opening + 1);
        s = s.substr(opening + 1);
        
        // This could be nested inside () or <> for types
        // <[^>]*> --> starts at <, then matches everything except > and stops at > including the >
        static const std::regex nestedPattern(R"(<[^>]*>)");
        s = std::regex_replace(s, nestedPattern, "");  
        
        // The comma splitters
        static const std::regex genericSplit(R"(([^,]*)(,|>))");
        static const std::regex functionSplit(R"(([^,]*)(,|\)))");

        // Select and Apply
        const std::regex& currentPattern = isGeneric ? genericSplit : functionSplit;
        s = std::regex_replace(s, currentPattern, "$2");  // $2 is used to replace the content with the second group

        s = name + s;
    }
}

// Removes all whitespace from string
//
void helperFunctions::removeWhitespace(std::string& s) {
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }), s.end());
}
