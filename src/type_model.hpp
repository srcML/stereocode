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
    std::string                                                      getParentsString                   ()               const;
    std::string                                                      getInheritedParentsString          ()               const;
    std::string                                                      getMethodSignaturesString          ()               const;
    std::string                                                      getInheritedMethodSignaturesString ()               const;
    std::string                                                      getStructure                       ()               const          { return structure;                              }
    const std::unordered_map<std::string, variableModel>&            getFields                          ()               const          { return fields;                                 }
    std::vector<functionModel>&                                      getMethods                         ()                              { return methods;                                }
    const std::string&                                               getUnitLanguage                    ()               const          { return unitLanguage;                           }
    const std::vector<std::string>&                                  getName                            ()               const          { return name;                                   }
    const std::vector<std::string>&                                  getStereotypes                     ()               const          { return stereotypes;                            }
    const std::unordered_set<std::string>&                           getParentNames                     ()               const          { return parentNames;                            }  
    const std::set<std::pair<std::string, std::string>>&             getMethodSignatures                ()               const          { return methodSignatures;                       }    
    const std::unordered_set<std::string>&                           getInheritedParentNames            ()               const          { return inheritedParentNames;                   }  
    const std::set<std::pair<std::string, std::string>>&             getInheritedMethodSignatures       ()               const          { return inheritedMethodSignatures;              }   
    std::unordered_map<std::string, variableModel>&                  getInheritedFields                 ()                              { return inheritedFields;                        }
    const std::unordered_map<int, std::vector<std::string>>&         getXpath                           ()               const          { return xpath;                                  }    
    int                                                              getConstructorDestructorCount      ()               const          { return constructorDestructorCount;             }
    bool                                                             isInherited                        ()               const          { return inherited;                              }
    bool                                                             isVisited                          ()               const          { return visited;                                }
       
    void                                                             setInherited                       (bool flag)                     { inherited = flag;                              }
    void                                                             setVisited                         (bool flag)                     { visited = flag;                                }
    void                                                             setStereotype                      (const std::string& s)          { stereotypes.push_back(s);                      }
    void                                                             setConstructorDestructorCount      (int c)                         { constructorDestructorCount = c;                }     
    void                                                             mergeData                          (typeModel& other);
    void                                                             findData                           (const std::string&, const std::string&, int);
    void                                                             findDataAfterCollection            ();
    
    // Inheritance does not need to check for private fields or methods, this is because
    //   a method in a child type will only use a field or call a method if it is not private in the parent type, 
    //   so we can simply collect them all, and during analysis, these private data members will simply not occur
    //
    void appendInheritedDataMembers(const std::unordered_map<std::string, variableModel>& fields_, 
                                    const std::set<std::pair<std::string, std::string>>& methodSignatures_,
                                    const std::unordered_set<std::string>& parentNames_,
                                    const std::unordered_map<std::string, variableModel>& inheritedFields_, 
                                    const std::set<std::pair<std::string, std::string>>& inheritedMethodSignatures_,
                                    const std::unordered_set<std::string>& inheritedParentNames_) { 
        inheritedFields.insert(fields_.begin(), fields_.end());
        inheritedParentNames.insert(parentNames_.begin(), parentNames_.end());
        inheritedMethodSignatures.insert(methodSignatures_.begin(), methodSignatures_.end());
        inheritedFields.insert(inheritedFields_.begin(), inheritedFields_.end());
        inheritedParentNames.insert(inheritedParentNames_.begin(), inheritedParentNames_.end());
        inheritedMethodSignatures.insert(inheritedMethodSignatures_.begin(), inheritedMethodSignatures_.end());
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
    void                                                    findMethod                         (const std::string&, const std::string&, int);
    void                                                    findProperties                     (const std::string&, const std::string&, int);
    void                                                    findStructure                      ();     
    void                                                    findMethodsInProperty              (const std::string&, const std::string&, const std::string&, int);
    void                                                    findAttributesOrAnnotations        (); 

    std::string                                             unitLanguage;                    // Unit language
    std::string                                             structure;                       // class, struct, interface, enum, union 
    std::vector<std::string>                                name;                            // Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and and in-between generic in ( <> ) | same as last but without ( <> )
    std::vector<std::string>                                stereotypes;                     // Type stereotype(s)
    std::vector<std::string>                                attributesOrAnnotations;         // List of attributes (C#) or annotations (Java)          
    std::vector<functionModel>                              methods;                         // List of methods (including methods in properties for C#)
    std::unordered_set<std::string>                         parentNames;                     // Parent names without whitespaces and namespaces
    std::unordered_set<std::string>                         inheritedParentNames;            // Inherited parent names
    std::set<std::pair<std::string, std::string>>           methodSignatures;                // List of method signatures
    std::set<std::pair<std::string, std::string>>           inheritedMethodSignatures;       // Inherited method signatures
    std::unordered_map<std::string, variableModel>          fields;                          // Key is field name and value is the field object
    std::unordered_map<std::string, variableModel>          inheritedFields;                 // Inherited fields
    std::unordered_map<int, std::vector<std::string>>       xpath;                           // Unique xpath for type (types if partial in C# or duplicate types) along with the unit number
    bool                                                    inherited{false};                // Did type inherit the data members yet? (Used for inheritance)
    bool                                                    visited{false};                  // Has type been visited yet when inheriting? (Used for inheritance)    
    int                                                     constructorDestructorCount{0};   // Number of constructor + destructor methods (Needed for type stereotypes calculations)
}; 

#endif
