// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file type_model.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef TYPE_MODEL_HPP
#define TYPE_MODEL_HPP

#include "function_model.hpp"

class typeModel {
public:
                                                              typeModel                          (const std::string&);

    std::string                                               getStereotypesString               ()               const;
    std::string                                               getParentsString                   ()               const;
    std::string                                               getStructureType                   ()               const          { return structureType;                         }
    std::unordered_map<std::string, variableModel>&           getDataMembers                     ()                              { return dataMembers;                            }
    std::vector<functionModel>&                               getMethods                         ()                              { return methods;                                }
    const std::string&                                        getUnitLanguage                    ()               const          { return unitLanguage;                           }
    const std::vector<std::string>&                           getName                            ()               const          { return name;                                   }
    const std::vector<std::string>&                           getStereotypes                     ()               const          { return stereotypes;                            }
    const std::unordered_set<std::string>&                    getParents                         ()               const          { return parents;                                }  
    const std::unordered_set<std::string>&                    getMethodSignatures                ()               const          { return methodSignatures;                       }    
    const std::unordered_map<int, std::vector<std::string>>&  getXpath                           ()               const          { return xpath;                                  }    
    int                                                       getConstructorDestructorCount      ()               const          { return constructorDestructorCount;             }
    bool                                                      isInherited                        ()               const          { return inherited;                              }
    bool                                                      isVisited                          ()               const          { return visited;                                }

    void                                                      setInherited                       (bool flag)                     { inherited = flag;                              }
    void                                                      setVisited                         (bool flag)                     { visited = flag;                                }
    void                                                      setStereotype                      (const std::string& s)          { stereotypes.push_back(s);                      }
    void                                                      setConstructorDestructorCount      (int c)                         { constructorDestructorCount = c;                }     
    void                                                      mergeData                          (typeModel& other);
    void                                                      findData                           (const std::string&, int);
    // Inheritance does not need to check for private data members or methods, this is because
    //   a method will only use a data member or call a method if it is not private, so we can simply collect them all
    //
    void appendInheritedDataMembers(const std::unordered_map<std::string, variableModel>& inheritedDataMembers, const std::unordered_set<std::string>& parentMethods) { 
        dataMembers.insert(inheritedDataMembers.begin(), inheritedDataMembers.end());   
        methodSignatures.insert(parentMethods.begin(), parentMethods.end());      
    }

    void buildMethodSignature() { for (const auto& m : methods) methodSignatures.insert(m.getNameSignature()); }
    void addMethod(functionModel& m)  { methods.push_back(m); }

private:
    void                                                    findName                           ();
    void                                                    findParentName                     ();
    std::string                                             findPropertyReturnType             ();
    void                                                    findDataMemberName                 (std::vector<variableModel>&);
    void                                                    findDataMemberType                 (std::vector<variableModel>&);
    void                                                    findMethod                         (const std::string&, int);
    void                                                    findProperty                       (const std::string&, int);
    void                                                    findStructureType                  ();     
    void                                                    findMethodsInProperty              (const std::string&, const std::string&, int);

    std::string                                             unitLanguage;                    // Unit language
    std::string                                             structureType;                   // e.g., class, struct, interface, enum, etc.   
    std::vector<std::string>                                name;                            // Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and generic types in <> | same as last but without <>
    std::vector<std::string>                                stereotypes;                     // Type stereotype(s)
    std::vector<functionModel>                              methods;                         // List of methods 
    std::unordered_set<std::string>                         parents;                         // Parent names without whitespaces and namespaces
    std::unordered_set<std::string>                         methodSignatures;                // List of method signatures (including parent method signatures)
    std::unordered_map<std::string, variableModel>          dataMembers;                     // Key is data member name and value is data member object (including inherited data members)
    std::unordered_map<int, std::vector<std::string>>       xpath;                           // Unique xpath for type (types if partial in C#) along with the unit number
    bool                                                    inherited{false};                // Did type inherit the data members yet? (Used for inheritance)
    bool                                                    visited{false};                  // Has type been visited yet when inheriting? (Used for inheritance)    
    int                                                     constructorDestructorCount{0};   // Number of constructor + destructor methods (Needed for type stereotypes)
}; 

#endif
