// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file IgnorableCalls.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "IgnorableCalls.hpp"

bool ignorableCalls::isCallIgnored(const std::string& call) const {
    return (ignoredCalls.find(call) != ignoredCalls.end()) || 
           (userIgnoredCalls.find(call) != userIgnoredCalls.end());
}

// Adds "ignoredCall" to user-defined calls to ignore if not already present
//
void ignorableCalls::addIgnoredCall(const std::string& ignoredCall) {
    if (!isCallIgnored(ignoredCall)) userIgnoredCalls.insert(ignoredCall);
}

// Reads a set of user-defined calls to ignore 
// File should list each one type per line
//
std::istream& operator>>(std::istream& in, ignorableCalls& calls)  {
    std::string name;
    while(std::getline(in, name))
        calls.addIgnoredCall(name);
    return in;
}

// Specific calls to ignore are used based on unit language
//
void ignorableCalls::setLanguage(const std::string& unitLang) {
    if (unitLanguage == unitLang) return;
    
    unitLanguage = unitLang;

    if (unitLanguage == "C++") {
        ignoredCalls = {
            "assert",
            "exit",
            "abort"
        };
    }
    else if (unitLanguage == "C#") {  
        ignoredCalls = {
            "WriteLine",
            "Write",
            "Trace",
            "Assert",
            "Exit"
        };
    }
    else if (unitLanguage == "Java") {
        ignoredCalls = {
            "println",
            "print",
            "printf",
            "assert",
            "exit"
        };
    }
}
