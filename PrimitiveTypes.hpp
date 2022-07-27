//
//  PrimitiveTypes.hpp
//
//  A set of primitive types for stereocode
//   The type names in this set will be considered primitives so
//   there will be no collaboration assumed
//
//  Created by jmaletic on 7/6/22.
//

#ifndef PrimitiveTypes_HPP
#define PrimitiveTypes_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <set>


class primitiveTypes {
public:
    primitiveTypes() : language(""), ptypes() {};

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
