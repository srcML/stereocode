// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassInfo.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef CLASSINFO_HPP
#define CLASSINFO_HPP

#include <fstream>
#include "method.hpp"
#include "set.hpp"

extern primitiveTypes PRIMITIVES;
extern bool           DEBUG;
extern int            METHODS_PER_CLASS_THRESHOLD;

class classModel {
public:
    classModel () : className(), parentClass(), attribute(), method(), unitOneCount(0), unitTwoCount(0), language(), classStereotype() {};
    classModel (srcml_archive*, srcml_unit*, srcml_unit*);

    std::string getClassStereotype   ()      const;
    std::string getClassName         ()      const { return className;    };
    int         getUnitOneCount      ()      const { return unitOneCount; };
    int         getUnitTwoCount      ()      const { return unitTwoCount; };
    bool        inherits             ()      const { return parentClass.size() > 0; };
    
    void findClassName(srcml_archive*, srcml_unit*);
    void findParentClassName(srcml_archive*, srcml_unit*);
    void findAttributeNames(srcml_archive*, srcml_unit*);
    void findAttributeTypes(srcml_archive*, srcml_unit*);
    void findMethods(srcml_archive*, srcml_unit*, bool);

    void findMethodNames();
    void findParameterLists();
    void findMethodReturnTypes();
    void findLocalVariablesNames();
    void findLocalVariablesTypes();
    void findParametersNames();
    void findParametersTypes();
    void findAllCalls();
    
    void isAttributeReturned();
    void isAttributeUsed();
    void isParameterModified();
    void isEmptyMethod();
    
    void countChangedAttributes();

    void ComputeClassStereotype();
    void ComputeMethodStereotype();
    void getter();
    void setter();
    void predicate();
    void property();
    void accessor();
    void command();
    void collaboratorController();
    void factory();
    void empty();
    void stateless();
    void wrapper();

    srcml_unit* outputUnitWithStereotypes(srcml_archive*, srcml_unit*, srcml_transform_result**, bool);
    void        outputReport(std::ofstream&, const std::string&);
    

    void printReturnTypes();
    void printStereotypes();
    void printMethodNames();
    void printMethodHeaders();
    void printAttributes();
    
    friend std::ostream& operator<<(std::ostream&, const classModel&);

protected:
    std::string                 className;
    std::vector<std::string>    parentClass;
    std::vector<AttributeInfo>  attribute;
    std::vector<methodModel>    method;
    int                         unitOneCount;    // Methods in .hpp, .java, .cs, etc.
    int                         unitTwoCount;    // Methods in .cpp - only C++ has two units
    std::string                 language;        // Language: "C++", "C#", "Java", "C"
    std::vector<std::string>    classStereotype; // Class stereotype
}; 

#endif
