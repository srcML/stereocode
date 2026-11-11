// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file txt_report.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "output/txt_report.hpp"
#include "output/output_names.hpp"

#include <algorithm>
#include <cctype>

namespace {
    // Width of the first column, as in the text report of the ICSME 2024 paper
    constexpr std::size_t columnWidth = 70;

    // Line after the block of each type and after the rows of the free functions
    const std::string separator = std::string(2 * columnWidth, '-') + '\n';

    // Gets a text on one line, with each run of white space inside it as one space, as in the CSV reports
    //
    std::string oneLine(const std::string& text) {
        std::string line;
        bool afterSpace = false;
        for (char character : text) {
            if (std::isspace(static_cast<unsigned char>(character))) {
                afterSpace = true;
                continue;
            }
            if (afterSpace && !line.empty()) line += ' ';
            afterSpace = false;
            line += character;
        }
        return line;
    }

    // Gets the number of characters of a UTF-8 text, which counts every byte but the continuation bytes
    //
    std::size_t characterCount(const std::string& text) {
        return static_cast<std::size_t>(std::count_if(text.begin(), text.end(), [](char byte) { return (static_cast<unsigned char>(byte) & 0xC0) != 0x80; }));
    }

    // Gets a row of two columns, where the first column fills 70 characters and a longer text keeps one space before the second column
    //
    std::string row(const std::string& first, const std::string& second) {
        const std::string firstColumn = oneLine(first);
        const std::size_t length = characterCount(firstColumn);
        return firstColumn + std::string(length < columnWidth ? columnWidth - length : 1, ' ') + oneLine(second) + '\n';
    }

    // Gets the name of a function without its type, namespaces, and generic arguments, as in the CSV reports
    //
    std::string functionName(const functionModel& function) {
        return function.getName().empty() ? "N/A" : function.getName()[3];
    }
}

// Creates the reports beside the srcML output archive, so that a report that cannot be written stops the run before any work
//
txtReports::txtReports(const std::string& outputFile, bool freeFunctions) : typeReport{reportNamesFor(outputFile, ".txt").types} {
    if (freeFunctions) {
        freeFunctionReport.emplace(reportNamesFor(outputFile, ".txt").freeFunctions);
        freeFunctionReport->write(row("Free Function Name:", "Free Function Stereotype:"));
    }
}

// Writes a block for each type with its name and stereotypes, a row for each of its methods, and a line of dashes
//
void txtReports::writeTypes(const std::vector<const typeModel*>& types) {
    for (const typeModel* type : types) {
        const std::string typeName = type->getName().empty() ? "N/A" : type->getName()[0];
        std::string block = row("Class Name:", "Class Stereotype:") + row(typeName, type->getStereotypesString()) + '\n' +
                            row("Method Name:", "Method Stereotype:");
        for (const functionModel& method : type->getMethods()) block += row(functionName(method), method.getStereotypesString());
        typeReport.write(block + separator);
    }
}

// Writes a row for each free function, when the reports include the free functions
//
void txtReports::writeFreeFunctions(const std::vector<functionModel>& functions) {
    if (!freeFunctionReport) return;

    for (const functionModel& function : functions) freeFunctionReport->write(row(functionName(function), function.getStereotypesString()));
}

// Ends the report of the free functions with a line of dashes and completes the reports in their temporary files
//
void txtReports::close() {
    typeReport.close();
    if (freeFunctionReport) {
        freeFunctionReport->write(separator);
        freeFunctionReport->close();
    }
}

// Moves the complete reports to their files
//
void txtReports::publish() {
    typeReport.publish();
    if (freeFunctionReport) freeFunctionReport->publish();
}
