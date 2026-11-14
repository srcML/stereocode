// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file user_lists.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "rules/user_lists.hpp"
#include "archive/source_language.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace {
    // Gets the language of a section header, such as C# of [C#], or an empty string for a line that is no header
    // C units take the C++ entries
    //
    std::string sectionLanguageOf(const std::string& line) {
        for (const sourceLanguage& language : sourceLanguages())
            if (line == "[" + std::string(language.name) + "]") return std::string(language.name);
        return "";
    }

    // Tells whether two texts hold the same letters in any case
    //
    bool equalsIgnoringCase(const std::string& text, const std::string& other) {
        return text.size() == other.size() && std::equal(text.begin(), text.end(), other.begin(), [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
    }
}

// Reads the entries of a list file, one per line, without the spaces, tabs, and carriage returns around them
// Empty lines and lines that start with # hold no entry, and a line such as [C++], [C#], or [Java] opens the section of its language
//
std::vector<listEntry> readListEntries(std::istream& in) {
    constexpr const char* blank = " \t\r";

    std::vector<listEntry> entries;
    std::string language;
    for (std::string line; std::getline(in, line);) {
        const std::size_t first = line.find_first_not_of(blank);
        if (first == std::string::npos || line[first] == '#') continue;

        std::string text = line.substr(first, line.find_last_not_of(blank) - first + 1);
        std::string header = sectionLanguageOf(text);
        if (!header.empty()) language = std::move(header);
        else entries.push_back({language, std::move(text)});
    }
    return entries;
}

// Gets the entries of a list file that differ from a section header only in case, such as [c#], which are entries for every language
//
std::vector<std::string> headersInOtherCase(std::istream& in) {
    std::vector<std::string> headers;
    for (const listEntry& entry : readListEntries(in)) {
        for (const sourceLanguage& language : sourceLanguages())
            if (equalsIgnoringCase(entry.text, "[" + std::string(language.name) + "]")) headers.push_back(entry.text);
    }
    return headers;
}

// Tells whether an entry holds for a language, as an entry for every language or of the section of the language
//
bool userEntries::contains(const std::string& entry, const std::string& language) const {
    for (const std::string& section : {std::string(), language}) {
        const auto found = entries.find(section);
        if (found != entries.end() && found->second.count(entry) > 0) return true;
    }
    return false;
}

// Gets the entries that hold for a language, each once: those for every language and those of the section of the language
//
std::vector<std::string> userEntries::entriesOf(const std::string& language) const {
    std::vector<std::string> held;
    for (const std::string& section : {std::string(), language}) {
        const auto found = entries.find(section);
        if (found == entries.end()) continue;
        for (const std::string& entry : found->second)
            if (std::find(held.begin(), held.end(), entry) == held.end()) held.push_back(entry);
    }
    return held;
}

// Prints the entries for every language as [User-Defined]: and the entries of each section as, for example, [User-Defined C#]:
//
void userEntries::print(std::ostream& out) const {
    std::vector<std::string> sections{""};
    for (const sourceLanguage& language : sourceLanguages()) sections.emplace_back(language.name);
    for (const std::string& section : sections) {
        const auto found = entries.find(section);
        if (found == entries.end()) continue;

        out << "\n[User-Defined" << (section.empty() ? "" : " " + section) << "]:";
        for (const std::string& entry : found->second) out << ' ' << entry;
    }
}

// Reads the entries of a list file
//
std::istream& operator>>(std::istream& in, userEntries& read) {
    for (listEntry& entry : readListEntries(in)) read.entries[entry.language].insert(std::move(entry.text));
    return in;
}
