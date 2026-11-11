// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_labels_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "stereotype_labels.hpp"

#include <set>
#include <string>

TEST_CASE("each stereotype has its own name and is found again by that name") {
    std::set<std::string> names;
    for (int i = 0; i <= static_cast<int>(stereotype::literal); ++i) {
        stereotype label = static_cast<stereotype>(i);
        std::string name(stereotypeName(label));
        CAPTURE(name);
        CHECK_FALSE(name.empty());
        CHECK(names.insert(name).second);
        CHECK(stereotypeFromName(name) == label);
    }
}

TEST_CASE("stereotype names are the names stereocode writes") {
    CHECK(stereotypeName(stereotype::voidAccessor) == "void-accessor");
    CHECK(stereotypeName(stereotype::nonVoidCommand) == "non-void-command");
    CHECK(stereotypeName(stereotype::copyConstructor) == "copy-constructor");
    CHECK(stereotypeName(stereotype::minimalEntity) == "minimal-entity");
    CHECK(stereotypeName(stereotype::dataProvider) == "data-provider");
    CHECK(stereotypeName(stereotype::pureController) == "pure-controller");
    CHECK(stereotypeName(stereotype::globalCommand) == "global-command");
}

TEST_CASE("an unknown name has no stereotype") {
    CHECK_FALSE(stereotypeFromName("large-class").has_value());
    CHECK_FALSE(stereotypeFromName("").has_value());
}

TEST_CASE("the size and data stereotypes of a type take its structure as a suffix") {
    CHECK(typeStereotypeName(stereotype::large, "class") == "large-class");
    CHECK(typeStereotypeName(stereotype::lazy, "struct") == "lazy-struct");
    CHECK(typeStereotypeName(stereotype::data, "union") == "data-union");
    CHECK(typeStereotypeName(stereotype::small, "enum") == "small-enum");
    CHECK(typeStereotypeName(stereotype::entity, "class") == "entity");
    CHECK(typeStereotypeName(stereotype::empty, "interface") == "empty");
}
