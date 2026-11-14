// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file expression_tokens.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/expression_tokens.hpp"
#include "archive/node_text.hpp"
#include "archive/srcml_elements.hpp"

// Gets the element children of an expression
//
std::vector<const xmlNode*> tokensOf(const xmlNode* expression) {
    std::vector<const xmlNode*> tokens;
    for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(expression)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child)))
        tokens.push_back(child);
    return tokens;
}

// Removes the parentheses that enclose a whole range of tokens, which srcML marks as operators
//
void removeEnclosingParentheses(const std::vector<const xmlNode*>& tokens, std::size_t& begin, std::size_t& end) {
    while (end - begin >= 2 && isOperator(tokens[begin], "(") && isOperator(tokens[end - 1], ")")) {
        int depth = 0;
        std::size_t close = begin;
        for (; close < end; ++close) {
            if      (isOperator(tokens[close], "(")) ++depth;
            else if (isOperator(tokens[close], ")") && --depth == 0) break;
        }
        if (close != end - 1) return;
        ++begin;
        --end;
    }
}

// Splits a range of tokens at an operator outside parentheses, such as or, into the ranges between its uses
//
tokenRanges splitAt(const std::vector<const xmlNode*>& tokens, std::size_t begin, std::size_t end, std::string_view op) {
    tokenRanges parts;
    int depth = 0;
    std::size_t start = begin;
    for (std::size_t i = begin; i < end; ++i) {
        if      (isOperator(tokens[i], "(")) ++depth;
        else if (isOperator(tokens[i], ")")) --depth;
        else if (depth == 0 && isOperator(tokens[i], op)) {
            parts.emplace_back(start, i);
            start = i + 1;
        }
    }
    parts.emplace_back(start, end);
    return parts;
}

// Gets the texts of the operators of a range of tokens outside its parentheses, such as == of (a + b) == c
//
std::vector<std::string> operatorsOutsideParentheses(const std::vector<const xmlNode*>& tokens, std::size_t begin, std::size_t end) {
    std::vector<std::string> operators;
    int depth = 0;
    for (std::size_t i = begin; i < end; ++i) {
        if      (isOperator(tokens[i], "(")) ++depth;
        else if (isOperator(tokens[i], ")")) --depth;
        else if (depth == 0 && isSrcElement(tokens[i], "operator")) operators.push_back(sourceOf(tokens[i]));
    }
    return operators;
}
