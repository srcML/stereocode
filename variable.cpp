//
//  variable.cpp
//  
//
//  Created by jmaletic on July 20 2022.
//

#include "variable.hpp"



std::ostream& operator<<(std::ostream& out, const variable& m) {
    out << "(" << m.getType() << " : " << m.getName() << ") ";
    return out;
}

