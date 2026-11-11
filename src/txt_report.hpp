// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file txt_report.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef TXT_REPORT_HPP
#define TXT_REPORT_HPP

#include "staged_file.hpp"
#include "type_model.hpp"

#include <optional>
#include <string>
#include <vector>

// The text reports of the types and of the free functions in the layout of the ICSME 2024 paper, which replace their files only when complete
// Each type has a block with its name and stereotypes and then its methods, and the blocks follow the order of the input archive
//
class txtReports {
public:
                                txtReports          (const std::string&, bool);

    void                        writeTypes          (const std::vector<const typeModel*>&);
    void                        writeFreeFunctions  (const std::vector<functionModel>&);
    void                        close               ();
    void                        publish             ();
private:
    stagedFile                  typeReport;             // Blocks of the types and their methods
    std::optional<stagedFile>   freeFunctionReport;     // Rows of the free functions, when they are labeled
};

#endif
