// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_labels.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "stereotype_labels.hpp"

#include <cstddef>
#include <iterator>

namespace {
    // Names in the order of the stereotype enumerators
    constexpr std::string_view STEREOTYPE_NAMES[] = {
        "get", "predicate", "property", "void-accessor", "set", "command", "non-void-command", "factory",
        "collaborator", "controller", "wrapper", "incidental", "stateless", "empty", "unclassified",
        "constructor", "copy-constructor", "destructor",
        "entity", "minimal-entity", "data-provider", "commander", "boundary", "pure-controller",
        "large", "lazy", "degenerate", "data", "small",
        "main", "global-command", "literal"
    };

    static_assert(std::size(STEREOTYPE_NAMES) == static_cast<std::size_t>(stereotype::literal) + 1,
                  "Each stereotype needs one name");
}

// Gets the name of a stereotype as it appears in the output
//
std::string_view stereotypeName(stereotype s) {
    return STEREOTYPE_NAMES[static_cast<std::size_t>(s)];
}

// Gets the stereotype with the given name, or nothing for an unknown name
//
std::optional<stereotype> stereotypeFromName(std::string_view name) {
    for (std::size_t i = 0; i < std::size(STEREOTYPE_NAMES); ++i)
        if (STEREOTYPE_NAMES[i] == name) return static_cast<stereotype>(i);
    return std::nullopt;
}

// Gets the name of a type stereotype, with the structure (e.g., class or struct) appended where the stereotype takes it
//
std::string typeStereotypeName(stereotype s, const std::string& structure) {
    std::string name(stereotypeName(s));
    if (s == stereotype::large || s == stereotype::lazy || s == stereotype::data || s == stereotype::small)
        name += "-" + structure;
    return name;
}
