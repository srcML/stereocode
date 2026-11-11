// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file staged_file.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef STAGED_FILE_HPP
#define STAGED_FILE_HPP

#include <cstdio>
#include <string>

// An output file written under a temporary name, which replaces the file only when published after its close
// Thus, a run that fails before it publishes its outputs leaves no partial output, and an output may replace the input of the run
//
class stagedFile {
public:
    explicit            stagedFile      (const std::string&);
                        ~stagedFile     ();
                        stagedFile      (const stagedFile&)   = delete;
    stagedFile&         operator=       (const stagedFile&)   = delete;

    std::FILE*          get             () const    { return file; }       // Open temporary file
    void                write           (const std::string&);
    void                close           ();
    void                publish         ();
private:
    std::string         fileName;               // Output file
    std::string         temporaryFileName;      // File written until it is published
    std::FILE*          file{nullptr};          // Open temporary file, null after the close
    bool                published{false};       // The temporary file replaced the output file
};

#endif
