// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file zero_processors.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

// Preloaded by the zero_processors test: glibc counts the processors with these functions,
// and std::thread::hardware_concurrency() returns 0 when they report none

// Reports no online processors
//
extern "C" int get_nprocs() { return 0; }

// Reports no configured processors
//
extern "C" int get_nprocs_conf() { return 0; }
