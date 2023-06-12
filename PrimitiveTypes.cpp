/**
 * @file PrimitiveTypes.cpp
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

#include "PrimitiveTypes.hpp"

// RETVAL == true if s == (ptypes[i] or usertypes[i])
//
bool primitiveTypes::isPrimitive(const std::string& s) const {
    return (ptypes.find(s) != ptypes.end()) || (usertypes.find(s) != usertypes.end());
}


// Adds s to user specified primitives if not already present
//
void primitiveTypes::addPrimitive(const std::string& s) {
    if (!isPrimitive(s)) usertypes.insert(s);
}


// Reads a set of user defined primitive types to be added to a list
//
// REQUIRES: in.open(fname)
//  fname will list each type name one per line
//  No spaces in type name if compound long int => longint
//
//
std::istream& operator>>(std::istream& in, primitiveTypes& prims)  {
    std::string name;
    while(std::getline(in, name)) prims.addPrimitive(name);
    return in;
}

// Outputs list of all primitive types defined
// REQUIRES: out.open(fname)
std::ostream& operator<<(std::ostream& out, const primitiveTypes& prims) {
    for (std::string i : prims.ptypes) out << i << std::endl;
    for (std::string i : prims.usertypes) out << i << std::endl;
    return out;
}





//Initially ptypes is empty or has user defined types.
//
// After language is determined then language specific primitive
//  types are added.
//
void primitiveTypes::setLanguage(const std::string& lang) {
    if (language == lang) return;        //Same language (done)
    if (language != "") ptypes.clear();  //Change of language
    language = lang;

    if (language == "C++") {
        ptypes = {
            "short",
            "int",
            "signed",
            "unsigned",
            "long",
            "float",
            "double",
            "char",
            "string",
            "string::size_type",
            "string::npos",
            "std::string",
            "std::string::size_type",
            "std::string::npos",
            "size_t",
            "wchar_t",
            "char16_t",
            "char32_t",
            "bool"
        };
    }
    if (language == "C#") {  //NOT correct
        ptypes = {
            "int",
            "short",
            "signed",
            "unsigned",
            "float",
            "long",
            "double",
            "char",
            "string",
            "bool"
        };
    }
    if (language == "Java") {  //NOT correct
        ptypes = {
            "int",
            "short",
            "signed",
            "unsigned",
            "float",
            "long",
            "double",
            "char",
            "string",
            "bool"
        };
    }

}


