// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModel.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
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
         classModel                  ();
         classModel                  (srcml_archive*, srcml_unit*, std::string);
         
    void findClassName               (srcml_archive*, srcml_unit*);
    void findPartialClass            (srcml_archive*, srcml_unit*);
    void findFriendFunction          (srcml_archive*, srcml_unit*);
    void findClassData               (srcml_archive*, srcml_unit*, std::string, int);
    void findParentClassName         (srcml_archive*, srcml_unit*);
    void findAttributeName           (srcml_archive*, srcml_unit*);
    void findAttributeType           (srcml_archive*, srcml_unit*, int);
    void findNonPrivateAttributeName (srcml_archive*, srcml_unit*);
    void findNonPrivateAttributeType (srcml_archive*, srcml_unit*, int);
    void findMethod                  (srcml_archive*, srcml_unit*, std::string, int);
    void findInheritedAttributes     ();

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

          std::string                                   getClassStereotype        ()               const;
          std::vector<std::string>                      getStereotypeList         ()               const          { return classStereotype;          }
          std::string                                   getClassName              ()               const          { return className;                }
          std::string                                   getUnitLanguage           ()               const          { return unitLanguage;             }
          std::vector<std::string>                      getParentClassName        ()               const          { return parentClassName;          }
    const std::vector<AttributeInfo>&                   getAttribute              ()               const          { return attribute;                }
    const std::vector<AttributeInfo>&                   getNonPrivateAttribute    ()               const          { return nonPrivateAttribute;      }
    const std::vector<methodModel>&                     getMethod                 ()               const          { return method;                   }
    const std::unordered_set<std::string>&              getFriendFunctionName     ()               const          { return friendFunctionName;       }
          bool                                          getIsPartial              ()               const          { return isPartial;                }
    
    srcml_unit*                                         outputUnitWithStereotypes (srcml_archive*, srcml_unit*, srcml_transform_result**, bool);

    void addMethod (srcml_archive* archive, srcml_unit* unit, methodModel& m, std::string xpathS, int unitNumber)  {
      m.findMethodInfo(archive, unit, attribute, trimWhitespace(className.substr(0, className.find("<"))), xpathS, unitNumber);
      method.push_back(m); 
    }

    void appendInheritedAttribute  (const std::vector<AttributeInfo>& inheritedAttribute){
      attribute.insert(attribute.end(), inheritedAttribute.begin(), inheritedAttribute.end());
    }

protected:
    std::string                                               className;              // Class name
    std::string                                               unitLanguage;           // Unit language where class is                 
    std::vector<std::string>                                  classStereotype;        // Class stereotype
    std::vector<std::string>                                  parentClassName;        // List of parent class names
    std::vector<AttributeInfo>                                attribute;              // List of all attributes
    std::vector<AttributeInfo>                                nonPrivateAttribute;    // List of attributes that are not private
    std::vector<methodModel>                                  method;                 // List of methods      
    std::unordered_set<std::string>                           friendFunctionName;     // List of friend function names (C++ only)
    std::unordered_map<int, std::vector<std::string>>         xpath;                  // Unique xpath for class (classes if partial) along with the unit number
    bool                                                      isPartial;              // Is class a partial class ? (C# only)
}; 

#endif
