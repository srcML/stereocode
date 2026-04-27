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

#include <set>
#include "function_model.hpp"

class typeModel {
public:
                                                                     typeModel                          (const std::string&);
       
    std::string                                                      getStereotypesString               ()               const;
    std::string                                                      getParentsString                   (bool)           const;
    std::string                                                      getMethodSignaturesString          (bool)           const;
    std::string                                                      getStructure                       ()               const          { return structure;                              }
    const std::unordered_map<std::string, variableModel>&            getFields                          ()               const          { return fields;                                 }
    std::vector<functionModel>&                                      getMethods                         ()                              { return methods;                                }
    const std::string&                                               getUnitLanguage                    ()               const          { return unitLanguage;                           }
    const std::vector<std::string>&                                  getName                            ()               const          { return name;                                   }
    const std::vector<std::string>&                                  getStereotypes                     ()               const          { return stereotypes;                            }
    const std::vector<std::vector<std::string>>&                     getParentNames                     ()               const          { return parentNames;                            } 
    const std::vector<std::vector<std::string>>&                     getInheritedParentNames            ()               const          { return inheritedParentNames;                   } 
    const std::set<std::pair<std::string, std::string>>&             getMethodSignatures                ()               const          { return methodSignatures;                       }      
    const std::set<std::pair<std::string, std::string>>&             getInheritedMethodSignatures       ()               const          { return inheritedMethodSignatures;              }   
    const std::set<std::pair<std::string, std::string>>&             getDeclMethodSignatures            ()               const          { return declMethodSignatures;                   }
    const std::set<std::pair<std::string, std::string>>&             getInheritedDeclMethodSignatures   ()               const          { return inheritedDeclMethodSignatures;          }
    std::unordered_map<std::string, variableModel>&                  getInheritedFields                 ()                              { return inheritedFields;                        }
    const std::unordered_map<int, std::vector<std::string>>&         getXpath                           ()               const          { return xpath;                                  }    
    int                                                              getConstructorDestructorCount      ()               const          { return constructorDestructorCount;             }
    bool                                                             isInherited                        ()               const          { return inherited;                              }
    bool                                                             isVisited                          ()               const          { return visited;                                }
    bool                                                             hasUnknownParent                   ()               const          { return unknownParent;                          }

    void                                                             setInherited                       (bool flag)                     { inherited = flag;                              }
    void                                                             setVisited                         (bool flag)                     { visited = flag;                                }
    void                                                             setUnknownParent                   (bool flag)                     { unknownParent = flag;                          }
    void                                                             setStereotype                      (const std::string& s)          { stereotypes.push_back(s);                      }
    void                                                             setConstructorDestructorCount      (int c)                         { constructorDestructorCount = c;                }     
    void                                                             findData                           (const std::string&, const std::string&, int);
    void                                                             findDataAfterCollection            ();
    void                                                             mergeData                          (typeModel& other);
    
    void appendInheritedDataMembers(const std::unordered_map<std::string, variableModel>& fields_, 
                                    const std::unordered_map<std::string, variableModel>& inheritedFields_,
                                    const std::set<std::pair<std::string, std::string>>& methodSignatures_,
                                    const std::set<std::pair<std::string, std::string>>& inheritedMethodSignatures_,
                                    const std::set<std::pair<std::string, std::string>>& declMethodSignatures_,
                                    const std::set<std::pair<std::string, std::string>>& inheritedDeclMethodSignatures_,
                                    const std::vector<std::vector<std::string>>& parentNames_,
                                    const std::vector<std::vector<std::string>>& inheritedParentNames_,
                                    bool hasUnknownParent_) { 
        inheritedFields.insert(fields_.begin(), fields_.end());
        inheritedFields.insert(inheritedFields_.begin(), inheritedFields_.end());
        inheritedMethodSignatures.insert(methodSignatures_.begin(), methodSignatures_.end()); // From a parent type
        inheritedMethodSignatures.insert(inheritedMethodSignatures_.begin(), inheritedMethodSignatures_.end());
        inheritedDeclMethodSignatures.insert(declMethodSignatures_.begin(), declMethodSignatures_.end()); // From a parent type
        inheritedDeclMethodSignatures.insert(inheritedDeclMethodSignatures_.begin(), inheritedDeclMethodSignatures_.end());
        inheritedParentNames.insert(inheritedParentNames.end(), parentNames_.begin(), parentNames_.end());
        inheritedParentNames.insert(inheritedParentNames.end(), inheritedParentNames_.begin(), inheritedParentNames_.end());

        // If the parent has unknown parents, then the child also has unknown parents
        // This is needed because if the parent is inherited already, then we will not be able to reach it again to check for unknown parents, 
        //  so we need to set the child as having unknown parents at this point if the parent has unknown parents
        if (!unknownParent && hasUnknownParent_) {
            unknownParent = true;
        }
    }

    void buildMethodSignature() { 
        for (const auto& m : methods) {
            methodSignatures.insert(m.getNameSignature()); 
        }
    }
    void addMethod(functionModel&& m)  { 
        methods.push_back(m); 
    }

private:
    void                                                    findName                           ();
    void                                                    findParentNames                    ();
    std::string                                             findPropertyReturnType             ();
    void                                                    findFieldNames                     (std::vector<variableModel>&);
    void                                                    findFieldTypes                     (std::vector<variableModel>&);
    void                                                    findMethods                        (const std::string&, const std::string&, int);
    void                                                    findMethodDeclNames                (std::vector<std::pair<std::string, std::string>>&);
    void                                                    findMethodDeclParameters           (std::vector<std::pair<std::string, std::string>>&);
    void                                                    findProperties                     (const std::string&, const std::string&, int);
    void                                                    findStructure                      ();     
    void                                                    findMethodsInProperty              (const std::string&, const std::string&, const std::string&, int);
    void                                                    findAttributesOrAnnotations        (); 

    std::string                                             unitLanguage;                    // Unit language (C, C#, C++, Java)
    std::string                                             structure;                       // class, struct, interface, enum, union 
    std::vector<std::string>                                name;                            // Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and and in-between generic in ( <> ) | same as last but without ( <> )
    std::vector<std::string>                                stereotypes;                     // Stereotype(s)
    std::vector<std::string>                                attributesOrAnnotations;         // Attributes (C#) or annotations (Java)          
    std::vector<functionModel>                              methods;                         // Methods (including methods inside properties for C#)
    std::vector<std::vector<std::string>>                   parentNames;                     // Parent names where each is of Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and and in-between generic in ( <> ) | same as last but without ( <> )
    std::vector<std::vector<std::string>>                   inheritedParentNames;            // Inherited parent names
    std::set<std::pair<std::string, std::string>>           declMethodSignatures;            // Method declaration signatures (Unique)
    std::set<std::pair<std::string, std::string>>           inheritedDeclMethodSignatures;   // Inherited method declaration signatures (Unique)
    std::set<std::pair<std::string, std::string>>           methodSignatures;                // Method signatures (Unique)
    std::set<std::pair<std::string, std::string>>           inheritedMethodSignatures;       // Inherited method signatures (Unique)
    std::unordered_map<std::string, variableModel>          fields;                          // Key is field name and value is the field object
    std::unordered_map<std::string, variableModel>          inheritedFields;                 // Inherited fields
    std::unordered_map<int, std::vector<std::string>>       xpath;                           // Unique xpath for type (types if partial in C# or duplicate types) along with the unit number
    bool                                                    inherited{false};                // Did type inherit the data members yet? (Used for inheritance)
    bool                                                    visited{false};                  // Has type been visited yet when inheriting? (Used for inheritance)    
    bool                                                    unknownParent{false};            // True if type has an unknown parent (Used for inheritance and for checking degenerate methods)
    int                                                     constructorDestructorCount{0};   // Number of constructor + destructor methods (Needed for type stereotypes calculations)
}; 

#endif
