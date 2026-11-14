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

#include "model/argument_range.hpp"
#include "archive/element_path.hpp"
#include "model/global_variables.hpp"
#include "model/name_resolution.hpp"
#include "model/variable_model.hpp"
#include "model/call_model.hpp"
#include "model/written_type.hpp"

#include <optional>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct languageTables;
struct boundValue;
class srcmlDocument;
class elementOrder;

// A name or an expression in the body of a function, with its place among the scopes of the locals
//
struct bodyText {
    std::string     text;                   // Source code of the name or the expression
    int             position{0};            // Position of its element in the function, in document order
    bool            dereferenced{false};    // A unary * writes the target of the name, as in *p = 1
};

// A C++ shift operator after a variable, as << of out << value, which calls an operator function where the variable holds an object
//
struct shiftOperation {
    callModel               call;           // Call of the operator function on the variable, as out.operator<<
    std::vector<bodyText>   extracted;      // Operands that a chain of >> extracts into, which the call writes
};

// A local variable with the positions at which its name refers to it
//
struct scopedLocal {
    variableModel   variable;
    int             declared{0};            // Position of the declaration
    int             scopeEnd{0};            // Position after the block, statement, or lambda that holds the declaration
    bool            isStatic{false};        // A static local of C or C++ keeps its value between calls, as a global does
};

// A data member that a function of a dynamic language writes through the object, as total of self.total = 0
//
struct memberWrite {
    std::string     name;
    std::string     type;                   // Type of the annotation of the write or of the parameter that the write assigns, if any
    std::string     callName;               // Name of the call whose result the write assigns, which gives the type where the call creates an object
};

// What a return or a yield of a Python or JavaScript function gives the caller, which tells the return type of a function without an annotation
//
struct returnedValue {
    bool            none{false};            // The literal None, which is no value
    bool            boolean{false};         // An expression that is Boolean by its form
    std::string     callName;               // Name of the Python call that alone is the value, if any
    std::string     createdClass;           // Class of the object that a JavaScript new creates as the whole value, if any
};

// The names that the type of a method declares or inherits, which tell the state of an object from the other names in its methods
//
struct typeMembers {
    std::unordered_map<std::string, variableModel>  fields;                 // Data members of the type and of its known ancestors, this included
    std::unordered_set<std::string>                 staticFields;           // Static fields of the type and of its known ancestors, which are globals
    std::unordered_set<std::string>                 methodNames;            // Methods that the type and its known ancestors define or declare
    std::unordered_set<std::string>                 implicitAccessors;      // Components of a record whose accessors the record does not define (Java)
    argumentsByName                                 methodArguments;        // Numbers of arguments that these methods take
    argumentsByName                                 inheritedMethodArguments; // The same for the methods of the known ancestors, which a call through super or base names
    std::unordered_set<std::string>                 staticMethodNames;      // Static and class methods of the type and its known ancestors that the object reaches (Python)
    std::unordered_set<std::string>                 holderFunctions;        // Static methods and methods of the own types and the static holders, which a call without an object may name (C# and Java)
    std::unordered_set<std::string>                 nestedTypeNames;        // Classes in the bodies of the type and its known ancestors, which the object reaches as well (Python)
    std::unordered_set<std::string>                 ownTypes;               // The type and its parents, whose names qualify members of the object itself
    std::unordered_set<std::string>                 staticHolders;          // Types whose static fields a name without a qualifier reaches, as the own types do
    bool                                            unknownParent{false};   // An ancestor outside the archive may declare any other name
    bool                                            outsideStaticImport{false}; // A static import names a type outside the archive, which may hold a function of any name
};

// The names of the types of an archive in one language, and the names of its free functions in a language in which a call of a class
// creates an object, where a call of a name that is neither is a class or a function by the form of the name
//
struct callableNames {
    std::unordered_set<std::string> classes;
    std::unordered_set<std::string> functions;
};

// The names that tell whether a call of a function creates an object in a language in which a call of a class does: the classes and the free
// functions of the archive, its globals, and the methods and the variables that the type of the function has, static or not
//
struct creationNames {
    const callableNames&                    archive;
    const globalVariables&                  globals;
    const std::unordered_set<std::string>&  methods;
    const std::unordered_set<std::string>&  variables;
};

class functionModel {
public:
                                                        functionModel                          (const languageTables&, const srcmlDocument&, const std::optional<elementLocation>&, const std::string&, const std::string&, const std::string&, const std::string&, int, bool);

    const std::vector<std::string>&                     getStereotypes                         () const                { return stereotypes;                          }
    const std::vector<std::string>&                     getAttributesOrAnnotations             () const                { return attributesOrAnnotations;              }
    const std::vector<std::string>&                     getSpecifiers                          () const                { return specifiers;                           }
    const std::vector<callModel>&                       getFunctionCalls                       () const                { return functionCalls;                        }
    const std::vector<callModel>&                       getMethodCalls                         () const                { return methodCalls;                          }
    const std::vector<callModel>&                       getNewConstructorCalls                 () const                { return newConstructorCalls;                  }
    const std::vector<callModel>&                       getCallsOnResults                      () const                { return callsOnResults;                       }
    const std::vector<callModel>&                       getExternalCalls                       () const                { return externalCalls;                        }
    const variableModel&                                getReturnType                          () const                { return returnType;                           }
    const std::string&                                  getParametersList                      () const                { return parametersList;                       }
    const std::vector<writtenType>&                     getParameterTypes                      () const                { return parameterTypes;                       }
    const std::string&                                  getSignatureTypes                      () const                { return signatureTypes;                       }
    const argumentRange&                                getArguments                           () const                { return arguments;                            }
    const std::vector<std::string>&                     getName                                () const                { return name;                                 }
    const std::pair<std::string, std::string>&          getNameSignature                       () const                { return nameSignature;                        }
    const std::optional<elementLocation>&               getLocation                            () const                { return location;                             }
    const std::string&                                  getUnitLanguage                        () const                { return unitLanguage;                         }
    const std::string&                                  getConstructorOrDestructor             () const                { return constructorOrDestructor;              }
    const std::string&                                  getFileName                            () const                { return fileName;                             }
    const std::string&                                  getPropertyName                        () const                { return propertyName;                         }
    const std::unordered_set<std::string>&              getStaticHolders                       () const                { return staticHolders;                        }
    const std::vector<memberWrite>&                     getWrittenMembers                      () const                { return writtenMembers;                       }
    std::string                                         getStereotypesString                   () const;
    std::string                                         getSpecifiersString                    () const;
    std::string                                         getAttributesOrAnnotationsString       () const;
    std::string                                         getReturnTypeParsed                    () const;
    std::string                                         getCallsString                         (bool) const;
    int                                                 getFieldsModifiedCount                 () const                { return fieldsModifiedCount;             }
    int                                                 getLineNumber                          () const                { return lineNumber;                           }
    int                                                 getExternalFunctionCallsCount          () const                { return externalFunctionCallsCount;           }
    int                                                 getExternalMethodCallsCount            () const                { return externalMethodCallsCount;             }
    int                                                 getNonCommentStatementsCount           () const                { return nonCommentStatementsCount;            }
    bool                                                isMethodConst                          () const                { return methodConst;                          }
    bool                                                hasDeclaredReturnType                  () const                { return returnTypeDeclared;                   }
    bool                                                isFieldUsed                            () const                { return fieldUsed;                       }
    bool                                                isParameterUsed                        () const                { return parameterUsed;                        }
    bool                                                isGlobalUsed                           () const                { return globalUsed;                           }
    bool                                                isObjectReturned                       () const                { return objectReturned;                       }
    bool                                                isObjectReplaced                       () const                { return objectReplaced;                       }
    bool                                                hasSimpleReturn                        () const                { return simpleReturn;                         }
    bool                                                hasComplexReturn                       () const                { return complexReturn;                        }
    bool                                                hasParameterComplexReturn              () const                { return parameterComplexReturn;               }
    bool                                                isParameterReturned                    () const                { return parameterReturned;                    }
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
    bool                                                isInConstantBody                       () const                { return inConstantBody;                       }
    void                                                setInConstantBody                      (bool flag)             { inConstantBody = flag;                       }
    void                                                setStereotype                          (const std::string& s) { stereotypes.push_back(s);}  
    void                                                addStaticHolders                       (const std::unordered_set<std::string>& h) { staticHolders.insert(h.begin(), h.end()); }
    void                                                findDataAfterCollection                (const typeMembers&, const globalVariables&, const callableNames&);
    void                                                findDataFreeFunctionAfterCollection    (const globalVariables&, const std::unordered_set<std::string>&, const callableNames&);
    void                                                confirmReturnTypeMacros                (const std::unordered_set<std::string>&);
    void                                                findCreatedObjects                     (const creationNames&);
    std::optional<std::string>                          createdClass                           (const std::string&, const creationNames&) const;

private:
    std::string                                         withObjectNames                        (std::string) const;
    std::string                                         withOwnType                            (std::string) const;
    void                                                findBoundParameter                     (const srcmlDocument&);
    void                                                findPropertyName                       (const srcmlDocument&);
    void                                                inferReturnType                        ();
    void                                                noteBinding                            (const boundValue&, const std::string&);
    variableScopes                                      scopesAt                               (int, const std::unordered_map<std::string, variableModel>&, const globalVariables&) const;
    int                                                 unitNumber                             () const;
    const scopedLocal*                                  localAt                                (const std::string&, int, bool) const;
    bool                                                isStaticLocal                          (const nameResolution&, int) const;
    static bool                                         refersToObject                         (const nameResolution&);
    void                                                noteUse                                (const nameResolution&, int, const std::unordered_map<std::string, variableModel>&);
    void                                                noteClassHolders                       (const std::unordered_map<std::string, variableModel>&, const globalVariables&);
    void                                                noteReturnedVariable                   (const nameResolution&);
    bool                                                changesCallerData                      (const std::string&, const nameResolution&) const;
    void                                                findVariablesInExpressions             (const std::unordered_map<std::string, variableModel>&, const globalVariables&);
    void                                                findIgnorableCalls                     (std::vector<callModel>&);
    std::string                                         declaredTypeOf                         (const nameResolution&, int, const std::unordered_map<std::string, variableModel>&) const;
    void                                                findCalledVariables                    (const std::unordered_map<std::string, variableModel>&, const globalVariables&,
                                                                                                const std::unordered_set<std::string>&, const callableNames&);
    void                                                findStreamCalls                        (const std::unordered_map<std::string, variableModel>&, const globalVariables&, const callableNames&);
    void                                                findCallsOnFields                      (const typeMembers&, const globalVariables&);
    bool                                                callsParentOutsideArchive              (const callModel&, const typeMembers&, const globalVariables&) const;
    bool                                                readsThroughUnknownMethod              (const callModel&, const typeMembers&) const;
    void                                                findReturnedVariables                  (const std::unordered_map<std::string, variableModel>&, const globalVariables&, bool);
    void                                                findModifiedVariables                  (const std::unordered_map<std::string, variableModel>&, const typeMembers&, const globalVariables&, bool);
    void                                                findNonPrimitive                       ();
    void                                                findNonPrimitiveReturnType             ();
    std::string                                         returnTypeWithoutMacros                () const;
    void                                                findName                               (const srcmlDocument&);
    void                                                findReturnType                         (const srcmlDocument&, const std::string&);
    void                                                findParameterList                      (const srcmlDocument&);
    void                                                findLocalVariables                     (const srcmlDocument&, elementOrder&);
    void                                                findParameters                         (const srcmlDocument&, const std::string&);
    void                                                findReturnExpression                   (const srcmlDocument&, elementOrder&);
    void                                                findCalls                              (const srcmlDocument&, elementOrder&);
    void                                                findOperatorCalls                      (const srcmlDocument&, elementOrder&);
    void                                                findLocalFunctions                     (const srcmlDocument&);
    void                                                findNewAssignedVariables               (const srcmlDocument&);
    void                                                findConst                              (const srcmlDocument&);
    void                                                findConstructorOrDestructorType        (const srcmlDocument&);
    void                                                findExpressionNames                    (const srcmlDocument&, elementOrder&);
    void                                                findExpressionAssignments              (const srcmlDocument&, elementOrder&);
    void                                                findNonCommentStatements               (const srcmlDocument&);
    void                                                findSpecifiers                         (const srcmlDocument&);
    void                                                findAttributesOrAnnotations            (const srcmlDocument&); 

    const languageTables*                             tables{nullptr};                            // XPaths and lists, which findDataAfterCollection() and getReturnTypeParsed() also read
    std::vector<std::string>                          name;                                       // Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and in-between generic in ( <> ) | same as last but without ( <> )
    std::pair<std::string, std::string>               nameSignature;                              // Name without whitespaces, namespaces, and generics <> + the types of the parameters. For example, foo(int,double)
    variableModel                                     returnType;                                 // Return type
    std::vector<std::string>                          returnTypeMacros;                           // Names after the first of a C++ return type spelled as macros, such as SCI_METHOD, which leave the type unless a type of the archive has their name
    std::string                                       fileName;                                   // File name where method is defined 
    std::string                                       parametersList;                             // Parameter list
    std::vector<writtenType>                          parameterTypes;                             // Types of the parameters as written, such as const, Count, and & of const Count& c, which tell overloads apart
    std::string                                       signatureTypes;                             // Types of the parameters in the signature, without names and specifiers, such as (int,char*)
    argumentRange                                     arguments;                                  // Numbers of arguments that the parameter list takes
    std::string                                       unitLanguage;                               // Unit language
    std::string                                       propertyName;                               // Property or indexer that holds a C# accessor, such as Count or this(int), or property that a Python method defines
    std::string                                       objectName;                                 // First parameter of a Python method, which names the object as this does
    std::string                                       classParameter;                             // First parameter of a Python class method, which names the class
    std::optional<elementLocation>                    location;                                   // Element that receives the stereotypes in the srcML output, if any
    std::string                                       constructorOrDestructor;                    // Method is a constructor or a destructor
    std::unordered_map<std::string, variableModel>    parameters;                                 // Map of all parameters. Key is parameter name
    std::unordered_set<std::string>                   arrayParameters;                            // Parameters declared as arrays after their name, such as a of int a[]
    std::unordered_set<std::string>                   referenceParameters;                        // Parameters passed by reference with out or ref (C# only)
    std::unordered_set<std::string>                   sharedParameters;                           // Parameters whose object the caller shares, which calls on them may change
    std::unordered_set<std::string>                   sharedPointees;                             // C++ parameters of a std::shared_ptr or std::weak_ptr, whose object a call or a write through -> may change
    std::unordered_set<std::string>                   staticHolders;                              // Types whose static fields a name without a qualifier reaches: the types around the function, the type of a method and its known ancestors, and the types of static imports
    std::unordered_multimap<std::string, scopedLocal> locals;                                     // All locals with their scopes. Key is local name
    std::string                                       typeNameParsed;                             // Type name without whitespaces, namespaces, and generics <>
    std::string                                       ownTypeName;                                // Type name with the namespaces and the types around it where the model knows them
    std::vector<std::string>                          specifiers;                                 // List of specifiers, each once, in source order
    std::unordered_set<std::string>                   variablesCreatedWithNew;                    // List of variables that are declared/initialized with the "new" operator
    std::vector<bodyText>                             expressionNames;                            // Names that the expressions of a method use
    std::vector<bodyText>                             expressionAssignments;                      // Names that the expressions of a method write
    std::vector<std::string>                          stereotypes;                                // Method stereotype(s)
    std::vector<std::string>                          attributesOrAnnotations;                    // List of attributes (C#) or annotations (Java) 
    std::vector<callModel>                            externalCalls;                              // External function calls + method calls + constructor calls
    std::vector<callModel>                            functionCalls;                              // List of function calls (e.g., foo() or bar::foo()) where ( foo ) is another method in the type ( bar )
    std::vector<callModel>                            methodCalls;                                // List of method calls (e.g., a.foo()) where ( a ) is a field in the type
    std::vector<callModel>                            newConstructorCalls;                        // List of constructor calls that uses the ( new ) operator whether the type or external types
    std::vector<callModel>                            callsOnResults;                             // List of calls on the result of a call or a cast (e.g., c() of a.b().c())
    std::vector<shiftOperation>                       shiftOperations;                            // C++ shift operators after a variable, which findStreamCalls() counts as calls where the variable holds an object
    std::unordered_set<std::string>                   localFunctions;                             // Local functions and locals that a lambda initializes, whose code is code of the function itself
    std::vector<bodyText>                             returnExpressions;                          // List of all return expressions in a method
    std::vector<returnedValue>                        returnedValues;                             // What each return expression of a Python or JavaScript function gives the caller
    std::vector<memberWrite>                          writtenMembers;                             // Data members that a function of a dynamic language writes through the object, which declares them
    std::vector<std::pair<std::string, std::string>>  assignedCalls;                              // Variables that a Python function binds to the result of a call, such as e or this.engine, each with the name of the call
    const typeMembers*                                ownType{nullptr};                           // Members of the type of the method, set while findDataAfterCollection() runs
    bool                                              dynamic{false};                             // The language declares no data members, so a name after the object is a data member unless it is a method
    bool                                              moduleGlobals{false};                       // A name without a qualifier reaches only the globals of the file of the function
    bool                                              returnTypeDeclared{false};                  // The code writes the return type, unlike a type that findCreatedObjects() infers
    bool                                              methodConst{false};                         // Is it a const method? (C++ only)
    bool                                              fieldUsed{false};                           // Does it use at least 1 field in an expression? 
    bool                                              parameterUsed{false};                       // Does it use at least 1 parameter in an expression?
    bool                                              globalUsed{false};                          // Does it use a global or a static, as a read, a write, or a call?
    bool                                              objectReturned{false};                      // Does a return expression return the object itself, as return this; or return *this; does?
    bool                                              objectReplaced{false};                      // Does it write the whole object, as this = value; or *this = value; does?
    bool                                              simpleReturn{false};                        // Does it contain at least 1 return expression that just returns a field?
    bool                                              complexReturn{false};                       // Does it contain at least 1 return expression that is not a simple return?
    bool                                              parameterComplexReturn{false};              // Does it contain at least 1 return expression that is not a simple return? (For parameters)
    bool                                              parameterReturned{false};                   // Does it contain at least 1 return expression that returns a parameter? (For free functions)
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
    bool                                              inConstantBody{false};                      // The body of an enum constant defines the method, which may override a method of the enum (Java)
    bool                                              unknownParent{false};                       // Its type has an ancestor outside the archive, which may declare any name that is no global of the unit of the function
    int                                               lineNumber{-1};                             // Line number of the method declaration (Requires --position)
    int                                               fieldsModifiedCount{0};                     // Number of modified fields
    int                                               externalFunctionCallsCount{0};              // Number of function calls (e.g., foo()) where ( foo ) is a free function or a static method
    int                                               externalMethodCallsCount{0};                // Number of method calls (e.g., a.foo()) where ( a ) is an external object that are filtered (removed)
    int                                               nonCommentStatementsCount{0};               // Number of non-comment statements 
};

#endif
