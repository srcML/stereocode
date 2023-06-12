/**
 * @file variable.hpp
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

#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <iostream>
#include <string>

class variable {
public:
                variable ()                                           { name = ""; type = ""; };
                variable (const std::string& n)                       { name = n;           };
                variable (const std::string& n, const std::string& t) { name = n; type = t; };
    void        setName  (const std::string& n)                       { name = n;           };
    void        setType  (const std::string& t)                       { type = t;           };
    std::string getName  () const                                     { return name;        };
    std::string getType  () const                                     { return type;        };

private:
    std::string name;
    std::string type;
};

std::ostream& operator<<(std::ostream&, const variable&);

#endif
