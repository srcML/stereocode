// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file member_access.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef MEMBER_ACCESS_HPP
#define MEMBER_ACCESS_HPP

#include <optional>
#include <string>

// The first two names of an expression such as this->a.b, Foo::a, or a.b
//
struct memberAccess {
    std::string         object;     // Name before the access operator, empty after this, base, super, and (*this)
    std::string         member;     // Name after the access operator
};

std::optional<memberAccess> splitMemberAccess (const std::string&, const std::string&, bool);

#endif
