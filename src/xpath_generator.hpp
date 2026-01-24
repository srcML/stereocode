// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file xpath_generator.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef XPATH_GENERATOR_HPP
#define XPATH_GENERATOR_HPP

#include <string>
#include <unordered_map>

class XPathGenerator {
public:
    void               generateXPathList ();
    const std::string& getXPathList      (const std::string&, const std::string&);
private:
    std::unordered_map
    <std::string, std::unordered_map
    <std::string, std::string>> xpathList; // language -> <key -> xpath>
};

#endif
