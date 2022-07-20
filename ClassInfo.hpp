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
#include "utils.hpp"
#include "method.hpp"
#include "PrimitiveTypes.hpp"


extern primitiveTypes primitives;


class classModel {
public:
    classModel () : className(), parentClass(), attribute(), method(), unitOneCount(0), unitTwoCount(0), language() {};
    classModel (srcml_archive*, srcml_unit*, srcml_unit*);

    std::string getClassName    ()      const { return className;    }
    int         getUnitOneCount ()      const { return unitOneCount; }
    int         getUnitTwoCount ()      const { return unitTwoCount; }

    srcml_unit* writeStereotypeAttribute  (srcml_archive*, srcml_unit*, bool);

    void stereotypeGetter                 (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeSetter                 (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypePredicate              (srcml_archive*, srcml_unit*, bool);
    void stereotypeProperty               (srcml_archive*, srcml_unit*, bool);
    void stereotypeVoidAccessor           (srcml_archive*, srcml_unit*, bool);
    void stereotypeCommand                (srcml_archive*, srcml_unit*, bool);
    void stereotypeCollaborationalCommand (srcml_archive*, srcml_unit*, bool);
    void stereotypeCollaborator           (srcml_archive*, srcml_unit*, bool);
    void stereotypeFactory                (srcml_archive*, srcml_unit*, bool);
    void stereotypeEmpty                  (srcml_archive*, srcml_unit*, bool);
    void stereotypeStateless              (srcml_archive*, srcml_unit*, bool);

    void findClassName                (srcml_archive*, srcml_unit*);
    void findParentClassName          (srcml_archive*, srcml_unit*);

    void findAttributeNames           (srcml_archive*, srcml_unit*);
    void findAttributeTypes           (srcml_archive*, srcml_unit*);

    void findMethods                  (srcml_archive*, srcml_unit*, bool);
    void findMethodNames              ();
    void findParameterLists           ();
    void findMethodReturnTypes        ();
    void findParameterTypes           ();
    void findParameterNames           ();
    void findLocalVariableNames       ();

    void countChangedAttributes       (srcml_archive*, srcml_unit*, bool);
    void returnsAttributes            (srcml_archive*, srcml_unit*, bool);

    bool isVoidAccessor               (srcml_archive*, srcml_unit*, int);
    bool variableChanged              (srcml_archive*, srcml_unit*, int, const std::string&);
    int  findAssignOperatorAttribute  (srcml_archive*, srcml_unit*, int, bool);
    int  findIncrementedAttribute     (srcml_archive*, srcml_unit*, int, bool);
    bool containsNonPrimitive         (srcml_archive*, srcml_unit*, int, const std::string&);
    bool usesAttributeObj             (srcml_archive*, srcml_unit*, int, const std::vector<std::string>&);
    bool usesAttribute                (srcml_archive*, srcml_unit*, int);
    bool isFactory                    (srcml_archive*, srcml_unit*, int);
    bool findConstructorCall          (srcml_archive*, srcml_unit*, int);
    bool isEmptyMethod                (srcml_archive*, srcml_unit*, int);

    std::vector<std::string> methodParameterTypes  (int);
    std::vector<std::string> methodParameterNames  (int);
    std::vector<std::string> findCalls             (srcml_archive*, srcml_unit*, int, const std::string&);
    std::vector<std::string> findReturnExpressions (srcml_archive*, srcml_unit*, int, bool);
    std::vector<std::string> methodLocalVariables  (int);

    bool isAttribute                  (std::string&) const;
    bool isPrimitiveContainer         (std::string);
    void methodsReturnPrimitive       (const std::vector<std::string>&, int, std::vector<bool>&);
    bool isInheritedAttribute         (const std::vector<std::string>&, const std::vector<std::string>&, const std::string&);
    int  countPureCalls               (const std::vector<std::string>&) const;
    bool callsAttributesMethod        (const std::vector<std::string>&, const std::vector<std::string>&, const std::vector<std::string>&);

    void printMethodHeaders              ();
    void printReturnTypes                ();
    void printStereotypes                ();
    void printAttributes                 ();
    void printMethodNames                ();
    void printReportToFile               (std::ofstream&, const std::string&);

private:
    std::string                 className;
    std::vector<std::string>    parentClass;
    std::vector<attributeModel> attribute;
    std::vector<methodModel>    method;
    int                         unitOneCount;   //Methods in .hpp, .java, .cs, etc.
    int                         unitTwoCount;   //Methods in .cpp - only C++ has two units
    std::string                 language;       //Language: "C++", "C#", "Java", "C"
}; 


#endif
