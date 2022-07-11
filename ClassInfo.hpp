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


extern primitiveTypes primitives; //Not ideal

//
class attributeModel {
public:
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
                methodModel           (const std::string& n) { name = n; stereotype = "nothing-yet"; };
    std::string getName               () const { return name; };
    std::string getReturnType         () const { return returnType; };
    std::string getParameters         () const { return parameters; };
    std::string getHeader             () const { return header; };
    std::string getConst              () const { if (constMethod) return "const"; else return ""; };
    int         getAttributesModified () const { return attributesModified; };
    std::string getStereotype         () const { return stereotype; };
    bool        isConst               () const { return constMethod; };
    bool        returnsAttribute      () const { return retAttribute; };

    void        setReturnType         (const std::string& s) { returnType = s; };
    void        setParameters         (const std::string& s) { parameters = s; };
    void        setHeader             (const std::string& s) { header = s; };
    void        setConst              (bool flag)            { constMethod = flag; };
    void        setReturnsAttribute   (bool flag)            { retAttribute = flag; };
    void        setAttributesModified (int n)                { attributesModified = n; };
    void        setStereotype         (const std::string& s) { stereotype = s; };

private:
    std::string name;
    std::string parameters;
    std::string header;
    std::string returnType;
    bool        constMethod;         //Is it a const method?
    bool        retAttribute;        //Does it return any attributes?
    int         attributesModified;  //# of attributes modified

    std::string stereotype;
};


class classModel {
public:
    classModel () {};
    classModel (srcml_archive*, srcml_unit*, srcml_unit*);

    std::string getClassName              ()      const { return className;                }
    int         getInlineFunctionCount    ()      const { return inline_function_count;    }
    int         getOutoflineFunctionCount ()      const { return outofline_function_count; }
    //std::string getReturnType             (int i) const { return return_types[i];          }
    //std::string getMethodHeader           (int i) const { return headers[i];               }
    //int         getNumberOfMethods        ()      const { return headers.size();           }

    srcml_unit* writeStereotypeAttribute  (srcml_archive*, srcml_unit*, bool);
    void stereotypeGetters                (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypePredicates             (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeProperties             (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeVoidAccessor           (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeSetters                (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeCommand                (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeCollaborationalCommand (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeCollaborators          (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeFactories              (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeEmpty                  (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeStateless              (srcml_archive*, srcml_unit*, srcml_unit*);

    void printMethodHeaders              ();
    void printReturnTypes                ();
    void printStereotypes                ();
    void printAttributes                 ();
    void printMethodNames                ();
    void printReportToFile               (std::ofstream&, const std::string&);


private:
    void findClassName                (srcml_archive*, srcml_unit*);
    void findParentClassName          (srcml_archive*, srcml_unit*);

    void findAttributeNames           (srcml_archive*, srcml_unit*);
    void findAttributeTypes           (srcml_archive*, srcml_unit*);

    void findMethodNames              (srcml_archive*, srcml_unit*);
    void findParameterLists           (srcml_archive*, srcml_unit*);
    void findMethodHeaders            (srcml_archive*, srcml_unit*, bool);
    void findMethodReturnTypes        (srcml_archive*, srcml_unit*);
    bool isVoidAccessor               (srcml_archive*, srcml_unit*, const int&);
    bool variableChanged              (srcml_archive*, srcml_unit*, const int&, const std::string&);
    void countChangedAttributes       (srcml_archive*, srcml_unit*, bool);
    int  findAssignOperatorDataMembers(srcml_archive*, srcml_unit*, const int&, bool);
    int  findIncrementedDataMembers   (srcml_archive*, srcml_unit*, const int&, bool);
    bool containsNonPrimitive         (srcml_archive*, srcml_unit*, const int&, const std::string&);
    bool usesAttributeObj             (srcml_archive*, srcml_unit*, const int&, const std::vector<std::string>&);
    bool usesAttribute                (srcml_archive*, srcml_unit*, const int&);
    bool isFactory                    (srcml_archive*, srcml_unit*, const int&);
    bool findConstructorCall          (srcml_archive*, srcml_unit*, const int&);
    bool isEmptyMethod                (srcml_archive*, srcml_unit*, const int&);
    void returnsAttributes            (srcml_archive*, srcml_unit*, const int&, bool);

    bool isAttribute                  (std::string&) const;
    bool isPrimitiveContainer         (std::string);
    void methodsReturnPrimitive       (const std::vector<std::string>&, const int&, std::vector<bool>&);
    bool isInheritedMember            (const std::vector<std::string>&, const std::vector<std::string>&, const std::string&);
    int  countPureCalls               (const std::vector<std::string>&) const;
    bool callsAttributesMethod        (const std::vector<std::string>&, const std::vector<std::string>&, const std::vector<std::string>&);

    std::vector<std::string> findParameterTypes    (srcml_archive*, srcml_unit*, const int&);
    std::vector<std::string> findParameterNames    (srcml_archive*, srcml_unit*, const int&);
    std::vector<std::string> findCalls             (srcml_archive*, srcml_unit*, const int&, const std::string&);
    std::vector<std::string> findReturnExpressions (srcml_archive*, srcml_unit*, const int&, bool);
    std::vector<std::string> findLocalNames        (srcml_archive*, srcml_unit*, const int&);

//Attributes:
    std::string                 className;
    std::vector<std::string>    parentClass;
    std::vector<attributeModel> attribute;
    std::vector<methodModel>    method;
    int                         inline_function_count;
    int                         outofline_function_count;
};


bool         checkConst       (std::string);
void         trimWhitespace   (std::string&);
std::string  separateTypeName (const std::string&);


#endif
