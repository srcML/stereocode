//File:   set.hpp
//
//Programmer:   Dr. J. Maletic
//Date:         7/2022
//Description:  Definition for Set ADT.
//              Set of integers
//

#ifndef CS2_SET_HPP_
#define CS2_SET_HPP_

#include <iostream>
#include <initializer_list>

////////////////////////////////////////
// A set of integers from {0, 1, ... SET_DOMAIN-1}
// CLASS INV: all elements must be between 0 and SET_DOMAIN-1
//
const int SET_DOMAIN = 500; //Or whatever

class set {
public:
          set        ();
          set        (int);
          set        (std::initializer_list<int>);
    
    int   card       ()           const;
    bool  operator[] (int)        const;
    set   operator+  (const set&) const;
    set&  operator+= (const set&);
    set   operator*  (const set&) const;
    set   operator-  (const set&) const;
    set&  operator-= (const set&);
    bool  operator== (const set&) const;
    bool  operator<= (const set&) const;

private:
    bool s[SET_DOMAIN];
};

set  operator+ (int,        const set&);
set  operator* (int,        const set&);
set  operator- (int,        const set&);
bool operator==(int,        const set&);
bool operator!=(const set&, const set&);
bool operator<=(int,        const set&);
bool operator< (const set&, const set&);
bool operator>=(const set&, const set&);
bool operator> (const set&, const set&);

std::ostream& operator<<(std::ostream&, const set&);

#endif