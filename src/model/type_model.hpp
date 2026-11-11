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

#include <algorithm>
#include <set>
#include "model/function_model.hpp"

class typeModel {
public:
    // The parameter types of the static methods that a C++ type declares, by the name of the method
    using staticSignatures = std::unordered_multimap<std::string, std::vector<writtenType>>;

                                                                     typeModel                          (const languageTables&, const std::string&, std::string);
       
    std::string                                                      getStereotypesString               ()               const;
    std::string                                                      getParentsString                   (bool)           const;
    std::string                                                      getInheritedMethodSignaturesString ()               const;
    std::string                                                      getStructure                       ()               const          { return structure;                              }
    std::vector<functionModel>&                                      getMethods                         ()                              { return methods;                                }
    const std::vector<functionModel>&                                getMethods                         ()               const          { return methods;                                }
    const std::string&                                               getUnitLanguage                    ()               const          { return unitLanguage;                           }
    const std::vector<std::string>&                                  getName                            ()               const          { return name;                                   }
    const std::vector<std::string>&                                  getScope                           ()               const          { return scope;                                  }
    const std::vector<std::string>&                                  getPartFileNames                   ()               const          { return partFileNames;                          }
    const std::vector<std::size_t>&                                  getParentParts                     ()               const          { return parentParts;                            }
    const std::vector<std::vector<std::string>>&                     getParentNames                     ()               const          { return parentNames;                            } 
    const std::vector<std::vector<std::string>>&                     getInheritedParentNames            ()               const          { return inheritedParentNames;                   } 
    const std::set<std::pair<std::string, std::string>>&             getInheritedMethodSignatures       ()               const          { return inheritedMethodSignatures;              }   
    const std::set<std::pair<std::string, std::string>>&             getInheritedDeclMethodSignatures   ()               const          { return inheritedDeclMethodSignatures;          }
    const std::vector<elementLocation>&                              getLocations                       ()               const          { return locations;                              }
    bool                                                             isPartial                          ()               const          { return partial;                                }
    const staticSignatures&                                          getStaticMethodSignatures          ()               const          { return staticMethodSignatures;                 }
    bool                                                             hasUnknownParent                   ()               const          { return unknownParent;                          }
    const std::unordered_set<std::string>&                           getStaticHolders                   ()               const          { return staticHolders;                          }
    argumentRange                                                    argumentsOf                        (const functionModel&) const;

    void                                                             setUnknownParent                   (bool flag)                     { unknownParent = flag;                          }
    void                                                             addStaticHolders                   (const std::unordered_set<std::string>& h) { staticHolders.insert(h.begin(), h.end()); }
    void                                                             setStereotype                      (const std::string& s)          { stereotypes.push_back(s);                      }
    void                                                             findData                           (const srcmlDocument&, int, std::vector<std::string>);
    void                                                             findDataAfterCollection            (const globalVariables&);
    void                                                             mergeData                          (typeModel& other);
    void                                                             appendInheritedDataMembers         (const typeModel&);
    void                                                             appendEnclosingDataMembers         (const typeModel&);

    void buildMethodSignature() { 
        for (const auto& m : methods) {
            methodSignatures.insert(m.getNameSignature()); 
        }
    }
    void addMethod(functionModel&& m)  { 
        methods.push_back(std::move(m));
    }

private:
    void                                                    findParentNames                    (const srcmlDocument&);
    std::string                                             findPropertyReturnType             (const srcmlDocument&);
    void                                                    findFields                         (const srcmlDocument&, int);
    void                                                    findStaticFields                   (const srcmlDocument&);
    void                                                    findMethods                        (const srcmlDocument&, int);
    void                                                    findMethodDeclNames                (const srcmlDocument&, std::vector<std::pair<std::string, std::string>>&);
    void                                                    findMethodDeclParameters           (const srcmlDocument&, std::vector<std::pair<std::string, std::string>>&);
    void                                                    findProperties                     (const srcmlDocument&, int);
    void                                                    findStructure                      (const srcmlDocument&);     
    void                                                    findMethodsInProperty              (const srcmlDocument&, const std::string&, const std::string&, int);
    void                                                    findStaticMethodSignatures         (const srcmlDocument&);

    const languageTables*                                   tables{nullptr};                 // XPaths and lists read while the type is built
    std::string                                             unitLanguage;                    // Unit language (C, C#, C++, Java)
    std::string                                             structure;                       // class, struct, interface, enum, union 
    std::vector<std::string>                                name;                            // Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and and in-between generic in ( <> ) | same as last but without ( <> )
    std::vector<std::string>                                stereotypes;                     // Stereotype(s)
    std::vector<functionModel>                              methods;                         // Methods (including methods inside properties for C#)
    std::vector<std::vector<std::string>>                   parentNames;                     // Parent names where each is of Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and and in-between generic in ( <> ) | same as last but without ( <> )
    std::vector<std::vector<std::string>>                   inheritedParentNames;            // Inherited parent names, each once
    std::set<std::pair<std::string, std::string>>           declMethodSignatures;            // Method declaration signatures (Unique)
    std::set<std::pair<std::string, std::string>>           inheritedDeclMethodSignatures;   // Inherited method declaration signatures (Unique)
    std::set<std::pair<std::string, std::string>>           methodSignatures;                // Method signatures (Unique)
    std::set<std::pair<std::string, std::string>>           inheritedMethodSignatures;       // Inherited method signatures (Unique)
    staticSignatures                                        staticMethodSignatures;          // Parameter types of each static method that the type declares, by name (C++)
    std::unordered_map<std::string, argumentRange>          declaredArguments;               // Numbers of arguments of each method declaration by name and parameter types without qualifiers
    std::unordered_map<std::string, variableModel>          fields;                          // Key is field name and value is the field object
    std::unordered_map<std::string, variableModel>          inheritedFields;                 // Inherited fields
    std::unordered_set<std::string>                         staticFields;                    // Names of the static fields, which are globals
    std::unordered_set<std::string>                         staticHolders;                   // Types around the type and types of static imports, whose static fields its methods reach without their names
    std::unordered_set<std::string>                         inheritedStaticFields;           // Names of the static fields of the known ancestors
    std::unordered_map<std::string, variableModel>          enclosingFields;                 // Fields of the enclosing instances of a Java inner class, which no child inherits
    std::unordered_set<std::string>                         enclosingStaticFields;           // Names of the static fields of the types of the enclosing instances
    std::vector<elementLocation>                            locations;                       // Elements of the type (several for a partial type in C#)
    std::vector<std::string>                                scope;                           // Namespaces or package around the type, outermost first
    std::vector<std::string>                                partFileNames;                   // File of each element in locations
    std::vector<std::size_t>                                parentParts;                     // Element in locations that names each parent in parentNames
    bool                                                    partial{false};                  // The type is one part of a C# partial type
    bool                                                    unknownParent{false};            // True if the type or one of its ancestors has a parent outside the archive, which only the verbose report reads
}; 

#endif
