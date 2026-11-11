// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_labels.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef STEREOTYPE_LABELS_HPP
#define STEREOTYPE_LABELS_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Stereotypes of methods, types, and free functions
// The type stereotypes large, lazy, data, and small take the structure as a suffix (e.g., large-class)
// literal stays last, since the table of names checks its size against it
//
enum class stereotype {
    get, predicate, property, voidAccessor, set, command, nonVoidCommand, factory,
    collaborator, controller, wrapper, incidental, stateless, empty, unclassified,
    constructor, copyConstructor, destructor,
    entity, minimalEntity, dataProvider, commander, boundary, pureController,
    large, lazy, degenerate, data, small,
    main, globalCommand, literal
};

using stereotypeList = std::vector<stereotype>;

std::string_view          stereotypeName     (stereotype);
std::optional<stereotype> stereotypeFromName (std::string_view);
std::string               typeStereotypeName (stereotype, const std::string&);

#endif
