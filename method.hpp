//
//  method.hpp
//
//  methodModel class for stereocode
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
#include "variable.hpp"


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

    std::vector<std::string> findReturnExpressions (bool) const;
    std::vector<std::string> findLocalVariables    () const;
    std::vector<std::string> findParameterNames    () const;
    std::vector<std::string> findParameterTypes    () const;

    std::vector<std::string> findCalls             (const std::string&) const;


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

    bool        findConstructorCall   () const;
    bool        isFactory             () const;
    bool        isEmptyMethod         () const;
    bool        containsNonPrimitive  (const std::string&, const std::string&) const;
    bool        isVoidAccessor        () const;
    bool        variableChanged       (const std::string&) const;

    friend std::ostream& operator<<(std::ostream&, const methodModel&);

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
