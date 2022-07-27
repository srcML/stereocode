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
#include <cctype>

#include "PrimitiveTypes.hpp"


extern primitiveTypes PRIMITIVES;

const char NO_STEREOTYPE[] = "none";
const std::vector<std::string> ASSIGNMENT_OPERATOR = {"=", "+=", "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|=", "<<"};

bool         isGlobalConstFormat  (const std::string&);
bool         checkConst           (const std::string&);
bool         isInheritedAttribute (const std::vector<std::string>&, const std::vector<std::string>&, const std::string&);
int          countPureCalls       (const std::vector<std::string>&) ;
bool         isPrimitiveContainer (std::string);

std::string  trimWhitespace       (const std::string&);
std::string  LRtoSpace            (const std::string&);
std::string  removeSpecifiers     (const std::string&);
std::string  separateTypeName     (const std::string&);


#endif
