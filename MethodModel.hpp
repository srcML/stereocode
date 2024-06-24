// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file MethodModel.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
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

    const std::vector<variable>&    getParametersOrdered                () const                { return parametersOrdered;     }
    const std::vector<calls>&       getFunctionCalls                    () const                { return functionCalls;         }
    const std::vector<calls>&       getMethodCalls                      () const                { return methodCalls;           }
    const std::vector<calls>&       getConstructorCalls                 () const                { return constructorCalls;      }
    
    const std::string&              getName                             () const                { return name;                                     }
    const std::string&              getNameSignature                    () const                { return nameSignature;                            }
    const std::string&              getParametersList                   () const                { return parametersList;                           }
    const std::string&              getSrcML                            () const                { return srcML;                                    }
    const std::string&              getReturnType                       () const                { return returnType;                               }
    const std::string&              getReturnTypeParsed                 () const                { return returnTypeParsed;                         }
    std::string                     getStereotype                       () const;
    const std::vector<std::string>& getStereotypeList                   () const                { return stereotype;                               }
    const std::string&              getXpath                            () const                { return xpath;                                    } 
    const std::string&              getUnitLanguage                     () const                { return unitLanguage;                             } 
    
    int                      getNumOfAttributesModified         () const                { return numOfAttributesModified;                    }
    int                      getUnitNumber                      () const                { return unitNumber;                                }  
    int                      getNumOfExternalFunctionCalls      () const                { return numOfExternalFunctionCalls;                } 
    int                      getNumOfExternalMethodCalls        () const                { return numOfExternalMethodCalls;                  } 
    bool                     IsStatic                           () const                { return staticMethod;                              } 
    bool                     IsConstMethod                      () const                { return constMethod;                               }
    bool                     IsAttributeReturned                () const                { return attributeReturned;                         }
    bool                     IsAttributeNotReturned             () const                { return attributeNotReturned;                      }
    bool                     IsParameterNotReturned             () const                { return parameterNotReturned;                      }
    bool                     IsAttributeUsed                    () const                { return attributeUsed;                             }
    bool                     IsParameterUsed                    () const                { return parameterUsed;                             }
    bool                     IsEmpty                            () const                { return empty;                                     }
    bool                     IsFactory                          () const                { return factory;                                   }
    bool                     IsStrictFactory                    () const                { return strictFactory;                             }    
    bool                     IsParameterRefChanged              () const                { return parameterRefChanged;                       }    
    bool                     IsGlobalOrStaticChanged            () const                { return globalOrStaticChanged;                     }          
    bool                     IsNonPrimitiveAttributeExternal    () const                { return nonPrimitiveAttributeExternal;             }
    bool                     IsNonPrimitiveReturnTypeExternal   () const                { return nonPrimitiveReturnTypeExternal;            }
    bool                     IsNonPrimitiveLocalExternal        () const                { return nonPrimitiveLocalExternal;                 }  
    bool                     IsNonPrimitiveParamaterExternal    () const                { return nonPrimitiveParamaterExternal;             }  
    bool                     IsConstructorDestructorUsed        () const                { return constructorDestructorUsed;                 }  
    
    bool                     IsNonPrimitiveLocalOrParameterChanged () const             { return nonPrimitiveLocalOrParameterChanged;                 }  
    void                     setStereotype                         (const std::string&);

    void                     findMethodData                        (std::unordered_map<std::string, variable>&,
                                                                   const std::unordered_set<std::string>&, 
                                                                   const std::unordered_set<std::string>&, const std::string&);

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
    void                     isConst                    (srcml_archive*, srcml_unit*);
    void                     isConstructorDestructor    (srcml_archive*, srcml_unit*);
    void                     isStatic                   (srcml_archive*, srcml_unit*);
    void                     isIgnorableCall            (std::vector<calls>&);
    void                     isCallOnAttribute          (std::unordered_map<std::string, variable>&, 
                                                         const std::unordered_set<std::string>&, const std::unordered_set<std::string>&);   
    void                     isAttributeUsedInCallArgument   (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&);                              
    void                     isCallOnParameter          ();
    void                     isVariableReturned         (std::unordered_map<std::string, variable>&, bool);
    void                     isVariableModified         (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&, bool);                             
    void                     isVariableUsedInExpression (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&, bool);
    void                     isParameterRefChanged      (std::string, bool);      

    bool                     isVariableUsed             (std::unordered_map<std::string, variable>&, const std::string&, bool, bool, bool, bool, bool);
    void                     isEmpty                    (srcml_archive*, srcml_unit*);
    void                     isFactory                  ();
                                             
private:
    std::string                                       name;                                       // Name without namespaces
    std::string                                       nameSignature;                              // Name without namespaces + parameters list (commas only). For example, foo(,,)
    std::string                                       returnType;                                 // Return type without whitespaces
    std::string                                       returnTypeParsed;                           // Return type without specifiers, containers, and whitespaces
    std::string                                       parametersList;                             // Parameters list
    std::string                                       unitLanguage;                               // Unit language
    std::string                                       xpath;                                      // Unique xpath
    std::string                                       srcML;                                      // Method srcML
    std::string                                       callsArguments;                             // All arguments of calls concatenated (Needed to find usage of attributes)
    std::vector<variable>                             parametersOrdered;                          // List of all parameters (Needed in order to build the parameters map)
    std::vector<variable>                             localsOrdered;                              // List of all local (Needed in order to build the locals map)     
    std::unordered_map<std::string, variable>         parameters;                                 // Map of all parameters. Key is parameter name
    std::unordered_map<std::string, variable>         locals;                                     // Map of all locals. Key is local name         
    std::string                                       classNameParsed;                            // Class name without whitespaces, namespaces, and generic types <>
    std::unordered_set<std::string>                   variablesCreatedWithNew;                    // List of variables that are declared/initialized with the "new" operator
    std::vector<std::string>                          stereotype;                                 // Method stereotype
    std::vector<calls>                                functionCalls;                              // List of function calls (e.g., foo()) to methods in class. Constructor calls to class are not considered
    std::vector<calls>                                methodCalls;                                // List of method calls (e.g., a.foo()) where 'a' is an attribute
    std::vector<calls>                                constructorCalls;                           // List of constructor calls
    std::vector<std::string>                          returnExpressions;                          // List of all return expressions in a method
    bool                                              constMethod{false};                         // Is it a const method? C++ only
    bool                                              attributeReturned{false};                   // Does it contains at least 1 simple return that returns an attribute? (e.g., return a; where 'a' is an attribute)
    bool                                              attributeNotReturned{false};                // Does it contains at least 1 return that is not a simple return?
    bool                                              parameterNotReturned{false};                // Does it contains at least 1 return that doesn't return a simple parameter?
    bool                                              parameterRefChanged{false};                 // Does it change any parameter(s) passed by reference?
    bool                                              nonPrimitiveLocalOrParameterChanged{false}; // Does it change any local(s) or parameter(s)
    bool                                              globalOrStaticChanged{false};               // Does it change any global or static?
    bool                                              parameterUsed{false};                       // Does it use any parameters in an expression?
    bool                                              attributeUsed{false};                       // Does it use any attribute in an expression?
    bool                                              empty{false};                               // Is method empty? (comments not counted)
    bool                                              strictFactory{false};                       // Is method a factory?
    bool                                              factory{false};                             // Does method contain at least one return expression that returns a newly created object?
    bool                                              nonPrimitiveAttributeExternal{false};       // True if method uses at least 1 non-primitive attribute that is not of the same type as class  
    bool                                              nonPrimitiveReturnType{false};              // True if method uses a non-primitive return type
    bool                                              nonPrimitiveReturnTypeExternal{false};      // True if method uses a non-primitive return type that is not of the same type as class   
    bool                                              nonPrimitiveLocalExternal{false};           // True if method uses at least 1 a non-primitive local that is not of the same type as class   
    bool                                              nonPrimitiveParamaterExternal{false};       // True if method uses at least 1 a non-primitive parameter that is not of the same type as class                                                
    bool                                              newReturned{false};                         // There is at least one return that a return a "new" call
    bool                                              constructorDestructorUsed{false};           // Method is a constructor or a destructor
    bool                                              staticMethod{false};                        // True if method is static
    int                                               unitNumber{-1};                             // Unit number
    int                                               numOfVariablesReturnedCreatedWithNew{0};    // Number of return expressions that return a local, parameter, or an attribute created with the "new" operator
    int                                               numOfAttributesModified{0};                 // Number of modified attributes
    int                                               numOfExternalFunctionCalls{0};              // Number of function calls that are filtered (removed)
    int                                               numOfExternalMethodCalls{0};                // Number of method calls that are filtered (removed)

};

#endif
