// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file output_archive.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef OUTPUT_ARCHIVE_HPP
#define OUTPUT_ARCHIVE_HPP

#include "srcml_handles.hpp"
#include "staged_file.hpp"

#include <string>

class annotatedUnit;

// The srcML output archive with the stereotype attributes, which replaces the output file only when complete
//
class outputArchive {
public:
                        outputArchive   (const std::string&, srcml_archive*);

    void                write           (const annotatedUnit&);
    void                close           ();
    void                publish         ();
private:
    stagedFile          file;                   // Temporary file until it is published
    archivePtr          archive;                // Clone of the input archive, opened for writing to the file, which it closes before the file closes
};

#endif
