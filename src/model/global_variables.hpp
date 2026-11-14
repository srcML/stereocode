// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file global_variables.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef GLOBAL_VARIABLES_HPP
#define GLOBAL_VARIABLES_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>

// A declaration of a global in a unit
//
struct globalDeclaration {
    std::string language;               // Language of the unit, where C units count as C++
    std::string name;
    std::string holder;                 // Type or namespace that holds the global, empty for the global namespace
    int         unit{0};                // Number of the unit of the declaration
    bool        namespaceScope{false};  // A namespace or the global namespace holds the global
    bool        fileLocal{false};       // Only the code of its unit reaches the global, as for a static variable at file scope
    bool        immutable{false};       // No call changes the global, as for a const std::string or a final String
};

// The globals of an archive: the variables declared at file or namespace scope and the static fields, by language and name
// Each name keeps the types and namespaces that hold a global of the name, where the global namespace has an empty name
//
class globalVariables {
public:
    void    add                         (const globalDeclaration&);
    void    merge                       (const globalVariables&);
    bool    atNamespaceScope            (const std::string&, const std::string&, int)                       const;
    bool    declaredAtNamespaceScope    (int, const std::string&)                                           const;
    bool    holds                       (const std::string&, const std::string&, const std::string&, int)   const;
    bool    heldByType                  (const std::string&, const std::string&, const std::string&, int)   const;
    bool    isImmutable                 (const std::string&, const std::string&, int)                       const;
private:
    // The types and namespaces that hold the globals of one name, the types among them, whether a namespace holds one of them,
    // and whether a call may change one of them
    struct holdersOfName {
        std::unordered_set<std::string> holders;
        std::unordered_set<std::string> types;
        bool                            namespaceScope{false};
        bool                            changeable{false};
    };
    using holdersByName = std::unordered_map<std::string, holdersOfName>;

    static void                 addHolder       (holdersByName&, const globalDeclaration&);
    static void                 mergeNames      (holdersByName&, const holdersByName&);
    static const holdersOfName* find            (const holdersByName*, const std::string&);
    const holdersByName*        languageGlobals (const std::string&)                                        const;
    const holdersByName*        unitGlobals     (int)                                                       const;

    std::unordered_map<std::string, holdersByName>  languages;  // Globals that the code of every unit of a language reaches
    std::unordered_map<int, holdersByName>          units;      // Globals that each unit declares, with those of internal linkage, by unit number
};

#endif
