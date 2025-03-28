// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file StructureModel.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef STRUCTUREMODEL_HPP
#define STRUCTUREMODEL_HPP

#include "MethodModel.hpp"

class structureModel {
public:
         structureModel                     (srcml_archive*, srcml_unit*, const std::string&);
         
    void findStructureName                  (srcml_archive*, srcml_unit*);
    void findStructureType                  (srcml_archive*, srcml_unit*);
    void findParentStructureName            (srcml_archive*, srcml_unit*);
    void findFieldName                      (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findFieldType                      (srcml_archive*, srcml_unit*, std::vector<variable>&, int);
    void findNonPrivateFieldName            (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findNonPrivateFieldType            (srcml_archive*, srcml_unit*, std::vector<variable>&, int);
    void findMethod                         (srcml_archive*, srcml_unit*, const std::string&, int);
    void findMethodInProperty               (srcml_archive*, srcml_unit*, const std::string&, int);
    void findStructureData                  (srcml_archive*, srcml_unit*, const std::string&, int);

    void computeStructureStereotype();
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
    std::unordered_map<std::string, variable>&             getField                           ()                              { return fields;                                 }
    std::vector<methodModel>&                              getMethods                         ()                              { return methods;                                }

    const std::vector<std::string>&                        getName                            ()               const          { return name;                                   }
    const std::vector<std::string>&                        getStereotypeList                  ()               const          { return stereotype;                             }
    const std::unordered_map<std::string, std::string>&    getParentStructureName             ()               const          { return parentStructureName;                    }
    const std::unordered_map<std::string, variable>&       getNonPrivateAndInheritedField     ()               const          { return nonPrivateAndInheritedFields;           }
    const std::unordered_set<std::string>&                 getInheritedMethodSignatures       ()               const          { return inheritedMethodSignatures;              }    
    const std::unordered_set<std::string>&                 getMethodSignatures                ()               const          { return methodSignatures;                       }    
    
    bool                                                   HasInherited                       ()               const          { return inherited;                              }
    bool                                                   IsVisited                          ()               const          { return visited;                                }
    
    void                                                   setInherited                       (bool flag)                     { inherited = flag;                              }
    void                                                   setVisited                         (bool flag)                     { visited = flag;                                }

    void addMethod(methodModel& m)  {
        methods.push_back(m); 
    }

    void inheritField(const std::unordered_map<std::string, variable>& inheritedNonPrivateField, 
                          const std::string& inheritanceSpecifier) { 
        fields.insert(inheritedNonPrivateField.begin(), inheritedNonPrivateField.end());
        if (inheritanceSpecifier != "private") 
            // Used to chain inheritance
            nonPrivateAndInheritedFields.insert(inheritedNonPrivateField.begin(), inheritedNonPrivateField.end());         
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
    std::unordered_map<std::string, std::string>            parentStructureName;             // Key is parent structure name without whitespaces and namespaces and value is specifier (public, private, or protected).
    std::string                                             structureType;                   // Class, or struct, or an interface
    std::string                                             unitLanguage;                    // Unit language                 
    std::vector<std::string>                                stereotype;                      // Structure stereotype(s)
    std::vector<methodModel>                                methods;                         // List of methods 
    std::unordered_set<std::string>                         methodSignatures;                // List of method signatures
    std::unordered_set<std::string>                         inheritedMethodSignatures;       // List of inherited method signatures                                           
    std::unordered_map<std::string, variable>               fields;                          // Key is field name and value is field object
    std::unordered_map<std::string, variable>               nonPrivateAndInheritedFields;    // Non-private fields of structure + inherited fields from all parent structures
    std::unordered_map<int, std::vector<std::string>>       xpath;                           // Unique xpath for structure (structures if partial in C#) along with the unit number
    bool                                                    inherited{false};                // Did structure inherit the fields yet? (Used for inheritance)
    bool                                                    visited{false};                  // Has structure been visited yet when inheriting? (Used for inheritance)    
    int                                                     constructorDestructorCount{0};   // Number of constructor + destructor methods (Needed for structure stereotypes)
}; 

#endif
