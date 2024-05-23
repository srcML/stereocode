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

class methodModel {
public:
    methodModel(srcml_archive*, srcml_unit*, const std::string&, const std::string&, const std::string&, int);

    const std::vector<variable>&                                         getParameterOrdered                 () const                { return parameterOrdered;     }
    const std::vector<std::pair<std::string, std::string>>&              getFunctionCall                     () const                { return functionCall;         }
    const std::vector<std::pair<std::string, std::string>>&              getMethodCall                       () const                { return methodCall;           }

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
    
    int                      getNumOfAttributeModified          () const                { return numOfAttributeModified;                    }
    int                      getUnitNumber                      () const                { return unitNumber;                                }  
    int                      getNumOfExternalFunctionCalls      () const                { return numOfExternalFunctionCalls;                } 
    int                      getNumOfExternalMethodCalls        () const                { return numOfExternalMethodCalls;                  } 

    bool                     IsCheckedCall                      () const                { return checkedCall;                               }    
    bool                     IsVisitedCall                      () const                { return visitedCall;                               } 

    bool                     IsConstMethod                      () const                { return constMethod;                               }
    bool                     IsAttributeReturned                () const                { return attributeReturned;                         }
    bool                     IsAttributeNotReturned             () const                { return attributeNotReturned;                      }
    bool                     IsNewCallReturned                  () const                { return newReturned;                               }
    bool                     IsVariableCreatedWithNewReturned   () const                { return variableCreatedWithNewReturned;            }
    bool                     IsAttributeUsed                    () const                { return attributeUsed;                             }
    bool                     IsEmpty                            () const                { return empty;                                     }
    bool                     IsParameterRefChanged              () const                { return parameterRefChanged;                       }      
    bool                     IsAccessorMethodCallUsed           () const                { return accessorMethodCallUsed;                    }      
    bool                     IsNonPrimitiveAttributeExternal    () const                { return nonPrimitiveAttributeExternal;             }
    bool                     IsNonPrimitiveReturnType           () const                { return nonPrimitiveReturnType;                    }
    bool                     IsNonPrimitiveReturnTypeExternal   () const                { return nonPrimitiveReturnTypeExternal;            }
    bool                     IsNonPrimitiveLocalExternal        () const                { return nonPrimitiveLocalExternal;                 }  
    bool                     IsNonPrimitiveParamaterExternal    () const                { return nonPrimitiveParamaterExternal;             }  
    bool                     IsConstructorDestructorUsed        () const                { return constructorDestructorUsed;                 }  
    
    void                     setStereotype                       (const std::string&);
    void                     setVisitedCall                      (bool s)               { visitedCall = s; }
    void                     setCheckedCall                      (bool s)               { checkedCall = s; }    


    void                     findMethodData(std::unordered_map<std::string, variable>&,
                                            const std::unordered_set<std::string>&, 
                                            const std::unordered_set<std::string>&, const std::string&);


    void                     findFriendData             ();
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

    void                     isIgnorableCall            (std::vector<std::pair<std::string, std::string>>&);
    void                     isFunctionCall             (std::unordered_map<std::string, variable>&);
    void                     isCallOnAttribute          (std::unordered_map<std::string, variable>&, 
                                                         const std::unordered_set<std::string>&, const std::unordered_set<std::string>&);

    
    void                     isAttributeReturned            (std::unordered_map<std::string, variable>&);
    void                     isAttributeOrParameterModified (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&);                             
    void                     isAttributeUsedInExpression    (srcml_archive*, srcml_unit*, std::unordered_map<std::string, variable>&);
    bool                     isAttributeUsed                (std::unordered_map<std::string, variable>&, const std::string&, bool, bool);
    
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
    std::vector<std::pair<std::string, std::string>>  functionCall;                           // Such as a(), b(), base.a(), this.a(), super.a(). Pair is call name and call argument list
    std::unordered_set<std::string>                   functionCallSet;                        // Needed for finding accessor methods
    std::vector<std::pair<std::string, std::string>>  methodCall;                             // Such as a.b() where 'a' is an attribute. Pair is call name and call argument list 
    std::vector<std::string>                          returnExpression;                       // List of all return expressions in a method
    bool                                              constMethod{false};                     // Is it a const method? C++ only
    bool                                              attributeReturned{false};               // Does it contains at least 1 return that returns an attribute? Simple return
    bool                                              attributeNotReturned{false};            // Does it contains at least 1 return that doesn't return an attribute? Simple or complex return
    bool                                              parameterRefChanged{false};             // Does it change any parameter passed by reference?
    bool                                              attributeUsed{false};                   // Does it use any attribute?
    bool                                              empty{false};                           // Is method empty? (comments not counted)
    bool                                              nonPrimitiveAttributeExternal{false};   // True if method uses at least 1 non-primitive attribute that is not of the same type as class  
    bool                                              nonPrimitiveReturnType{false};          // True if method uses a non-primitive return type
    bool                                              nonPrimitiveReturnTypeExternal{false};  // True if method uses a non-primitive return type that is not of the same type as class   
    bool                                              nonPrimitiveLocalExternal{false};       // True if method uses at least 1 a non-primitive local that is not of the same type as class   
    bool                                              nonPrimitiveParamaterExternal{false};   // True if method uses at least 1 a non-primitive parameter that is not of the same type as class                                            
    bool                                              checkedCall{false};                     // Has method been analyzed for function calls yet? (Used for the deep analysis of calls)
    bool                                              visitedCall{false};                     // Has method been visited yet when analyzing function calls? (Used for the deep analysis of calls)    
    bool                                              newReturned{false};                     // There is at least one return that a return a "new" call
    bool                                              variableCreatedWithNewReturned{false};
    bool                                              accessorMethodCallUsed{false};
    bool                                              constructorDestructorUsed{false};
    int                                               numOfAttributeModified{0};              // Number of modified attributes
    int                                               unitNumber{-1};                         // Unit number
    int                                               numOfExternalFunctionCalls{0};          // Number of function calls that are filtered (removed)
    int                                               numOfExternalMethodCalls{0};            // Number of method calls that are filtered (removed)

};

#endif
