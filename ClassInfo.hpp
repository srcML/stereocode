/**
 * @file ClassInfo.hpp
 *
 * @copyright Copyright (C) 2010-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of Stereocode.
 * 
 * Stereocode is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Stereocode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stereocode; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef CLASSINFO_HPP
#define CLASSINFO_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <algorithm>
#include <srcml.h>
#include "PrimitiveTypes.hpp"
#include "utils.hpp"
#include "variable.hpp"
#include "method.hpp"
#include "set.hpp"


extern primitiveTypes PRIMITIVES;
extern bool           DEBUG;
extern int            METHODS_PER_CLASS_THRESHOLD;

class classModel {
public:
    classModel () : className(), parentClass(), attribute(), method(), unitOneCount(0), unitTwoCount(0), language() {};
    classModel (srcml_archive*, srcml_unit*, srcml_unit*);

    std::string getClassName    ()      const { return className;    };
    int         getUnitOneCount ()      const { return unitOneCount; };
    int         getUnitTwoCount ()      const { return unitTwoCount; };
    bool        inherits        ()      const { return parentClass.size() > 0; };
    bool        isAttribute     (const std::string&) const;

    srcml_unit* outputUnitWithStereotypes(srcml_archive*, srcml_unit*, srcml_transform_result**, bool);
    void        outputReport(std::ofstream&, const std::string&);

    void findClassName(srcml_archive*, srcml_unit*);
    void findParentClassName(srcml_archive*, srcml_unit*);
    void findAttributeNames(srcml_archive*, srcml_unit*);
    void findAttributeTypes(srcml_archive*, srcml_unit*);
    void findMethods(srcml_archive*, srcml_unit*, bool);

    void findMethodNames();
    void findParameterLists();
    void findMethodReturnTypes();
    void findParameterTypes();
    void findParameterNames();
    void findLocalVariableNames();
    void countChangedAttributes();
    void returnsAttributes();
    int  findAssignOperatorAttribute(int, bool) const;
    int  findIncrementedAttribute(int, bool) const;
    bool usesAttributeObj(int, const std::vector<std::string>&);
    bool usesAttribute(int);
    bool callsAttributesMethod(const std::vector<std::string>&,
                               const std::vector<std::string>&,
                               const std::vector<std::string>&);

    void ComputeClassStereotype();
    void ComputeMethodStereotype();
    void getter();
    void setter();
    void predicate();
    void property();
    void voidAccessor();
    void command();
    void commandCollaborator();
    void collaborator();
    void factory();
    void empty();
    void stateless();

    void printMethodHeaders();
    void printReturnTypes();
    void printStereotypes();
    void printAttributes();
    void printMethodNames();

    friend std::ostream& operator<<(std::ostream&, const classModel&);

protected:
    std::string                 className;
    std::vector<std::string>    parentClass;
    std::vector<variable>       attribute;
    std::vector<methodModel>    method;
    int                         unitOneCount;    //Methods in .hpp, .java, .cs, etc.
    int                         unitTwoCount;    //Methods in .cpp - only C++ has two units
    std::string                 language;        //Language: "C++", "C#", "Java", "C"
    std::string                 classStereotype; //Class stereotype
}; 


#endif
