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

const std::string NO_STEREOTYPE = "unclassified";

const std::vector<std::string> METHOD_STEREOTYPE = {"get", "non-const-get", "set", "predicate",
    "property", "void-accessor", "collaborator", "command", "non-void-command",
    "controller", "factory", "empty", "stateless", "wrapper", "incidental" };

const std::vector<std::string> CLASS_STEREOTYPE = {"entity", "minimal-entity", "data-provider", "command", "boundary",
    "control", "pure-control", "factory",  "large-class", "lazy-class", "degenerate", "data-class", "small-class" };


const std::vector<std::string> ASSIGNMENT_OPERATOR = {"=", "+=", "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|=", "<<"};


bool         isGlobalConstFormat  (const std::string&);
bool         checkConst           (const std::string&);
bool         isInheritedAttribute (const std::vector<std::string>&, const std::vector<std::string>&, const std::string&);
int          countPureCalls       (const std::vector<std::string>&) ;
bool         isPrimitiveContainer (const std::string&);

std::string  removeSpecifiers     (const std::string&);
std::string  separateTypeName     (const std::string&);
std::string  trimWhitespace       (const std::string&);
std::string  WStoBlank            (const std::string&);
std::string  Ltrim                (const std::string&);
std::string  Rtrim                (const std::string&);
std::string  multiBlanksToBlank   (const std::string&);


#endif
