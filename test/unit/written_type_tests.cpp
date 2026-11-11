// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file written_type_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "model/written_type.hpp"

TEST_CASE("added text joins the text part before it and starts a part of its own after a name") {
    writtenType type;
    addText(type, "const");
    type.push_back({"Count", true});
    addText(type, "");
    addText(type, "*");
    addText(type, "&");
    REQUIRE(type.size() == 3);
    CHECK(type[0].text == "const");
    CHECK(type[1].isName);
    CHECK(type[2].text == "*&");
    CHECK_FALSE(type[2].isName);
}

TEST_CASE("the text of a parameter list joins the parts of each type, with a comma between the types, in parentheses") {
    CHECK(typeListText({}) == "()");
    CHECK(typeListText({{{"const", false}, {"Count", true}, {"&", false}}}) == "(constCount&)");
    CHECK(typeListText({{{"const", false}, {"Count", true}, {"&", false}}, {{"std::size_t", true}}, {{"unsigned", true}, {"int", true}}}) ==
          "(constCount&,std::size_t,unsignedint)");
}
