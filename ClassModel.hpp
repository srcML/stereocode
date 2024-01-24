// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModel.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef CLASSMODEL_HPP
#define CLASSMODEL_HPP

#include <fstream>
#include "MethodModel.hpp"

extern primitiveTypes                                                             PRIMITIVES;
extern int                                                                        METHODS_PER_CLASS_THRESHOLD;
extern std::unordered_map<int, std::vector<std::pair<std::string, std::string>>>  xpathList; 

class classModel {
public:
         classModel                         (srcml_archive*, srcml_unit*);
         
    void findClassData                      (srcml_archive*, srcml_unit*, std::string, std::string, int);    
    void findClassName                      (srcml_archive*, srcml_unit*);
    void findPartialClass                   (srcml_archive*, srcml_unit*);
    void findFriendFunctionDecl             (srcml_archive*, srcml_unit*);
    void findFriendFunctionParameterType    (srcml_archive* , srcml_unit* );
    void findParentClassName                (srcml_archive*, srcml_unit*);
    void findAttributeName                  (srcml_archive*, srcml_unit*);
    void findAttributeType                  (srcml_archive*, srcml_unit*, int);
    void findNonPrivateAttributeName        (srcml_archive*, srcml_unit*);
    void findNonPrivateAttributeType        (srcml_archive*, srcml_unit*, int);
    void findMethod                         (srcml_archive*, srcml_unit*, std::string, int);
    void findInheritedAttributes            ();

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

    std::string                                      getStereotype                      ()               const;

    std::string                                      getName                            ()               const          { return className;                }
    std::string                                      getNameParsed                      ()               const          { return classNameParsed;          }
    std::string                                      getUnitLanguage                    ()               const          { return unitLanguage;             }
   
    std::vector<std::string>                         getStereotypeList                  ()               const          { return classStereotype;          }
    const std::unordered_set<std::string>&           getParentClassName                 ()               const          { return parentClassName;          }
    const std::unordered_map<std::string, Variable>& getAttribute                       ()                              { return attribute;                }
    const std::vector<Variable>&                     getNonPrivateAttribute             ()                              { return nonPrivateAttribute;      }
    std::vector<methodModel>&                        getMethod                          ()                              { return method;                   }
    const std::unordered_set<std::string>&           getFriendFunctionDecl              ()                              { return friendFunctionDecl;       }
    
    bool                                             IsPartial                          ()               const          { return partial;                }
    bool                                             HasInherited                       ()               const          { return inherited;             }
    bool                                             IsVisited                          ()               const          { return visited;             }
    
    void                                             setInherited                      (bool flag)                      { inherited = flag;             }
    void                                             setVisited                        (bool flag)                      { visited = flag;             }

    srcml_unit*                                            outputUnitWithStereotypes (srcml_archive*, srcml_unit*, srcml_transform_result**, bool);

    void addMethod(methodModel& m)  {
        m.findMethodData(attribute, parentClassName, className);
        method.push_back(m); 
    }

    void appendInheritedAttribute(const std::vector<Variable>& inheritedAttribute) {
        for (const auto& c : inheritedAttribute)
          attribute.insert({c.getName(), c});
    }

protected:
    std::string                                               className{};              // Class name
    std::string                                               classNameParsed{};        // Class name without <>
    std::string                                               unitLanguage{};           // Unit language                 
    std::vector<std::string>                                  classStereotype{};        // Class stereotype
    std::vector<Variable>                                     nonPrivateAttribute{};    // List of attributes that are not private
    std::vector<Variable>                                     attributeOrdered{};       // List of all attributes
    std::vector<methodModel>                                  method{};                 // List of methods      
    std::unordered_set<std::string>                           parentClassName{};        // Set of parent class names
    std::unordered_set<std::string>                           friendFunctionDecl{};     // Set of friend function signatures (C++ only)                                             
    std::unordered_map<std::string, Variable>                 attribute{};              // Map of all attributes. Key is attribute name
    std::unordered_map<int, std::vector<std::string>>         xpath{};                  // Unique xpath for class (classes if partial) along with the unit number
    bool                                                      partial{false};           // Is class a partial class ? (C# only)
    bool                                                      inherited{false};         // Did class inherit the attributes yet? (Used for inheritance)
    bool                                                      visited{false};           // Has class been visited yet when inheriting? (Used for inheritance)    
}; 

#endif
