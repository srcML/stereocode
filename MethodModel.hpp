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
    std::string temp;
    methodModel(srcml_archive*, srcml_unit*, const std::string&, const std::string&, const std::string&, int);

    const std::vector<variable>&           getParameterOrdered                 () const                { return parameterOrdered;     }
    std::vector<calls>&                    getFunctionCall                     ()                      { return functionCall;         }
    const std::vector<calls>&              getMethodCall                       () const                { return methodCall;           }

    std::string              getName                             () const                { return name;                                     }
    std::string              getNameSignature                    () const                { return nameSignature;                            }
    std::string              getParameterList                    () const                { return parameterList;                            }
    std::string              getSrcML                            () const                { return srcML;                                    }
    std::string              getReturnType                       () const                { return returnType;                               }
    std::string              getReturnTypeParsed                 () const                { return returnTypeParsed;                         }
    std::string              getStereotype                       () const;
    std::vector<std::string> getStereotypeList                   () const                { return stereotype;                               }
    std::string              getConst                            () const                { if (constMethod) return "const"; else return ""; }
    std::string              getXpath                            () const                { return xpath;                                    } 
    std::string              getUnitLanguage                     () const                { return unitLanguage;                             } 
    
    void                     getDataFromCall                    (methodModel&, calls&);

    int                      getNumOfAttributeModified          () const                { return numOfAttributeModified;                    }
    int                      getNumOfAttributeModifiedByCalls   () const                { return numOfAttributeModifiedByCalls;             }
    int                      getUnitNumber                      () const                { return unitNumber;                                }  
    int                      getNumOfExternalFunctionCalls      () const                { return numOfExternalFunctionCalls;                } 
    int                      getNumOfExternalMethodCalls        () const                { return numOfExternalMethodCalls;                  } 
  
    bool                     IsVisitedCall                      () const                { return visitedCall;                               } 
    bool                     IsStatic                           () const                { return staticMethod;                              } 
    bool                     IsConstMethod                      () const                { return constMethod;                               }
    bool                     IsAttributeReturned                () const                { return attributeReturned;                         }
    bool                     IsAttributeNotReturned             () const                { return attributeNotReturned;                      }
    bool                     IsParameterNotReturned             () const                { return parameterNotReturned;                      }
    bool                     IsNewCallReturned                  () const                { return newReturned;                               }
    bool                     IsVariableCreatedWithNewReturned   () const                { return variableCreatedWithNewReturned;            }
    bool                     IsAttributeUsed                    () const                { return attributeUsed;                             }
    bool                     IsParameterUsed                    () const                { return parameterUsed;                             }
    bool                     IsEmpty                            () const                { return empty;                                     }
    bool                     IsParameterRefChanged              () const                { return parameterRefChanged;                       }    
    bool                     IsGlobalOrStaticChanged            () const                { return globalOrStaticChanged;                     }     
    bool                     IsAccessorMethodCallUsed           () const                { return accessorMethodCallUsed;                    }      
    bool                     IsNonPrimitiveAttributeExternal    () const                { return nonPrimitiveAttributeExternal;             }
    bool                     IsNonPrimitiveReturnType           () const                { return nonPrimitiveReturnType;                    }
    bool                     IsNonPrimitiveReturnTypeExternal   () const                { return nonPrimitiveReturnTypeExternal;            }
    bool                     IsNonPrimitiveLocalExternal        () const                { return nonPrimitiveLocalExternal;                 }  
    bool                     IsNonPrimitiveParamaterExternal    () const                { return nonPrimitiveParamaterExternal;             }  
    bool                     IsConstructorDestructorUsed        () const                { return constructorDestructorUsed;                 }  
    
    void                     setStereotype                       (const std::string&);
    void                     setVisitedCall                      (bool s)               { visitedCall = s; }


    void                     findMethodData(std::unordered_map<std::string, variable>&,
                                            const std::unordered_map<std::string, methodModel>&, 
                                            const std::unordered_set<std::string>&, const std::string&);

    void                     findCommonData             ();
    void                     findFriendData             ();
    void                     findFreeFunctionData       ();
    void                     findFriendFunctionInfo     (srcml_archive*, srcml_unit*);
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
    void                     findAccessorMethods        (srcml_archive*, srcml_unit*);
    void                     isEmpty                    (srcml_archive*, srcml_unit*);
    void                     isConst                    (srcml_archive*, srcml_unit*);
    void                     isConstructorDestructor    (srcml_archive*, srcml_unit*);
    void                     isStatic                   (srcml_archive*, srcml_unit*);
    void                     isIgnorableCall            (std::vector<calls>&);
    void                     isCallOnAttribute          (std::unordered_map<std::string, variable>&, 
                                                         const std::unordered_map<std::string, methodModel>&, const std::unordered_set<std::string>&);
    void                     isCallOnParameter ();
    bool                     isAttributeUsedAsArgument  (std::unordered_map<std::string, variable>&, calls&, bool);
    
    void                     isAttributeOrParameterReturned            (std::unordered_map<std::string, variable>&, bool);
    void                     isAttributeOrParameterModified (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&);                             
    void                     isAttributeOrParamaterUsedInExpression    (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&);

    bool                     isAttributeOrParameterUsed                (std::unordered_map<std::string, variable>&, const std::string&, bool, bool, bool);
    
    void                     isParameterRefChanged        (std::string, bool);                                               
private:
    std::string                                       name;                                   // Name
    std::string                                       nameSignature;                          // Name (without namespaces) + parameter list (commas only). For example, foo(,,)
    std::string                                       returnType;                             // Return type
    std::string                                       returnTypeParsed;                       // Return type without specifiers and whitespaces.
    std::string                                       parameterList;                          // Parameter list
    std::string                                       unitLanguage;                           // Unit language
    std::string                                       xpath;                                  // Unique xpath
    std::string                                       srcML;                                  // Method srcML
    std::vector<variable>                             parameterOrdered;                       // List of all parameters (Needed in order to build the parameter map)
    std::vector<variable>                             localOrdered;                           // List of all local (Needed in order to build the local map)     
    std::unordered_map<std::string, variable>         parameter;                              // Map of all parameters. Key is parameter name
    std::unordered_map<std::string, variable>         local;                                  // Map of all locals. Key is local name         
    std::string                                       classNameParsed;                        // Class name without whitespaces, namespaces, and generic types <>
    std::unordered_set<std::string>                   variablesCreatedWithNew;                // List of variables that are declared/initialized with the "new" operator
    std::vector<std::string>                          stereotype;                             // Method stereotype
    std::vector<calls>                                functionCall;                           // List of function calls (e.g., foo()) to methods in class
    std::vector<calls>                                methodCall;                             // List of method calls (e.g., a.foo()) where 'a' is an attribute
    std::set<int>                                     parameterRefChangedPos;                 // Set of positions for parameters changed by reference
    std::vector<std::string>                          returnExpression;                       // List of all return expressions in a method
    bool                                              constMethod{false};                     // Is it a const method? C++ only
    bool                                              attributeReturned{false};               // Does it contains at least 1 return that returns an attribute? Simple return
    bool                                              attributeNotReturned{false};            // Does it contains at least 1 return that doesn't return an attribute? Simple or complex return
    bool                                              parameterNotReturned{false};            // Does it contains at least 1 return that doesn't return a parameter? Simple or complex return
    bool                                              parameterRefChanged{false};             // Does it change any parameter passed by reference?
    bool                                              globalOrStaticChanged{false};           // Does it change any global or static?
    bool                                              attributeUsed{false};                   // Does it use any attribute in an expression?
    bool                                              parameterUsed{false};                   // Does it use any parameters in an expression?
    bool                                              empty{false};                           // Is method empty? (comments not counted)
    bool                                              nonPrimitiveAttributeExternal{false};   // True if method uses at least 1 non-primitive attribute that is not of the same type as class  
    bool                                              nonPrimitiveReturnType{false};          // True if method uses a non-primitive return type
    bool                                              nonPrimitiveReturnTypeExternal{false};  // True if method uses a non-primitive return type that is not of the same type as class   
    bool                                              nonPrimitiveLocalExternal{false};       // True if method uses at least 1 a non-primitive local that is not of the same type as class   
    bool                                              nonPrimitiveParamaterExternal{false};   // True if method uses at least 1 a non-primitive parameter that is not of the same type as class                                            
    bool                                              visitedCall{false};                     // Has method been visited yet when analyzing function calls? (Used for the deep analysis of calls)    
    bool                                              newReturned{false};                     // There is at least one return that a return a "new" call
    bool                                              variableCreatedWithNewReturned{false};  // Locals, parameters, or attributes created wuth "new" operator
    bool                                              accessorMethodCallUsed{false};          // True if method uses at least one call that returns a value
    bool                                              constructorDestructorUsed{false};       // Method is a constructor or a destructor
    bool                                              staticMethod{false};                    // True if method is static
    int                                               numOfAttributeModified{0};              // Number of modified attributes
    int                                               numOfAttributeModifiedByCalls{0};       // Number of modified attributes by local function calls to other methods of the class
    int                                               unitNumber{-1};                         // Unit number
    int                                               numOfExternalFunctionCalls{0};          // Number of function calls that are filtered (removed)
    int                                               numOfExternalMethodCalls{0};            // Number of method calls that are filtered (removed)

};

#endif
