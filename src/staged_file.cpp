// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file staged_file.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "staged_file.hpp"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <system_error>

// Creates the temporary file, so that an output that cannot be written stops the run before any work
//
stagedFile::stagedFile(const std::string& fileName_) : fileName{fileName_}, temporaryFileName{fileName_ + ".tmp"} {
    file = std::fopen(temporaryFileName.c_str(), "wb");
    if (!file) throw std::runtime_error("cannot create " + fileName + ": " + std::strerror(errno));
}

// Removes the temporary file of an output that was never published
//
stagedFile::~stagedFile() {
    if (file) std::fclose(file);
    if (!published) std::remove(temporaryFileName.c_str());
}

// Appends text to the temporary file
//
void stagedFile::write(const std::string& text) {
    if (std::fwrite(text.data(), 1, text.size(), file) != text.size())
        throw std::runtime_error("cannot write " + fileName + ": " + std::strerror(errno));
}

// Closes the temporary file, so that a failed write stops the run before any output replaces its file
//
void stagedFile::close() {
    const bool written = std::fflush(file) == 0 && !std::ferror(file);
    const bool closed = std::fclose(file) == 0;
    file = nullptr;
    if (!written || !closed) throw std::runtime_error("cannot write " + fileName);
}

// Moves the closed temporary file to the output file
//
void stagedFile::publish() {
    std::error_code error;
    std::filesystem::rename(temporaryFileName, fileName, error);
    if (error) throw std::runtime_error("cannot move " + temporaryFileName + " to " + fileName + ": " + error.message());
    published = true;
}
