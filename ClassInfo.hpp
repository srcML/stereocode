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


extern primitiveTypes PRIMITIVES;


class classModel {
public:
    classModel () : className(), parentClass(), attribute(), method(), unitOneCount(0), unitTwoCount(0), language() {};
    classModel (srcml_archive*, srcml_unit*, srcml_unit*);

    std::string getClassName    ()      const { return className;    };
    int         getUnitOneCount ()      const { return unitOneCount; };
    int         getUnitTwoCount ()      const { return unitTwoCount; };
    bool        inherits        ()      const { return parentClass.size() > 0; };

    srcml_unit* writeStereotypeAttribute  (srcml_archive*, srcml_unit*, bool);

    void stereotypeGetter                 ();
    void stereotypeSetter                 ();
    void stereotypePredicate              ();
    void stereotypeProperty               ();
    void stereotypeVoidAccessor           ();
    void stereotypeCommand                ();
    void stereotypeCollaborationalCommand ();
    void stereotypeCollaborator           ();
    void stereotypeFactory                ();
    void stereotypeEmpty                  ();
    void stereotypeStateless              ();

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

    void countChangedAttributes       ();
    void returnsAttributes            ();

    bool isVoidAccessor               (int);
    bool variableChanged              (int, const std::string&);
    int  findAssignOperatorAttribute  (int, bool);
    int  findIncrementedAttribute     (int, bool);
    bool containsNonPrimitive         (int, const std::string&);
    bool usesAttributeObj             (int, const std::vector<std::string>&);
    bool usesAttribute                (int);
    bool isFactory                    (int);
    bool findConstructorCall          (int);
    bool isEmptyMethod                (int);
    std::vector<std::string> findCalls(int, const std::string&);

    std::vector<std::string> methodParameterTypes  (int);
    std::vector<std::string> methodParameterNames  (int);
    std::vector<std::string> findReturnExpressions (int, bool);
    std::vector<std::string> methodLocalVariables  (int);

    bool isAttribute                  (std::string&) const;
    bool isPrimitiveContainer         (std::string);
    void methodsReturnPrimitive       (const std::vector<std::string>&, int, std::vector<bool>&);
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
    std::vector<variable>       attribute;
    std::vector<methodModel>    method;
    int                         unitOneCount;   //Methods in .hpp, .java, .cs, etc. 
    int                         unitTwoCount;   //Methods in .cpp - only C++ has two units
    std::string                 language;       //Language: "C++", "C#", "Java", "C"
}; 



#endif
