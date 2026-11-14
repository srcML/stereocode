// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file check_stereotypes.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 *
 * The helper program of the tests. The mode comments compares the "// @stereotype" comments of a stereocode output
 * archive, "# @stereotype" in Python, with the st:stereotype labels of the elements that follow them. The mode table writes the table of the
 * labeled elements of an archive and, given a reference table, prints the first rows that differ.
 */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

const char* const SRC_NAMESPACE        = "http://www.srcML.org/srcML/src";
const char* const TYPESCRIPT_NAMESPACE = "http://www.srcML.org/srcML/TypeScript";
const char* const POSITION_NAMESPACE   = "http://www.srcML.org/srcML/position";
const char* const STEREOTYPE_NAMESPACE = "http://www.srcML.org/srcML/stereotype";

const char* const USAGE =
    "Usage: check_stereotypes comments [-v|--verbose] <stereocode output archive>\n"
    "       check_stereotypes table <stereocode output archive> <table> [<reference table>]\n"
    "The mode comments compares each \"// @stereotype <labels> [@defect <reason>]\" comment,\n"
    "which starts with # in Python, with the st:stereotype labels of the element that follows it,\n"
    "and counts the labeled elements without such a comment, which -v lists.\n"
    "The mode table writes a row for each labeled element of the archive to the table, with its file, line, element,\n"
    "name, and stereotypes, and compares the table with the reference table.\n";

// Elements that stereocode annotates, in the namespace of srcML or, as ts:interface, of TypeScript; any other element that carries st:stereotype counts as well
const std::set<std::string> ANNOTATABLE = {
    "function", "constructor", "destructor",
    "class", "struct", "union", "interface", "enum", "record"
};

const char* const HEADER = "file\tline\telement\tname\tstereotypes";

// A comparison lists this many differing rows and counts the rest
const std::size_t SHOWN_DIFFERENCES = 10;

const std::size_t NOT_FOUND = std::numeric_limits<std::size_t>::max();

// One "@stereotype" comment: the expected labels and the reason of a known defect that keeps them from holding, if any
struct Expectation {
    std::string              fileName;
    int                      line = 0;
    std::set<std::string>    labels;
    std::string              defect;
    std::string              error;
};

// The counts of the expectations of an archive by their outcome, and of the labeled elements without an expectation
struct Totals {
    int expectations      = 0;
    int held              = 0;
    int expectedFailures  = 0;
    int problems          = 0;
    int unchecked         = 0;
};

// Tells whether a node is the element of srcML with the given name
bool isSrcElement(const xmlNode* node, const char* name) {
    return node && node->type == XML_ELEMENT_NODE && node->ns && node->ns->href &&
           std::strcmp(reinterpret_cast<const char*>(node->ns->href), SRC_NAMESPACE) == 0 &&
           std::strcmp(reinterpret_cast<const char*>(node->name), name) == 0;
}

// Gets the text of a node with the text of its descendants
std::string textOf(const xmlNode* node) {
    xmlChar* content = xmlNodeGetContent(node);
    std::string text = content ? reinterpret_cast<const char*>(content) : "";
    xmlFree(content);
    return text;
}

// Gets an attribute of an element, or an empty text for a missing attribute
std::string attributeOf(const xmlNode* node, const char* name, const char* nameSpace = nullptr) {
    const xmlChar* attribute = reinterpret_cast<const xmlChar*>(name);
    xmlChar* value = nameSpace ? xmlGetNsProp(node, attribute, reinterpret_cast<const xmlChar*>(nameSpace))
                               : xmlGetProp(node, attribute);
    std::string text = value ? reinterpret_cast<const char*>(value) : "";
    xmlFree(value);
    return text;
}

// Gets an attribute of the element at the reader, or an empty text for a missing attribute
std::string attributeOf(xmlTextReaderPtr reader, const char* name, const char* nameSpace) {
    xmlChar* value = nameSpace ? xmlTextReaderGetAttributeNs(reader, reinterpret_cast<const xmlChar*>(name), reinterpret_cast<const xmlChar*>(nameSpace))
                               : xmlTextReaderGetAttribute(reader, reinterpret_cast<const xmlChar*>(name));
    std::string text = value ? reinterpret_cast<const char*>(value) : "";
    xmlFree(value);
    return text;
}

// Gets the words of a comment without its delimiters, or nothing for a comment that is no expectation
std::vector<std::string> expectationWords(const xmlNode* comment) {
    std::string text = textOf(comment);
    if (text.compare(0, 2, "//") == 0) text.erase(0, 2);
    else if (text.compare(0, 1, "#") == 0) text.erase(0, 1);
    else if (text.compare(0, 2, "/*") == 0 && text.size() >= 4 && text.compare(text.size() - 2, 2, "*/") == 0)
        text = text.substr(2, text.size() - 4);
    else return {};

    std::istringstream in(text);
    std::vector<std::string> words;
    for (std::string word; in >> word;) words.push_back(word);
    if (words.empty() || words.front() != "@stereotype") return {};
    return words;
}

// Gets the labels and the tags of an expectation, or the error that makes it malformed
Expectation parseExpectation(const std::vector<std::string>& words) {
    static const std::regex label(R"([a-z]+(-[a-z]+)*)");

    Expectation expectation;
    bool tagged = false;
    for (std::size_t i = 1; i < words.size() && expectation.error.empty(); ++i) {
        const std::string& word = words[i];
        if (word == "@defect" && tagged)       expectation.error = "more than one tag";
        else if (word == "@defect")            tagged = true;
        else if (word[0] == '@')               expectation.error = "unknown tag " + word;
        else if (tagged)                       expectation.defect += (expectation.defect.empty() ? "" : " ") + word;
        else if (std::regex_match(word, label)) expectation.labels.insert(word);
        else                                   expectation.error = "malformed label " + word;
    }
    if (!expectation.error.empty()) return expectation;

    if (tagged && expectation.defect.empty()) expectation.error = "tag without a reason";
    else if (expectation.labels.empty())     expectation.error = "no expected label";
    return expectation;
}

// Tells whether an element has an st:stereotype attribute
bool carriesStereotype(const xmlNode* node) {
    return xmlHasNsProp(node, reinterpret_cast<const xmlChar*>("stereotype"),
                        reinterpret_cast<const xmlChar*>(STEREOTYPE_NAMESPACE)) != nullptr;
}

// Gets the first element child of a node, or null
const xmlNode* firstElementChild(const xmlNode* node) {
    const xmlNode* child = node->children;
    while (child && child->type != XML_ELEMENT_NODE) child = child->next;
    return child;
}

// Gets the annotatable element that directly follows the comment, looking inside a C++ friend and into the type of a C++ typedef
const xmlNode* annotatedElementAfter(const xmlNode* comment) {
    const xmlNode* node = comment->next;
    while (node && (node->type != XML_ELEMENT_NODE ||
                    (isSrcElement(node, "comment") && expectationWords(node).empty())))
        node = node->next;

    if (isSrcElement(node, "friend")) node = firstElementChild(node);
    else if (isSrcElement(node, "typedef")) {
        const xmlNode* type = firstElementChild(node);
        node = isSrcElement(type, "type") ? firstElementChild(type) : nullptr;
    }

    if (!node || node->type != XML_ELEMENT_NODE) return nullptr;
    if (carriesStereotype(node)) return node;
    const char* nameSpace = node->ns ? reinterpret_cast<const char*>(node->ns->href) : nullptr;
    if (!nameSpace || (std::strcmp(nameSpace, SRC_NAMESPACE) != 0 && std::strcmp(nameSpace, TYPESCRIPT_NAMESPACE) != 0) ||
        ANNOTATABLE.count(reinterpret_cast<const char*>(node->name)) == 0)
        return nullptr;
    return node;
}

// Gets the text with each run of white space as one space, which keeps a name that spans lines in one cell
std::string oneLine(const std::string& text) {
    std::string line;
    bool space = false;
    for (char c : text) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') space = !line.empty();
        else {
            if (space) line += ' ';
            line += c;
            space = false;
        }
    }
    return line;
}

// Gets the text of the first name child of an element, or an empty text for an element without a name of its own
std::string nameOf(const xmlNode* element) {
    for (const xmlNode* child = element ? element->children : nullptr; child; child = child->next)
        if (isSrcElement(child, "name")) return oneLine(textOf(child));
    return "";
}

// Gets the labels of the st:stereotype attribute of an element
std::set<std::string> labelsOf(const xmlNode* element) {
    std::istringstream in(attributeOf(element, "stereotype", STEREOTYPE_NAMESPACE));
    std::set<std::string> labels;
    for (std::string label; in >> label;) labels.insert(label);
    return labels;
}

// Gets the labels separated by spaces in braces, such as {get set}
std::string joined(const std::set<std::string>& words) {
    std::string text;
    for (const std::string& word : words) text += (text.empty() ? "" : " ") + word;
    return "{" + text + "}";
}

// Records the line of every comment element and every labeled element of a unit, counting the newlines of its text
void findLines(const xmlNode* node, int& line, std::unordered_map<const xmlNode*, int>& lines) {
    for (const xmlNode* child = node->children; child; child = child->next) {
        if (child->type == XML_TEXT_NODE || child->type == XML_CDATA_SECTION_NODE) {
            for (const xmlChar* c = child->content; c && *c; ++c)
                if (*c == '\n') ++line;
        }
        else if (child->type == XML_ELEMENT_NODE) {
            if (isSrcElement(child, "comment") || carriesStereotype(child)) lines[child] = line;
            findLines(child, line, lines);
        }
    }
}

// Collects the comment elements and the labeled elements of a unit in document order, without those of a nested unit
void collectElements(const xmlNode* node, std::vector<const xmlNode*>& comments, std::vector<const xmlNode*>& labeled) {
    for (const xmlNode* child = node->children; child; child = child->next) {
        if (isSrcElement(child, "unit")) continue;
        if (isSrcElement(child, "comment")) comments.push_back(child);
        else if (child->type == XML_ELEMENT_NODE) {
            if (carriesStereotype(child)) labeled.push_back(child);
            collectElements(child, comments, labeled);
        }
    }
}

// Prints a message about an expectation after its file and line
void report(const Expectation& expectation, const std::string& message) {
    std::cout << expectation.fileName << ':' << expectation.line << ": " << message << '\n';
}

// Checks the expectations of a unit: an untagged one must hold, and one tagged @defect must fail until its defect is fixed
// The labeled elements that no expectation names count as unchecked, and -v lists them
void checkUnit(const xmlNode* unit, bool verbose, Totals& totals) {
    const std::string fileName = attributeOf(unit, "filename");

    std::unordered_map<const xmlNode*, int> lines;
    int line = 1;
    findLines(unit, line, lines);

    std::vector<const xmlNode*> comments;
    std::vector<const xmlNode*> labeled;
    collectElements(unit, comments, labeled);
    std::unordered_set<const xmlNode*> checked;

    for (const xmlNode* comment : comments) {
        const std::vector<std::string> words = expectationWords(comment);
        if (words.empty()) continue;

        Expectation expectation = parseExpectation(words);
        expectation.fileName = fileName;
        expectation.line = lines[comment];
        ++totals.expectations;

        if (!expectation.error.empty()) {
            ++totals.problems;
            report(expectation, "malformed expectation: " + expectation.error);
            continue;
        }

        const xmlNode* element = annotatedElementAfter(comment);
        if (element) checked.insert(element);
        const std::set<std::string> observed = element ? labelsOf(element) : std::set<std::string>{};
        const bool same = element && observed == expectation.labels;
        const std::string found = !element         ? "no annotatable element after the comment"
                                : observed.empty() ? "no stereotype"
                                                   : joined(observed);
        const std::string comparison = "expected " + joined(expectation.labels) + ", got " + found;

        if (!expectation.defect.empty()) {
            if (same) {
                ++totals.problems;
                report(expectation, "unexpected pass, remove @defect " + expectation.defect + ": " + comparison);
            }
            else {
                ++totals.expectedFailures;
                if (verbose) report(expectation, "expected failure, " + expectation.defect + ": " + comparison);
            }
        }
        else if (same) {
            ++totals.held;
            if (verbose) report(expectation, "holds: " + comparison);
        }
        else {
            ++totals.problems;
            report(expectation, "wrong stereotype: " + comparison);
        }
    }

    for (const xmlNode* element : labeled) {
        if (checked.count(element)) continue;
        ++totals.unchecked;
        if (verbose)
            std::cout << fileName << ':' << lines[element] << ": no expectation for the " << reinterpret_cast<const char*>(element->name)
                      << ' ' << nameOf(element) << ' ' << joined(labelsOf(element)) << '\n';
    }
}

// Checks the "// @stereotype" comments of an archive and returns 1 when one of them does not hold
int checkComments(const std::vector<std::string>& arguments) {
    bool verbose = false;
    std::vector<std::string> paths;
    for (const std::string& argument : arguments) {
        if (argument == "-v" || argument == "--verbose") verbose = true;
        else paths.push_back(argument);
    }
    if (paths.size() != 1) {
        std::cerr << USAGE;
        return 2;
    }
    const std::string& archivePath = paths.front();

    xmlDocPtr document = xmlReadFile(archivePath.c_str(), nullptr, XML_PARSE_NONET | XML_PARSE_HUGE);
    const xmlNode* root = document ? xmlDocGetRootElement(document) : nullptr;
    if (!isSrcElement(root, "unit")) {
        std::cerr << "check_stereotypes: " << archivePath << " is not a srcML archive\n";
        xmlFreeDoc(document);
        return 2;
    }

    std::vector<const xmlNode*> units;
    for (const xmlNode* child = root->children; child; child = child->next)
        if (isSrcElement(child, "unit")) units.push_back(child);
    if (units.empty()) units.push_back(root);

    Totals totals;
    for (const xmlNode* unit : units) checkUnit(unit, verbose, totals);
    xmlFreeDoc(document);

    if (totals.expectations == 0) {
        ++totals.problems;
        std::cout << archivePath << ": no \"// @stereotype\" expectation found\n";
    }

    std::cout << totals.expectations << " expectations: " << totals.held << " hold, "
              << totals.expectedFailures << " expected failures, " << totals.problems << " problems, "
              << totals.unchecked << " labeled elements without an expectation\n";
    return totals.problems == 0 ? 0 : 1;
}

// Appends a row for each labeled element of the archive in document order, and tells whether the file is a srcML archive
// A reader holds one labeled element at a time, since the document of a whole system would take gigabytes of memory
bool readRows(const char* archivePath, std::vector<std::string>& rows) {
    xmlTextReaderPtr reader = xmlReaderForFile(archivePath, nullptr, XML_PARSE_NONET | XML_PARSE_HUGE);
    if (!reader) return false;

    bool archive = false;
    std::string fileName;
    int status = 0;
    while ((status = xmlTextReaderRead(reader)) == 1) {
        if (xmlTextReaderNodeType(reader) != XML_READER_TYPE_ELEMENT) continue;

        const char* nameSpace = reinterpret_cast<const char*>(xmlTextReaderConstNamespaceUri(reader));
        const char* element   = reinterpret_cast<const char*>(xmlTextReaderConstLocalName(reader));
        const bool  src       = nameSpace && std::strcmp(nameSpace, SRC_NAMESPACE) == 0;
        const bool  typeScript = nameSpace && std::strcmp(nameSpace, TYPESCRIPT_NAMESPACE) == 0;
        if (xmlTextReaderDepth(reader) == 0) archive = src && std::strcmp(element, "unit") == 0;
        if (!src && !typeScript) continue;
        if (std::strcmp(element, "unit") == 0) {
            fileName = oneLine(attributeOf(reader, "filename", nullptr));
            continue;
        }

        const std::string stereotypes = attributeOf(reader, "stereotype", STEREOTYPE_NAMESPACE);
        if (stereotypes.empty()) continue;

        const std::string start = attributeOf(reader, "start", POSITION_NAMESPACE);
        rows.push_back(fileName + '\t' + start.substr(0, start.find(':')) + '\t' + element + '\t' +
                       nameOf(xmlTextReaderExpand(reader)) + '\t' + stereotypes);
    }
    xmlFreeTextReader(reader);
    return status == 0 && archive;
}

// Writes the rows to a file, one per line, and tells whether the file was written
bool writeTable(const char* tablePath, const std::vector<std::string>& rows) {
    std::ofstream table(tablePath, std::ios::binary);
    for (const std::string& row : rows) table << row << '\n';
    table.close();
    return !table.fail();
}

// Reads the rows of a table, where a line may end with a carriage return, and tells whether the file was read
bool readTable(const char* tablePath, std::vector<std::string>& rows) {
    std::ifstream table(tablePath, std::ios::binary);
    if (!table) return false;
    for (std::string row; std::getline(table, row);) {
        if (!row.empty() && row.back() == '\r') row.pop_back();
        rows.push_back(row);
    }
    return !table.bad();
}

using keyPositions = std::unordered_map<std::string, std::vector<std::size_t>>;

// Gets the columns of a row before its stereotypes, which name the element
std::string keyOf(const std::string& row) {
    return row.substr(0, row.rfind('\t'));
}

// Gets the positions of the rows of a table by key, each list in increasing order
keyPositions positionsOf(const std::vector<std::string>& rows) {
    keyPositions positions;
    for (std::size_t i = 0; i < rows.size(); ++i) positions[keyOf(rows[i])].push_back(i);
    return positions;
}

// Gets the first position at or after a start that holds a row with the key, or NOT_FOUND
std::size_t nextPosition(const keyPositions& positions, const std::string& key, std::size_t start) {
    const auto found = positions.find(key);
    if (found == positions.end()) return NOT_FOUND;
    const auto next = std::lower_bound(found->second.begin(), found->second.end(), start);
    return next == found->second.end() ? NOT_FOUND : *next;
}

// Prints the first rows that only the reference (-) or only the output (+) holds and returns the number of such rows
// After the rows of elements that one table lacks, the walk resumes at the nearest element of both tables, so an added row shifts no other row
std::size_t printDifferences(const std::vector<std::string>& reference, const std::vector<std::string>& output) {
    const keyPositions inReference = positionsOf(reference);
    const keyPositions inOutput    = positionsOf(output);
    std::size_t removed = 0;
    std::size_t added   = 0;
    auto report = [&removed, &added](char side, std::size_t position, const std::string& row) {
        if (side == '-') ++removed;
        else             ++added;
        if (removed + added == 1) std::cout << "The first rows that only the reference (-) or only the output (+) holds, with their lines:\n";
        if (removed + added <= SHOWN_DIFFERENCES) std::cout << side << ' ' << position + 1 << ": " << row << '\n';
    };

    std::size_t r = 0;
    std::size_t o = 0;
    while (r < reference.size() || o < output.size()) {
        const bool both = r < reference.size() && o < output.size();
        if (both && keyOf(reference[r]) == keyOf(output[o])) {
            if (reference[r] != output[o]) {
                report('-', r, reference[r]);
                report('+', o, output[o]);
            }
            ++r;
            ++o;
            continue;
        }
        const std::size_t inOutputAgain    = r < reference.size() ? nextPosition(inOutput, keyOf(reference[r]), o) : NOT_FOUND;
        const std::size_t inReferenceAgain = o < output.size()    ? nextPosition(inReference, keyOf(output[o]), r) : NOT_FOUND;
        if (inOutputAgain != NOT_FOUND && (inReferenceAgain == NOT_FOUND || inOutputAgain - o <= inReferenceAgain - r)) {
            for (; o < inOutputAgain; ++o) report('+', o, output[o]);
        }
        else if (inReferenceAgain != NOT_FOUND) {
            for (; r < inReferenceAgain; ++r) report('-', r, reference[r]);
        }
        else {
            if (r < reference.size()) {
                report('-', r, reference[r]);
                ++r;
            }
            if (o < output.size()) {
                report('+', o, output[o]);
                ++o;
            }
        }
    }
    if (removed + added > 0)
        std::cout << removed << " of the " << reference.size() << " rows of the reference and " << added << " of the "
                  << output.size() << " rows of the output differ\n";
    return removed + added;
}

// Writes the table of the labeled elements of an archive and, given a reference table, returns 1 when the two tables differ
int compareTables(const std::vector<std::string>& arguments) {
    if (arguments.size() != 2 && arguments.size() != 3) {
        std::cerr << USAGE;
        return 2;
    }
    const char* archivePath = arguments[0].c_str();
    const char* tablePath   = arguments[1].c_str();

    std::vector<std::string> rows{HEADER};
    if (!readRows(archivePath, rows)) {
        std::cerr << "check_stereotypes: " << archivePath << " is not a complete srcML archive\n";
        return 2;
    }
    if (!writeTable(tablePath, rows)) {
        std::cerr << "check_stereotypes: cannot write " << tablePath << '\n';
        return 2;
    }
    if (arguments.size() == 2) return 0;

    std::vector<std::string> reference;
    if (!readTable(arguments[2].c_str(), reference)) {
        std::cerr << "check_stereotypes: cannot read " << arguments[2] << '\n';
        return 2;
    }
    return printDifferences(reference, rows) == 0 ? 0 : 1;
}

}

// Runs the mode that the first argument names, comments or table, with the arguments after it
int main(int argc, char* argv[]) {
    const std::string mode = argc > 1 ? argv[1] : "";
    const std::vector<std::string> arguments(argv + std::min(argc, 2), argv + argc);
    if (mode == "comments") return checkComments(arguments);
    if (mode == "table")    return compareTables(arguments);
    std::cerr << USAGE;
    return 2;
}
