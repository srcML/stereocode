// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file MethodModel.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#ifndef METHOD_HPP
#define METHOD_HPP

#include <srcml.h>
#include "utils.hpp"

class methodModel {
public:
    methodModel();
    methodModel(srcml_archive*, srcml_unit*, std::string, int);

    std::vector<std::string> getLocalVariablesNames              () const                { return localVariableName; }
    std::vector<std::string> getLocalVariablesTypes              () const                { return localVariableType; }
    std::vector<std::string> getParameterNames                   () const                { return parameterName;     }
    std::vector<std::string> getParameterTypes                   () const                { return parameterType;     }
    std::vector<std::string> getFunctionCalls                    () const                { return functionCall;      }
    std::vector<std::string> getMethodCalls                      () const                { return methodCall;        }
    std::vector<std::string> getConstructorCalls                 () const                { return constructorCall;   }
    std::vector<std::string> getReturnExpressions                () const                { return returnExpression;  }

    std::string              getMethodName                       () const                { return methodName;                }
    std::string              getMethodHeader                     () const                { return methodHeader;              }
    std::string              getReturnType                       () const                { return returnType;                }
    std::string              getReturnTypeSeparated              () const                { return returnTypeSeparated;       }
    std::string              getStereotype                       () const;
    std::vector<std::string> getStereotypeList                   () const                { return stereotype;                      }
    std::string              getMethodSrcML                      () const                { return methodSrcML;                     }
    bool                     getConstMethod                      () const                { return constMethod;                     }
    bool                     getAttributeReturned                () const                { return attributeReturned;               }
    bool                     getParaOrLocalReturned              () const                { return paraOrLocalReturned;             }
    bool                     getReturnsNew                       () const                { return returnsNew;                      }
    bool                     getReturnsCall                      () const                { return returnsCall;                     }
    bool                     getAttributeUsed                    () const                { return attributeUsed;                   }
    bool                     getEmpty                            () const                { return empty;                           }
    bool                     getParameterChanged                 () const                { return parameterChanged;                }
    bool                     getNonPrimitiveAttribute            () const                { return nonPrimitiveAttribute;           }
    bool                     getNonPrimitiveAttributeExternal    () const                { return nonPrimitiveAttributeExternal;   }
    bool                     getNonPrimitiveReturnType           () const                { return nonPrimitiveReturnType;          }
    bool                     getNonPrimitiveReturnTypeExternal   () const                { return nonPrimitiveReturnTypeExternal;  }
    bool                     getNonPrimitiveLocal                () const                { return nonPrimitiveLocal;               }
    bool                     getNonPrimitiveLocalExternal        () const                { return nonPrimitiveLocalExternal;       }
    bool                     getNonPrimitiveParamater            () const                { return nonPrimitiveParamater;           }
    bool                     getNonPrimitiveParamaterExternal    () const                { return nonPrimitiveParamaterExternal;   }        
    int                      getAttributeModified                () const                { return attributeModified;               }

    std::string              getXpath                            (int unitNumber)        { return xpath[unitNumber];               }   
    
    void                     setStereotype              (const std::string&);

    void                     findMethodInfo             (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&, std::string);
    void                     findMethodName             (srcml_archive*, srcml_unit*);
    void                     findMethodReturnType       (srcml_archive*, srcml_unit*, std::string);
    void                     findParameterList          (srcml_archive*, srcml_unit*);
    void                     findLocalVariablesNames    (srcml_archive*, srcml_unit*);
    void                     findLocalVariablesTypes    (srcml_archive*, srcml_unit*, std::string);
    void                     findParametersNames        (srcml_archive*, srcml_unit*);
    void                     findParametersTypes        (srcml_archive*, srcml_unit*, std::string);
    void                     findAllCalls               (srcml_archive*, srcml_unit*);
    void                     findReturnCalls            (srcml_archive* , srcml_unit*);
    void                     findReturnExpressions      (srcml_archive*, srcml_unit*);   
    int                      findAssignOperatorAttribute(srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&);
    int                      findIncrementedAttribute   (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&);
    std::vector<std::string> findCalls                  (srcml_archive*, srcml_unit*, const std::string&, bool) const;

    void                     isAttributeUsed           (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&, std::string className);
    bool                     usesNonPrimitiveAttributes(const std::vector<AttributeInfo>&, std::string);
    bool                     callOnAttribute           (std::vector<AttributeInfo>&, std::string);
    bool                     callOnArgument            (std::vector<AttributeInfo>&, std::string, int);
    void                     isEmpty                   (srcml_archive*, srcml_unit*);
    void                     isConst                   (srcml_archive*, srcml_unit*);
    void                     isAttributeReturned       (std::vector<AttributeInfo>&, std::string);
    void                     countChangedAttribute     (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&);

protected:
    std::string                             methodName;                    // Method name
    std::string                             methodHeader;                  // Method header (all before {)
    std::string                             returnType;                    // Method return type
    std::string                             returnTypeSeparated;           // Returned type with specifiers and whitespaces removed
    std::string                             parameterList;                 // Method parameter list
    std::vector<std::string>                parameterName;                 // List of all parameters names
    std::vector<std::string>                parameterType;                 // List of all parameters names
    std::vector<std::string>                localVariableName;             // List of all local variables names
    std::vector<std::string>                localVariableType;             // List of all local variables types
    std::vector<std::string>                stereotype;                    // stereotype
    std::vector<std::string>                functionCall;                  // a(), b()
    std::vector<std::string>                methodCall;                    // a.b()
    std::vector<std::string>                constructorCall;               // uses "new" operator
    std::vector<std::string>                returnExpression;              // List of all return expressions in a method
    std::unordered_map<int, std::string>    xpath;                         // unit number along with unique xpath for the method
    std::string                             methodSrcML;                   // Method srcML
    bool                                    constMethod;                   // Is it a const method?
    bool                                    attributeReturned;             // Does it return any attribute?
    bool                                    paraOrLocalReturned;           // Does it return a local or a parameter?
    bool                                    returnsNew;                    // Does any of the return expressions (if any) contain 'new' operator?
    bool                                    returnsCall;                   // Does any of the return expressions (if any) contain a call? 
    bool                                    attributeUsed;                 // Does it use any attribute?
    bool                                    parameterChanged;              // Does it change any parameter passed by reference?
    bool                                    empty;                         // Empty method (comments not counted)
    bool                                    nonPrimitiveAttribute;         // True if method uses at least 1 non-primitive attribute
    bool                                    nonPrimitiveAttributeExternal; // True if all of the non-primitive attributes (if any) are not of the same type as class  
    bool                                    nonPrimitiveReturnType;        // True if return type is non-primitive 
    bool                                    nonPrimitiveReturnTypeExternal;// True if all of the non-primitive returns (if any) are not of the same type as class  
    bool                                    nonPrimitiveLocal;             // True if method uses at least 1 non-primitive local variable
    bool                                    nonPrimitiveLocalExternal;     // True if all of the non-primitive local variables (if any) are not of the same type as class  
    bool                                    nonPrimitiveParamater;         // True if method uses at least 1 non-primitive parameter
    bool                                    nonPrimitiveParamaterExternal; // True if all of the non-primitive parameters (if any) are not of the same type as class      
    int                                     attributeModified;             // Number of modified attributes 
    
};

#endif
