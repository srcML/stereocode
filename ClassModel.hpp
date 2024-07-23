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
         classModel                         (srcml_archive*, srcml_unit*, const std::string&);
         
    void findClassName                      (srcml_archive*, srcml_unit*);
    void findStaticSpecifier                (srcml_archive*, srcml_unit*);
    void findStructureType                  (srcml_archive*, srcml_unit*);
    void findParentClassName                (srcml_archive*, srcml_unit*);
    void findAttributeName                  (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findAttributeType                  (srcml_archive*, srcml_unit*, std::vector<variable>&, int);
    void findNonPrivateAttributeName        (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findNonPrivateAttributeType        (srcml_archive*, srcml_unit*, std::vector<variable>&, int);
    void findMethod                         (srcml_archive*, srcml_unit*, std::vector<methodModel>&, const std::string&, int);
    void findMethodInProperty               (srcml_archive*, srcml_unit*, std::vector<methodModel>&, const std::string&, int);
    void findClassData                      (srcml_archive*, srcml_unit*, std::vector<methodModel>&, const std::string&, int);

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
    const std::string&                                     getUnitLanguage                    ()               const          { return unitLanguage;                           }
    std::unordered_map<std::string, variable>&             getAttribute                       ()                              { return attributes;                             }
    std::vector<methodModel>&                              getMethods                         ()                              { return methods;                                }

    const std::vector<std::string>&                        getName                            ()               const          { return name;                                   }
    const std::vector<std::string>&                        getStereotypeList                  ()               const          { return stereotype;                             }
    const std::unordered_map<std::string, std::string>&    getParentClassName                 ()               const          { return parentClassName;                        }
    const std::unordered_map<std::string, variable>&       getNonPrivateAndInheritedAttribute ()               const          { return nonPrivateAndInheritedAttributes;       }
    const std::unordered_set<std::string>&                 getInheritedMethodSignatures       ()               const          { return inheritedMethodSignatures;              }    
    const std::unordered_set<std::string>&                 getMethodSignatures                ()               const          { return methodSignatures;                       }    
    
    bool                                                   HasInherited                       ()               const          { return inherited;                              }
    bool                                                   IsVisited                          ()               const          { return visited;                                }
    
    void                                                   setInherited                       (bool flag)                     { inherited = flag;                              }
    void                                                   setVisited                         (bool flag)                     { visited = flag;                                }

    void addMethod(methodModel& m)  {
        methods.push_back(m); 
    }

    void inheritAttribute(const std::unordered_map<std::string, variable>& inheritedNonPrivateAttribute, 
                          const std::string& inheritanceSpecifier) { 
        attributes.insert(inheritedNonPrivateAttribute.begin(), inheritedNonPrivateAttribute.end());
        if (inheritanceSpecifier != "private") 
            // Used to chain inheritance
            nonPrivateAndInheritedAttributes.insert(inheritedNonPrivateAttribute.begin(), inheritedNonPrivateAttribute.end());         
    }

    void appendInheritedMethod(const std::unordered_set<std::string>& parentMethods, 
                               const std::unordered_set<std::string>& parentInheritedMethodSignature) {       
        inheritedMethodSignatures.insert(parentInheritedMethodSignature.begin(), parentInheritedMethodSignature.end());  
        inheritedMethodSignatures.insert(parentMethods.begin(), parentMethods.end());
    }

    void buildMethodSignature() {
        for (const auto& m : methods) 
            methodSignatures.insert(m.getNameSignature());
    }

private:
    std::vector<std::string>                                name;                            // Original name, name without whitespaces, name without whitespaces, namespaces, and generic types in <>, same as last but without <>
    std::unordered_map<std::string, std::string>            parentClassName;                 // Key is parent class name without whitespaces and namespaces and value is specifier (public, private, or protected).
    std::string                                             structureType;                   // Class, or struct, or an interface
    std::string                                             unitLanguage;                    // Unit language                 
    std::vector<std::string>                                stereotype;                      // Class stereotype(s)
    std::vector<methodModel>                                methods;                         // List of methods 
    std::unordered_set<std::string>                         methodSignatures;                // List of method signatures
    std::unordered_set<std::string>                         inheritedMethodSignatures;       // List of inherited method signatures                                           
    std::unordered_map<std::string, variable>               attributes;                      // Key is attribute name and value is attribute object
    std::unordered_map<std::string, variable>               nonPrivateAndInheritedAttributes;// Non-private attributes of class + inherited attributes from all parent classes
    std::unordered_map<int, std::vector<std::string>>       xpath;                           // Unique xpath for class (classes if partial in C#) along with the unit number
    bool                                                    staticClass{false};              // Is class static?
    bool                                                    inherited{false};                // Did class inherit the attributes yet? (Used for inheritance)
    bool                                                    visited{false};                  // Has class been visited yet when inheriting? (Used for inheritance)    
    int                                                     constructorDestructorCount{0};   // Number of constructor + destructor methods (Needed for class stereotypes)
}; 

#endif
