//
//  PrimitiveTypes.hpp
//
//  A set of primitive types for stereocode
//   The type names in this set will be considered primitives so
//   there will be no collaboration assumed
//
//  Created by jmaletic on 7/6/22.
//  TODO: This needs to be improved to add new types and an option.
//

#ifndef PrimitiveTypes_HPP
#define PrimitiveTypes_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <set>


class primitiveTypes {
public:
    primitiveTypes();

    bool isPrimitive(const std::string&) const;

    friend std::ostream& operator<<(std::ostream&, const primitiveTypes&);
    friend std::istream& operator>>(std::istream&, primitiveTypes&);

private:
    std::set<std::string> ptypes;
};






#endif /* PrimitiveTypes_hpp */
