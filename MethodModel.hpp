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
#include <unordered_map>
#include <fstream>
#include <thread>

#include "utils.hpp"

class methodModel {
public:
    methodModel(srcml_archive*, srcml_unit*, const std::string& , std::string, int);

    const std::vector<Variable>&                 getParameterOrdered                 () const                { return parameterOrdered;  }
    const std::vector<std::string>&              getFunctionCall                     () const                { return functionCall;      }
    const std::vector<std::string>&              getMethodCall                       () const                { return methodCall;        }
    const std::vector<std::string>&              getConstructorCall                  () const                { return constructorCall;   }

    std::string              getName                             () const                { return name;                      }
    std::string              getSrcML                            () const                { return srcML;                     }
    std::string              getReturnType                       () const                { return returnType;                }
    std::string              getReturnTypeSeparated              () const                { return returnTypeSeparated;       }
    std::string              getStereotype                       () const;
    std::vector<std::string> getStereotypeList                   () const                { return stereotype;                }
    std::string              getConst                            () const                { if (constMethod) return "const"; else return ""; }
    std::string              getXpath                            () const                { return xpath;                     }  

    bool                     IsConstMethod                      () const                { return constMethod;               }
    bool                     IsAttributeReturned                () const                { return attributeReturned;         }
    bool                     IsParameterOrLocalReturned         () const                { return paraOrLocalReturned;       }
    bool                     IsNewReturned                      () const                { return newReturned;               }
    bool                     IsMethodCallOnAttribute            () const                { return methodCallOnAttribute;     }
    bool                     IsFunctionCallOnAttribute          () const                { return functionCallOnAttribute;   }
    bool                     IsAttributeUsed                    () const                { return attributeUsed;             }
    bool                     IsEmpty                            () const                { return empty;                     }
    bool                     IsParameterChanged                 () const                { return parameterChanged;          }
    bool                     IsNonPrimitiveAttribute            () const                { return nonPrimitiveAttribute;           }
    bool                     IsNonPrimitiveAttributeExternal    () const                { return nonPrimitiveAttributeExternal;   }
    bool                     IsNonPrimitiveReturnType           () const                { return nonPrimitiveReturnType;          }
    bool                     IsNonPrimitiveReturnTypeExternal   () const                { return nonPrimitiveReturnTypeExternal;  }
    bool                     IsNonPrimitiveLocal                () const                { return nonPrimitiveLocal;               }
    bool                     IsNonPrimitiveLocalExternal        () const                { return nonPrimitiveLocalExternal;       }
    bool                     IsNonPrimitiveParamater            () const                { return nonPrimitiveParamater;           }
    bool                     IsNonPrimitiveParamaterExternal    () const                { return nonPrimitiveParamaterExternal;   }  

    int                      getAttributeModified               () const                { return numOfAttributeModified;          }
    int                      getUnitNumber                      () const                { return unitNumber;                      }  
    
    void                     setStereotype              (const std::string&);

    void                     findMethodData             (const std::unordered_map<std::string, Variable>&, 
                                                         const std::unordered_set<std::string>&, std::string);
    void                     findMethodName             (srcml_archive*, srcml_unit*);
    void                     findMethodReturnType       (srcml_archive*, srcml_unit*, const std::string&);
    void                     findLocalVariableName      (srcml_archive*, srcml_unit*);
    void                     findLocalVariableType      (srcml_archive*, srcml_unit*, const std::string&);
    void                     findParameterName          (srcml_archive*, srcml_unit*);
    void                     findParameterType          (srcml_archive*, srcml_unit*, const std::string&);
    void                     findReturnExpression       (srcml_archive*, srcml_unit*);   
    void                     findCall                   (srcml_archive*, srcml_unit*, std::string, bool);
   
    
    void                     isEmpty                   (srcml_archive*, srcml_unit*);
    void                     isConst                   (srcml_archive*, srcml_unit*);
    void                     isAttributeReturned       (const std::unordered_map<std::string, Variable>&, const std::string&, const std::unordered_set<std::string>&);
    void                     isAttributeChanged        (srcml_archive*, srcml_unit*, const std::unordered_map<std::string, Variable>&,
                                                        const std::string&, const std::unordered_set<std::string>&);                             
    void                     isAttributeUsed           (srcml_archive*, srcml_unit*, const std::unordered_map<std::string, Variable>&,
                                                        const std::string&, const std::unordered_set<std::string>&);
    void                     isCallOnAttribute         (const std::unordered_map<std::string, Variable>&, std::string,
                                                        const std::string&, const std::unordered_set<std::string>&);
    
    bool                     isAttributeUsedAsArgument (const std::unordered_map<std::string, Variable>&, std::string,
                                                        const std::string&, const std::unordered_set<std::string>&);
    bool                     isAttribute               (const std::unordered_map<std::string, Variable>&, const std::string&, 
                                                        const std::string&, const std::unordered_set<std::string>&, bool);
protected:
    std::string                                  name{};                               // Method name
    std::string                                  returnType{};                         // Method return type
    std::string                                  returnTypeSeparated{};                // Return type with specifiers, containers, and whitespaces removed. Only comma separated. For example string,int or bool
    std::string                                  unitLanguage{};                       // Unit language
    std::string                                  xpath{};                              // Unique xpath for method
    std::string                                  srcML{};
    std::vector<Variable>                        parameterOrdered{};
    std::vector<Variable>                        localOrdered{};
    std::unordered_map<std::string, Variable>    parameter{};
    std::unordered_map<std::string, Variable>    local{};  
    std::vector<std::string>                     stereotype{};                         // Method stereotype
    std::vector<std::string>                     functionCall{};                       // Such as a(), b()
    std::vector<std::string>                     methodCall{};                         // Such as a.b()
    std::vector<std::string>                     constructorCall{};                    // A call that uses the "new" operator
    std::vector<std::string>                     returnExpression{};                   // List of all return expressions in a method
    bool                                         constMethod{false};                   // Is it a const method? C++ only
    bool                                         attributeReturned{false};             // Does it return any attribute (a)? For example, return a or return {a}
    bool                                         paraOrLocalReturned{false};           // Does it return a local (a) or a parameter (b)? For example, return a or return {b}
    bool                                         newReturned{false};                   // Does any of the return expressions contain a 'new' operator?
    bool                                         methodCallOnAttribute{false};         // Does method have any method call on attribute? 
    bool                                         functionCallOnAttribute{false};       // Does method have any function call on attribute?
    bool                                         attributeUsed{false};                 // Does it use any attribute?
    bool                                         parameterChanged{false};              // Does it change any parameter passed by reference?
    bool                                         empty{false};                         // Empty method (comments not counted)
    bool                                         nonPrimitiveAttribute{false};         // True if method uses at least 1 non-primitive attribute
    bool                                         nonPrimitiveAttributeExternal{false}; // True if all of the non-primitive attributes (if any) are not of the same type as class  
    bool                                         nonPrimitiveReturnType{false};        // True if return type is non-primitive 
    bool                                         nonPrimitiveReturnTypeExternal{false};// True if all of the non-primitive returns (if any) are not of the same type as class  
    bool                                         nonPrimitiveLocal{false};             // True if method uses at least 1 non-primitive local variable
    bool                                         nonPrimitiveLocalExternal{false};     // True if all of the non-primitive local variables (if any) are not of the same type as class  
    bool                                         nonPrimitiveParamater{false};         // True if method uses at least 1 non-primitive parameter
    bool                                         nonPrimitiveParamaterExternal{false}; // True if all of the non-primitive parameters (if any) are not of the same type as class  
    int                                          numOfAttributeModified{-1};           // Number of modified attributes
    int                                          unitNumber{-1};                       // Number of unit for xpath
};

#endif
