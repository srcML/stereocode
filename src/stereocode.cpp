// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereocode.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "CLI11.hpp"
#include "options.hpp"
#include "output/output_names.hpp"
#include "stereotypes_analyzer.hpp"

#include <algorithm>
#include <charconv>
#include <exception>
#include <iostream>
#include <limits>
#include <string>
#include <system_error>
#include <thread>

namespace {
    // Each worker keeps a query engine and units of its own in memory, and the limit keeps a mistyped count from exhausting the memory
    constexpr unsigned maxThreads = 256;

    // Accepts a whole number from 1 to a limit, with a message that tells a word apart from a number out of range
    //
    CLI::Validator countUpTo(long long limit, const std::string& description) {
        return CLI::Validator([limit](std::string& input) {
            long long value = 0;
            const char* end = input.data() + input.size();
            const auto [last, error] = std::from_chars(input.data(), end, value);
            if (error == std::errc::invalid_argument || last != end) return input + " is not a whole number";
            if (error == std::errc::result_out_of_range || value < 1 || value > limit) return input + " is not from 1 to " + std::to_string(limit);
            return std::string();
        }, description);
    }
}

int main (int argc, char const *argv[]) try {
    options settings;
    settings.threads = std::clamp(std::thread::hardware_concurrency(), 1u, maxThreads);

    CLI::App app{"Stereocode: Determines function (method and free functions) and type (class, struct, interface, union, enum) stereotypes\nSupports C, C++, C#, and Java\n" };

    app.add_option("input-archive",            settings.inputFile,               "srcML archive to analyze, a file name that ends in .xml")->required()->check(CLI::ExistingFile);
    app.add_option("-o,--output-file",         settings.outputFile,              "srcML output archive with the stereotypes (default <input>.stereotypes.xml), whose name the CSV and text reports share");
    app.add_option("-p,--primitive-file",      settings.primitivesFile,          "File of primitive types to add to the default set, one per line, with optional [C++], [C#], and [Java] sections")->check(CLI::ExistingFile);
    app.add_option("-c,--call-file",           settings.callsFile,               "File of calls to ignore in addition to the default set, one per line, with optional [C++], [C#], and [Java] sections")->check(CLI::ExistingFile);
    app.add_option("-s,--specifier-file",      settings.specifiersFile,          "File of type specifiers to remove from types in addition to the default set, one per line, with optional [C++], [C#], and [Java] sections")->check(CLI::ExistingFile);
    app.add_option("-q,--query-file",          settings.queryMethodsFile,        "File of query methods, whose calls on data members, parameters, and globals read them, in addition to the default set, one per line, with optional [C++], [C#], and [Java] sections")->check(CLI::ExistingFile);
    app.add_option("-l,--large-class",         settings.largeClassThreshold,     "Number of methods that a large type exceeds (e.g., large-class) (default = mean plus standard deviation of the methods per class of the run)")
        ->check(countUpTo(std::numeric_limits<int>::max(), "POSITIVE"));
    app.add_option("-j,--threads",             settings.threads,                 "Number of worker threads (default = as many as the machine has, at most 256)")
        ->check(countUpTo(maxThreads, "1 to 256"));
    app.add_flag  ("-f,--free-function",       settings.freeFunctions,           "Identify stereotypes for free functions, including static methods (C, C++, C#, and Java)");
    app.add_flag  ("-i,--interface",           settings.interfaces,              "Identify stereotypes for interfaces (C# and Java)");
    app.add_flag  ("-u,--union",               settings.unions,                  "Identify stereotypes for unions (C and C++)");
    app.add_flag  ("-e,--enum",                settings.enums,                   "Identify stereotypes for enums (Java)");
    app.add_flag  ("-t,--struct",              settings.structs,                 "Identify stereotypes for structs (C, C++, and C#)");
    app.add_flag  ("-r,--csv-report",          settings.csvReport,               "Write the CSV report of the types and, with -f, the CSV report of the free functions");
    app.add_flag  ("-x,--txt-report",          settings.txtReport,               "Write the text report of the types and, with -f, the text report of the free functions, as in the ICSME 2024 paper");
    app.add_flag  ("-b,--verbose",             settings.verboseReport,           "Write the CSV reports with the facts of each function, and print the large-class threshold, the primitive types, ignored calls, specifiers, and query methods (implies -r)");
    app.add_flag  ("-d,--no-srcml-output,--srcmlOutput", settings.disableSrcmlOutput, "Write no srcML output archive (requires -r, -b, or -x)");
    app.add_flag  ("-m,--comment",             settings.comments,                "Insert the comment // @stereotype <labels> before each labeled type, method, and free function of the srcML output archive, as in the ICSME 2024 paper (excludes -d)");
    app.set_version_flag("-v,--version",       "Stereocode v" STEREOCODE_VERSION, "Display version information");

    CLI11_PARSE(app, argc, argv);

    // Validate input file extension
    if (settings.inputFile.size() < 4 || settings.inputFile.substr(settings.inputFile.size() - 4) != ".xml") {
        std::cerr << "Error: Input archive must have a valid .xml extension.\n";
        return 1;
    }

    // The comments belong to the srcML output archive, which -d disables
    if (settings.comments && settings.disableSrcmlOutput) {
        std::cerr << "Error: The --comment (-m) flag writes the comments into the srcML output archive, which --no-srcml-output (-d) disables.\n";
        return 1;
    }

    // Ensure that if srcML output is disabled, a report is requested
    if (settings.disableSrcmlOutput && !settings.csvReport && !settings.verboseReport && !settings.txtReport) {
        std::cerr << "Error: The --no-srcml-output (-d) flag disables the srcML output archive. "
                  << "You must specify --csv-report (-r), --verbose (-b), or --txt-report (-x) to generate output.\n";
        return 1;
    }

    // The CSV and text reports take their names from the output archive, also with -d
    if (settings.outputFile.empty()) settings.outputFile = defaultOutputFile(settings.inputFile);

    // Compute stereotypes
    stereotypesAnalyzer analyzer(settings);
    analyzer.run();

    return 0;
}
catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << '\n';
    return 1;
}
