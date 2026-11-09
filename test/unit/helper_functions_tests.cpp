// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file helper_functions_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "helper_functions.hpp"

#include <string>

TEST_CASE("a signature has one slot per parameter or argument") {
    CHECK(HELPERS::signatureSlots(0) == "()");
    CHECK(HELPERS::signatureSlots(1) == "(_)");
    CHECK(HELPERS::signatureSlots(3) == "(_,_,_)");
}

TEST_CASE("a conversion operator names its type after the keyword operator") {
    CHECK(HELPERS::conversionTypeOf("operator bool") == "bool");
    CHECK(HELPERS::conversionTypeOf("Ptr::operator bool") == "bool");
    CHECK(HELPERS::conversionTypeOf("operator const char*") == "const char*");
    CHECK(HELPERS::conversionTypeOf("operator std::string") == "std::string");
    CHECK(HELPERS::conversionTypeOf("operator IntPtr") == "IntPtr");
}

TEST_CASE("other operators and names name no conversion type") {
    CHECK(HELPERS::conversionTypeOf("operator==").empty());
    CHECK(HELPERS::conversionTypeOf("operator").empty());
    CHECK(HELPERS::conversionTypeOf("getOperator").empty());
    CHECK(HELPERS::conversionTypeOf("cooperator bool").empty());
    CHECK(HELPERS::conversionTypeOf("size").empty());
}
