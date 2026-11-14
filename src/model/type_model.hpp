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
#include "model/overloads.hpp"

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
    const std::unordered_set<std::string>&                           getStaticImports                   ()               const          { return staticImports;                          }
    const std::unordered_set<std::string>&                           getMisreadMethods                  ()               const          { return misreadMethods;                         }
    const std::unordered_map<std::string, std::string>&              getParentAliases                   ()               const          { return parentAliases;                          }
    declaredMethods                                                  undefinedMethods                   ()               const;
    std::unordered_set<std::string>                                  methodNames                        ()               const;
    std::unordered_set<std::string>                                  implicitAccessors                  ()               const;
    argumentRange                                                    argumentsOf                        (const functionModel&) const;
    bool                                                             namesOwnMethod                     (const std::string&) const;

    void                                                             setUnknownParent                   (bool flag)                     { unknownParent = flag;                          }
    void                                                             addStaticHolders                   (const std::unordered_set<std::string>& h) { staticHolders.insert(h.begin(), h.end()); }
    void                                                             addStaticImports                   (const std::unordered_set<std::string>&);
    void                                                             addMisreadMethods                  (const std::unordered_set<std::string>& m) { misreadMethods.insert(m.begin(), m.end()); }
    void                                                             setHolderFunctions                 (std::unordered_set<std::string>, bool);
    void                                                             setStereotype                      (const std::string& s)          { stereotypes.push_back(s);                      }
    void                                                             findData                           (const srcmlDocument&, int, std::vector<std::string>);
    void                                                             findDataAfterCollection            (const globalVariables&, const callableNames&);
    void                                                             mergeData                          (typeModel& other);
    void                                                             appendInheritedDataMembers         (const typeModel&);
    void                                                             appendEnclosingDataMembers         (const typeModel&);
    void                                                             declareMethodMembers               (const callableNames&, const globalVariables&);
    void                                                             findCreatedObjects                 (const callableNames&, const globalVariables&);
    void                                                             buildMethodSignature               ();

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
    argumentsByName                                         methodArguments;                 // Numbers of arguments that the methods of the type take, defined or declared
    argumentsByName                                         inheritedMethodArguments;        // The same for the methods of the known ancestors
    std::unordered_map<std::string, variableModel>          fields;                          // Key is field name and value is the field object
    std::unordered_map<std::string, variableModel>          inheritedFields;                 // Inherited fields
    std::unordered_set<std::string>                         staticFields;                    // Names of the static fields, which are globals
    std::unordered_set<std::string>                         componentNames;                  // Components of a record, each with an accessor of its name that the record may define (Java)
    std::unordered_set<std::string>                         staticMethodNames;               // Names of the static and class methods of the body of the type (Python)
    std::unordered_set<std::string>                         inheritedStaticMethodNames;      // Names of the static and class methods of the known ancestors
    std::unordered_set<std::string>                         nestedTypeNames;                 // Names of the classes in the body of the type (Python)
    std::unordered_set<std::string>                         inheritedNestedTypeNames;        // Names of the classes in the bodies of the known ancestors
    std::unordered_set<std::string>                         staticHolders;                   // Types around the type and types of static imports, whose static fields its methods reach without their names
    std::unordered_set<std::string>                         staticImports;                   // Types of the static imports that hold for the type
    std::unordered_set<std::string>                         misreadMethods;                  // Names of the methods that srcML marks as other elements or outside the type, which get no stereotype
    std::unordered_map<std::string, std::string>            memberAliases;                   // Member aliases of the type and of its known ancestors, each with the name of the type that it stands for (C++)
    std::unordered_map<std::string, std::string>            parentAliases;                   // Member aliases that stand for the type or a parent, as Base of using Base = Shape;
    std::unordered_set<std::string>                         holderFunctions;                 // Names of the static methods and the methods of the own types and the static holders, which a call without an object may name
    std::unordered_set<std::string>                         inheritedStaticFields;           // Names of the static fields of the known ancestors
    std::unordered_map<std::string, variableModel>          enclosingFields;                 // Fields of the enclosing instances of a Java inner class, which no child inherits
    std::unordered_set<std::string>                         enclosingStaticFields;           // Names of the static fields of the types of the enclosing instances
    std::unordered_set<std::string>                         enclosingMethodNames;            // Names of the methods of the types of the enclosing instances
    argumentsByName                                         enclosingMethodArguments;        // Numbers of arguments that these methods take
    std::vector<elementLocation>                            locations;                       // Elements of the type (several for a partial type in C#)
    std::vector<std::string>                                scope;                           // Namespaces or package around the type, outermost first
    std::string                                             scopedName;                      // Name with the namespaces or the package and the types around the type, without generic arguments
    std::vector<std::string>                                partFileNames;                   // File of each element in locations
    std::vector<std::size_t>                                parentParts;                     // Element in locations that names each parent in parentNames
    bool                                                    partial{false};                  // The type is one part of a C# partial type
    bool                                                    unknownParent{false};            // True if the type or one of its ancestors has a parent outside the archive, which only the verbose report reads
    bool                                                    outsideStaticImport{false};      // A static import names a type outside the archive, which may hold a function of any name
}; 

#endif
