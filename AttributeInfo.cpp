// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file AttributeInfo.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "AttributeInfo.hpp"

std::ostream& operator<<(std::ostream& out, const AttributeInfo& m) {
    out << "(" << m.getType() << " : " << m.getName() << ") ";
    return out;
}
