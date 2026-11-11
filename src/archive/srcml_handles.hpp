// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file srcml_handles.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef SRCML_HANDLES_HPP
#define SRCML_HANDLES_HPP

#include <srcml.h>

#include <memory>

// Closes and frees an archive
//
struct archiveDeleter {
    void operator()(srcml_archive* archive) const { srcml_archive_close(archive); srcml_archive_free(archive); }
};

// Frees a unit
//
struct unitDeleter {
    void operator()(srcml_unit* unit) const { srcml_unit_free(unit); }
};

// Frees a transform result with the units it holds
//
struct transformResultDeleter {
    void operator()(srcml_transform_result* result) const { srcml_transform_free(result); }
};

using archivePtr         = std::unique_ptr<srcml_archive, archiveDeleter>;
using unitPtr            = std::unique_ptr<srcml_unit, unitDeleter>;
using transformResultPtr = std::unique_ptr<srcml_transform_result, transformResultDeleter>;

archivePtr createArchive ();
void       checkStatus   (int, const char*);

#endif
