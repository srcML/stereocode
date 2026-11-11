// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file output_archive.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "output_archive.hpp"
#include "annotated_unit.hpp"

#include <stdexcept>

// Creates the temporary file and opens a clone of the input archive, which keeps its namespaces, for writing to it
//
outputArchive::outputArchive(const std::string& fileName, srcml_archive* inputArchive) : file{fileName} {
    archive.reset(srcml_archive_clone(inputArchive));
    if (!archive || srcml_archive_write_open_FILE(archive.get(), file.get()) != SRCML_STATUS_OK)
        throw std::runtime_error("cannot open the output archive " + fileName + " for writing");
}

// Appends a unit to the archive
//
void outputArchive::write(const annotatedUnit& unit) {
    checkStatus(srcml_archive_write_unit(archive.get(), unit.get()), "cannot write a unit to the output archive");
}

// Completes the archive in its temporary file
//
void outputArchive::close() {
    archive.reset();
    file.close();
}

// Moves the complete archive to the output file
//
void outputArchive::publish() {
    file.publish();
}
