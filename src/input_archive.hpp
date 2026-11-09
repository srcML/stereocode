// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file input_archive.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef INPUT_ARCHIVE_HPP
#define INPUT_ARCHIVE_HPP

#include "srcml_handles.hpp"

#include <string>

archivePtr openInputArchive (const std::string&);
void       checkWellFormed  (const std::string&);

#endif
