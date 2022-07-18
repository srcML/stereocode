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


const char NO_STEREOTYPE[] = "none";

extern primitiveTypes primitives;

//
class attributeModel {
public:
                attributeModel()                                           { name = ""; type = ""; };
                attributeModel(const std::string& s)                       { name = s;           };
                attributeModel(const std::string& n, const std::string& t) { name = n; type = t; };
    void        setType       (const std::string& s)                       { type = s;           };
    std::string getName       () const                                     { return name;        };
    std::string getType       () const                                     { return type;        };

private:
    std::string name;
    std::string type;
};


//
class methodModel {
public:
                methodModel           () : name(), parameters(), parameterNames(), parameterTypes(), header(),
                                           returnType(), constMethod(false), retAttribute(false), attributesModified(0),
                                           localVariables(), stereotype(NO_STEREOTYPE) {};
                methodModel           (const std::string& s, bool f) : methodModel() { header = s; constMethod = f; };
    std::string getName               () const { return name; };
    std::string getReturnType         () const { return returnType; };
    std::string getParameters         () const { return parameters; };
    std::string getHeader             () const { return header; };
    std::string getConst              () const { if (constMethod) return "const"; else return ""; };
    int         getAttributesModified () const { return attributesModified; };
    bool        isConst               () const { return constMethod; };
    bool        returnsAttribute      () const { return retAttribute; };
    std::string getStereotype         () const { return stereotype; };

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
    std::string                 header;
    std::string                 returnType;
    bool                        constMethod;         //Is it a const method?
    bool                        retAttribute;        //Does it return any attributes?
    int                         attributesModified;  //# of attributes modified
    std::vector<std::string>    localVariables;
    std::string                 stereotype;
};


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

    void printMethodHeaders              ();
    void printReturnTypes                ();
    void printStereotypes                ();
    void printAttributes                 ();
    void printMethodNames                ();
    void printReportToFile               (std::ofstream&, const std::string&);

    void findClassName                (srcml_archive*, srcml_unit*);
    void findParentClassName          (srcml_archive*, srcml_unit*);

    void findAttributeNames           (srcml_archive*, srcml_unit*);
    void findAttributeTypes           (srcml_archive*, srcml_unit*);

    void findMethodHeaders            (srcml_archive*, srcml_unit*, bool);
    void findMethodNames              (srcml_archive*, srcml_unit*, bool);
    void findParameterLists           (srcml_archive*, srcml_unit*, bool);
    void findMethodReturnTypes        (srcml_archive*, srcml_unit*, bool);
    void findParameterTypes           (srcml_archive*, srcml_unit*, bool);
    void findParameterNames           (srcml_archive*, srcml_unit*, bool);
    void findLocalVariableNames       (srcml_archive*, srcml_unit*, bool);

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

    std::vector<std::string> methodParameterTypes  (srcml_archive*, srcml_unit*, int);
    std::vector<std::string> methodParameterNames  (srcml_archive*, srcml_unit*, int);
    std::vector<std::string> findCalls             (srcml_archive*, srcml_unit*, int, const std::string&);
    std::vector<std::string> findReturnExpressions (srcml_archive*, srcml_unit*, int, bool);
    std::vector<std::string> methodLocalVariables  (srcml_archive*, srcml_unit*, int);

    bool isAttribute                  (std::string&) const;
    bool isPrimitiveContainer         (std::string);
    void methodsReturnPrimitive       (const std::vector<std::string>&, int, std::vector<bool>&);
    bool isInheritedAttribute         (const std::vector<std::string>&, const std::vector<std::string>&, const std::string&);
    int  countPureCalls               (const std::vector<std::string>&) const;
    bool callsAttributesMethod        (const std::vector<std::string>&, const std::vector<std::string>&, const std::vector<std::string>&);

private:
    std::string                 className;
    std::vector<std::string>    parentClass;
    std::vector<attributeModel> attribute;
    std::vector<methodModel>    method;
    int                         unitOneCount;   //Methods in .hpp, .java, .cs, etc.
    int                         unitTwoCount;   //Methods in .cpp - only C++ has two units
    std::string                 language;       //Language: "C++", "C#", "Java", "C"
}; 


bool         checkConst       (std::string);
void         trimWhitespace   (std::string&);
std::string  separateTypeName (const std::string&);


#endif
