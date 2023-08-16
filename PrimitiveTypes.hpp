// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file PrimitiveTypes.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef PRIMITIVETYPES_HPP
#define PRIMITIVETYPES_HPP

#include <iostream>
#include <set>

class primitiveTypes {
public:
    primitiveTypes() : language("") {};

    bool isPrimitive(const std::string&) const;
    void addPrimitive(const std::string&);
    void setLanguage(const std::string&);

    friend std::ostream& operator<<(std::ostream&, const primitiveTypes&);
    friend std::istream& operator>>(std::istream&, primitiveTypes&);

private:
    std::string                 language;      // Language: "C++", "C#", "Java", "C"
    std::set<std::string>       ptypes;        // List of language primitives
    std::set<std::string>       usertypes;     // List of user defined primitives 
};

#endif
