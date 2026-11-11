// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file csv_report.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "output/csv_report.hpp"
#include "helper_functions.hpp"
#include "output/output_names.hpp"
#include "model/qualified_name.hpp"

#include <utility>

namespace {
    const char* const header =
        "type_name,type_stereotype,function_name,function_stereotype\n";

    const char* const verboseHeader =
        "type_file_name,type_name,type_stereotype,type_parents,type_inherited_parents,type_unknown_parents,type_inherited_function_signatures,"
        "function_file_name,function_name,function_stereotype,function_line_number,function_signature,function_unit_language,function_parameters_list,"
        "function_return_type,function_specifiers,function_external_calls,function_internal_calls,function_attributes,function_field_used,"
        "function_fields_modified,function_calls_on_fields\n";

    using columns = std::vector<std::string>;

    // Gets a text, or N/A for an empty text
    //
    std::string orNotApplicable(const std::string& text) {
        return text.empty() ? "N/A" : text;
    }

    // Gets the type columns of a type, or N/A in every column for the rows of free functions
    // The file of a type is the file of its first part, and the verbose name of a nested type holds the types around it without generic arguments
    //
    columns typeColumns(const typeModel* type, bool verbose) {
        if (!type) return columns(verbose ? 7 : 2, "N/A");

        const std::vector<std::string>& name = type->getName();
        if (!verbose) return {name.empty() ? "N/A" : name[0], type->getStereotypesString()};

        const bool inheritsSignatures = !type->getInheritedMethodSignatures().empty() || !type->getInheritedDeclMethodSignatures().empty();
        return {type->getPartFileNames().empty() ? "N/A" : orNotApplicable(type->getPartFileNames().front()),
                name.empty() ? "N/A" : withoutGenericArguments(name[1]),
                type->getStereotypesString(),
                type->getParentNames().empty() ? "N/A" : type->getParentsString(false),
                type->getInheritedParentNames().empty() ? "N/A" : type->getParentsString(true),
                type->hasUnknownParent() ? "True" : "False",
                inheritsSignatures ? type->getInheritedMethodSignaturesString() : "N/A"};
    }

    // Gets the function columns of a function, or N/A in every column for the row of a type without methods
    // The name is the name of the function without its type, namespaces, and generic arguments, as in the signatures
    //
    columns functionColumns(const functionModel* function, bool verbose) {
        if (!function) return columns(verbose ? 15 : 2, "N/A");

        const std::string name = function->getName().empty() ? "N/A" : function->getName()[3];
        if (!verbose) return {name, function->getStereotypesString()};

        const std::pair<std::string, std::string>& signature = function->getNameSignature();
        const bool hasAttributes = function->getUnitLanguage() != "C++" && !function->getAttributesOrAnnotations().empty();
        return {orNotApplicable(function->getFileName()),
                name,
                function->getStereotypesString(),
                function->getLineNumber() != -1 ? std::to_string(function->getLineNumber()) : "N/A",
                signature.first.empty() ? "N/A" : signature.first + signature.second,
                orNotApplicable(function->getUnitLanguage()),
                orNotApplicable(function->getParametersList()),
                orNotApplicable(function->getReturnType().getType()),
                function->getSpecifiers().empty() ? "N/A" : function->getSpecifiersString(),
                function->getExternalCalls().empty() ? "N/A" : function->getCallsString(true),
                function->getFunctionCalls().empty() ? "N/A" : function->getCallsString(false),
                hasAttributes ? function->getAttributesOrAnnotationsString() : "N/A",
                function->isFieldUsed() ? "True" : "False",
                function->getFieldsModifiedCount() > 0 ? "True" : "False",
                function->getMethodCalls().empty() ? "False" : "True"};
    }

    // Writes one row with the type columns and then the function columns
    //
    void writeRow(stagedFile& report, const columns& typePart, const columns& functionPart) {
        std::string row;
        for (const columns* part : {&typePart, &functionPart}) {
            for (const std::string& column : *part) {
                if (!row.empty()) row += ',';
                row += HELPERS::escapeCSV(column);
            }
        }
        report.write(row + '\n');
    }
}

// Creates the reports beside the srcML output archive, so that a report that cannot be written stops the run before any work
//
csvReports::csvReports(const std::string& outputFile, bool freeFunctions, bool verbose_) :
    verbose{verbose_}, typeReport{reportNamesFor(outputFile, ".csv").types} {
    typeReport.write(verbose ? verboseHeader : header);
    if (freeFunctions) {
        freeFunctionReport.emplace(reportNamesFor(outputFile, ".csv").freeFunctions);
        freeFunctionReport->write(verbose ? verboseHeader : header);
    }
}

// Writes a row for each method of each type, or one row for a type without methods, in the order of the types
//
void csvReports::writeTypes(const std::vector<const typeModel*>& types) {
    for (const typeModel* type : types) {
        const columns typePart = typeColumns(type, verbose);
        if (type->getMethods().empty()) writeRow(typeReport, typePart, functionColumns(nullptr, verbose));
        for (const functionModel& method : type->getMethods()) writeRow(typeReport, typePart, functionColumns(&method, verbose));
    }
}

// Writes a row for each free function, when the reports include the free functions
//
void csvReports::writeFreeFunctions(const std::vector<functionModel>& functions) {
    if (!freeFunctionReport) return;

    const columns typePart = typeColumns(nullptr, verbose);
    for (const functionModel& function : functions) writeRow(*freeFunctionReport, typePart, functionColumns(&function, verbose));
}

// Completes the reports in their temporary files
//
void csvReports::close() {
    typeReport.close();
    if (freeFunctionReport) freeFunctionReport->close();
}

// Moves the complete reports to their files
//
void csvReports::publish() {
    typeReport.publish();
    if (freeFunctionReport) freeFunctionReport->publish();
}
