//
//  variable.hpp
//
//  variable (name, type) class for stereocode
//
//  Created by jmaletic on July 20 2022.
//

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
