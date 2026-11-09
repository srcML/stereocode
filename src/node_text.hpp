// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file node_text.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef NODE_TEXT_HPP
#define NODE_TEXT_HPP

#include <libxml/tree.h>

#include <string>

// The text of a node of a srcML document, as libsrcml gives it for the unit of a query result

std::string sourceOf            (const xmlNode*);
std::string srcmlOf             (xmlNode*);
std::string unparsedSourceOf    (const xmlNode*);

#endif
