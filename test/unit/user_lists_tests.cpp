// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file user_lists_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "rules/user_lists.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

using entries = std::vector<std::string>;

namespace {
    // Reads every entry of a list file held in a string, each entry of a section after the header of its language, such as [C#] Guid
    //
    entries entriesOf(const std::string& text) {
        std::istringstream file(text);
        entries read;
        for (const listEntry& entry : readListEntries(file))
            read.push_back(entry.language.empty() ? entry.text : "[" + entry.language + "] " + entry.text);
        return read;
    }

    // Reads the user entries of a list file held in a string
    //
    userEntries userEntriesOf(const std::string& text) {
        std::istringstream file(text);
        userEntries read;
        file >> read;
        return read;
    }

    // Gets the entries that hold for a language in sorted order
    //
    entries sortedEntriesOf(const userEntries& read, const std::string& language) {
        entries sorted = read.entriesOf(language);
        std::sort(sorted.begin(), sorted.end());
        return sorted;
    }
}

TEST_CASE("a list file holds one entry per line, with or without a final line end") {
    CHECK(entriesOf("QString\nQByteArray\n") == entries{"QString", "QByteArray"});
    CHECK(entriesOf("QString\nQByteArray") == entries{"QString", "QByteArray"});
    CHECK(entriesOf("").empty());
}

TEST_CASE("an entry of a list file loses the spaces, tabs, and carriage return around it") {
    CHECK(entriesOf("QString\r\n  QByteArray\t \nQ_INVOKABLE") == entries{"QString", "QByteArray", "Q_INVOKABLE"});
    CHECK(entriesOf("QString\r\nQByteArray\r\n") == entries{"QString", "QByteArray"});
    CHECK(entriesOf("  QString \t\n\tunsigned long\n") == entries{"QString", "unsigned long"});
}

TEST_CASE("empty lines and lines that start with # hold no entry") {
    CHECK(entriesOf("# primitives of Qt\n\n   \r\n  # indented comment\nqreal\n") == entries{"qreal"});
    CHECK(entriesOf("# Qt types\n\nQString\n   \r\n  # strings\nQByteArray\n") == entries{"QString", "QByteArray"});
    CHECK(entriesOf("#\n# only comments\n").empty());
}

TEST_CASE("an entry keeps the spaces and the characters inside it") {
    CHECK(entriesOf("unsigned char \n[[nodiscard]]\n*\n") == entries{"unsigned char", "[[nodiscard]]", "*"});
    CHECK(entriesOf("C#Type\n") == entries{"C#Type"});
}

TEST_CASE("a section header gives the entries after it to its language, while the entries before the first header hold for every language") {
    CHECK(entriesOf("QString\n[C++]\nQByteArray\n[C#]\nGuid\n[Java]\nUUID\n") == entries{"QString", "[C++] QByteArray", "[C#] Guid", "[Java] UUID"});
    CHECK(entriesOf("[Java]\nUUID\n[C++]\nqreal\n[Java]\nBigDecimal\n") == entries{"[Java] UUID", "[C++] qreal", "[Java] BigDecimal"});
}

TEST_CASE("a section header stands on a line of its own, with blanks and a carriage return around it, and holds until the next header") {
    CHECK(entriesOf("  [C#] \r\nGuid\r\n# Money\n\nDecimal\n") == entries{"[C#] Guid", "[C#] Decimal"});
}

TEST_CASE("a line in brackets other than [C++], [C#], and [Java] is an entry") {
    CHECK(entriesOf("[[nodiscard]]\n[C]\n[c#]\n[Python]\n[C#] Guid\n") == entries{"[[nodiscard]]", "[C]", "[c#]", "[Python]", "[C#] Guid"});
}

TEST_CASE("a line in brackets that names a language of a header in another case is a header in another case") {
    std::istringstream file("[c++]\n  [JAVA] \r\n[C#]\n[C]\n[c#]Guid\n[Python]\n[[nodiscard]]\n[C# ]\n# [java]\n[c#]\n");
    CHECK(headersInOtherCase(file) == entries{"[c++]", "[JAVA]", "[c#]"});
}

TEST_CASE("an entry before the first header holds for every language, and an entry of a section only for its language") {
    const userEntries read = userEntriesOf("QString\n[C#]\nGuid\n[Java]\nQString\n");

    CHECK(read.contains("QString", "C++"));
    CHECK(read.contains("QString", "C#"));
    CHECK(read.contains("QString", "Java"));
    CHECK(read.contains("Guid", "C#"));
    CHECK_FALSE(read.contains("Guid", "C++"));
    CHECK_FALSE(read.contains("Guid", "Java"));

    CHECK(sortedEntriesOf(read, "C#") == entries{"Guid", "QString"});
    CHECK(sortedEntriesOf(read, "C++") == entries{"QString"});
    CHECK(sortedEntriesOf(read, "Java") == entries{"QString"});
}
