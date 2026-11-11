// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file output_names_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "output/output_names.hpp"

TEST_CASE("The default srcML output archive sits beside the input archive") {
    CHECK(defaultOutputFile("code.xml") == "code.stereotypes.xml");
    CHECK(defaultOutputFile("data/RxJava-3.1.12.xml") == "data/RxJava-3.1.12.stereotypes.xml");
}

TEST_CASE("The CSV reports take the name of the srcML output archive") {
    const reportNames besideInput = reportNamesFor("data/code.stereotypes.xml", ".csv");
    CHECK(besideInput.types == "data/code.stereotypes.csv");
    CHECK(besideInput.freeFunctions == "data/code.free_functions.stereotypes.csv");

    const reportNames besideOutput = reportNamesFor("results/code.out.xml", ".csv");
    CHECK(besideOutput.types == "results/code.out.stereotypes.csv");
    CHECK(besideOutput.freeFunctions == "results/code.out.free_functions.stereotypes.csv");
}

TEST_CASE("The text reports take the name of the srcML output archive like the CSV reports") {
    const reportNames besideInput = reportNamesFor("data/code.stereotypes.xml", ".txt");
    CHECK(besideInput.types == "data/code.stereotypes.txt");
    CHECK(besideInput.freeFunctions == "data/code.free_functions.stereotypes.txt");

    const reportNames besideOutput = reportNamesFor("results/code.out.xml", ".txt");
    CHECK(besideOutput.types == "results/code.out.stereotypes.txt");
    CHECK(besideOutput.freeFunctions == "results/code.out.free_functions.stereotypes.txt");
}

TEST_CASE("An output archive without the extension .xml keeps its whole name in the names of the reports") {
    CHECK(reportNamesFor("results/code.srcml", ".csv").types == "results/code.srcml.stereotypes.csv");
    CHECK(reportNamesFor("results/code.stereotypes", ".csv").types == "results/code.stereotypes.csv");
    CHECK(reportNamesFor("results/code.srcml", ".txt").types == "results/code.srcml.stereotypes.txt");
}
