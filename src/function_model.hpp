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
                                                        functionModel                          (const std::string&, const std::string&, const std::string&, const std::string&, int, int, bool);

    const std::vector<std::string>&                     getStereotypes                         () const                { return stereotypes;                          }
    const std::vector<std::string>&                     getAttributesOrAnnotations             () const                { return attributesOrAnnotations;              }
    const std::unordered_set<std::string>&              getSpecifiers                          () const                { return specifiers;                           }
    const std::vector<callModel>&                       getFunctionCalls                       () const                { return functionCalls;                        }
    const std::vector<callModel>&                       getMethodCalls                         () const                { return methodCalls;                          }
    const std::vector<callModel>&                       getNewConstructorCalls                 () const                { return newConstructorCalls;                  }
    const std::vector<callModel>&                       getExternalCalls                       () const                { return externalCalls;                        }
    const variableModel&                                getReturnType                          () const                { return returnType;                           }
    const std::string&                                  getParametersList                      () const                { return parametersList;                       }
    const std::vector<std::string>&                     getName                                () const                { return name;                                 }
    const std::pair<std::string, std::string>&          getNameSignature                       () const                { return nameSignature;                        }
    const std::string&                                  getXpath                               () const                { return xpath;                                }
    const std::string&                                  getUnitLanguage                        () const                { return unitLanguage;                         }
    const std::string&                                  getConstructorOrDestructor             () const                { return constructorOrDestructor;              }
    const std::string&                                  getFileName                            () const                { return fileName;                             }
    std::string                                         getStereotypesString                   () const;
    std::string                                         getSpecifiersString                    () const;
    std::string                                         getAttributesOrAnnotationsString       () const;
    std::string                                         getReturnTypeParsed                    () const;
    std::string                                         getCallsString                         (bool) const;
    int                                                 getFieldsModifiedCount                 () const                { return fieldsModifiedCount;             }
    int                                                 getUnitNumber                          () const                { return unitNumber;                           }
    int                                                 getLineNumber                          () const                { return lineNumber;                           }
    int                                                 getExternalFunctionCallsCount          () const                { return externalFunctionCallsCount;           }
    int                                                 getExternalMethodCallsCount            () const                { return externalMethodCallsCount;             }
    int                                                 getNonCommentStatementsCount           () const                { return nonCommentStatementsCount;            }
    bool                                                isMethodConst                          () const                { return methodConst;                          }
    bool                                                isFieldUsed                            () const                { return fieldUsed;                       }
    bool                                                isParameterUsed                        () const                { return parameterUsed;                        }
    bool                                                hasSimpleReturn                        () const                { return simpleReturn;                         }
    bool                                                hasComplexReturn                       () const                { return complexReturn;                        }
    bool                                                hasParameterComplexReturn              () const                { return parameterComplexReturn;               }
    bool                                                isParameterRefModified                 () const                { return parameterRefModified;                 }
    bool                                                isNewReturned                          () const                { return newReturned;                          }
    bool                                                isGlobalOrStaticVariableModified       () const                { return globalOrStaticVariableModified;       }
    bool                                                isNonPrimitiveFieldExternal            () const                { return nonPrimitiveFieldExternal;       }
    bool                                                isNonPrimitiveReturnTypeExternal       () const                { return nonPrimitiveReturnTypeExternal;       }
    bool                                                isNonPrimitiveLocalExternal            () const                { return nonPrimitiveLocalExternal;            }
    bool                                                isNonPrimitiveParamaterExternal        () const                { return nonPrimitiveParamaterExternal;        }
    bool                                                isNonPrimitiveReturnType               () const                { return nonPrimitiveReturnType;               }
    bool                                                isVariableCreatedAndReturnedWithNew    () const                { return variableCreatedWithNewAndReturned;    }
    bool                                                isNonPrimitiveLocalOrParameterModified () const                { return nonPrimitiveLocalOrParameterModified; }
    void                                                setStereotype                          (const std::string& s) { stereotypes.push_back(s);}  
    void                                                findDataAfterCollection                (const std::unordered_map<std::string, variableModel>&, const std::unordered_set<std::string>&);
    void                                                findDataFreeFunctionAfterCollection    ();

private:
    bool                                                isVariableUsed                         (const std::unordered_map<std::string, variableModel>&, std::unordered_set<std::string>*, const std::string&, bool, bool, bool, bool, bool);
    int                                                 countMethodsInProperty                 () const;
    void                                                findModifiedRefParameter               (std::string, bool);      
    void                                                findVariablesInExpressions             (const std::unordered_map<std::string, variableModel>&, bool);
    void                                                findIgnorableCalls                     (std::vector<callModel>&);
    void                                                findCallsOnFields                      (const std::unordered_map<std::string, variableModel>&, const std::unordered_set<std::string>&);   
    void                                                findReturnedVariables                  (const std::unordered_map<std::string, variableModel>&, bool);
    void                                                findModifiedVariables                  (const std::unordered_map<std::string, variableModel>&, bool);
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
    void                                                findSpecifiers                         ();
    void                                                findAttributesOrAnnotations            (); 
    void                                                findPropertyAttributes                 (); 

    std::vector<std::string>                          name;                                       // Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and in-between generic in ( <> ) | same as last but without ( <> )
    std::pair<std::string, std::string>               nameSignature;                              // Name without name without whitespaces, namespaces, and generics <> + parameters list (commas only). For example, foo(,,)
    variableModel                                     returnType;                                 // Return type
    std::string                                       fileName;                                   // File name where method is defined 
    std::string                                       parametersList;                             // Parameter list
    std::string                                       unitLanguage;                               // Unit language
    std::string                                       xpath;                                      // Unique xpath
    std::string                                       constructorOrDestructor;                    // Method is a constructor or a destructor
    std::vector<variableModel>                        parametersOrdered;                          // List of all parameters (Needed in order to build the parameters map)
    std::vector<variableModel>                        localsOrdered;                              // List of all local (Needed in order to build the locals map)     
    std::unordered_map<std::string, variableModel>    parameters;                                 // Map of all parameters. Key is parameter name
    std::unordered_map<std::string, variableModel>    locals;                                     // Map of all locals. Key is local name         
    std::string                                       typeNameParsed;                             // Type name without whitespaces, namespaces, and generics <>
    std::unordered_set<std::string>                   specifiers;                                 // List of specifiers
    std::unordered_set<std::string>                   variablesCreatedWithNew;                    // List of variables that are declared/initialized with the "new" operator
    std::unordered_set<std::string>                   expressionNames;                            // List of names of expressions in a method
    std::unordered_set<std::string>                   expressionAssignments;                      // List of assignments of expressions in a method
    std::vector<std::string>                          stereotypes;                                // Method stereotype(s)
    std::vector<std::string>                          attributesOrAnnotations;                    // List of attributes (C#) or annotations (Java) 
    std::vector<std::string>                          propertyAttributes;                         // List of property attributes (C#)
    std::vector<callModel>                            externalCalls;                              // External function calls + method calls + constructor calls
    std::vector<callModel>                            functionCalls;                              // List of function calls (e.g., foo() or bar::foo()) where ( foo ) is another method in the type ( bar )
    std::vector<callModel>                            methodCalls;                                // List of method calls (e.g., a.foo()) where ( a ) is a field in the type
    std::vector<callModel>                            newConstructorCalls;                        // List of constructor calls that uses the ( new ) operator whether the type or external types
    std::vector<std::string>                          returnExpressions;                          // List of all return expressions in a method
    bool                                              methodConst{false};                         // Is it a const method? (C++ only)
    bool                                              fieldUsed{false};                           // Does it use at least 1 field in an expression? 
    bool                                              parameterUsed{false};                       // Does it use at least 1 parameter in an expression?
    bool                                              simpleReturn{false};                        // Does it contain at least 1 return expression that just returns a field?
    bool                                              complexReturn{false};                       // Does it contain at least 1 return expression that is not a simple return?
    bool                                              parameterComplexReturn{false};              // Does it contain at least 1 return expression that is not a simple return? (For parameters)
    bool                                              parameterRefModified{false};                // Does it modify at least 1 parameter that is passed by reference?
    bool                                              nonPrimitiveLocalOrParameterModified{false};// Does it modify at least 1 non-primitive parameter or a non-primitive local?
    bool                                              globalOrStaticVariableModified{false};      // Does it change any global or static variables?
    bool                                              nonPrimitiveFieldExternal{false};           // True if method uses at least 1 non-primitive field that is not of the same type as type  
    bool                                              nonPrimitiveReturnType{false};              // True if method uses a non-primitive return type
    bool                                              nonPrimitiveReturnTypeExternal{false};      // True if method uses a non-primitive return type that is not of the same type as type   
    bool                                              nonPrimitiveLocalExternal{false};           // True if method uses at least 1 a non-primitive local that is not of the same type as type   
    bool                                              nonPrimitiveParamaterExternal{false};       // True if method uses at least 1 a non-primitive parameter that is not of the same type as type                                                
    bool                                              newReturned{false};                         // There is at least one return that a return a "new" call
    bool                                              variableCreatedWithNewAndReturned{false};   // There is at least 1 return expression that returns a field, a local, a parameter, a static, or a global created with the 'new' operator 
    bool                                              isProperty{false};                          // Is it a property? (C# only)
    int                                               unitNumber{-1};                             // srcML Unit number   
    int                                               lineNumber{-1};                             // Line number of the method declaration (Requires --position)
    int                                               fieldsModifiedCount{0};                     // Number of modified fields
    int                                               externalFunctionCallsCount{0};              // Number of function calls (e.g., foo()) where ( foo ) is a free function or a static method
    int                                               externalMethodCallsCount{0};                // Number of method calls (e.g., a.foo()) where ( a ) is an external object that are filtered (removed)
    int                                               nonCommentStatementsCount{0};               // Number of non-comment statements 
};

#endif
