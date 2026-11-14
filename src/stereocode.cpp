// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereocode.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "CLI11.hpp"
#include "archive/source_language.hpp"
#include "options.hpp"
#include "output/output_names.hpp"
#include "stereotypes_analyzer.hpp"

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
    settings.threads = defaultThreads(std::thread::hardware_concurrency());

    CLI::App app{"Stereocode classifies methods, free functions, and types (e.g., classes) of C, C++, C#, Java, Python, JavaScript, and TypeScript code by their stereotypes.\n"};
    app.footer("The files of -p, -c, -s, and -q hold one entry per line. A line " + sectionHeadersText() + " limits the entries after it to that language. "
               "A line that starts with # is a comment.");

    // The descriptions follow the options table of README.md
    app.add_option("input-archive",            settings.inputFile,               "srcML archive to analyze, a file name that ends in .xml")->required()->check(CLI::ExistingFile);
    app.add_option("-o,--output-file",         settings.outputFile,              "Output archive (default <input>.stereotypes.xml)");
    app.add_option("-p,--primitive-file",      settings.primitivesFile,          "File of extra types that count as primitive (e.g., a typedef of int)")->check(CLI::ExistingFile);
    app.add_option("-c,--call-file",           settings.callsFile,               "File of extra calls to ignore (e.g., a logging function)")->check(CLI::ExistingFile);
    app.add_option("-s,--specifier-file",      settings.specifiersFile,          "File of extra specifiers to strip from types (e.g., an export macro)")->check(CLI::ExistingFile);
    app.add_option("-q,--query-file",          settings.queryMethodsFile,        "File of extra methods whose calls leave the object unchanged (e.g., size)")->check(CLI::ExistingFile);
    app.add_option("-l,--large-class",         settings.largeClassThreshold,     "Method count above which a type can be a large-class (default mean plus standard deviation over all types)")
        ->check(countUpTo(std::numeric_limits<int>::max(), "POSITIVE"));
    app.add_option("-j,--threads",             settings.threads,                 "Number of threads (default all available up to 8, at most 256)")
        ->check(countUpTo(maxThreads, "1 to 256"));
    app.add_flag  ("-f,--free-function",       settings.freeFunctions,           "Stereotypes free functions and static methods");
    app.add_flag  ("-i,--interface",           settings.interfaces,              "Stereotypes interfaces (C#, Java, and TypeScript)");
    app.add_flag  ("-u,--union",               settings.unions,                  "Stereotypes unions (C and C++)");
    app.add_flag  ("-e,--enum",                settings.enums,                   "Stereotypes enums (Java)");
    app.add_flag  ("-t,--struct",              settings.structs,                 "Stereotypes structs (C, C++, and C#)");
    app.add_flag  ("-r,--csv-report",          settings.csvReport,               "Writes a CSV report of the stereotypes");
    app.add_flag  ("-x,--txt-report",          settings.txtReport,               "Writes a text report of the stereotypes");
    app.add_flag  ("-b,--verbose",             settings.verboseReport,           "Writes a detailed CSV report (e.g., the calls of each function) and prints the settings in use");
    app.add_flag  ("-d,--no-srcml-output,--srcmlOutput", settings.disableSrcmlOutput, "Writes no output archive, only the reports of -r, -x, or -b");
    app.add_flag  ("-m,--comment",             settings.comments,                "Inserts a // @stereotype <labels> comment (# @stereotype <labels> in Python) before each stereotyped element");
    app.set_version_flag("-v,--version",       "Stereocode v" STEREOCODE_VERSION, "Prints the version");
    app.set_help_flag("-h,--help",             "Prints the options");

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
