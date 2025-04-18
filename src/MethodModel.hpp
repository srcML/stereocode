// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file MethodModel.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef METHODMODEL_HPP
#define METHODMODEL_HPP

#include <srcml.h>
#include "utils.hpp"
#include "variable.hpp"
#include "XPathBuilder.hpp"
#include "IgnorableCalls.hpp"
#include "call.hpp"

class methodModel {
public:
    methodModel(srcml_archive*, srcml_unit*, const std::string&, const std::string&, const std::string&, int);

    std::string                     getStereotype                          () const;
    const std::vector<variable>&    getParametersOrdered                   () const                { return parametersOrdered;                    }
    const std::vector<call>&        getFunctionCalls                       () const                { return functionCalls;                        }
    const std::vector<call>&        getMethodCalls                         () const                { return methodCalls;                          }
    const std::vector<call>&        getNewConstructorCalls                 () const                { return newConstructorCalls;                  }
    const std::vector<std::string>& getStereotypeList                      () const                { return stereotype;                           }
    const std::string&              getName                                () const                { return name;                                 }
    const std::string&              getNameSignature                       () const                { return nameSignature;                        }
    const std::string&              getParameterList                       () const                { return parameterList;                        }
    const std::string&              getSrcML                               () const                { return srcML;                                }
    const std::string&              getReturnType                          () const                { return returnType;                           }
    const std::string&              getReturnTypeParsed                    () const                { return returnTypeParsed;                     }
    const std::string&              getXpath                               () const                { return xpath;                                }
    const std::string&              getUnitLanguage                        () const                { return unitLanguage;                         }
    int                             getDataMembersModifiedCount            () const                { return dataMembersModifiedCount;             }
    int                             getUnitNumber                          () const                { return unitNumber;                           }
    int                             getExternalFunctionCallsCount          () const                { return externalFunctionCallsCount;           }
    int                             getExternalMethodCallsCount            () const                { return externalMethodCallsCount;             }
    int                             getNonCommentStatementsCount           () const                { return nonCommentStatementsCount;            }
    bool                            isMethodConst                          () const                { return methodConst;                          }
    bool                            isDataMemberUsed                       () const                { return dataMemberUsed;                       }
    bool                            isParameterUsed                        () const                { return parameterUsed;                        }
    bool                            hasSimpleReturn                        () const                { return simpleReturn;                         }
    bool                            hasComplexReturn                       () const                { return complexReturn;                        }
    bool                            hasParameterComplexReturn              () const                { return parameterComplexReturn;               }
    bool                            isParameterRefModified                 () const                { return parameterRefModified;                 }
    bool                            isNewReturned                          () const                { return newReturned;                          }
    bool                            isGlobalOrStaticVariableModified       () const                { return globalOrStaticVariableModified;       }
    bool                            isNonPrimitiveDataMemberExternal       () const                { return nonPrimitiveDataMemberExternal;       }
    bool                            isNonPrimitiveReturnTypeExternal       () const                { return nonPrimitiveReturnTypeExternal;       }
    bool                            isNonPrimitiveLocalExternal            () const                { return nonPrimitiveLocalExternal;            }
    bool                            isNonPrimitiveParamaterExternal        () const                { return nonPrimitiveParamaterExternal;        }
    bool                            isNonPrimitiveReturnType               () const                { return nonPrimitiveReturnType;               }
    bool                            isConstructorOrDestructor              () const                { return constructorOrDestructor;              }
    bool                            isVariableCreatedAndReturnedWithNew    () const                { return variableCreatedWithNewAndReturned;    }
    bool                            isNonPrimitiveLocalOrParameterModified () const                { return nonPrimitiveLocalOrParameterModified; }
    bool                            isVariableUsed                         (std::unordered_map<std::string, variable>&, std::unordered_set<std::string>*, const std::string&, bool, bool, bool, bool, bool);
 
    void                     findNameSignature          ();
    void                     findFreeFunctionData       ();
    void                     findData                   (std::unordered_map<std::string, variable>&, const std::unordered_set<std::string>&, const std::string&);
    void                     findName                   (srcml_archive*, srcml_unit*);
    void                     findReturnType             (srcml_archive*, srcml_unit*);
    void                     findParameterList          (srcml_archive*, srcml_unit*);
    void                     findLocalVariableName      (srcml_archive*, srcml_unit*);
    void                     findLocalVariableType      (srcml_archive*, srcml_unit*);
    void                     findParameterName          (srcml_archive*, srcml_unit*);
    void                     findParameterType          (srcml_archive*, srcml_unit*);
    void                     findReturnExpression       (srcml_archive*, srcml_unit*);   
    void                     findCallName               (srcml_archive*, srcml_unit*);
    void                     findCallArgument           (srcml_archive*, srcml_unit*);
    void                     findNewAssignedVariables   (srcml_archive*, srcml_unit*);
    void                     findConst                  (srcml_archive*, srcml_unit*);
    void                     findConstructorOrDestructor(srcml_archive*, srcml_unit*);
    void                     findIgnorableCalls         (std::vector<call>&);
    void                     findCallsOnDataMembers     (std::unordered_map<std::string, variable>&, const std::unordered_set<std::string>&);   
    void                     findReturnedVariables      (std::unordered_map<std::string, variable>&, bool);
    void                     findModifiedVariables      (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&, bool);                             
    void                     findVariablesInExpressions (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&, bool);
    void                     findNonCommentStatements   (srcml_archive*, srcml_unit*);
    void                     findModifiedRefParameter   (std::string, bool);      
    void                     setStereotype              (const std::string& s) { stereotype.push_back(s);}
                                             
private:
    std::vector<std::string>                          callType;
    std::string                                       name;                                       // Name without namespaces
    std::string                                       nameSignature;                              // Name without namespaces + parameters list (commas only). For example, foo(,,)
    std::string                                       returnType;                                 // Return type without whitespaces
    std::string                                       returnTypeParsed;                           // Return type without specifiers, containers, and whitespaces
    std::string                                       parameterList;                              // Parameter list
    std::string                                       unitLanguage;                               // Unit language
    std::string                                       xpath;                                      // Unique xpath
    std::string                                       srcML;                                      // Method srcML
    std::vector<variable>                             parametersOrdered;                          // List of all parameters (Needed in order to build the parameters map)
    std::vector<variable>                             localsOrdered;                              // List of all local (Needed in order to build the locals map)     
    std::unordered_map<std::string, variable>         parameters;                                 // Map of all parameters. Key is parameter name
    std::unordered_map<std::string, variable>         locals;                                     // Map of all locals. Key is local name         
    std::string                                       classNameParsed;                            // Class name without whitespaces, namespaces, and generic types <>
    std::unordered_set<std::string>                   variablesCreatedWithNew;                    // List of variables that are declared/initialized with the "new" operator
    std::vector<std::string>                          stereotype;                                 // Method stereotype
    std::vector<call>                                 functionCalls;                              // List of function calls (e.g., foo()) to methods in class
    std::vector<call>                                 methodCalls;                                // List of method calls (e.g., a.foo()) where 'a' is an data member
    std::vector<call>                                 newConstructorCalls;                        // List of constructor calls that uses the 'new' operator
    std::vector<std::string>                          returnExpressions;                          // List of all return expressions in a method
    bool                                              methodConst{false};                         // Is it a const method? (C++ only)
    bool                                              dataMemberUsed{false};                      // Does it use at least 1 data member in an expression? 
    bool                                              parameterUsed{false};                       // Does it use at least 1 parameter in an expression?
    bool                                              simpleReturn{false};                        // Does it contain at least 1 return expression that just returns a data member?
    bool                                              complexReturn{false};                       // Does it contain at least 1 return expression that is not a simple return?
    bool                                              parameterComplexReturn{false};              // Does it contain at least 1 return expression that is not a simple return? (For parameters)
    bool                                              parameterRefModified{false};                // Does it modify at least 1 parameter that is passed by reference?
    bool                                              nonPrimitiveLocalOrParameterModified{false};// Does it modify at least 1 non-primitive parameter or a non-primitive local?
    bool                                              globalOrStaticVariableModified{false};      // Does it change any global or static variables?
    bool                                              nonPrimitiveDataMemberExternal{false};      // True if method uses at least 1 non-primitive data member that is not of the same type as class  
    bool                                              nonPrimitiveReturnType{false};              // True if method uses a non-primitive return type
    bool                                              nonPrimitiveReturnTypeExternal{false};      // True if method uses a non-primitive return type that is not of the same type as class   
    bool                                              nonPrimitiveLocalExternal{false};           // True if method uses at least 1 a non-primitive local that is not of the same type as class   
    bool                                              nonPrimitiveParamaterExternal{false};       // True if method uses at least 1 a non-primitive parameter that is not of the same type as class                                                
    bool                                              newReturned{false};                         // There is at least one return that a return a "new" call
    bool                                              constructorOrDestructor{false};             // Method is a constructor or a destructor
    bool                                              variableCreatedWithNewAndReturned{false};   // There is at least 1 return expression that returns a data member, a local, a parameter, a static, or a global created with the 'new' operator 
    int                                               unitNumber{-1};                             // srcML Unit number   
    int                                               dataMembersModifiedCount{0};                // Number of modified data members
    int                                               externalFunctionCallsCount{0};              // Number of function calls that are filtered (removed)
    int                                               externalMethodCallsCount{0};                // Number of method calls that are filtered (removed)
    int                                               nonCommentStatementsCount{0};               // Number of non-comment statements 


};

#endif
