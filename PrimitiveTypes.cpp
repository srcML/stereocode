//
//  PrimitiveTypes.cpp
//  
//
//  Created by jmaletic on 7/6/22.
//

#include "PrimitiveTypes.hpp"

// RETVAL == true if s == (ptypes[i] or usertypes[i])
//
bool primitiveTypes::isPrimitive(const std::string& s) const {
    return (ptypes.find(s) != ptypes.end()) || (usertypes.find(s) != usertypes.end());
}


// Adds s to user specified primitives if not already present
//
void primitiveTypes::addPrimitive(const std::string& s) {
    if (!isPrimitive(s)) usertypes.insert(s);
}


// Reads a set of user defined primitive types to be added to a list
//
// REQUIRES: in.open(fname)
//  fname will list each type name one per line
//  No spaces in type name if compound long int => longint
//
//
std::istream& operator>>(std::istream& in, primitiveTypes& prims)  {
    std::string name;
    while(std::getline(in, name)) prims.addPrimitive(name);
    return in;
}

// Outputs list of all primitive types defined
// REQUIRES: out.open(fname)
std::ostream& operator<<(std::ostream& out, const primitiveTypes& prims) {
    for (std::string i : prims.ptypes) out << i << std::endl;
    return out;
}





//Initially ptypes is empty or has user defined types.
//
// After language is determined then language specific primitive
//  types are added.
//
void primitiveTypes::setLanguage(const std::string& lang) {
    if (language == lang) return;        //Same language (done)
    if (language != "") ptypes.clear();  //Change of language
    language = lang;

    if (language == "C++") {
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
    if (language == "C++2") {  //C++ with spaces
        ptypes = {
            "int",
            "short",
            "signed",
            "unsigned",
            "float",
            "long",
            "double",
            "char",
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
    if (language == "C#") {  //NOT correct
        ptypes = {
            "int",
            "short",
            "signed",
            "unsigned",
            "float",
            "long",
            "double",
            "char",
            "string",
            "bool"
        };
    }
}


