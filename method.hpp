// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file method.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef METHOD_HPP
#define METHOD_HPP

#include <srcml.h>
#include "utils.hpp"

class methodModel {
public:
    methodModel();
    methodModel(const std::string&, const std::string&, bool);

    std::vector<std::string> findReturnExpressions         (bool) const;
    std::vector<std::string> findLocalVariables            (const std::string&, bool);
    std::vector<std::string> findParameters                (const std::string&, bool);
    std::vector<std::string> findCalls                     (const std::string&, bool) const;
    int                      findAssignOperatorAttribute   (std::vector<AttributeInfo>&, bool);
    int                      findIncrementedAttribute      (std::vector<AttributeInfo>&, bool);

    std::string getName                   () const { return name; };
    std::string getReturnType             () const { return returnType; };
    std::string getParametersList         () const { return parametersList; };
    std::string getIsConst                () const { if (constMethod) return "const"; else return ""; };
    std::string getHeader                 () const { return header; };
    std::string getStereotype             () const;
    std::string getMethod                 () const { return method; };
    int         getAttributesModified     () const { return attributesModified; };
    bool        getConst                  () const { return constMethod; };
    bool        getIsAttributeReturned    () const { return isAttributeReturned; };
    bool        getIsAttributeUsed        () const { return isAttributeUsed; };
    bool        getIsEmpty                () const { return isEmpty; };
    bool        getIsParamChanged         () const { return isParamChanged; };

    std::vector<std::string> getLocalVariablesNames() const      { return localVariablesNames; };
    std::vector<std::string> getLocalVariablesTypes() const      { return localVariablesTypes; };
    std::vector<std::string> getParameterNames     () const      { return parametersNames; };
    std::vector<std::string> getParameterTypes     () const      { return parametersTypes; };
    std::vector<std::string> getFunctionCalls      () const      { return functionCalls; };
    std::vector<std::string> getMethodCalls        () const      { return methodCalls; };
    std::vector<std::string> getConstructorCalls   () const      { return constructorCalls; };

    void        setName                   (const std::string& s)              { name = s; };
    void        setReturnType             (const std::string& s)              { returnType = s; };
    void        setParametersList         (const std::string& s)              { parametersList = s; };
    void        setHeader                 (const std::string& s)              { header = s; };
    void        setConst                  (bool flag)                         { constMethod = flag; };
    void        setIsAttributeReturned    (bool flag)                         { isAttributeReturned = flag; };
    void        setIsAttributeUsed        (bool flag)                         { isAttributeUsed = flag;};
    void        setIsEmpty                (bool flag)                         { isEmpty = flag;};
    void        setIsParamChanged         (bool flag)                         { isParamChanged = flag;};
    void        setAttributesModified     (int n)                             { attributesModified = n; };
    void        setLocalVariablesNames    (const std::vector<std::string>& s) { localVariablesNames = s; };
    void        setLocalVariablesTypes    (const std::vector<std::string>& s) { localVariablesTypes = s; };
    void        setParameterNames         (const std::vector<std::string>& s) { parametersNames = s; };
    void        setParameterTypes         (const std::vector<std::string>& s) { parametersTypes = s; };
    void        setFunctionCalls          (const std::vector<std::string>& s) { functionCalls = s; };
    void        setMethodCalls            (const std::vector<std::string>& s) { methodCalls = s; };
    void        setConstructorCalls       (const std::vector<std::string>& s) { constructorCalls = s; };
    void        setStereotype             (const std::string&);

    bool        variableChanged           (const std::string& ) const;
    bool        usesAttribute             (std::vector<AttributeInfo>&, bool);
    bool        usesNonPrimitiveAttributes(const std::vector<AttributeInfo>&, std::string);
    bool        callsOnAttributes         (std::vector<AttributeInfo>&, std::string);
    bool        callsOnArguments          (std::vector<AttributeInfo>&, std::string, int);
    bool        isEmptyMethod             ();
    bool        returnsAttribute          (std::vector<AttributeInfo>&, bool);

    friend      std::ostream& operator<<(std::ostream&, const methodModel&);

protected:
    std::string                       name;                // method name unparsed
    std::string                       returnType;          // method return type unparsed
    std::string                       parametersList;      // SrcML of <parameter_list>
    std::vector<std::string>          parametersNames;     // parameter name unparsed
    std::vector<std::string>          parametersTypes;     // parameter type unparsed
    std::vector<std::string>          localVariablesNames; // local variables names unparsed
    std::vector<std::string>          localVariablesTypes; // local variables types unparsed
    std::vector<std::string>          stereotype;          // method stereotype
    std::vector<std::string>          functionCalls;       // a(), b()
    std::vector<std::string>          methodCalls;         // a.b()
    std::vector<std::string>          constructorCalls;    // uses "new"
    std::string                       method;              // SrcML of the method as an archive
    std::string                       header;              // method header unparsed (all before { ) 
    bool                              constMethod;         // Is it a const method?
    bool                              isAttributeReturned; // Does it return any attribute?
    bool                              isAttributeUsed;     // Does it use any attribute in an expression?
    bool                              isParamChanged;      // Does change any parameter passed by reference?
    bool                              isEmpty;             // Empty method (comments not counted)
    int                               attributesModified;  // # of attributes modified
};

#endif