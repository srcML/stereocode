// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file type_index.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/type_index.hpp"
#include "model/qualified_name.hpp"

#include <algorithm>
#include <iterator>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace {
    // Gets the key of a name in a language
    //
    std::string nameKey(const std::string& language, const std::string& name) {
        return language + '\n' + name;
    }

    // Gets the positions of a key, or none
    //
    const std::vector<std::size_t>& positionsOf(const std::unordered_map<std::string, std::vector<std::size_t>>& byKey, const std::string& key) {
        static const std::vector<std::size_t> none;
        const auto found = byKey.find(key);
        return found == byKey.end() ? none : found->second;
    }

    // Gets the place of a type or of an alias
    //
    const indexedType& placeOf(const indexedType& type)   { return type; }
    const indexedType& placeOf(const indexedAlias& alias) { return alias.place; }

    // Gets the number of parts of a name that reaches a place from outside its scope through an import or a using directive: the name of a type alone,
    // and the name of an alias that a type declares with the names of the types around it, since no directive brings in the members of a type
    //
    std::size_t partsFromOutside(const indexedType&)        { return 1; }
    std::size_t partsFromOutside(const indexedAlias& alias) { return alias.enclosingTypes + 1; }

    // Tells whether a path ends with the parts of another path
    //
    bool endsWith(const std::vector<std::string>& path, const std::vector<std::string>& ending) {
        return path.size() >= ending.size() && std::equal(ending.rbegin(), ending.rend(), path.rbegin());
    }

    // Tells whether a path of names or of element positions starts with the parts of another path
    //
    template <typename part>
    bool startsWith(const std::vector<part>& path, const std::vector<part>& start) {
        return path.size() >= start.size() && std::equal(start.begin(), start.end(), path.begin());
    }

    // Tells whether an alias holds where code writes a name: a C# using alias directive only in its own file unless it is global,
    // and a C++ alias anywhere, since the index does not see the includes of a unit
    //
    bool holdsAt(const indexedAlias& alias, const typeReference& reference) {
        return alias.place.language != "C#" || alias.otherUnits || alias.place.unitNumber == reference.unitNumber;
    }

    // Tells whether the code that writes a name sees an alias, which then hides the types of its name from a farther scope:
    // an alias of the unit of the name before the name or inside the type that writes it, or an alias of another unit that holds there
    //
    bool seenAt(const indexedAlias& alias, const typeReference& reference) {
        if (alias.place.unitNumber != reference.unitNumber) return alias.otherUnits;
        return alias.place.position < reference.position || startsWith(alias.place.position, reference.position);
    }

    // Gets the number of leading folders that two file names share
    //
    std::size_t sharedFolders(const std::string& first, const std::string& second) {
        std::size_t folders = 0;
        for (std::size_t i = 0; i < first.size() && i < second.size() && first[i] == second[i]; ++i)
            if (first[i] == '/' || first[i] == '\\') ++folders;
        return folders;
    }

    // Keeps the positions that meet a condition, unless none meets it
    //
    template <typename condition>
    void keepIfAny(std::vector<std::size_t>& positions, condition keep) {
        std::vector<std::size_t> kept;
        std::copy_if(positions.begin(), positions.end(), std::back_inserter(kept), keep);
        if (!kept.empty()) positions = std::move(kept);
    }

    // The places that a written name refers to in the innermost scope around the name that holds one, with the number of the parts of the scope
    // that qualify them, or -1 for the places that a using directive or an import brings into scope
    //
    struct scopedMatches {
        std::vector<std::size_t>    positions;
        int                         depth{-1};
    };

    // Gets the places whose path is a written path after the namespaces of a scope, from the innermost namespaces that hold such a place
    //
    template <typename entry>
    scopedMatches inScope(const std::vector<entry>& entries, const std::vector<std::size_t>& candidates, const std::vector<std::string>& written,
                          const std::vector<std::string>& scope) {
        for (std::size_t depth = scope.size() + 1; depth-- > 0;) {
            std::vector<std::string> path(scope.begin(), scope.begin() + static_cast<std::ptrdiff_t>(depth));
            path.insert(path.end(), written.begin(), written.end());

            scopedMatches matches{{}, static_cast<int>(depth)};
            for (std::size_t position : candidates)
                if (placeOf(entries[position]).path == path) matches.positions.push_back(position);
            if (!matches.positions.empty()) return matches;
        }
        return {};
    }

    // Gets the places that the C++ using directives before a name, in its namespace or a namespace around it, bring into scope
    // Only includes, which the index does not see, bring in the places of other units, so the places of the unit of the name come first
    //
    template <typename entry>
    std::vector<std::size_t> nominated(const std::vector<entry>& entries, std::vector<std::size_t> candidates, const typeReference& reference) {
        if (reference.directives.empty()) return {};

        // A type that the unit declares after the name is out of its reach, and a method defined outside its class lies in a namespace around the class
        auto inUnit = [&](std::size_t position) { return placeOf(entries[position]).unitNumber == reference.unitNumber; };
        auto outOfReach = [&](std::size_t position) {
            const indexedType& place = placeOf(entries[position]);
            return (inUnit(position) && !(place.position < reference.position)) || !startsWith(place.path, reference.scope);
        };
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(), outOfReach), candidates.end());

        std::vector<std::size_t> matches;
        for (const usingDirective& directive : reference.directives) {
            if (!(directive.position < reference.position) || !startsWith(reference.scope, directive.scope)) continue;

            // A directive names its namespace from the namespaces around the directive, as Detail in namespace Lib names Lib::Detail
            std::vector<std::string> written = directive.name;
            written.insert(written.end(), reference.path.begin(), reference.path.end());
            const std::vector<std::size_t> reached = inScope(entries, candidates, written, directive.scope).positions;
            matches.insert(matches.end(), reached.begin(), reached.end());
        }
        keepIfAny(matches, inUnit);
        return matches;
    }

    // Gets the places among the places of a name that a written name refers to, other than an excluded place
    // C# and Java tell types apart by their number of generic parameters, while a C++ name with template arguments may name the template itself
    //
    template <typename entry>
    scopedMatches matchesOf(const std::vector<entry>& entries, const std::vector<std::size_t>& named, const typeReference& reference,
                            std::optional<std::size_t> excluded) {
        std::vector<std::size_t> candidates;
        for (std::size_t position : named) {
            if (excluded && position == *excluded) continue;
            if (reference.language != "C++" && placeOf(entries[position]).arity != reference.arity) continue;
            candidates.push_back(position);
        }

        scopedMatches matches = inScope(entries, candidates, reference.path, reference.scope);
        if (!matches.positions.empty()) return matches;

        // C++ lookup through using directives accepts a name only when they bring in one type
        matches.positions = nominated(entries, candidates, reference);
        if (!matches.positions.empty()) return matches;

        // A name that an import or a using directive that the index does not see, such as one in a header, brings into scope
        for (std::size_t position : candidates)
            if (endsWith(placeOf(entries[position]).path, reference.path) && reference.path.size() >= partsFromOutside(entries[position]))
                matches.positions.push_back(position);
        return matches;
    }

    // Picks among places with the same path: a C++ specialization with the written template arguments, then the template itself,
    // then a place of the unit of the name, then the place whose file shares the most folders with the file of the name
    //
    template <typename entry>
    std::size_t closest(const std::vector<entry>& entries, std::vector<std::size_t> matches, const typeReference& reference) {
        auto place = [&entries](std::size_t position) -> const indexedType& { return placeOf(entries[position]); };

        if (reference.language == "C++") {
            const bool specialized = std::any_of(matches.begin(), matches.end(), [&](std::size_t position) { return place(position).arguments == reference.arguments; });
            keepIfAny(matches, [&](std::size_t position) { return specialized ? place(position).arguments == reference.arguments : place(position).arguments.empty(); });
        }
        keepIfAny(matches, [&](std::size_t position) { return place(position).unitNumber == reference.unitNumber; });

        std::size_t best = matches.front();
        for (std::size_t position : matches)
            if (sharedFolders(place(position).fileName, reference.fileName) > sharedFolders(place(best).fileName, reference.fileName)) best = position;
        return best;
    }

    // Picks the place that a written name refers to among its matches, or nothing when no place fits or the places
    // that a using directive or an import brings into scope have several paths
    //
    template <typename entry>
    std::optional<std::size_t> pick(const std::vector<entry>& entries, const scopedMatches& matches, const typeReference& reference) {
        if (matches.positions.empty()) return std::nullopt;
        for (std::size_t position : matches.positions)
            if (placeOf(entries[position]).path != placeOf(entries[matches.positions.front()]).path) return std::nullopt;
        return closest(entries, matches.positions, reference);
    }

    // Gets a name that code writes at the place of another name, such as a name in the type that an alias stands for
    //
    typeReference nameAt(const std::string& name, const typeReference& place) {
        typeReference reference = referenceOf(splitQualifiedName(name, place.language), place.language, place.scope, place.unitNumber, place.fileName);
        reference.directives = place.directives;
        reference.position   = place.position;
        return reference;
    }

    // Tells whether the text of a type ends with const or volatile, which qualify the name after them
    //
    bool endsWithQualifier(const std::string& text) {
        for (std::string_view qualifier : {"const", "volatile"})
            if (text.size() >= qualifier.size() && text.compare(text.size() - qualifier.size(), qualifier.size(), qualifier) == 0) return true;
        return false;
    }

    // Tells whether the text of a type holds a pointer, a reference, an array, or a function outside its template arguments, as int* does
    //
    bool holdsDeclarator(const std::string& text) {
        int depth = 0;
        for (char c : text) {
            if (c == '<') ++depth;
            else if (c == '>') --depth;
            else if (depth == 0 && (c == '*' || c == '&' || c == '[' || c == '(')) return true;
        }
        return false;
    }
}

// Gets a name of a type that code in a scope writes, from the parts of the name, such as a parent, a qualifier, or the type of an alias,
// or without parts the place of a type that has no name of its own, as unsigned char
//
typeReference referenceOf(const std::vector<std::string>& parts, const std::string& language, const std::vector<std::string>& scope,
                          int unitNumber, const std::string& fileName) {
    typeReference reference;
    reference.language   = language;
    for (const std::string& part : parts) reference.path.push_back(withoutGenericArguments(part));
    if (!parts.empty()) {
        reference.arguments = genericArguments(parts.back());
        reference.arity     = genericArity(parts.back());
    }
    reference.scope      = scope;
    reference.unitNumber = unitNumber;
    reference.fileName   = fileName;
    return reference;
}

// Adds a type, whose position is the number of types added before it
//
void typeIndex::add(indexedType type) {
    if (!type.path.empty()) byName[nameKey(type.language, type.path.back())].push_back(types.size());
    types.push_back(std::move(type));
}

// Adds an alias, which find() leaves out and findThroughAliases() follows
//
void typeIndex::addAlias(indexedAlias alias) {
    if (alias.place.path.empty()) return;
    aliasesByName[nameKey(alias.place.language, alias.place.path.back())].push_back(aliases.size());
    aliases.push_back(std::move(alias));
}

// Finds the position of the type that a written name refers to, other than an excluded type, or nothing when no type or several types fit
//
std::optional<std::size_t> typeIndex::find(const typeReference& reference, std::optional<std::size_t> excluded) const {
    if (reference.path.empty()) return std::nullopt;
    return pick(types, matchesOf(types, positionsOf(byName, nameKey(reference.language, reference.path.back())), reference, excluded), reference);
}

// Finds the position of the type that a written name refers to, where the name may be an alias of the type, also through other aliases,
// and for a call through -> an alias of a smart pointer to the type, or nothing for a cycle of aliases
//
std::optional<std::size_t> typeIndex::findThroughAliases(typeReference reference, bool throughPointer) const {
    std::unordered_set<std::size_t> followed;
    std::optional<std::size_t> alias;
    while (!reference.path.empty()) {
        alias = aliasOf(reference, alias);
        if (!alias) return find(reference);
        if (!followed.insert(*alias).second) return std::nullopt;

        // -> reaches the object that an alias of a pointer points to, after which the aliases lead to the types that they name
        const bool dereferenced = throughPointer && !aliases[*alias].pointee.path.empty();
        reference = dereferenced ? aliases[*alias].pointee : aliases[*alias].target;
        throughPointer = throughPointer && !dereferenced;
    }
    return std::nullopt;
}

// Gets a type that code writes at the place of a name, where each name of an alias gives way to the type that the alias stands for,
// as int* of CountPtr after typedef int Count, *CountPtr;, also through the aliases that the alias writes
//
writtenType typeIndex::withoutAliases(const writtenType& type, const typeReference& place) const {
    std::unordered_set<std::size_t> followed;
    return replaceAliases(type, place, std::nullopt, followed);
}

// Replaces the names of aliases in a type that code writes at a place, other than an excluded alias and the aliases being followed,
// so that the names of a cycle of aliases stay as written
// A name after const or volatile keeps an alias of a pointer or a reference, since const NodePtr is Node* const and not const Node*
//
writtenType typeIndex::replaceAliases(const writtenType& type, const typeReference& place, std::optional<std::size_t> excluded,
                                      std::unordered_set<std::size_t>& followed) const {
    writtenType replaced;
    for (const typePart& part : type) {
        const std::optional<std::size_t> alias = part.isName ? aliasOf(nameAt(part.text, place), excluded) : std::nullopt;
        if (!alias || aliases[*alias].spelledType.empty() || !followed.insert(*alias).second) {
            replaced.push_back(part);
            continue;
        }
        const std::string text = typeText(replaceAliases(aliases[*alias].spelledType, aliases[*alias].target, alias, followed));
        followed.erase(*alias);
        if (!replaced.empty() && endsWithQualifier(replaced.back().text) && holdsDeclarator(text)) replaced.push_back(part);
        else addText(replaced, text);
    }
    return replaced;
}

// Finds the alias that a written name refers to, other than an excluded alias, or nothing when no alias fits or a type of the same
// or a nearer scope hides the aliases of the name
// An alias resolves a name that no type fits wherever it holds, and hides the types of its name from a farther scope where the code sees it,
// as the name lookup of C++ does
//
std::optional<std::size_t> typeIndex::aliasOf(const typeReference& reference, std::optional<std::size_t> excluded) const {
    if (reference.path.empty()) return std::nullopt;
    const std::string key = nameKey(reference.language, reference.path.back());
    const scopedMatches typeMatches = matchesOf(types, positionsOf(byName, key), reference, std::nullopt);
    const bool typed = !typeMatches.positions.empty();

    std::vector<std::size_t> candidates;
    for (std::size_t position : positionsOf(aliasesByName, key))
        if (typed ? seenAt(aliases[position], reference) : holdsAt(aliases[position], reference)) candidates.push_back(position);

    // The name of an alias holds only after its declaration, so the target of the alias never names the alias itself
    const scopedMatches aliasMatches = matchesOf(aliases, candidates, reference, excluded);
    if (typed && aliasMatches.depth <= typeMatches.depth) return std::nullopt;
    return pick(aliases, aliasMatches, reference);
}
