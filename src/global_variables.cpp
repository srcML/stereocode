// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file global_variables.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "global_variables.hpp"

// Adds a global of a unit, which the units of its language reach unless it has internal linkage, where C units count as C++
//
void globalVariables::add(const globalDeclaration& global) {
    addHolder(units[global.unit], global);
    if (!global.fileLocal) addHolder(languages[global.language], global);
}

// Adds the globals of another set, such as those of another unit
//
void globalVariables::merge(const globalVariables& other) {
    for (const auto& [language, names] : other.languages) mergeNames(languages[language], names);
    for (const auto& [unit, names] : other.units) mergeNames(units[unit], names);
}

// Tells whether the code of a unit reaches a global of a name at namespace scope, which a name without a qualifier reaches
//
bool globalVariables::atNamespaceScope(const std::string& language, const std::string& name, int unit) const {
    const holdersOfName* shared = find(languageGlobals(language), name);
    const holdersOfName* own    = find(unitGlobals(unit), name);
    return (shared && shared->namespaceScope) || (own && own->namespaceScope);
}

// Tells whether a unit declares a global of a name at namespace scope, an extern declaration included
//
bool globalVariables::declaredAtNamespaceScope(int unit, const std::string& name) const {
    const holdersOfName* own = find(unitGlobals(unit), name);
    return own && own->namespaceScope;
}

// Tells whether the code of a unit reaches a global of a name in a type or a namespace, or in the global namespace for an empty holder
//
bool globalVariables::holds(const std::string& language, const std::string& holder, const std::string& name, int unit) const {
    const holdersOfName* shared = find(languageGlobals(language), name);
    const holdersOfName* own    = find(unitGlobals(unit), name);
    return (shared && shared->holders.count(holder) > 0) || (own && own->holders.count(holder) > 0);
}

// Tells whether no call changes the globals of a name that the code of a unit reaches, as for a const std::string or a final String
//
bool globalVariables::isImmutable(const std::string& language, const std::string& name, int unit) const {
    const holdersOfName* shared = find(languageGlobals(language), name);
    const holdersOfName* own    = find(unitGlobals(unit), name);
    return (shared || own) && !(shared && shared->changeable) && !(own && own->changeable);
}

// Adds the holder of a global to the globals of its name
//
void globalVariables::addHolder(holdersByName& names, const globalDeclaration& global) {
    holdersOfName& holders = names[global.name];
    holders.holders.insert(global.holder);
    holders.namespaceScope = holders.namespaceScope || global.namespaceScope;
    holders.changeable     = holders.changeable || !global.immutable;
}

// Adds the holders of the globals of each name of a set to another set
//
void globalVariables::mergeNames(holdersByName& names, const holdersByName& other) {
    for (const auto& [name, global] : other) {
        holdersOfName& merged = names[name];
        merged.holders.insert(global.holders.begin(), global.holders.end());
        merged.namespaceScope = merged.namespaceScope || global.namespaceScope;
        merged.changeable     = merged.changeable || global.changeable;
    }
}

// Gets the holders of the globals of a name, or null
//
const globalVariables::holdersOfName* globalVariables::find(const holdersByName* names, const std::string& name) {
    if (!names) return nullptr;
    const auto found = names->find(name);
    return found == names->end() ? nullptr : &found->second;
}

// Gets the globals that the units of a language reach, or null
//
const globalVariables::holdersByName* globalVariables::languageGlobals(const std::string& language) const {
    const auto found = languages.find(language);
    return found == languages.end() ? nullptr : &found->second;
}

// Gets the globals that a unit declares, or null
//
const globalVariables::holdersByName* globalVariables::unitGlobals(int unit) const {
    const auto found = units.find(unit);
    return found == units.end() ? nullptr : &found->second;
}
