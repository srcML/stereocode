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

#include "MethodModel.hpp"

class classModel {
public:
         classModel                         (srcml_archive*, srcml_unit*, std::vector<methodModel>&, const std::string&, const std::string&, int);
         
    void findClassName                      (srcml_archive*, srcml_unit*);
    void findStructureType                  (srcml_archive*, srcml_unit*);
    void findPartialClass                   (srcml_archive*, srcml_unit*);
    void findFriendFunctionDecl             (srcml_archive*, srcml_unit*);
    void findFriendFunctionParameterType    (srcml_archive* , srcml_unit* );
    void findParentClassName                (srcml_archive*, srcml_unit*);
    void findAttributeName                  (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findAttributeType                  (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findNonPrivateAttributeName        (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findNonPrivateAttributeType        (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findMethod                         (srcml_archive*, srcml_unit*, std::vector<methodModel>&, const std::string&, int);
    void findMethodInProperty               (srcml_archive*, srcml_unit*, std::vector<methodModel>&, const std::string&, int);
    

    void computeClassStereotype();
    void computeMethodStereotype();

    void constructorDestructor();
    void getter();
    void setter();
    void predicate();
    void property();
    void voidAccessor();
    void command();
    void wrapperControllerCollaborator();
    void factory();

    void incidental();
    void stateless();
    void empty();
    
    std::string                                            getStereotype                      ()               const;
    std::string                                            getUnitLanguage                    ()               const          { return unitLanguage;              }
    std::unordered_map<std::string, variable>&             getAttribute                       ()                              { return attribute;                 }
    std::unordered_map<std::string, methodModel>&          getMethods                         ()                              { return methods;                   }

    const std::vector<std::string>&                        getName                            ()               const          { return name;                      }
    const std::vector<std::string>&                        getStereotypeList                  ()               const          { return stereotype;                }
    const std::unordered_map<std::string, std::string>&    getParentClassName                 ()               const          { return parentClassName;           }
    const std::unordered_map<std::string, variable>&       getNonPrivateAndInheritedAttribute ()               const          { return nonPrivateAndInheritedAttribute;       }
    const std::unordered_set<std::string>&                 getInheritedMethodSignature        ()               const          { return inheritedMethodSignature;  }    
    const std::unordered_set<std::string>&                 getFriendFunctionDecl              ()               const          { return friendFunctionDecl;        }
    
    bool                                                   HasInherited                       ()               const          { return inherited;                 }
    bool                                                   IsVisited                          ()               const          { return visited;                   }
    
    void                                                   setInherited                       (bool flag)                     { inherited = flag;                 }
    void                                                   setVisited                         (bool flag)                     { visited = flag;                   }

    srcml_unit*                                            outputUnitWithStereotypes          (srcml_archive*, srcml_unit*, srcml_transform_result**, bool);

    void addMethod(methodModel& m)  {
        methods.insert({m.getNameSignature(), m}); 
    }

    void inheritAttribute(const std::unordered_map<std::string, variable>& inheritedNonPrivateAttribute, 
                                  const std::string& inheritanceSpecifier) { 
        if (inheritanceSpecifier == "private") // C++ Only
            // Inherit attributes as private
            attribute.insert(inheritedNonPrivateAttribute.begin(), inheritedNonPrivateAttribute.end());
        else {
            attribute.insert(inheritedNonPrivateAttribute.begin(), inheritedNonPrivateAttribute.end());
            // Used to chain inheritance
            nonPrivateAndInheritedAttribute.insert(inheritedNonPrivateAttribute.begin(), inheritedNonPrivateAttribute.end());     
        }     
    }

    void appendInheritedMethod(const std::unordered_map<std::string, methodModel>& parentMethods, 
                               const std::unordered_set<std::string>& parentInheritedMethodSignature) {       
        inheritedMethodSignature.insert(parentInheritedMethodSignature.begin(), parentInheritedMethodSignature.end());  

        for (const auto& pair : parentMethods)
            inheritedMethodSignature.insert(pair.first);      
    }


private:
    std::vector<std::string>                                name;                            // Original name, name without whitespaces, name without whitespaces, namespaces, and generic types in <>, same as last but without <>
    std::unordered_map<std::string, std::string>            parentClassName;                 // Key is parent class name without whitespaces and namespaces and value is specifier (public, private, or protected).
    std::string                                             structureType;                   // Class, or struct, or an interface
    std::string                                             unitLanguage;                    // Unit language                 
    std::vector<std::string>                                stereotype;                      // Class stereotype(s)
    std::unordered_map<std::string, methodModel>            methods;                         // List of methods 
    std::unordered_set<std::string>                         inheritedMethodSignature;        // List of inherited method signatures
    std::unordered_set<std::string>                         friendFunctionDecl;              // Set of friend function signatures (C++ only)                                             
    std::unordered_map<std::string, variable>               attribute;                       // Key is attribute name and value is attribute object
    std::unordered_map<std::string, variable>               nonPrivateAndInheritedAttribute; // Non-private attributes of class + inherited attributes from all parent classes
    std::unordered_map<int, std::string>                    xpath;                           // Unique xpath for class (classes if partial in C#) along with the unit number
    bool                                                    inherited{false};                // Did class inherit the attributes yet? (Used for inheritance)
    bool                                                    visited{false};                  // Has class been visited yet when inheriting? (Used for inheritance)    
    int                                                     numConstructorDestructor{0};     // Number of constructor + destructor methods (Needed for class stereotypes)
}; 

#endif
