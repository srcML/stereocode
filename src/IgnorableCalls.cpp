// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file IgnorableCalls.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "IgnorableCalls.hpp"

extern std::vector<std::string> LANGUAGE;

// Checks if 'call' is ignored
// User-defined calls are checked for all languages
//
bool ignorableCalls::isIgnored(const std::string& call, const std::string& unitLanguage) {
return (ignoredCalls.at(unitLanguage).find(call) != ignoredCalls.at(unitLanguage).end()) || 
       (userIgnoredCalls.find(call) != userIgnoredCalls.end());
}

// Reads a set of user-defined calls to ignore 
// File should list each one type per line
//
std::istream& operator>>(std::istream& in, ignorableCalls& calls)  {
    std::string name;
    while(std::getline(in, name))
        calls.addCall(name);
    return in;
}

// Adds "ignoredCall" to user-defined calls to ignore if not already present
//
void ignorableCalls::addCall(const std::string& ignoredCall) {
    userIgnoredCalls.insert(ignoredCall);
}

void ignorableCalls::outputCalls() {
    std::cerr<<"---Ignored Calls---";
    for (const auto& pair : ignoredCalls) {
        std::cerr<<"\n[" << pair.first << "]:" ;
        for (const std::string& call : pair.second) 
            std::cerr << ' ' << call;
    }
    if (userIgnoredCalls.size() > 0) {
        std::cerr<<"\n[User-Defined]:";
        for (const std::string& call : userIgnoredCalls) 
            std::cerr << ' ' << call;
    }
    std::cerr << "\n\n";
}

// Specific calls to ignore are used based on unit language
//
void ignorableCalls::createCallList() {
    for (const auto& l : LANGUAGE) {
        // cout, cin, streams, casts are all ignored (not collected) for C++ since they are not considered as a <call>
        if (l == "C++") {
            ignoredCalls.insert({l, {
                "assert",
                "exit",
                "abort"
            }});
        }
        else if (l == "C#") {  
            ignoredCalls.insert({l, {
                "WriteLine",
                "Write",
                "Trace",
                "Assert",
                "Exit"
            }});
        }
        else if (l == "Java") {
            ignoredCalls.insert({l, {
                "println",
                "print",
                "printf",
                "assert",
                "exit"
            }});
        }
    }   
}
