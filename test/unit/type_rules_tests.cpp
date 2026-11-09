// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file type_rules_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "joined_names.hpp"
#include "stereotype_classifier.hpp"

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace {
    using s = stereotype;

    // Groups of methods with the same stereotypes, each with its number of methods
    using methodGroups = std::initializer_list<std::pair<int, stereotypeList>>;

    // Counts the stereotypes of a type made of the given groups of methods
    //
    typeCounts countsOf(methodGroups groups) {
        std::vector<stereotypeList> methods;
        for (const auto& [count, labels] : groups) methods.insert(methods.end(), count, labels);
        return countTypeStereotypes(methods);
    }

    // Gets the stereotypes of a type made of the given groups of methods, with the large-class threshold 21 unless given
    //
    std::string labelsOf(methodGroups groups, int methodsPerTypeThreshold = 21) {
        return joinedNames(classifyType(countsOf(groups), methodsPerTypeThreshold));
    }
}

TEST_CASE("the counts give the number of methods in each group of stereotypes") {
    typeCounts c = countsOf({{2, {s::get, s::collaborator}}, {1, {s::property, s::set}}, {1, {s::controller, s::stateless}}});
    CHECK(c.methods == 4);
    CHECK(c.getters == 2);
    CHECK(c.accessors == 3);
    CHECK(c.setters == 1);
    CHECK(c.mutators == 1);
    CHECK(c.controllers == 1);
    CHECK(c.collaboratorsOrWrappers == 2);
    CHECK(c.collaborators == 3);
    CHECK(c.nonCollaborators == 1);
    CHECK(c.degenerates == 1);
}

TEST_CASE("a method counts once in each group that holds one of its stereotypes") {
    typeCounts c = countsOf({{1, {s::get, s::property}}, {1, {s::get, s::set}}, {1, {s::command, s::incidental}}});
    CHECK(c.methods == 3);
    CHECK(c.accessors == 2);
    CHECK(c.getters == 2);
    CHECK(c.mutators == 2);
    CHECK(c.degenerates == 1);
}

TEST_CASE("a type without methods is empty") {
    CHECK(labelsOf({}) == "empty");
}

TEST_CASE("a type that matches no rule is unclassified") {
    CHECK(labelsOf({{1, {s::property}}, {1, {s::set}}, {1, {s::controller}}}) == "unclassified");
}

TEST_CASE("a type with one or two methods is small") {
    CHECK(labelsOf({{1, {s::get}}, {1, {s::set}}}) == "data small");
    CHECK(labelsOf({{2, {s::get}}, {1, {s::set}}}) == "data");
}

TEST_CASE("a type of get and set methods only is a data type") {
    CHECK(labelsOf({{2, {s::get}}, {2, {s::set}}}) == "data");
    CHECK(labelsOf({{2, {s::get}}, {2, {s::set}}, {1, {s::command}}}) == "minimal-entity");

    SUBCASE("a method labeled get and set counts once") {
        CHECK(labelsOf({{1, {s::get, s::set}}, {1, {s::property}}}) == "small");
    }
}

TEST_CASE("data-provider counts accessor methods, not accessor stereotypes") {
    CHECK(labelsOf({{2, {s::get, s::property}}, {1, {s::command}}}) == "entity");
    CHECK(labelsOf({{3, {s::get, s::property}}, {1, {s::command}}}) == "entity data-provider");
}

TEST_CASE("lazy counts the methods that are neither get, set, nor degenerate") {
    CHECK(labelsOf({{1, {s::get, s::set}}, {2, {s::incidental}}, {1, {s::command}}}) == "unclassified");
}

TEST_CASE("degenerate needs more than half degenerate methods") {
    CHECK(labelsOf({{3, {s::incidental}}, {1, {s::wrapper}}}) == "degenerate");
    CHECK(labelsOf({{2, {s::incidental}}, {2, {s::wrapper}}}) == "unclassified");
}

TEST_CASE("lazy needs get or set methods, more than a third degenerate methods, and at most a fifth of other methods") {
    CHECK(labelsOf({{1, {s::get}}, {1, {s::set}}, {1, {s::incidental}}}) == "unclassified");
    CHECK(labelsOf({{1, {s::get}}, {1, {s::set}}, {2, {s::incidental}}}) == "lazy");

    SUBCASE("33 degenerate methods of 100 are too few") {
        CHECK(labelsOf({{67, {s::get}}, {33, {s::stateless}}}) == "data-provider");
        CHECK(labelsOf({{66, {s::get}}, {34, {s::stateless}}}) == "data-provider lazy");
    }
    SUBCASE("other methods up to a fifth") {
        CHECK(labelsOf({{2, {s::get}}, {2, {s::empty}}, {1, {s::command}}}) == "lazy");
        CHECK(labelsOf({{1, {s::get}}, {2, {s::empty}}, {2, {s::command}}}) == "unclassified");
    }
    SUBCASE("no get or set methods") {
        CHECK(labelsOf({{3, {s::incidental}}}) == "degenerate");
        CHECK(labelsOf({{1, {s::get}}, {1, {s::set}}, {3, {s::incidental}}}) == "lazy degenerate");
    }
}

TEST_CASE("data-provider needs more than twice as many accessors as mutators and as controllers and factories") {
    CHECK(labelsOf({{3, {s::property}}, {1, {s::command}}}) == "entity data-provider");
    CHECK(labelsOf({{2, {s::property}}, {1, {s::command}}}) == "entity");
    CHECK(labelsOf({{5, {s::property}}, {2, {s::controller}}}) == "data-provider");
    CHECK(labelsOf({{4, {s::property}}, {2, {s::controller}}}) == "unclassified");
}

TEST_CASE("commander needs more than twice as many mutators as accessors and as controllers and factories") {
    CHECK(labelsOf({{3, {s::command}}, {1, {s::property}}}) == "entity commander");
    CHECK(labelsOf({{2, {s::command}}, {1, {s::property}}}) == "entity");
    CHECK(labelsOf({{5, {s::command}}, {2, {s::factory}}}) == "commander");
    CHECK(labelsOf({{4, {s::command}}, {2, {s::factory}}}) == "unclassified");
}

TEST_CASE("boundary needs more collaborators than other methods, under half factories, and under a third controllers") {
    CHECK(labelsOf({{2, {s::property, s::collaborator}}, {1, {s::property}}}) == "data-provider boundary");
    CHECK(labelsOf({{1, {s::property, s::collaborator}}, {1, {s::property}}}) == "data-provider small");

    SUBCASE("factories") {
        CHECK(labelsOf({{1, {s::factory, s::collaborator}}, {2, {s::property, s::collaborator}}, {1, {s::property}}}) ==
              "data-provider boundary");
        CHECK(labelsOf({{2, {s::factory, s::collaborator}}, {1, {s::property, s::collaborator}}, {1, {s::property}}}) ==
              "unclassified");
    }
    SUBCASE("33 controllers of 100 are under a third") {
        CHECK(labelsOf({{33, {s::controller}}, {18, {s::get, s::collaborator}}, {49, {s::get}}}) == "data-provider boundary");
        CHECK(labelsOf({{33, {s::controller}}, {18, {s::get, s::collaborator}}, {48, {s::get}}}) == "unclassified");
    }
}

TEST_CASE("factory needs more than two thirds factories") {
    CHECK(labelsOf({{3, {s::factory}}}) == "factory");
    CHECK(labelsOf({{67, {s::factory}}, {33, {s::get}}}) == "factory controller");
    CHECK(labelsOf({{66, {s::factory}}, {33, {s::get}}}) == "unclassified");
}

TEST_CASE("controller needs more than two thirds controllers and factories and an accessor or a mutator") {
    CHECK(labelsOf({{3, {s::controller}}, {1, {s::get}}}) == "controller");
    CHECK(labelsOf({{2, {s::controller}}, {1, {s::get}}}) == "unclassified");
    CHECK(labelsOf({{67, {s::controller}}, {33, {s::get}}}) == "controller");
    CHECK(labelsOf({{3, {s::controller}}, {1, {s::incidental}}}) == "pure-controller");
}

TEST_CASE("pure-controller needs controllers, possibly factories, and no accessor, mutator, collaborator, or wrapper") {
    CHECK(labelsOf({{2, {s::controller}}}) == "pure-controller small");
    CHECK(labelsOf({{1, {s::controller}}, {1, {s::factory}}, {1, {s::incidental}}}) == "pure-controller");
    CHECK(labelsOf({{1, {s::controller}}, {1, {s::wrapper}}, {1, {s::incidental}}}) == "unclassified");
    CHECK(labelsOf({{2, {s::factory}}}) == "factory small");
}

TEST_CASE("large needs more methods than the threshold with roles in balanced proportions") {
    methodGroups large22 = {{3, {s::get}}, {3, {s::property}}, {3, {s::set}}, {2, {s::command}},
                            {5, {s::controller}}, {5, {s::factory}}, {1, {s::incidental}}};
    CHECK(labelsOf(large22) == "large");
    CHECK(labelsOf(large22, 22) == "unclassified");

    SUBCASE("accessors and mutators below two thirds of the methods") {
        CHECK(labelsOf({{4, {s::get}}, {4, {s::property}}, {4, {s::set}}, {4, {s::command}},
                        {4, {s::controller}}, {4, {s::factory}}}) == "unclassified");
        CHECK(labelsOf({{4, {s::get}}, {4, {s::property}}, {4, {s::set}}, {3, {s::command}},
                        {4, {s::controller}}, {4, {s::factory}}}) == "large");
    }
    SUBCASE("controllers and factories above a fifth of the methods") {
        CHECK(labelsOf({{4, {s::get}}, {4, {s::property}}, {4, {s::set}}, {4, {s::command}},
                        {3, {s::controller}}, {2, {s::factory}}, {4, {s::incidental}}}) == "unclassified");
        CHECK(labelsOf({{4, {s::get}}, {4, {s::property}}, {4, {s::set}}, {4, {s::command}},
                        {3, {s::controller}}, {3, {s::factory}}, {3, {s::incidental}}}) == "large");
    }
    SUBCASE("every role present") {
        CHECK(labelsOf({{6, {s::get}}, {5, {s::set}}, {11, {s::controller}}}) == "unclassified");
    }
}

TEST_CASE("the large-class threshold is the mean plus the population standard deviation of the methods per type, rounded down") {
    methodCountSummary summary = summarizeMethodCounts({0, 23, 22, 0});
    CHECK(summary.types == 4);
    CHECK(summary.mean == doctest::Approx(11.25));
    CHECK(summary.standardDeviation == doctest::Approx(11.256).epsilon(0.001));
    CHECK(summary.largeTypeThreshold == 22);

    SUBCASE("a mean plus standard deviation that is a whole number is the threshold") {
        CHECK(summarizeMethodCounts({1, 3}).largeTypeThreshold == 3);
        CHECK(summarizeMethodCounts({0, 0, 0, 0, 5}).largeTypeThreshold == 3);
    }
    SUBCASE("equal numbers of methods give that number") {
        CHECK(summarizeMethodCounts({5, 5, 5}).largeTypeThreshold == 5);
    }
    SUBCASE("a run without types gives zero") {
        CHECK(summarizeMethodCounts({}).types == 0);
        CHECK(summarizeMethodCounts({}).largeTypeThreshold == 0);
    }
}

TEST_CASE("entity needs an accessor besides get, a mutator besides set, at most two collaborators per other method, and no controller") {
    CHECK(labelsOf({{1, {s::property}}, {1, {s::command}}}) == "entity small");
    CHECK(labelsOf({{1, {s::property, s::collaborator}}, {1, {s::command, s::collaborator}}, {1, {s::get}}}) ==
          "entity boundary");
    CHECK(labelsOf({{1, {s::property, s::collaborator}}, {1, {s::command, s::collaborator}}, {1, {s::get}}, {1, {s::set}}}) ==
          "entity");
    CHECK(labelsOf({{1, {s::property, s::collaborator}}, {1, {s::command, s::collaborator}}, {1, {s::get, s::collaborator}}}) ==
          "boundary");
    CHECK(labelsOf({{1, {s::property, s::collaborator}}, {1, {s::command, s::collaborator}}, {1, {s::controller}}}) ==
          "unclassified");
    CHECK(labelsOf({{1, {s::get, s::collaborator}}, {1, {s::command, s::collaborator}}, {1, {s::get}}}) == "boundary");
}

TEST_CASE("entity counts a method labeled get and property as an accessor besides get") {
    CHECK(labelsOf({{1, {s::get, s::property, s::collaborator}}, {1, {s::command, s::collaborator}}, {1, {s::get}}}) ==
          "entity boundary");
}

TEST_CASE("minimal-entity counts a method labeled get and set once") {
    CHECK(labelsOf({{1, {s::get, s::set}}, {1, {s::command}}}) == "minimal-entity small");
}

TEST_CASE("minimal-entity needs get, set, and command methods only, each present, and at most two collaborators per other method") {
    CHECK(labelsOf({{1, {s::get}}, {1, {s::set}}, {1, {s::nonVoidCommand}}}) == "minimal-entity");
    CHECK(labelsOf({{1, {s::get, s::collaborator}}, {1, {s::set, s::collaborator}}, {1, {s::command}}}) ==
          "minimal-entity boundary");
    CHECK(labelsOf({{1, {s::get, s::collaborator}}, {1, {s::set, s::collaborator}}, {1, {s::command, s::collaborator}},
                    {1, {s::get}}}) == "boundary");
    CHECK(labelsOf({{1, {s::get, s::collaborator}}, {1, {s::set, s::collaborator}}, {1, {s::get}}}) == "boundary data");
}
