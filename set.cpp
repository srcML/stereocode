/**
 * @file set.cpp
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

#include "set.hpp"

inline bool isValid(int x) {
    return (0 <= x) && (x < SET_DOMAIN);
}

//Empty set
//  set a();
set::set() {
    for (int i=0; i<SET_DOMAIN; ++i)
        s[i] = false;
}

//set a(2);
set::set(int a) : set() {
    if (isValid(a)) s[a] = true;
}

// set a = {1, 3, 4};
set::set(std::initializer_list<int> lst) : set() {
    for (int i : lst) {
        if (isValid(i)) s[i] = true;
    }
}

// Cardinality of a set |s|
int set::card() const {
    int result = 0;
    for (int i=0; i < SET_DOMAIN; ++i)
        if (s[i]) ++result;
    return result;
}

// if (a[4])
bool set::operator[](int i) const {
    if (isValid(i)) return s[i];
    return false;
}

//Union
set set::operator+(const set& rhs) const {
    set result;
    for (int i = 0; i<SET_DOMAIN; ++i)
        result.s[i] = s[i] || rhs.s[i];
    return result;
}

set& set::operator+=(const set& rhs)  {
    for (int i = 0; i<SET_DOMAIN; ++i)
        s[i] = s[i] || rhs.s[i];
    return *this;
}

set operator+(int lhs, const set& rhs) { return set(lhs) + rhs; }

//Intersection
set set::operator*(const set& rhs) const {
    set result;
    for (int i = 0; i<SET_DOMAIN; ++i)
        result.s[i] = s[i] && rhs.s[i];
    return result;
}

set operator*(int lhs, const set& rhs) { return set(lhs) * rhs; }

//Difference
set set::operator-(const set& rhs) const {
    set result;
    for (int i = 0; i<SET_DOMAIN; ++i)
        result.s[i] = s[i] && !rhs.s[i];
    return result;
}

set& set::operator-=(const set& rhs) {
    for (int i = 0; i<SET_DOMAIN; ++i)
        s[i] = s[i] && !rhs.s[i];
    return *this;
}

set operator-(int lhs, const set& rhs) { return set(lhs) - rhs; }

//Relational operators
bool set::operator==(const set& rhs) const {
    for (int i=0; i<SET_DOMAIN; ++i)
        if (s[i] != rhs.s[i]) return false;
    return true;
}

bool operator==(int lhs,        const set& rhs) { return set(lhs) == rhs; }
bool operator!=(const set& lhs, const set& rhs) { return !(lhs == rhs);   }

//Subset
bool set::operator<=(const set& rhs) const {
    for (int i=0; i<SET_DOMAIN; ++i)
        if (s[i] && !rhs.s[i]) return false;
    return true;
}

bool operator<=(int lhs,        const set& rhs) { return set(lhs) <= rhs;          }
bool operator>=(const set& lhs, const set& rhs) { return rhs <= lhs;               }
bool operator< (const set& lhs, const set& rhs) { return lhs != rhs && lhs <= rhs; }
bool operator> (const set& lhs, const set& rhs) { return rhs < lhs;                }

std::ostream& operator<<(std::ostream& out, const set& rhs) {
    bool printComma = false;
    out << "{";
    for(int i=0; i<SET_DOMAIN; ++i) {
        if (rhs[i]) {
            if (printComma) out << ", ";
            out << i;
            printComma = true;
        }
    }
    out << "}";
    return out;
}

