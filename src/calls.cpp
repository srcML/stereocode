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

// Checks if the name of a call, or a qualified part of the name that ends it, is ignored
// For example, System.Console.WriteLine is ignored through Console.WriteLine, and std::exit through exit
// The last part is also checked without its generic arguments, such as WriteLine of WriteLine<T>
// A user-defined call holds for all languages or for the language of its section
//
bool calls::isIgnored(const std::string& call, const std::string& unitLanguage) const {
    auto listed = [this, &unitLanguage](const std::string& name) {
        return ignoredCallsList.at(unitLanguage).find(name) != ignoredCallsList.at(unitLanguage).end() || userIgnoredCallsList.contains(name, unitLanguage);
    };
    if (listed(call)) return true;

    std::size_t lastPart = 0;
    for (std::size_t i = 0; i < call.size(); ++i) {
        std::size_t separator = 0;
        if      (call.compare(i, 2, "::") == 0 || call.compare(i, 2, "->") == 0) separator = 2;
        else if (call[i] == '.')                                                 separator = 1;
        if (separator == 0) continue;

        lastPart = i + separator;
        if (listed(call.substr(lastPart))) return true;
        i += separator - 1;
    }

    const std::string lastName = call.substr(lastPart, call.find('<', lastPart) - lastPart);
    return lastName != call && listed(lastName);
}

// Reads a set of user-defined calls to ignore 
// File should list each one type per line
//
std::istream& operator>>(std::istream& in, calls& calls)  {
    return in >> calls.userIgnoredCallsList;
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
    userIgnoredCallsList.print(std::cerr);
    std::cerr << "\n\n";
}

// Specific calls to ignore are used based on unit language
// cout, cin, streams, casts are all ignored (not collected) for C++ since they are not considered as a <call> in srcML
// srcML marks the C# operator nameof as a call, while it marks the Java assert as a statement
//
void calls::initializeIgnorableCalls() {
    ignoredCallsList.insert({"C++", {"assert","exit","abort"}});
    ignoredCallsList.insert({"C#", {"Console.WriteLine","Console.Write","Console.Error.WriteLine","Trace.WriteLine","Debug.Assert","Debug.WriteLine","Environment.Exit","nameof"}});
    ignoredCallsList.insert({"Java", {"System.out.println","System.out.print","System.out.printf","System.err.println","System.exit"}});
}
