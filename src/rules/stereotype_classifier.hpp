// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_classifier.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef STEREOTYPE_CLASSIFIER_HPP
#define STEREOTYPE_CLASSIFIER_HPP

#include "rules/stereotype_labels.hpp"

#include <cstddef>
#include <string>
#include <vector>

// A call of a method to the object itself or on a field, with the methods of the run that the call may reach
//
struct methodCall {
    std::vector<std::size_t> callees;                        // Positions of the methods among the methods of the run, none when no method with stereotypes fits
    bool                     resultWritten{false};           // The expression writes through the result of the call, as at(i) = x does
};

// Facts of a method that the method stereotype rules read
//
struct methodFacts {
    std::string language;                                    // Language of the analysis, where C units count as C++
    std::string constructorOrDestructor;                     // constructor, copy-constructor, or destructor. Empty for other methods
    std::string returnType;                                  // Return type without specifiers and whitespace, or a whole container
    std::string declaredReturnType;                          // Return type as declared, without whitespace
    int         fieldsModified{0};                           // Number of fields changed
    std::vector<methodCall> callsOnFields;                   // Calls on fields (e.g., a.foo() where a is a field), one per call
    std::vector<methodCall> typeMethodCalls;                 // Calls to the methods of the type, one per call
    int         newConstructorCalls{0};                      // Number of constructor calls with new
    int         callsOnFreeFunctions{0};                     // Number of calls to free functions
    int         callsToOtherTypeMethods{0};                  // Number of calls to methods of other types
    int         nonCommentStatements{0};                     // Number of statements other than comments
    bool        fieldUsed{false};                            // Does it use at least 1 field?
    bool        isConst{false};                              // Is it a const method? (C++ only)
    bool        simpleReturn{false};                         // Does a return expression just return a field?
    bool        complexReturn{false};                        // Does a return expression do more than return a field?
    bool        returnsObject{false};                        // Does a return expression return the object itself, as return this; and return *this; do?
    bool        objectReplaced{false};                       // Does it write the whole object, as this = value; and *this = value; do?
    bool        parameterRefModified{false};                 // Does it change a parameter passed by reference?
    bool        nonPrimitiveReturnType{false};               // Is the return type non-primitive?
    bool        newReturned{false};                          // Does a return expression create an object with new?
    bool        variableCreatedWithNewAndReturned{false};    // Does it return a variable created with new?
    bool        nonPrimitiveLocalOrParameterModified{false}; // Does it change a non-primitive local or parameter?
    bool        nonPrimitiveFieldExternal{false};            // Does it use a non-primitive field of another type?
    bool        nonPrimitiveLocalExternal{false};            // Does it use a non-primitive local of another type?
    bool        nonPrimitiveParameterExternal{false};        // Does it use a non-primitive parameter of another type?
    bool        nonPrimitiveReturnTypeExternal{false};       // Does it return a non-primitive type of another type?
};

// Numbers of methods of a type by group of stereotypes, constructors and destructors excluded
// A method counts once in each group that holds one of its stereotypes
//
struct typeCounts {
    int methods{0};                    // All methods
    int getters{0};                    // get
    int accessors{0};                  // get, predicate, property, or void-accessor
    int accessorsBesidesGet{0};        // predicate, property, or void-accessor
    int setters{0};                    // set
    int commands{0};                   // command or non-void-command
    int mutators{0};                   // set, command, or non-void-command
    int controllers{0};                // controller
    int collaboratorsOrWrappers{0};    // collaborator or wrapper
    int collaborators{0};              // controller, collaborator, or wrapper
    int nonCollaborators{0};           // None of controller, collaborator, and wrapper
    int factories{0};                  // factory
    int degenerates{0};                // incidental, stateless, or empty
    int getsOrSets{0};                 // get or set
    int getsSetsOrCommands{0};         // get, set, command, or non-void-command
    int getsSetsOrDegenerates{0};      // get, set, incidental, stateless, or empty
};

// Facts of a free function that the free-function stereotype rules read
//
struct freeFunctionFacts {
    std::string language;                                    // Language of the analysis, where C units count as C++
    std::string name;                                        // Name without whitespace
    std::string returnType;                                  // Return type without specifiers and whitespace, or a whole container
    std::string declaredReturnType;                          // Return type as declared, without whitespace
    int         calls{0};                                    // Number of calls, constructor calls with new excluded
    int         nonCommentStatements{0};                     // Number of statements other than comments
    bool        parameterUsed{false};                        // Does it use at least 1 parameter?
    bool        globalUsed{false};                           // Does it use a global or a static?
    bool        parameterComplexReturn{false};               // Does a return expression do more than return a parameter?
    bool        parameterReturned{false};                    // Does a return expression return a parameter itself?
    bool        parameterRefModified{false};                 // Does it change a parameter passed by reference?
    bool        globalOrStaticVariableModified{false};       // Does it change a global or a static variable?
    bool        nonPrimitiveReturnType{false};               // Is the return type non-primitive?
    bool        newReturned{false};                          // Does a return expression create an object with new?
    bool        variableCreatedWithNewAndReturned{false};    // Does it return a variable created with new?
};

// Numbers of methods of the types of a run, constructors and destructors excluded
//
struct methodCountSummary {
    int    types{0};                   // Number of types
    double mean{0};                    // Mean number of methods per type
    double standardDeviation{0};       // Population standard deviation of the number of methods per type
    int    largeTypeThreshold{0};      // Mean plus standard deviation, rounded down, which a large type exceeds
};

stereotypeList              classifyMethod        (const methodFacts&);
std::vector<stereotypeList> classifyMethods       (const std::vector<methodFacts>&);
typeCounts                  countTypeStereotypes  (const std::vector<stereotypeList>&);
methodCountSummary          summarizeMethodCounts (const std::vector<int>&);
stereotypeList              classifyType          (const typeCounts&, int);
stereotypeList              classifyFreeFunction  (const freeFunctionFacts&);

#endif
