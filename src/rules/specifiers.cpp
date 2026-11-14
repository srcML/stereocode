// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file specifiers.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "rules/specifiers.hpp"
#include "helper_functions.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <vector>

// Removes specifiers from a type string based on unit language
//
void specifiers::removeSpecifiers(std::string& type, const std::string& unitLanguage) const {
    auto pattern = specifiersPatterns.find(unitLanguage);
    if (pattern != specifiersPatterns.end()) type = pattern->second.replace(type);
}

// Gets the names in a type once its specifiers are removed, such as Node and T in Node<T>*
// A specifier or a comma separates two names, while the whitespace inside a name goes, as in longint for long int
// A name keeps no array brackets and, unless asked for, no qualifier, and an integer argument, such as 3 in std::array<int, 3>, names no type
//
std::vector<std::string> specifiers::typeNames(const std::string& type, const std::string& unitLanguage, bool qualified) const {
    const auto pattern = specifiersPatterns.find(unitLanguage);
    const std::vector<std::string> pieces = pattern != specifiersPatterns.end() ? pattern->second.split(type) : std::vector<std::string>{type};

    std::vector<std::string> names;
    for (std::string piece : pieces) {
        // The parentheses of a C++ function type separate names, as in bool(*)(int)
        if (unitLanguage == "C++") std::replace_if(piece.begin(), piece.end(), [](char c) { return c == '(' || c == ')'; }, ',');
        std::stringstream stream(piece);
        for (std::string name; std::getline(stream, name, ',');) {
            HELPERS::removeWhitespace(name);
            HELPERS::removeBracketSuffix(name);
            std::string bare = name;
            HELPERS::removeNamespace(bare, unitLanguage, true);
            if (!bare.empty() && !std::isdigit(static_cast<unsigned char>(bare.front()))) names.push_back(qualified ? name : bare);
        }
    }
    return names;
}

// Tells whether a type names a container of its language, as List does in List<Boolean>, whose elements are no value of the type itself
//
bool specifiers::namesContainer(const std::string& type, const std::string& unitLanguage) const {
    const auto pattern = containerPatterns.find(unitLanguage);
    return pattern != containerPatterns.end() && pattern->second.finds(type);
}

// Reads a set of user-defined type specifiers
// File should list one type per line
//
std::istream& operator>>(std::istream& in, specifiers& specifiers) {
    return in >> specifiers.userSpecifiersList;
}

// Outputs the list of specifiers to standard error for debugging purposes
//
void specifiers::outputSpecifiers() const {
    std::cerr<<"---Specifiers---";
    for (const auto& pair : specifiersList) {
        std::cerr<<"\n[" << pair.first << "]:" ;
        for (const std::string& type : pair.second) 
            std::cerr << ' ' << type;
        const auto containers = containersList.find(pair.first);
        if (containers == containersList.end()) continue;
        for (const std::string& type : containers->second)
            std::cerr << ' ' << type;
    }
    userSpecifiersList.print(std::cerr);
    std::cerr << "\n\n";
}

// Specific type specifiers are used based on unit language
// A container is a specifier as well, whose generic arguments name the types that count, while its own value is none of these types
//
void specifiers::initializeSpecifiersList() {
    specifiersList.insert({"C++", { "const", "volatile", "inline", "virtual", "friend", "extern", "&", "&&", "\\*", "public", "private", "protected",
                "mutable", "static", "thread_local", "register", "constexpr", "explicit", "signed", "unsigned",
                "<", ">", "restrict", "_Noreturn", "_Thread_local",
                // srcML marks a calling convention of Windows as a name of the type
                "__stdcall", "__cdecl", "__fastcall", "__thiscall", "__vectorcall", "__clrcall", "WINAPI", "WINAPIV", "APIENTRY", "CALLBACK",
                "PASCAL", "STDMETHODCALLTYPE", "STDAPICALLTYPE"}});
    containersList.insert({"C++", { "vector", "list", "set", "map", "unordered_map", "array", "multimap", "unordered_multimap",
                "::iterator", "::const_iterator", "forward_list", "stack", "queue", "priority_queue", "deque", "multiset",
                "unordered_set", "unordered_multiset", "pair", "span"}});

    specifiersList.insert({"C#", { "readonly", "ref", "out", "in", "unsafe", "internal", "params",
                "public", "private", "protected", "static", "virtual", "\\*", "volatile",
                "this",  "override", "abstract",  "extern", "async", "partial", "explicit", "implicit",
                "new", "sealed", "event", "const", "required", "scoped", "\\?", "Nullable(?=<)", "<", ">"}});
    containersList.insert({"C#", { "List", "Dictionary", "HashSet", "Queue", "Stack", "SortedList", "LinkedList",
                "IEnumerable(?=<)", "IList(?=<)", "ICollection(?=<)", "IReadOnlyList", "IReadOnlyCollection", "ISet",
                "BitArray", "KeyedCollection", "SortedSet", "BlockingCollection", "ConcurrentQueue", "ConcurrentStack",
                "ConcurrentDictionary", "ConcurrentBag", "ReadOnlyCollection", "ReadOnlyDictionary", "Tuple", "ValueTuple",
                "NameValueCollection", "StringCollection", "StringDictionary", "HybridDictionary", "OrderedDictionary"}});

    specifiersList.insert({"Java", { "public", "private", "protected", "static", "final", "transient", "\\?", "@\\w+",
                "volatile", "synchronized", "native", "strictfp", "abstract", "default", "super", "extends", "\\.\\.\\.", "<", ">"}});
    containersList.insert({"Java", { "Collection", "Iterable", "List", "ArrayList", "LinkedList", "Set", "HashSet", "LinkedHashSet", "SortedSet",
                "TreeSet", "Map", "HashMap", "Hashtable", "LinkedHashMap", "SortedMap", "TreeMap", "Deque", "ArrayDeque", "Queue", "PriorityQueue",
                "Vector", "Stack", "EnumSet", "EnumMap", "Iterator"}});

    // The wrappers of a type, which hold a value of the type, and | between the alternatives of a type
    specifiersList.insert({"Python", { "<", ">", "\\|", "\\.\\.\\.", "Optional", "Union", "Final", "ClassVar", "Annotated"}});
    // A class, an awaitable, a generator, and a callable hold no value of their argument types either
    containersList.insert({"Python", { "list", "dict", "set", "frozenset", "tuple", "List", "Dict", "Set", "FrozenSet", "Tuple", "Sequence",
                "MutableSequence", "Iterable", "Iterator", "Collection", "Mapping", "MutableMapping", "MutableSet", "AbstractSet", "Deque", "deque",
                "DefaultDict", "defaultdict", "OrderedDict", "Counter", "ChainMap", "Generator", "AsyncIterator", "AsyncIterable", "AsyncGenerator",
                "Awaitable", "Coroutine", "Callable", "Type", "type"}});

    // The wrappers of a type that hold a value of the type, and the operators of the union and the intersection of types
    specifiersList.insert({"JavaScript", { "<", ">", "\\|", "&", "\\?", "readonly", "keyof", "typeof", "Readonly", "Partial", "Required", "NonNullable", "Awaited"}});
    containersList.insert({"JavaScript", { "Array", "ReadonlyArray", "Map", "ReadonlyMap", "WeakMap", "Set", "ReadonlySet", "WeakSet", "Record",
                "Iterable", "IterableIterator", "Iterator", "AsyncIterable", "AsyncIterableIterator", "AsyncIterator", "Generator", "AsyncGenerator",
                "Promise", "PromiseLike"}});

    // The pattern of each language joins its specifiers, its containers, and then the user specifiers for all languages and for the language
    // of a section, while the pattern of the containers holds the containers alone
    for (const auto& languageSpecifiers : specifiersList) {
        const std::string& language = languageSpecifiers.first;
        const auto containers = containersList.find(language);
        const std::vector<std::string> containerEntries = containers != containersList.end()
            ? std::vector<std::string>(containers->second.begin(), containers->second.end()) : std::vector<std::string>{};

        std::vector<std::string> defaults(languageSpecifiers.second.begin(), languageSpecifiers.second.end());
        defaults.insert(defaults.end(), containerEntries.begin(), containerEntries.end());
        specifiersPatterns.try_emplace(language, defaults, userSpecifiersList.entriesOf(language));
        containerPatterns.try_emplace(language, containerEntries, std::vector<std::string>{});
    }
}
