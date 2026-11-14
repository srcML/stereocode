// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file source_language.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "archive/source_language.hpp"

// Gets the languages of the analysis, in the order of the verbose output
//
const std::vector<sourceLanguage>& sourceLanguages() {
    static const std::vector<sourceLanguage> languages{
        {"C++",        "//", false, false, false, "operator()", false, true,  true,  "",       "",  {}},
        {"C#",         "//", false, false, false, "Invoke",     true,  true,  true,  "",       "_", {"Task", "ValueTask"}},
        {"Java",       "//", false, false, false, "",           true,  true,  false, "",       "",  {}},
        {"Python",     "#",  true,  true,  true,  "__call__",   false, false, false, "object", "",  {}},
        {"JavaScript", "//", true,  true,  false, "call",       false, false, false, "",       "",  {"Promise"}},
    };
    return languages;
}

// Gets a language of the analysis by its name, or a language without a name that has none of the special traits
//
const sourceLanguage& languageOf(std::string_view name) {
    static const sourceLanguage unknown{"", "//", false, false, false, "", false, false, false, "", "", {}};
    for (const sourceLanguage& language : sourceLanguages())
        if (language.name == name) return language;
    return unknown;
}

// Gets the language whose queries and tables apply to a unit of a srcML language, where C units take those of C++,
// or an empty string for a language without analysis
//
std::string analysisLanguageOf(std::string_view srcmlLanguage) {
    if (srcmlLanguage == "C") return "C++";
    return std::string(languageOf(srcmlLanguage).name);
}

// Gets the section headers of the user lists as a text, such as [C++], [C#], or [Java]
//
std::string sectionHeadersText() {
    const std::vector<sourceLanguage>& languages = sourceLanguages();
    std::string text;
    for (std::size_t i = 0; i < languages.size(); ++i) {
        if (i > 0) text += i + 1 == languages.size() ? ", or " : ", ";
        text += "[" + std::string(languages[i].name) + "]";
    }
    return text;
}
