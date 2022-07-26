#ifndef CLASS_INFO_HPP
#define CLASS_INFO_HPP

//classModel for stereocode
// 

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
#include "method.hpp"


extern primitiveTypes PRIMITIVES;


class classModel {
public:
    classModel () : className(), parentClass(), attribute(), method(), unitOneCount(0), unitTwoCount(0), language() {};
    classModel (srcml_archive*, srcml_unit*, srcml_unit*);

    std::string getClassName    ()      const { return className;    };
    int         getUnitOneCount ()      const { return unitOneCount; };
    int         getUnitTwoCount ()      const { return unitTwoCount; };
    bool        inherits        ()      const { return parentClass.size() > 0; };
    bool        isAttribute     (const std::string&) const;

    srcml_unit* outputUnitWithStereotypes(srcml_archive*, srcml_unit*, bool);
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

    void stereotype();
    void getter();
    void setter();
    void predicate();
    void property();
    void voidAccessor();
    void command();
    void collaborationalCommand();
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

private:
    std::string                 className;
    std::vector<std::string>    parentClass;
    std::vector<variable>       attribute;
    std::vector<methodModel>    method;
    int                         unitOneCount;   //Methods in .hpp, .java, .cs, etc. 
    int                         unitTwoCount;   //Methods in .cpp - only C++ has two units
    std::string                 language;       //Language: "C++", "C#", "Java", "C"
}; 


#endif
