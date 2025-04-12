// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModel.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
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
    void findClassType                      (srcml_archive*, srcml_unit*);
    void findParentClassName                (srcml_archive*, srcml_unit*);
    void findFieldName                      (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findFieldType                      (srcml_archive*, srcml_unit*, std::vector<variable>&, int);
    void findNonPrivateFieldName            (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findNonPrivateFieldType            (srcml_archive*, srcml_unit*, std::vector<variable>&, int);
    void findMethod                         (srcml_archive*, srcml_unit*, const std::string&, int);
    void findMethodInProperty               (srcml_archive*, srcml_unit*, const std::string&, int);
    void findClassData                      (srcml_archive*, srcml_unit*, const std::string&, int);

    std::string                                               getStereotype                      ()               const;
    std::unordered_map<std::string, variable>&                getField                           ()                              { return fields;                                 }
    std::vector<methodModel>&                                 getMethods                         ()                              { return methods;                                }
    
    const std::string&                                        getUnitLanguage                    ()               const          { return unitLanguage;                           }
    const std::vector<std::string>&                           getName                            ()               const          { return name;                                   }
    const std::vector<std::string>&                           getStereotypeList                  ()               const          { return stereotype;                             }
    const std::unordered_map<std::string, std::string>&       getParentClassName                 ()               const          { return parentClassName;                        }
    const std::unordered_map<std::string, variable>&          getNonPrivateInheritedField        ()               const          { return nonPrivateInheritedFields;              }
    const std::unordered_set<std::string>&                    getInheritedMethodSignatures       ()               const          { return inheritedMethodSignatures;              }    
    const std::unordered_set<std::string>&                    getMethodSignatures                ()               const          { return methodSignatures;                       }    
    const std::unordered_map<int, std::vector<std::string>>&  getXpath                           ()               const          { return xpath;                                  }    
    int                                                       getConstructorDestructorCount      ()               const          { return constructorDestructorCount;             }
    bool                                                      isInherited                        ()               const          { return inherited;                              }
    bool                                                      isVisited                          ()               const          { return visited;                                }
    
    void                                                      setInherited                       (bool flag)                     { inherited = flag;                              }
    void                                                      setVisited                         (bool flag)                     { visited = flag;                                }
    void                                                      setStereotype                      (const std::string& s)          { stereotype.push_back(s);                       }
    void                                                      setConstructorDestructorCount      (int c)                         { constructorDestructorCount = c;                }
    
    void addMethod(methodModel& m)  { methods.push_back(m); }

    void inheritField(const std::unordered_map<std::string, variable>& inheritedNonPrivateField, const std::string& inheritanceSpecifier) { 
        fields.insert(inheritedNonPrivateField.begin(), inheritedNonPrivateField.end());
        if (inheritanceSpecifier != "private") 
            nonPrivateInheritedFields.insert(inheritedNonPrivateField.begin(), inheritedNonPrivateField.end()); // Used to chain inheritance          
    }

    void appendInheritedMethod(const std::unordered_set<std::string>& parentMethods, const std::unordered_set<std::string>& parentInheritedMethodSignature) {       
        inheritedMethodSignatures.insert(parentInheritedMethodSignature.begin(), parentInheritedMethodSignature.end());  
        inheritedMethodSignatures.insert(parentMethods.begin(), parentMethods.end());
    }

    void buildMethodSignature() { for (const auto& m : methods) methodSignatures.insert(m.getNameSignature()); }

private:
    std::vector<std::string>                                name;                            // Size = 4 containing | Original name | name without whitespaces | name without whitespaces, namespaces, and generic types in <> | same as last but without <>
    std::unordered_map<std::string, std::string>            parentClassName;                 // Key is parent class name without whitespaces and namespaces and value is specifier (public, private, or protected).
    std::string                                             classType;                       // Class, or struct, or an interface
    std::string                                             unitLanguage;                    // Unit language                 
    std::vector<std::string>                                stereotype;                      // Class stereotype(s)
    std::vector<methodModel>                                methods;                         // List of methods 
    std::unordered_set<std::string>                         methodSignatures;                // List of method signatures
    std::unordered_set<std::string>                         inheritedMethodSignatures;       // List of inherited method signatures                                           
    std::unordered_map<std::string, variable>               fields;                          // Key is field name and value is field object
    std::unordered_map<std::string, variable>               nonPrivateInheritedFields;       // Non-private fields of class + inherited fields from all parent classs
    std::unordered_map<int, std::vector<std::string>>       xpath;                           // Unique xpath for class (classs if partial in C#) along with the unit number
    bool                                                    inherited{false};                // Did class inherit the fields yet? (Used for inheritance)
    bool                                                    visited{false};                  // Has class been visited yet when inheriting? (Used for inheritance)    
    int                                                     constructorDestructorCount{0};   // Number of constructor + destructor methods (Needed for class stereotypes)
}; 

#endif
