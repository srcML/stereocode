// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file helper_functions.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#include "helper_functions.hpp"
#include "qualified_name.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

// Helper to escape CSV fields for CSV parsers
// Outer quotes handle commas, newlines, carriage returns, etc.
// Inner quotes handle inner quotes
//
std::string HELPERS::escapeCSV(const std::string& data) {
    std::string result = "\""; // Outer
    bool lastWasSpace = false;

    for (char c : data) {
        if (c == '\"') {
            result += "\"\""; // Escape quotes
            lastWasSpace = false;
        } 
        // std::isspace catches spaces, tabs, and newlines (\n, \r)
        else if (std::isspace(static_cast<unsigned char>(c))) {
            if (!lastWasSpace) {
                result += " "; // Replace with a single space
                lastWasSpace = true;
            }
        } 
        else {
            result += c;
            lastWasSpace = false;
        }
    }
    
    result += "\""; // Outer
    return result;
}

namespace {
    // Tells whether an element with a name starts at a position of a srcML text
    //
    bool startsElement(const std::string& text, std::size_t position, const std::string& name) {
        const std::size_t end = position + 1 + name.size();
        return text.compare(position, name.size() + 1, "<" + name) == 0 && end < text.size() && (text[end] == ' ' || text[end] == '>');
    }

    // Gets the position after the attributes or annotations that the element at the start of a srcML text holds before its other children
    // The elements of the same name inside an attribute or an annotation, such as the annotation of @A(@B), are counted to find its end
    //
    std::size_t afterLeadingAttributes(const std::string& text) {
        std::size_t position = text.find('>');
        if (position == std::string::npos) return 0;

        for (++position;;) {
            while (position < text.size() && std::isspace(static_cast<unsigned char>(text[position]))) ++position;

            std::string name;
            for (const char* candidate : {"attribute", "annotation"})
                if (startsElement(text, position, candidate)) name = candidate;
            if (name.empty()) return position;

            const std::string endTag = "</" + name + ">";
            int depth = 0;
            do {
                if (startsElement(text, position, name)) {
                    ++depth;
                    position += name.size() + 1;
                }
                else if (text.compare(position, endTag.size(), endTag) == 0) {
                    --depth;
                    position += endTag.size();
                }
                else if (++position >= text.size()) {
                    return text.size();
                }
            } while (depth > 0);
        }
    }
}

// Extracts the line number from a string"
//
int HELPERS::extractFirstLineNumber(const std::string& text) {
    size_t searchFrom = 0;

    // Find where the method/constructor body begins
    size_t blockPos = text.find("<block");
    if (blockPos == std::string::npos) {
        blockPos = text.length(); // Safe fallback if no block is found
    }

    // Jump to the actual function signature to bypass preceding attributes (if any)
    // Visual Studio reports like numbers at the function level. srcML does it at the attribute level. We want to match Visual Studio's behavior for our analysis
    // We look for "<type " ONLY before the <block> starts
    // This prevents constructors from grabbing <type> tags from local variables inside the body
    size_t typePos = text.find("<type ");
    
    if (typePos != std::string::npos && typePos < blockPos) {
        searchFrom = typePos;
    }
    // A constructor has no type, so its line is the line after its attributes
    else {
        searchFrom = afterLeadingAttributes(text);
    }

    std::string target = "pos:start=\"";
    size_t startPos = text.find(target, searchFrom);

    if (startPos != std::string::npos) {
        // Move the index forward to where the actual numbers begin
        startPos += target.length();

        // Find the colon ':' that separates the line and column numbers
        size_t endPos = text.find(':', startPos);

        if (endPos != std::string::npos) {
            // Extract the substring between the quote and the colon
            std::string numberStr = text.substr(startPos, endPos - startPos);
            return std::stoi(numberStr);
        }
    }
    
    return -1; // Return -1 if the target wasn't found
}

// This function checks whether a given 'substring' appears in the beginning of 'text' as a whole word
// For example:
//   text = "hello, there" and substring = "hello" will match
//   text = "hello, there" and substring = "there" will not match
//   text = "hellothere" and substring = "hello" will not match
// A word boundary character is a character that is not from these [A-Z, a-z, 0-9,  _]
//
bool HELPERS::isSubstringAtBeginning(const std::string& text, const std::string& substring) {
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
void HELPERS::removeBracketSuffix(std::string& text) {
    std::size_t startPosition = text.find("[");
    if (startPosition != std::string::npos) {
        text = text.substr(0, startPosition);
        removeWhitespace(text);
    }
}

// Removes namespaces by finding the last :: or . and removing everything after it
// if 'removeAll = false', then it keeps the last :: or .
//
void HELPERS::removeNamespace(std::string& name, std::string_view unitLanguage, bool removeAll) {
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

// Removes all whitespace from string
//
void HELPERS::removeWhitespace(std::string& s) {
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }), s.end());
}

// Removes whitespaces, commas, and namespaces to get the core name of a type or a function for matching purposes
// Each generic argument list is reduced to its commas, or removed, before the qualifier ends at the last :: or .,
// so that a member of a class template, such as Box<T>::reset, keeps its own name, and operator< keeps its symbol
//
void HELPERS::nameFilter(std::string& name, std::vector<std::string>& nameVector,  std::string_view unitLanguage, bool removeNamespace) {
    nameVector.push_back(name); 

    HELPERS::removeWhitespace(name);
    nameVector.push_back(name);

    std::string skeleton = genericSkeleton(name);
    std::string bare     = withoutGenericArguments(name);
    if (removeNamespace) {
        HELPERS::removeNamespace(skeleton, unitLanguage, true);
        HELPERS::removeNamespace(bare, unitLanguage, true);
    }
    nameVector.push_back(std::move(skeleton));
    nameVector.push_back(std::move(bare));
}

// Gets the structure of a type, such as class or struct, from the text that its element holds besides its children
// The text can hold the semicolon after a type body in any language, and a type without a keyword is a class
//
std::string HELPERS::typeStructure(std::string text) {
    text.erase(std::remove_if(text.begin(), text.end(), [](unsigned char c) { return std::isspace(c) || c == ';'; }), text.end());
    return text.empty() ? "class" : text;
}

// Gets a parameter or argument list of a signature, with one slot per item, such as (_,_) for two items
//
std::string HELPERS::signatureSlots(int itemCount) {
    std::string slots = "(";
    for (int i = 0; i < itemCount; ++i) slots += i == 0 ? "_" : ",_";
    return slots + ")";
}

// Gets the type that the name of a conversion operator names, such as bool of 'operator bool' or of 'Ptr::operator bool', or nothing for another name
//
std::string HELPERS::conversionTypeOf(const std::string& functionName) {
    const std::string keyword = "operator";
    const std::size_t start = functionName.rfind(keyword);
    if (start == std::string::npos || (start != 0 && (start < 2 || functionName.compare(start - 2, 2, "::") != 0))) return "";

    const std::size_t typeStart = start + keyword.size();
    if (typeStart >= functionName.size() || !std::isspace(static_cast<unsigned char>(functionName[typeStart]))) return "";

    const std::size_t first = functionName.find_first_not_of(" \t\r\n", typeStart);
    const std::size_t last  = functionName.find_last_not_of(" \t\r\n");
    return first == std::string::npos ? "" : functionName.substr(first, last - first + 1);
}
