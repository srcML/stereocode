// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file source_language.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef SOURCE_LANGUAGE_HPP
#define SOURCE_LANGUAGE_HPP

#include <string>
#include <string_view>
#include <vector>

// A language whose units the analysis reads, with what the modules ask about it besides its queries and its tables
//
struct sourceLanguage {
    std::string_view    name;           // Name of the language in srcML, in the reports, and in the section headers of the user lists
    std::string_view    lineComment;    // Start of a comment that runs to the end of its line
    bool                dynamic;        // The code declares neither its data members nor the types of its variables, and reaches each member through the object
    bool                modules;        // A file is a module, so a name without a qualifier reaches only the globals of its own file
    bool                callsCreate;    // A call of a class creates an object, since the language has no operator new
    std::string_view    callOperator;   // Method that a call of a variable by its name calls, as handler.Invoke() for handler(), if the language has such calls
    bool                methodsOnly;    // The language has no functions outside types, so a call without an object names a method of a type
    bool                overloads;      // Methods of one name differ in their parameters, so a call may name a method of a parent under the name of a method of the type
    bool                preprocessor;   // A type may define one method in several #if alternatives, which count as one method of the type
    std::string_view    rootClass;      // Parent of every class that a call may name with the object as its argument, as object.__repr__(self), if any
    std::string_view    discard;        // Name that takes a value and is no variable, as _ of _ = Run(); and of Parse(text, out _), if any
    std::vector<std::string_view> asyncResults; // Types through which an async function returns its value, as Task of async Task<bool>, if any
};

const std::vector<sourceLanguage>&  sourceLanguages     ();
const sourceLanguage&               languageOf          (std::string_view);
std::string                         analysisLanguageOf  (std::string_view);
std::string                         sectionHeadersText  ();

#endif
