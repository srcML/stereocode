//
//  PrimitiveTypes.cpp
//  
//
//  Created by jmaletic on 7/6/22.
//

#include "PrimitiveTypes.hpp"


//Initial (default) set of primitive types
primitiveTypes::primitiveTypes() {
    ptypes = {
        "short",
        "shortint",
        "intshort",
        "signedshort",
        "shortsigned",
        "signedshortint",
        "signedintshort",
        "shortsignedint",
        "shortintsigned",
        "intsignedshort",
        "intshortsigned",
        "unsignedshort",
        "shortunsigned",
        "unsignedshortint",
        "unsignedintshort",
        "shortunsignedint",
        "shortintunsigned",
        "intshortunsigned",
        "intunsignedshort",
        "int",
        "signed",
        "signedint",
        "intsigned",
        "unsigned",
        "unsignedint",
        "intunsigned",
        "long",
        "longint",
        "intlong",
        "signedlong",
        "longsigned",
        "signedlongint",
        "signedintlong",
        "longsignedint",
        "longintsigned",
        "intsignedlong",
        "intlongsigned",
        "unsignedlong",
        "longunsigned",
        "unsignedlongint",
        "unsignedintlong",
        "longunsignedint",
        "longintunsigned",
        "intunsignedlong",
        "intlongunsigned",
        "longlong",
        "longlongint",
        "longintlong",
        "intlonglong",
        "signedlonglong",
        "longsignedlong",
        "longlongsigned",
        "signedlonglongint",
        "signedlongintlong",
        "signedintlonglong",
        "longlongsignedint",
        "longlongintsigned",
        "longsignedintlong",
        "longsignedlongint",
        "longintsignedlong",
        "longintlongsigned",
        "intlonglongsigned",
        "intlongsignedlong",
        "intsignedlonglong",
        "unsignedlonglong",
        "longunsingedlong",
        "longlongunsigned",
        "unsingedlonglongint",
        "unsignedlongintlong",
        "unsignedintlonglong",
        "longlongunsignedint",
        "longlongintunsigned",
        "longunsignedintlong",
        "longunsignedlongint",
        "longintunsignedlong",
        "longintlongunsigned",
        "intlonglongunsigned",
        "intlongunsignedlong",
        "intunsignedlonglong",
        "float",
        "double",
        "longdouble",
        "doublelong",
        "char",
        "signedchar",
        "charsigned",
        "unsignedchar",
        "charunsigned",
        "string",
        "string::size_type",
        "string::npos",
        "std::string",
        "std::string::size_type",
        "std::string::npos",
        "size_t",
        "wchar_t",
        "char16_t",
        "char32_t",
        "bool"
    };

}


// Reads a set of user defined primitive types to be added to the default list
//
// REQUIRES: in.open(fname)
//  fname will list each type name one per line
//  No spaces in type name if compound long int => longint
//
//
std::istream& operator>>(std::istream& in, primitiveTypes& prims)  {
    std::string name;
    while(std::getline(in, name)) prims.ptypes.insert(name);
    return in;
}

// Outputs list of all primitive types defined
// REQUIRES: out.open(fname)
std::ostream& operator<<(std::ostream& out, const primitiveTypes& prims) {
    for (std::string i : prims.ptypes) out << i << std::endl;
    return out;
}


//
// Check if str is in the set of primitive types
//
bool primitiveTypes::isPrimitive(const std::string& str) const {
    return ptypes.find(str) != ptypes.end();
}



