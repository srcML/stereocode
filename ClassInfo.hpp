#ifndef CLASS_INFO_HPP
#define CLASS_INFO_HPP

//ClassInfo for stereocode
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <srcml.h>

class ClassInfo{
public:
    ClassInfo () {};
    ClassInfo (srcml_archive*, srcml_unit*, srcml_unit*);

    std::string getClassName              ()      const { return class_name;               }
    int         getInlineFunctionCount    ()      const { return inline_function_count;    }
    int         getOutoflineFunctionCount ()      const { return outofline_function_count; }
    std::string getReturnType             (int i) const { return return_types[i];          }
    std::string getMethodHeader           (int i) const { return headers[i];               }
    int         getNumberOfMethods        ()      const { return headers.size();           }

    srcml_unit* writeStereotypeAttribute  (srcml_archive*, srcml_unit*, bool);

    void stereotypeGetters               (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypePredicates            (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeProperties            (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeVoidAccessor          (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeSetters               (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeCommand               (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeCollaborationalCommand(srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeCollaborators         (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeFactories             (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeEmpty                 (srcml_archive*, srcml_unit*, srcml_unit*);
    void stereotypeStateless             (srcml_archive*, srcml_unit*, srcml_unit*);

    void printMethodHeaders              ();
    void printReturnTypes                ();
    void printStereotypes                ();
    void printAttributes                 ();
    void printReportToFile               (std::ofstream&, const std::string&);


private:
    void readPrimitives();

    void findClassName                (srcml_archive*, srcml_unit*);
    void findParentClassName          (srcml_archive*, srcml_unit*);
    void findAttributeNames           (srcml_archive*, srcml_unit*);
    void findAttributeTypes           (srcml_archive*, srcml_unit*);
    void findMethodHeaders            (srcml_archive*, srcml_unit*, bool);
    void findMethodReturnTypes        (srcml_archive*, srcml_unit*);
    void findMethodNames              (srcml_archive*, srcml_unit*);
    void findParameterLists           (srcml_archive*, srcml_unit*);
    bool isVoidAccessor               (srcml_archive*, srcml_unit*, const int&);
    bool variableChanged              (srcml_archive*, srcml_unit*, const int&, const std::string&);
    void countChangedDataMembers      (srcml_archive*, srcml_unit*, bool);
    int  findAssignOperatorDataMembers(srcml_archive*, srcml_unit*, const int&, bool);
    int  findIncrementedDataMembers   (srcml_archive*, srcml_unit*, const int&, bool);
    bool containsNonPrimitive         (srcml_archive*, srcml_unit*, const int&, const std::string&);
    bool usesAttributeObj             (srcml_archive*, srcml_unit*, const int&, const std::vector<std::string>&);
    bool usesAttribute                (srcml_archive*, srcml_unit*, const int&);
    bool isFactory                    (srcml_archive*, srcml_unit*, const int&);
    bool findConstructorCall          (srcml_archive*, srcml_unit*, const int&);
    bool isEmptyMethod                (srcml_archive*, srcml_unit*, const int&);
    void returnsDataMembers           (srcml_archive*, srcml_unit*, const int&, bool);

    void addConstSpecifier            (std::string);
    bool isAttribute                  (std::string&) const;
    bool isPrimitiveContainer         (std::string);
    bool isPrimitive                  (const std::string&);
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
    std::string              class_name;
    std::vector<std::string> parent_class_names;
    int                      inline_function_count;
    int                      outofline_function_count;
    std::vector<std::string> attribute_names;
    std::vector<std::string> attribute_types;
    std::vector<std::string> method_names;
    std::vector<std::string> parameter_lists;
    std::vector<std::string> headers;
    std::vector<std::string> return_types;
    std::vector<std::string> specifiers;
    std::vector<bool>        returns_data_members;
    std::vector<int>         changes_to_data_members;
    std::vector<std::string> stereotypes;
    std::vector<std::string> primitive_types;
};


void        trimWhitespace   (std::string&);
std::string separateTypeName (const std::string&);



#endif

