// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file srcml_handles.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "srcml_handles.hpp"

#include <stdexcept>
#include <string>

// Creates an empty archive
//
archivePtr createArchive() {
    archivePtr archive(srcml_archive_create());
    if (!archive) throw std::runtime_error("cannot create a srcML archive");
    return archive;
}

// Throws with the failed action in the message when a libsrcml call does not return SRCML_STATUS_OK
//
void checkStatus(int status, const char* action) {
    if (status != SRCML_STATUS_OK)
        throw std::runtime_error(std::string(action) + " (srcML status " + std::to_string(status) + ")");
}
