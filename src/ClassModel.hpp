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
         
    void findName                           (srcml_archive*, srcml_unit*);
    void findType                           (srcml_archive*, srcml_unit*);
    void findParentName                     (srcml_archive*, srcml_unit*);
    void findDataMemberName                 (srcml_archive*, srcml_unit*, std::vector<variable>&);
    void findDataMemberType                 (srcml_archive*, srcml_unit*, std::vector<variable>&, int);
    void findMethod                         (srcml_archive*, srcml_unit*, const std::string&, int);
    void findMethodInProperty               (srcml_archive*, srcml_unit*, const std::string&, int);
    void findData                           (srcml_archive*, srcml_unit*, const std::string&, int);

    std::string                                               getStereotype                      ()               const;
    std::unordered_map<std::string, variable>&                getDataMembers                     ()                              { return dataMembers;                            }
    std::vector<methodModel>&                                 getMethods                         ()                              { return methods;                                }
    
    const std::string&                                        getUnitLanguage                    ()               const          { return unitLanguage;                           }
    const std::vector<std::string>&                           getName                            ()               const          { return name;                                   }
    const std::vector<std::string>&                           getStereotypeList                  ()               const          { return stereotype;                             }
    const std::unordered_map<std::string, std::string>&       getParentClassName                 ()               const          { return parentNames;                            }  
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

    // Inheritance does not need to check for private data members or methods, this is because
    //   a method will only use a data member or call a method if it is not private, so we can simply collect them all
    //
    void appendInheritedDataMembers(const std::unordered_map<std::string, variable>& inheritedDataMembers) { 
        dataMembers.insert(inheritedDataMembers.begin(), inheritedDataMembers.end());         
    }
    void appendInheritedMethod(const std::unordered_set<std::string>& parentMethods) {
        methodSignatures.insert(parentMethods.begin(), parentMethods.end());
    }

    void buildMethodSignature() { for (const auto& m : methods) methodSignatures.insert(m.getNameSignature()); }

private:
    std::vector<std::string>                                name;                            // Size = 4 containing | Original name | name without whitespaces | name without whitespaces, namespaces, and generic types in <> | same as last but without <>
    std::unordered_map<std::string, std::string>            parentNames;                     // Key is parent class name without whitespaces and namespaces and value is specifier (public, private, or protected).
    std::string                                             type;                            // Class, or struct, or an interface
    std::string                                             unitLanguage;                    // Unit language
    std::vector<std::string>                                stereotype;                      // Class stereotype(s)
    std::vector<methodModel>                                methods;                         // List of methods 
    std::unordered_set<std::string>                         methodSignatures;                // List of method signatures (including parent method signatures)
    std::unordered_map<std::string, variable>               dataMembers;                     // Key is data member name and value is data member object (including inherited data members)
    std::unordered_map<int, std::vector<std::string>>       xpath;                           // Unique xpath for class (classs if partial in C#) along with the unit number
    bool                                                    inherited{false};                // Did class inherit the data members yet? (Used for inheritance)
    bool                                                    visited{false};                  // Has class been visited yet when inheriting? (Used for inheritance)    
    int                                                     constructorDestructorCount{0};   // Number of constructor + destructor methods (Needed for class stereotypes)
}; 

#endif
