/**
 * @file PrimitiveTypes.hpp
 *
 * @copyright Copyright (C) 2010-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of Stereocode.
 * 
 * Stereocode is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Stereocode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stereocode; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef PRIMITIVETYPES_HPP
#define PRIMITIVETYPES_HPP

#include <iostream>
#include <fstream>
#include <string>
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
    std::string                 language;  //Language: "C++", "C#", "Java", "C"
    std::set<std::string>       ptypes;    //List of language primitives
    std::set<std::string>       usertypes;    //List of user defined primitives 
};


#endif
