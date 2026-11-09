// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file calls_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "calls.hpp"

#include <sstream>
#include <string>

namespace {
    // Gets the default lists of calls to ignore
    //
    calls defaultIgnoredCalls() {
        calls ignored;
        ignored.initializeIgnorableCalls();
        return ignored;
    }

    // Gets the default lists of calls to ignore with the entries of a user list file held in a string
    //
    calls ignoredCallsWith(const std::string& userFile) {
        calls ignored;
        std::istringstream in(userFile);
        in >> ignored;
        ignored.initializeIgnorableCalls();
        return ignored;
    }
}

TEST_CASE("a listed call is ignored whole or through a qualified part that ends it") {
    const calls ignored = defaultIgnoredCalls();

    CHECK(ignored.isIgnored("Console.WriteLine", "C#"));
    CHECK(ignored.isIgnored("System.Console.WriteLine", "C#"));
    CHECK(ignored.isIgnored("System.out.println", "Java"));
    CHECK(ignored.isIgnored("std::exit", "C++"));
    CHECK(ignored.isIgnored("exit", "C++"));
    CHECK(ignored.isIgnored("a->b.assert", "C++"));
}

TEST_CASE("the last part of a call is also checked without its generic arguments") {
    CHECK(defaultIgnoredCalls().isIgnored("std::exit<int>", "C++"));
}

TEST_CASE("nameof is no call in C#") {
    const calls ignored = defaultIgnoredCalls();
    CHECK(ignored.isIgnored("nameof", "C#"));
    CHECK_FALSE(ignored.isIgnored("nameof", "Java"));
}

TEST_CASE("assertions and messages to the debugger or the error stream are no calls") {
    const calls ignored = defaultIgnoredCalls();
    CHECK(ignored.isIgnored("Debug.Assert", "C#"));
    CHECK(ignored.isIgnored("System.Diagnostics.Debug.WriteLine", "C#"));
    CHECK(ignored.isIgnored("Console.Error.WriteLine", "C#"));
    CHECK(ignored.isIgnored("System.err.println", "Java"));
}

TEST_CASE("Java lists no assert, which srcML marks as a statement") {
    const calls ignored = defaultIgnoredCalls();
    CHECK_FALSE(ignored.isIgnored("assert", "Java"));
    CHECK(ignored.isIgnored("assert", "C++"));
}

TEST_CASE("a call whose name only contains a listed call is not ignored") {
    const calls ignored = defaultIgnoredCalls();
    CHECK_FALSE(ignored.isIgnored("WriteLine", "C#"));
    CHECK_FALSE(ignored.isIgnored("Console.WriteLineAsync", "C#"));
    CHECK_FALSE(ignored.isIgnored("exitCode", "C++"));
    CHECK_FALSE(ignored.isIgnored("MyConsole.WriteLine", "C#"));
}

TEST_CASE("a call to ignore under a section header holds only for the units of its language") {
    const calls ignored = ignoredCallsWith("logMessage\n[C#]\nLogger.Trace\n");

    CHECK(ignored.isIgnored("_log.Logger.Trace", "C#"));
    CHECK_FALSE(ignored.isIgnored("Logger.Trace", "Java"));
    CHECK_FALSE(ignored.isIgnored("Logger.Trace", "C++"));
    CHECK(ignored.isIgnored("logMessage", "Java"));
    CHECK(ignored.isIgnored("app::logMessage", "C++"));
}
