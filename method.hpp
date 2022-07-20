//
//  method.hpp
//
//  methodModel and attributeModel class for stereocode
//
//  Created by jmaletic on July 20 2022.
//

#ifndef stereocodeMETHOD_HPP
#define stereocodeMETHOD_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <algorithm>
#include <srcml.h>
#include "utils.hpp"

//
class variable {
public:
                variable ()                                           { name = ""; type = ""; };
                variable (const std::string& s)                       { name = s;           };
                variable (const std::string& n, const std::string& t) { name = n; type = t; };
    void        setType  (const std::string& s)                       { type = s;           };
    std::string getName  () const                                     { return name;        };
    std::string getType  () const                                     { return type;        };

private:
    std::string name;
    std::string type;
};


//
class methodModel {
public:
                methodModel();
                methodModel(const std::string&, const std::string&, bool);

    std::string getName               () const { return name; };
    std::string getReturnType         () const { return returnType; };
    std::string getParameters         () const { return parameters; };
    std::string getHeader             () const { return header; };
    std::string getConst              () const { if (constMethod) return "const"; else return ""; };
    int         getAttributesModified () const { return attributesModified; };
    bool        isConst               () const { return constMethod; };
    bool        returnsAttribute      () const { return retAttribute; };
    std::string getStereotype         () const { return stereotype; };

    std::string getsrcML () const {return srcML; };

    std::vector<std::string> getLocalVariables() const { return localVariables; };
    std::vector<std::string> getParameterNames() const { return parameterNames; };
    std::vector<std::string> getParameterTypes() const { return parameterTypes; };

    void        setName               (const std::string& s) { name = s; };
    void        setReturnType         (const std::string& s) { returnType = s; };
    void        setParameters         (const std::string& s) { parameters = s; };
    void        setHeader             (const std::string& s) { header = s; };
    void        setConst              (bool flag)            { constMethod = flag; };
    void        setReturnsAttribute   (bool flag)            { retAttribute = flag; };
    void        setAttributesModified (int n)                { attributesModified = n; };
    void        setLocalVariables     (const std::vector<std::string>& s) { localVariables = s; };
    void        setParameterNames     (const std::vector<std::string>& s) { parameterNames = s; };
    void        setParameterTypes     (const std::vector<std::string>& s) { parameterTypes = s; };

    void        setStereotype         (const std::string& s) { stereotype = s; };

private:
    std::string                 name;
    std::string                 parameters;
    std::vector<std::string>    parameterNames;
    std::vector<std::string>    parameterTypes;
    std::string                 srcML;               //srcML archive of the method
    std::string                 header;
    std::string                 returnType;
    bool                        constMethod;         //Is it a const method?
    bool                        retAttribute;        //Does it return any attributes?
    int                         attributesModified;  //# of attributes modified
    std::vector<std::string>    localVariables;
    std::string                 stereotype;
};



#endif
