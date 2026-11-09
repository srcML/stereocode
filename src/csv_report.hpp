// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file csv_report.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef CSV_REPORT_HPP
#define CSV_REPORT_HPP

#include "staged_file.hpp"
#include "type_model.hpp"

#include <optional>
#include <string>
#include <vector>

// The CSV reports of the types and of the free functions, which replace their files only when complete
// A type without methods has a row of its own, and the rows follow the order of the input archive
//
class csvReports {
public:
                                csvReports          (const std::string&, bool, bool);

    void                        writeTypes          (const std::vector<const typeModel*>&);
    void                        writeFreeFunctions  (const std::vector<functionModel>&);
    void                        close               ();
    void                        publish             ();
private:
    bool                        verbose;                // Write the facts of each type and function
    stagedFile                  typeReport;             // Rows of the types and their methods
    std::optional<stagedFile>   freeFunctionReport;     // Rows of the free functions, when they are labeled
};

#endif
