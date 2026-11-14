// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file overloads.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/overloads.hpp"

// Gets the positions of the overloads of a name that take a number of arguments, or of every overload when none takes that number
// The fallback keeps a call whose arguments the markup counts in another way, as a default value declared elsewhere, among the overloads of its name
//
std::vector<std::size_t> overloadsTaking(const std::vector<overload>& overloads, int arguments) {
    std::vector<std::size_t> taking;
    std::vector<std::size_t> all;
    for (const overload& candidate : overloads) {
        all.push_back(candidate.position);
        if (takes(candidate.arguments, arguments)) taking.push_back(candidate.position);
    }
    return taking.empty() ? all : taking;
}
