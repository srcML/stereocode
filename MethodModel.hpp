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
    methodModel(srcml_archive*, srcml_unit*);

    std::vector<std::string> getLocalVariablesName               () const                { return localVariableName; }
    std::vector<std::string> getLocalVariablesType               () const                { return localVariableType; }
    std::vector<std::string> getParameterName                    () const                { return parameterName;     }
    std::vector<std::string> getParameterType                    () const                { return parameterType;     }
    std::vector<std::string> getParameterTypeSeparated           () const                { return parameterTypeSeparated;     }
    std::vector<std::string> getFunctionCall                     () const                { return functionCall;      }
    std::vector<std::string> getMethodCall                       () const                { return methodCall;        }
    std::vector<std::string> getConstructorCall                  () const                { return constructorCall;   }
    std::vector<std::string> getReturnExpression                 () const                { return returnExpression;  }

    std::string              getName                             () const                { return name;                      }
    std::string              getReturnType                       () const                { return returnType;                }
    std::string              getReturnTypeSeparated              () const                { return returnTypeSeparated;       }
    std::string              getStereotype                       () const;
    std::vector<std::string> getStereotypeList                   () const                { return stereotype;                }
    std::string              getSrcML                            () const                { return srcML;                     }
    std::string              getConst                            () const                { if (constMethod) return "const"; else return ""; };
    std::string              getXpath                            () const                { return xpath;                     }   
    bool                     getConstMethod                      () const                { return constMethod;               }
    bool                     getAttributeReturned                () const                { return attributeReturned;         }
    bool                     getParaOrLocalReturned              () const                { return paraOrLocalReturned;       }
    bool                     getReturnsNew                       () const                { return returnsNew;                }
    bool                     getReturnsCall                      () const                { return returnsCall;               }
    bool                     getMethodCallOnAttribute            () const                { return methodCallOnAttribute;     }
    bool                     getFunctionCallOnAttribute          () const                { return functionCallOnAttribute;   }
    bool                     getAttributeUsed                    () const                { return attributeUsed;             }
    bool                     getEmpty                            () const                { return empty;                     }
    bool                     getParameterChanged                 () const                { return parameterChanged;          }
    bool                     getNonPrimitiveAttribute            () const                { return nonPrimitiveAttribute;           }
    bool                     getNonPrimitiveAttributeExternal    () const                { return nonPrimitiveAttributeExternal;   }
    bool                     getNonPrimitiveReturnType           () const                { return nonPrimitiveReturnType;          }
    bool                     getNonPrimitiveReturnTypeExternal   () const                { return nonPrimitiveReturnTypeExternal;  }
    bool                     getNonPrimitiveLocal                () const                { return nonPrimitiveLocal;               }
    bool                     getNonPrimitiveLocalExternal        () const                { return nonPrimitiveLocalExternal;       }
    bool                     getNonPrimitiveParamater            () const                { return nonPrimitiveParamater;           }
    bool                     getNonPrimitiveParamaterExternal    () const                { return nonPrimitiveParamaterExternal;   }        
    int                      getAttributeModified                () const                { return attributeModified;               }
    int                      getUnitNumber                       () const                { return unitNumber;                      }
    
    void                     setStereotype              (const std::string&);

    void                     findMethodInfo             (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&, std::string, std::string, int);
    void                     findMethodName             (srcml_archive*, srcml_unit*);
    void                     findMethodReturnType       (srcml_archive*, srcml_unit*);
    void                     findLocalVariableName      (srcml_archive*, srcml_unit*);
    void                     findLocalVariableType      (srcml_archive*, srcml_unit*, std::string);
    void                     findParameterName          (srcml_archive*, srcml_unit*);
    void                     findParameterType          (srcml_archive*, srcml_unit*, std::string);
    void                     findAllCall                (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&);
    void                     findReturnCall             (srcml_archive*, srcml_unit*);
    void                     findReturnExpression       (srcml_archive*, srcml_unit*);   
    int                      findAssignAttributeOrPara  (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&);
    int                      findIncrementedAttribute   (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&);
    std::vector<std::string> findCall                   (srcml_archive*, srcml_unit*, const std::string&, bool) const;

    void                     isAttributeUsed           (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&, std::string className);
    bool                     usesNonPrimitiveAttribute (const std::vector<AttributeInfo>&, std::string);
    bool                     callOnAttribute           (std::vector<AttributeInfo>&, std::string);
    bool                     callOnArgument            (std::vector<AttributeInfo>&, std::string, int);
    void                     isEmpty                   (srcml_archive*, srcml_unit*);
    void                     isConst                   (srcml_archive*, srcml_unit*);
    void                     isAttributeReturned       (std::vector<AttributeInfo>&, std::string);
    void                     countChangedAttribute     (srcml_archive*, srcml_unit*, std::vector<AttributeInfo>&);

protected:
    std::string                             name;                          // Method name
    std::string                             returnType;                    // Method return type
    std::string                             returnTypeSeparated;           // Return type with specifiers, containers, and whitespaces removed. Only comma separated. For example string,int or bool
    std::vector<std::string>                parameterName;                 // List of all parameters names
    std::vector<std::string>                parameterNameUnparsed;         // List of all parameters names with array indicies kept
    std::vector<std::string>                parameterType;                 // List of all parameters types
    std::vector<std::string>                parameterTypeSeparated;        // Parameters types with specifiers, containers, and whitespaces removed. Only comma separated. For example string,int or bool
    std::vector<std::string>                localVariableName;             // List of all local variables names
    std::vector<std::string>                localVariableType;             // List of all local variables types
    std::vector<std::string>                stereotype;                    // Method stereotype
    std::vector<std::string>                functionCall;                  // Such as a(), b()
    std::vector<std::string>                methodCall;                    // Such as a.b()
    std::vector<std::string>                constructorCall;               // A call that uses the "new" operator
    std::vector<std::string>                returnExpression;              // List of all return expressions in a method
    std::string                             xpath;                         // Unique xpath for method
    std::string                             srcML;                         // Method srcML
    bool                                    constMethod;                   // Is it a const method? C++ only
    bool                                    attributeReturned;             // Does it return any attribute (a)? For example, return a or return {a}
    bool                                    paraOrLocalReturned;           // Does it return a local (a) or a parameter (b)? For example, return a or return {b}
    bool                                    returnsNew;                    // Does any of the return expressions contain a 'new' operator?
    bool                                    returnsCall;                   // Does any of the return expressions contain a call? 
    bool                                    methodCallOnAttribute;         // Does method have any method call on attribute? 
    bool                                    functionCallOnAttribute;       // Does method have any function call on attribute?
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
    int                                     unitNumber;                    // Number of unit for xpath
};

#endif
