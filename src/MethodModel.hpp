// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file MethodModel.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef METHOD_HPP
#define METHOD_HPP

#include <srcml.h>
#include "utils.hpp"
#include "variable.hpp"
#include "XPathBuilder.hpp"
#include "IgnorableCalls.hpp"
#include "calls.hpp"

class methodModel {
public:
    methodModel(srcml_archive*, srcml_unit*, const std::string&, const std::string&, const std::string&, int);

    std::string                     getStereotype                          () const;
    const std::vector<variable>&    getParametersOrdered                   () const                { return parametersOrdered;                    }
    const std::vector<calls>&       getFunctionCalls                       () const                { return functionCalls;                        }
    const std::vector<calls>&       getMethodCalls                         () const                { return methodCalls;                          }
    const std::vector<calls>&       getConstructorCalls                    () const                { return constructorCalls;                     }
    const std::vector<std::string>& getStereotypeList                      () const                { return stereotype;                           }
    const std::string&              getName                                () const                { return name;                                 }
    const std::string&              getNameSignature                       () const                { return nameSignature;                        }
    const std::string&              getParametersList                      () const                { return parametersList;                       }
    const std::string&              getSrcML                               () const                { return srcML;                                }
    const std::string&              getReturnType                          () const                { return returnType;                           }
    const std::string&              getReturnTypeParsed                    () const                { return returnTypeParsed;                     }
    const std::string&              getXpath                               () const                { return xpath;                                } 
    const std::string&              getUnitLanguage                        () const                { return unitLanguage;                         } 
    int                             getNumOfFieldsModified                 () const                { return numOfFieldsModified;                  }
    int                             getUnitNumber                          () const                { return unitNumber;                           }  
    int                             getNumOfExternalFunctionCalls          () const                { return numOfExternalFunctionCalls;           } 
    int                             getNumOfExternalMethodCalls            () const                { return numOfExternalMethodCalls;             } 
    int                             getNumOfNonCommentsStatements          () const                { return numOfNonCommentsStatements;           }
    
    bool                            isConstMethod                          () const                { return constMethod;                          }
    bool                            isFieldReturned                        () const                { return fieldReturned;                        }
    bool                            isComplexReturn                        () const                { return complexReturn;                        }
    bool                            isParameterNotReturned                 () const                { return parameterNotReturned;                 }
    bool                            isFieldUsed                            () const                { return fieldUsed;                            }
    bool                            isParameterUsed                        () const                { return parameterUsed;                        } 
    bool                            isParameterRefModified                 () const                { return parameterRefModified;                 }    
    bool                            isNewReturned                          () const                { return newReturned;                          }
    bool                            isGlobalOrStaticModified               () const                { return globalOrStaticModified;               }          
    bool                            isNonPrimitiveFieldExternal            () const                { return nonPrimitiveFieldExternal;            }
    bool                            isNonPrimitiveReturnTypeExternal       () const                { return nonPrimitiveReturnTypeExternal;       }
    bool                            isNonPrimitiveLocalExternal            () const                { return nonPrimitiveLocalExternal;            }  
    bool                            isNonPrimitiveParamaterExternal        () const                { return nonPrimitiveParamaterExternal;        }
    bool                            isNonPrimitiveReturnType               () const                { return nonPrimitiveReturnType;               }    
    bool                            isConstructorOrDestructor              () const                { return constructorOrDestructor;              }  
    bool                            isFieldsCreatedAndReturnedWithNew      () const                { return fieldsCreatedAndReturnedWithNew;      }  
    bool                            isNonPrimitiveLocalOrParameterModified () const                { return nonPrimitiveLocalOrParameterModified; }  
    bool                            isVariableUsed                         (std::unordered_map<std::string, variable>&, std::unordered_set<std::string>*, const std::string&, bool, bool, bool, bool, bool);
 

    void                     findMethodData             (std::unordered_map<std::string, variable>&, const std::unordered_set<std::string>&, const std::unordered_set<std::string>&, const std::string&);
    void                     findCommonData             ();
    void                     findFreeFunctionData       ();
    void                     findMethodName             (srcml_archive*, srcml_unit*);
    void                     findMethodReturnType       (srcml_archive*, srcml_unit*);
    void                     findParameterList          (srcml_archive*, srcml_unit*);
    void                     findLocalVariableName      (srcml_archive*, srcml_unit*);
    void                     findLocalVariableType      (srcml_archive*, srcml_unit*);
    void                     findParameterName          (srcml_archive*, srcml_unit*);
    void                     findParameterType          (srcml_archive*, srcml_unit*);
    void                     findReturnExpression       (srcml_archive*, srcml_unit*);   
    void                     findCallName               (srcml_archive*, srcml_unit*);
    void                     findCallArgument           (srcml_archive*, srcml_unit*);
    void                     findNewAssign              (srcml_archive*, srcml_unit*);
    void                     findConst                  (srcml_archive*, srcml_unit*);
    void                     findConstructorOrDestructor(srcml_archive*, srcml_unit*);
    void                     findIgnorableCalls         (std::vector<calls>&);
    void                     findCallsOnField           (std::unordered_map<std::string, variable>&, const std::unordered_set<std::string>&, const std::unordered_set<std::string>&);   
    void                     findCallsOnParameters      ();
    void                     findReturnedVariables      (std::unordered_map<std::string, variable>&, bool);
    void                     findModifiedVariables      (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&, bool);                             
    void                     findVariablesInExpressions (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&, bool);
    void                     findNonCommentsStatements  (srcml_archive*, srcml_unit*);
    void                     findModifiedRefParameter   (std::string, bool);      

    void                     setStereotype              (const std::string& s) { stereotype.push_back(s);}
                                             
private:
    std::string                                       name;                                       // Name without namespaces
    std::string                                       nameSignature;                              // Name without namespaces + parameters list (commas only). For example, foo(,,)
    std::string                                       returnType;                                 // Return type without whitespaces
    std::string                                       returnTypeParsed;                           // Return type without specifiers, containers, and whitespaces
    std::string                                       parametersList;                             // Parameters list
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
    std::vector<calls>                                functionCalls;                              // List of function calls (e.g., foo()) to methods in class
    std::vector<calls>                                methodCalls;                                // List of method calls (e.g., a.foo()) where 'a' is an field
    std::vector<calls>                                constructorCalls;                           // List of constructor calls
    std::vector<std::string>                          returnExpressions;                          // List of all return expressions in a method
    bool                                              constMethod{false};                         // Is it a const method? C++ only
    bool                                              fieldReturned{false};                       // Does it contains at least 1 simple return that returns an field? (e.g., return a; where 'a' is an field)
    bool                                              complexReturn{false};                       // Does it contains at least 1 return that is not a simple return?
    bool                                              parameterNotReturned{false};                // Does it contains at least 1 return that doesn't return a simple parameter?
    bool                                              parameterRefModified{false};                 // Does it change any parameter(s) passed by reference?
    bool                                              nonPrimitiveLocalOrParameterModified{false}; // Does it change any local(s) or parameter(s)
    bool                                              globalOrStaticModified{false};               // Does it change any global or static?
    bool                                              parameterUsed{false};                       // Does it use any parameters in an expression?
    bool                                              fieldUsed{false};                           // Does it use any field in an expression?
    bool                                              nonPrimitiveFieldExternal{false};           // True if method uses at least 1 non-primitive field that is not of the same type as class  
    bool                                              nonPrimitiveReturnType{false};              // True if method uses a non-primitive return type
    bool                                              nonPrimitiveReturnTypeExternal{false};      // True if method uses a non-primitive return type that is not of the same type as class   
    bool                                              nonPrimitiveLocalExternal{false};           // True if method uses at least 1 a non-primitive local that is not of the same type as class   
    bool                                              nonPrimitiveParamaterExternal{false};       // True if method uses at least 1 a non-primitive parameter that is not of the same type as class                                                
    bool                                              newReturned{false};                         // There is at least one return that a return a "new" call
    bool                                              constructorOrDestructor{false};             // Method is a constructor or a destructor
    bool                                              fieldsCreatedAndReturnedWithNew;            // Number of return expressions that return a local, parameter, or an field created with the "new" operator
    int                                               unitNumber{-1};                             // Unit number
    int                                               numOfFieldsModified{0};                     // Number of modified fields
    int                                               numOfExternalFunctionCalls{0};              // Number of function calls that are filtered (removed)
    int                                               numOfExternalMethodCalls{0};                // Number of method calls that are filtered (removed)
    int                                               numOfNonCommentsStatements{0};              // Number of non-comment statements
};

#endif
