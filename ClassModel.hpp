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

class classModel {
public:
         classModel                         (srcml_archive*, srcml_unit*);
         
    void findClassData                      (srcml_archive*, srcml_unit*, const std::string&, const std::string&, int);    
    void findClassName                      (srcml_archive*, srcml_unit*);
    void findStructureType                  (srcml_archive*, srcml_unit*);
    void findPartialClass                   (srcml_archive*, srcml_unit*);
    void findFriendFunctionDecl             (srcml_archive*, srcml_unit*);
    void findFriendFunctionParameterType    (srcml_archive* , srcml_unit* );
    void findParentClassName                (srcml_archive*, srcml_unit*);
    void findAttributeName                  (srcml_archive*, srcml_unit*, std::vector<Variable>&);
    void findAttributeType                  (srcml_archive*, srcml_unit*, std::vector<Variable>&, int);
    void findNonPrivateAttributeName        (srcml_archive*, srcml_unit*, std::vector<Variable>&);
    void findNonPrivateAttributeType        (srcml_archive*, srcml_unit*, std::vector<Variable>&, int);
    void findMethod                         (srcml_archive*, srcml_unit*, const std::string&, int);
    void findMethodInProperty               (srcml_archive*, srcml_unit*, const std::string&, int);
    

    void ComputeClassStereotype();
    void ComputeMethodStereotype();
    void getter();
    void setter();
    void predicate();
    void property();
    void voidAccessor();
    void command();
    void collaboratorController();
    void factory();
    void empty();
    void stateless();
    void wrapper();

    std::string                                            getStereotype                      ()               const;
    std::vector<std::string>                               getName                            ()               const          { return name;                }
    std::string                                            getUnitLanguage                    ()               const          { return unitLanguage;        }
   
    std::vector<std::string>                               getStereotypeList                  ()               const          { return stereotype;            }
    const std::unordered_map<std::string, std::string>&    getParentClassName                 ()               const          { return parentClassName;       }
    std::unordered_map<std::string, Variable>&             getAttribute                       ()                              { return attribute;             }
    const std::unordered_map<std::string, Variable>&       getNonPrivateAttribute             ()                              { return nonPrivateAttribute;   }
    std::vector<methodModel>&                              getMethod                          ()                              { return method;                }
    std::unordered_set<std::string>&                       getInheritedMethodSignature        ()                              { return inheritedMethodSignature;  }
    std::unordered_set<std::string>&                       getMethodSignature                 ()                              { return methodSignature;           }    
    const std::unordered_set<std::string>&                 getFriendFunctionDecl              ()                              { return friendFunctionDecl;        }
    
    bool                                                   HasInherited                       ()               const          { return inherited;           }
    bool                                                   IsVisited                          ()               const          { return visited;             }
    
    void                                                   setInherited                       (bool flag)                     { inherited = flag;           }
    void                                                   setVisited                         (bool flag)                     { visited = flag;             }

    srcml_unit*                                            outputUnitWithStereotypes          (srcml_archive*, srcml_unit*, srcml_transform_result**, bool);

    void addMethod(methodModel& m)  {
        method.push_back(m); 
    }

    void appendInheritedAttribute(const std::unordered_map<std::string, Variable>& inheritedNonPrivateAttribute, 
                                  const std::string& inheritanceSpecifier) { 
        if (inheritanceSpecifier == "private") // C++ Only
            // Inherit attributes as private
            inheritedAsPrivateAttribute.insert(inheritedNonPrivateAttribute.begin(), inheritedNonPrivateAttribute.end()); 
        else {
            attribute.insert(inheritedNonPrivateAttribute.begin(), inheritedNonPrivateAttribute.end());
            // Needed for chaining inheritance
            nonPrivateAttribute.insert(inheritedNonPrivateAttribute.begin(), inheritedNonPrivateAttribute.end()); 
        }            
    }

    void appendInheritedMethod(const std::vector<methodModel>& meths, const std::unordered_set<std::string>& inheritedMeths) {       
        // Check inheritedMeths first.
        // Order matters, classes can override inherited methods
        inheritedMethodSignature.insert(inheritedMeths.begin(), inheritedMeths.end());  

        for (const auto& m : meths)
            inheritedMethodSignature.insert(m.getNameSignature());          
    }

    void appendInheritedPrivateAttribute() {
        attribute.insert(inheritedAsPrivateAttribute.begin(), inheritedAsPrivateAttribute.end());
    }

    void buildMethodSignature() {
        for (const auto& m : method)
            methodSignature.insert(m.getNameSignature());  
    }
private:
    std::vector<std::string>                                name;                           // Original name, name without whitespaces, name without whitespaces, namespaces, and generic types in <>, same as last but without <>
    std::unordered_map<std::string, std::string>            parentClassName;                // Key is parent class name without whitespaces and namespaces and value is specifier (public, private, or protected).
    std::string                                             structureType;                  // Class, or struct, or an interface
    std::string                                             unitLanguage;                   // Unit language                 
    std::vector<std::string>                                stereotype;                     // Class stereotype
    std::vector<methodModel>                                method;                         // List of methods
    std::unordered_set<std::string>                         methodSignature;                // List of method signatures  
    std::unordered_set<std::string>                         inheritedMethodSignature;       // List of inherited method signatures
    std::unordered_set<std::string>                         friendFunctionDecl;             // Set of friend function signatures (C++ only)                                             
    std::unordered_map<std::string, Variable>               attribute;                      // Map of all attributes. Key is attribute name
    std::unordered_map<std::string, Variable>               nonPrivateAttribute;            // Non-private attributes (Only used by inheriting classes)
    std::unordered_map<std::string, Variable>               inheritedAsPrivateAttribute;    // Attributes inherited as private (C++ only, combined with the attribute map) 
    std::unordered_map<int, std::string>                    xpath;                          // Unique xpath for class (classes if partial) along with the unit number
    bool                                                    inherited{false};               // Did class inherit the attributes yet? (Used for inheritance)
    bool                                                    visited{false};                 // Has class been visited yet when inheriting? (Used for inheritance)    
}; 

#endif
