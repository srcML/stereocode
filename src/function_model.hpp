// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file function_model.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef FUNCTION_MODEL_HPP
#define FUNCTION_MODEL_HPP

#include "variable_model.hpp"
#include "call_model.hpp"

#include <srcml.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class functionModel {
public:
                                                        functionModel                          (const std::string&, const std::string&, const std::string&, const std::string&, int, bool);

    const std::vector<std::string>&                     getStereotypes                         () const                { return stereotypes;                          }
    const std::string&                                  getSourceCode                          () const                { return sourceCode;                           }
    const std::unordered_set<std::string>&              getSpecifiers                          () const                { return specifiers;                           }
    const std::vector<callModel>&                       getFunctionCalls                       () const                { return functionCalls;                        }
    const std::vector<callModel>&                       getMethodCalls                         () const                { return methodCalls;                          }
    const std::vector<callModel>&                       getNewConstructorCalls                 () const                { return newConstructorCalls;                  }
    const std::string&                                  getName                                () const                { return name;                                 }
    const std::string&                                  getNameSignature                       () const                { return nameSignature;                        }
    const variableModel&                                getReturnType                          () const                { return returnType;                           }
    const std::string&                                  getXpath                               () const                { return xpath;                                }
    const std::string&                                  getUnitLanguage                        () const                { return unitLanguage;                         }
    const std::string&                                  getConstructorOrDestructor             () const                { return constructorOrDestructor;              }
    const std::string&                                  getFileName                            () const                { return fileName;                             }
    int                                                 getDataMembersModifiedCount            () const                { return dataMembersModifiedCount;             }
    int                                                 getUnitNumber                          () const                { return unitNumber;                           }
    int                                                 getExternalFunctionCallsCount          () const                { return externalFunctionCallsCount;           }
    int                                                 getExternalMethodCallsCount            () const                { return externalMethodCallsCount;             }
    int                                                 getNonCommentStatementsCount           () const                { return nonCommentStatementsCount;            }
    bool                                                isMethodConst                          () const                { return methodConst;                          }
    bool                                                isDataMemberUsed                       () const                { return dataMemberUsed;                       }
    bool                                                isParameterUsed                        () const                { return parameterUsed;                        }
    bool                                                hasSimpleReturn                        () const                { return simpleReturn;                         }
    bool                                                hasComplexReturn                       () const                { return complexReturn;                        }
    bool                                                hasParameterComplexReturn              () const                { return parameterComplexReturn;               }
    bool                                                isParameterRefModified                 () const                { return parameterRefModified;                 }
    bool                                                isNewReturned                          () const                { return newReturned;                          }
    bool                                                isGlobalOrStaticVariableModified       () const                { return globalOrStaticVariableModified;       }
    bool                                                isNonPrimitiveDataMemberExternal       () const                { return nonPrimitiveDataMemberExternal;       }
    bool                                                isNonPrimitiveReturnTypeExternal       () const                { return nonPrimitiveReturnTypeExternal;       }
    bool                                                isNonPrimitiveLocalExternal            () const                { return nonPrimitiveLocalExternal;            }
    bool                                                isNonPrimitiveParamaterExternal        () const                { return nonPrimitiveParamaterExternal;        }
    bool                                                isNonPrimitiveReturnType               () const                { return nonPrimitiveReturnType;               }
    bool                                                isVariableCreatedAndReturnedWithNew    () const                { return variableCreatedWithNewAndReturned;    }
    bool                                                isNonPrimitiveLocalOrParameterModified () const                { return nonPrimitiveLocalOrParameterModified; }
    std::string                                         getStereotypesString                   () const;
    std::string                                         getSpecifiersString                    () const;
    std::string                                         getReturnTypeParsed                    () const;    
    void                                                setStereotype                          (const std::string& s) { stereotypes.push_back(s);}  
    void                                                findDataAfterCollection                (std::unordered_map<std::string, variableModel>&, const std::unordered_set<std::string>&);
    void                                                findDataFreeFunctionAfterCollection    ();

private:
    bool                                                isVariableUsed                         (std::unordered_map<std::string, variableModel>&, std::unordered_set<std::string>*, const std::string&, bool, bool, bool, bool, bool);
    int                                                 countMethodsInProperty                 () const;
    void                                                findModifiedRefParameter               (std::string, bool);      
    void                                                findVariablesInExpressions             (std::unordered_map<std::string, variableModel>&, bool);
    void                                                findIgnorableCalls                     (std::vector<callModel>&);
    void                                                findCallsOnDataMembers                 (std::unordered_map<std::string, variableModel>&, const std::unordered_set<std::string>&);   
    void                                                findReturnedVariables                  (std::unordered_map<std::string, variableModel>&, bool);
    void                                                findModifiedVariables                  (std::unordered_map<std::string, variableModel>&, bool);
    void                                                findNameSignature                      ();
    void                                                findNonPrimitive                       ();
    void                                                findName                               ();
    void                                                findReturnType                         ();
    void                                                findParameterList                      ();
    void                                                findLocalVariableName                  ();
    void                                                findLocalVariableType                  ();
    void                                                findParameterName                      ();
    void                                                findParameterType                      ();
    void                                                findReturnExpression                   ();   
    void                                                findCallName                           ();
    void                                                findCallArgument                       ();
    void                                                findNewAssignedVariables               ();
    void                                                findConst                              ();
    void                                                isConstructorOrDestructor              ();
    void                                                findConstructorOrDestructorType        ();
    void                                                findExpressionNames                    ();
    void                                                findExpressionAssignments              ();
    void                                                findNonCommentStatements               ();               
    void                                                findMethodBody                         ();
    void                                                findSpecifiers                         ();

    std::string                                       sourceCode;
    std::string                                       name;                                       // Name
    std::string                                       fileName;                        // File name where type is defined 
    std::string                                       nameSignature;                              // Name without namespaces + parameters list (commas only). For example, foo(,,)
    variableModel                                     returnType;                                 // Return type without whitespaces
    std::string                                       parameterList;                              // Parameter list
    std::string                                       unitLanguage;                               // Unit language
    std::string                                       xpath;                                      // Unique xpath
    std::string                                       constructorOrDestructor;                    // Method is a constructor or a destructor
    std::vector<variableModel>                        parametersOrdered;                          // List of all parameters (Needed in order to build the parameters map)
    std::vector<variableModel>                        localsOrdered;                              // List of all local (Needed in order to build the locals map)     
    std::unordered_map<std::string, variableModel>    parameters;                                 // Map of all parameters. Key is parameter name
    std::unordered_map<std::string, variableModel>    locals;                                     // Map of all locals. Key is local name         
    std::string                                       typeNameParsed;                             // Type name without whitespaces, namespaces, and generic types <>
    std::unordered_set<std::string>                   specifiers;
    std::unordered_set<std::string>                   variablesCreatedWithNew;                    // List of variables that are declared/initialized with the "new" operator
    std::unordered_set<std::string>                   expressionNames;                            // List of names of expressions in a method
    std::unordered_set<std::string>                   expressionAssignments;                      // List of assignments of expressions in a method
    std::vector<std::string>                          stereotypes;                                 // Method stereotype(s)
    std::vector<callModel>                            functionCalls;                              // List of function calls (e.g., foo()) to methods in type
    std::vector<callModel>                            methodCalls;                                // List of method calls (e.g., a.foo()) where 'a' is an data member
    std::vector<callModel>                            newConstructorCalls;                        // List of constructor calls that uses the 'new' operator
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
    bool                                              nonPrimitiveDataMemberExternal{false};      // True if method uses at least 1 non-primitive data member that is not of the same type as type  
    bool                                              nonPrimitiveReturnType{false};              // True if method uses a non-primitive return type
    bool                                              nonPrimitiveReturnTypeExternal{false};      // True if method uses a non-primitive return type that is not of the same type as type   
    bool                                              nonPrimitiveLocalExternal{false};           // True if method uses at least 1 a non-primitive local that is not of the same type as type   
    bool                                              nonPrimitiveParamaterExternal{false};       // True if method uses at least 1 a non-primitive parameter that is not of the same type as type                                                
    bool                                              newReturned{false};                         // There is at least one return that a return a "new" call
    bool                                              variableCreatedWithNewAndReturned{false};   // There is at least 1 return expression that returns a data member, a local, a parameter, a static, or a global created with the 'new' operator 
    bool                                              isProperty{false};                          // Is it a property? (C# only)
    int                                               unitNumber{-1};                             // srcML Unit number   
    int                                               dataMembersModifiedCount{0};                // Number of modified data members
    int                                               externalFunctionCallsCount{0};              // Number of function calls that are filtered (removed)
    int                                               externalMethodCallsCount{0};                // Number of method calls that are filtered (removed)
    int                                               nonCommentStatementsCount{0};               // Number of non-comment statements 
};

#endif
