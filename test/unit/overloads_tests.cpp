// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file overloads_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "overloads.hpp"

#include <vector>

using positions = std::vector<std::size_t>;

namespace {
    // Gets an overload at a position that takes from a minimum to a maximum number of arguments
    //
    overload takingBetween(std::size_t position, int minimum, int maximum, bool variadic = false) {
        return {position, {minimum, maximum, variadic}};
    }
}

TEST_CASE("a call reaches the overloads of its name that take as many arguments as it passes") {
    const std::vector<overload> overloads{takingBetween(4, 0, 0), takingBetween(7, 1, 1), takingBetween(9, 2, 2)};
    CHECK(overloadsTaking(overloads, 0) == positions{4});
    CHECK(overloadsTaking(overloads, 1) == positions{7});
    CHECK(overloadsTaking(overloads, 2) == positions{9});
}

TEST_CASE("an overload with default values or a variadic parameter takes several numbers of arguments") {
    const std::vector<overload> overloads{takingBetween(1, 0, 1), takingBetween(2, 1, 2, true), takingBetween(3, 2, 2)};
    CHECK(overloadsTaking(overloads, 0) == positions{1});
    CHECK(overloadsTaking(overloads, 1) == positions{1, 2});
    CHECK(overloadsTaking(overloads, 2) == positions{2, 3});
    CHECK(overloadsTaking(overloads, 5) == positions{2});
}

TEST_CASE("a call that no overload takes by its number of arguments reaches every overload of its name") {
    const std::vector<overload> overloads{takingBetween(5, 1, 1), takingBetween(6, 2, 2)};
    CHECK(overloadsTaking(overloads, 0) == positions{5, 6});
    CHECK(overloadsTaking(overloads, 3) == positions{5, 6});
}
