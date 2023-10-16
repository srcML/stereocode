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

extern primitiveTypes PRIMITIVES;
extern int            METHODS_PER_CLASS_THRESHOLD;

class classModel {
public:
         classModel                  ();
         classModel                  (srcml_archive*, srcml_unit*, std::string, int);
         
    void findClassName               (srcml_archive*, srcml_unit*);
    void findClassInfo               (srcml_archive*, srcml_unit*, int, int);
    void findFriendFunction          (srcml_archive*, srcml_unit*);
    void findParentClassName         (srcml_archive*, srcml_unit*);
    void findAttributeName           (srcml_archive*, srcml_unit*);
    void findAttributeType           (srcml_archive*, srcml_unit*);
    void findNonPrivateAttributeName (srcml_archive*, srcml_unit*);
    void findNonPrivateAttributeType (srcml_archive*, srcml_unit*);
    void findMethod                  (srcml_archive*, srcml_unit*, int, int);
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

    void addMethod (srcml_archive* archive, srcml_unit* unit, methodModel& m, std::string methodHeader)  {
        m.findMethodInfo(archive, unit, attribute, trimWhitespace(className.substr(0, className.find("<"))));
        method[methodHeader] = m; // Use method header as key allows for overloaded methods (same name, different parameters) to be added
    }
    void appendInheritedAttribute  (const std::vector<AttributeInfo>& inheritedAttribute){
        attribute.insert(attribute.end(), inheritedAttribute.begin(), inheritedAttribute.end());
    }
    srcml_unit*                                   outputUnitWithStereotypes(srcml_archive*, srcml_unit*, srcml_transform_result**, bool);

    
          std::string                                   getClassStereotype        () const;
          std::string                                   getClassName              () const          { return className;           }
    const std::vector<std::string>&                     getParentClassName        () const          { return parentClassName;     }
          std::vector<AttributeInfo>                    getAttribute              () const          { return attribute;           }
    const std::vector<AttributeInfo>&                   getNonPrivateAttribute    () const          { return nonPrivateAttribute; }
          std::unordered_map<std::string, methodModel>& getMethod                 ()                { return method;              }
    const std::vector<std::string>&                     getFriendFunctionName     () const          { return friendFunctionName;  }
          std::string                                   getXpath                  (int unitNumber)  { return xpath[unitNumber];  }

protected:
    std::vector<std::string>                     classStereotype;        // Class stereotype
    std::string                                  className;              // Class name
    std::vector<std::string>                     parentClassName;        // List of parent class names
    std::vector<AttributeInfo>                   attribute;              // List of all attributes
    std::vector<AttributeInfo>                   nonPrivateAttribute;    // List of attributes that are not private
    std::unordered_map<std::string, methodModel> method;                 // List of methods      
    std::vector<std::string>                     friendFunctionName;     // List of friend function names
    std::unordered_map<int, std::string>         xpath;                  // unit number along with unique xpath for the class
}; 

#endif
