// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file XPathBuilder.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef XPATHBUILDER_HPP
#define XPATHBUILDER_HPP

#include <string>
#include <unordered_map>

class XPathBuilder {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> xpathTable;

public:
          void         generateXpath ();

    const std::string& getXpath      (const std::string&, const std::string&);
};

#endif
