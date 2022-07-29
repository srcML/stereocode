//File:   set.cpp
//
//Programmer:   Dr. J. Maletic   
//Date:
//Description:  Definition for Set ADT.
//              Set of integers
//
//
#include "set.hpp"

inline bool isValid(int x) {
    return (0 <= x) && (x < SET_DOMAIN);
}

// set a;
set::set() {         //Empty set
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

// a.card()
int set::card() const {
    int result = 0;
    for (int i=0; i < SET_DOMAIN; ++i) {
        if (s[i]) {
            ++result;
        }
    }
    return result;
}

// if (a[4])
bool set::operator[](int i) const {
    return s[i];
}

//Union
set set::operator+(const set& rhs) const {
    set result;
    for (int i = 0; i<SET_DOMAIN; ++i) {
        result.s[i] = s[i] || rhs.s[i];
    }
    return result;
}

set& set::operator+=(const set& rhs)  {
    for (int i = 0; i<SET_DOMAIN; ++i) {
        s[i] = s[i] || rhs.s[i];
    }
    return *this;
}

set operator+(int lhs, const set& rhs) { return set(lhs) + rhs; }

//Intersection
set set::operator*(const set& rhs) const {
    set result;
    for (int i = 0; i<SET_DOMAIN; ++i) {
        result.s[i] = s[i] && rhs.s[i];
    }
    return result;
}

set operator*(int lhs, const set& rhs) { return set(lhs) * rhs; }


//Difference
set set::operator-(const set& rhs) const {
    set result;
    for (int i = 0; i<SET_DOMAIN; ++i) {
        result.s[i] = s[i] && !rhs.s[i];
    }
    return result;
}

set& set::operator-=(const set& rhs) {
    for (int i = 0; i<SET_DOMAIN; ++i) {
        s[i] = s[i] && !rhs.s[i];
    }
    return *this;
}

set operator-(int lhs, const set& rhs) { return set(lhs) - rhs; }


//Relational operators
bool set::operator==(const set& rhs) const {
    for (int i=0; i<SET_DOMAIN; ++i)
        if (s[i] != rhs.s[i]) return false;
    return true;
}

bool operator==(int lhs, const set& rhs)        { return set(lhs) == rhs; }
bool operator!=(const set& lhs, const set& rhs) { return !(lhs == rhs);   }

//Subset
bool set::operator<=(const set& rhs) const {
    for (int i=0; i<SET_DOMAIN; ++i) {
        if (s[i] && !rhs.s[i]) return false;
    }
    return true;
}

bool operator<=(int lhs, const set& rhs)        { return set(lhs) <= rhs;         }
bool operator>=(const set& lhs, const set& rhs) { return rhs <= lhs;              }
bool operator< (const set& lhs, const set& rhs) { return lhs != rhs && lhs < rhs; }
bool operator> (const set& lhs, const set& rhs) { return rhs < lhs;               }

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

