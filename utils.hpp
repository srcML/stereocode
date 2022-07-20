//
//  utils.hpp
//
//  Utilities and supporting materials for stereocode
//
//  Created by jmaletic on 7/6/22.
//

#ifndef stereocodeUTILS_HPP
#define stereocodeUTILS_HPP

#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <algorithm>




const char NO_STEREOTYPE[] = "none";

const std::vector<std::string> ASSIGNMENT_OPERATOR = {"=", "+=", "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|=", "<<"};


bool         checkConst           (std::string);
void         trimWhitespace       (std::string&);
std::string  separateTypeName     (const std::string&);
bool         isInheritedAttribute (const std::vector<std::string>&, const std::vector<std::string>&, const std::string&);
int          countPureCalls       (const std::vector<std::string>&) ;


#endif
