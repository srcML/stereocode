// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file calls.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "calls.hpp"

#include <iostream>
#include <vector>

// Checks if ( call ) is ignored
//
bool calls::isIgnored(const std::string& call, const std::string& unitLanguage) {
    return (ignoredCallsList.at(unitLanguage).find(call) != ignoredCallsList.at(unitLanguage).end()) || (userIgnoredCallsList.find(call) != userIgnoredCallsList.end());
}

// Reads a set of user-defined calls to ignore 
// File should list each one type per line
//
std::istream& operator>>(std::istream& in, calls& calls)  {
    std::string ignoredCall;
    while(std::getline(in, ignoredCall))
        calls.userIgnoredCallsList.insert(ignoredCall);
    return in;
}

// Outputs the list of ignorable calls to stderr for debugging purposes
//
void calls::outputIgnorableCalls() const {
    std::cerr<<"---Ignored Calls---";
    for (const auto& pair : ignoredCallsList) {
        std::cerr<<"\n[" << pair.first << "]:" ;
        for (const std::string& call : pair.second) 
            std::cerr << ' ' << call;
    }
    if (userIgnoredCallsList.size() > 0) {
        std::cerr<<"\n[User-Defined]:";
        for (const std::string& call : userIgnoredCallsList) 
            std::cerr << ' ' << call;
    }
    std::cerr << "\n\n";
}

// Specific calls to ignore are used based on unit language
// cout, cin, streams, casts are all ignored (not collected) for C++ since they are not considered as a <call> in srcML
//
void calls::initializeIgnorableCalls() {
    ignoredCallsList.insert({"C++", {"assert","exit","abort"}});
    ignoredCallsList.insert({"C#", {"Console.WriteLine","Console.Write","Trace.WriteLine", "Environment.Exit"}});
    ignoredCallsList.insert({"Java", {"System.out.println","System.out.print","System.out.printf","assert","System.exit"}});
}
