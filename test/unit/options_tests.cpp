// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file options_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "options.hpp"

TEST_CASE("A run without -j uses every hardware thread up to 8") {
    CHECK(defaultThreads(1) == 1);
    CHECK(defaultThreads(4) == 4);
    CHECK(defaultThreads(8) == 8);
    CHECK(defaultThreads(24) == 8);
    CHECK(defaultThreads(256) == 8);
}

TEST_CASE("A run without -j uses one thread when the system reports no processors") {
    CHECK(defaultThreads(0) == 1);
}
