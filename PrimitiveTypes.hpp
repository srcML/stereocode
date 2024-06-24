// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file PrimitiveTypes.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef PRIMITIVETYPES_HPP
#define PRIMITIVETYPES_HPP

#include <string>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <iostream>

class primitiveTypes {
public:
    bool                 isPrimitive             (const std::string&, const std::string&);

    void                 addPrimitive            (const std::string&);
    void                 createPrimitiveList     ();
    void                 outputPrimitives        ();

    friend std::istream& operator>>              (std::istream&, primitiveTypes&);
private:
    std::unordered_map<std::string, std::unordered_set<std::string>>     ptypes;         // List of primitives
    std::unordered_set<std::string>                                      userTypes;      // List of user-defined primitives
};

#endif
