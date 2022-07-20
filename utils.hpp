//
//  utils.hpp
//
//  Utilities and supporting materials for stereocode
//
//  Created by jmaletic on 7/6/22.
//

#ifndef stereocodeUTILS_HPP
#define stereocodeUTILS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <set>



const char NO_STEREOTYPE[] = "none";


bool         checkConst       (std::string);
void         trimWhitespace   (std::string&);
std::string  separateTypeName (const std::string&);



#endif
