// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file written_type.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef WRITTEN_TYPE_HPP
#define WRITTEN_TYPE_HPP

#include <string>
#include <string_view>
#include <vector>

// A part of a type as the code writes it without whitespace: a name of a type without template arguments, such as Count or units::Meters,
// which an alias may stand for, or the text between such names, such as const, std::vector<, or &
//
struct typePart {
    std::string text;                   // Source code of the part without whitespace
    bool        isName{false};          // Names a type
};

// A type as the code writes it, as its parts in order, such as const, Count, and & of const Count&
//
using writtenType = std::vector<typePart>;

void        addText         (writtenType&, std::string_view);
std::string typeText        (const writtenType&);
std::string typeListText    (const std::vector<writtenType>&);
std::string withoutQualifiers (const std::string&);

#endif
