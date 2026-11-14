// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file expression_tokens.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef EXPRESSION_TOKENS_HPP
#define EXPRESSION_TOKENS_HPP

#include <libxml/tree.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// The element children of an expression as a sequence of tokens, which the rules of Boolean forms read in ranges

using tokenRanges = std::vector<std::pair<std::size_t, std::size_t>>;

std::vector<const xmlNode*> tokensOf                    (const xmlNode*);
void                        removeEnclosingParentheses  (const std::vector<const xmlNode*>&, std::size_t&, std::size_t&);
tokenRanges                 splitAt                     (const std::vector<const xmlNode*>&, std::size_t, std::size_t, std::string_view);
std::vector<std::string>    operatorsOutsideParentheses (const std::vector<const xmlNode*>&, std::size_t, std::size_t);

#endif
