// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_classifier.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "rules/stereotype_classifier.hpp"
#include "rules/return_types.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <string_view>
#include <utility>

namespace {
    // Names of the functions that start a program: main of C, C++, and Java, Main of C#, and the entry points of Windows
    constexpr std::string_view entryPointNames[] = {"main", "Main", "wmain", "WinMain", "wWinMain", "_tmain", "_tWinMain"};

    // Rounds after the first in which classifyMethods() classifies the callers of the methods with new stereotypes again
    // A chain of calls needs a round per call, far fewer than the limit, which ends the alternating stereotypes of a cycle of calls
    constexpr int methodRoundLimit = 100;

    // Tells whether a method has one of the stereotypes of a group
    //
    bool hasAny(const stereotypeList& labels, std::initializer_list<stereotype> group) {
        return std::any_of(labels.begin(), labels.end(), [group](stereotype s) {
            return std::find(group.begin(), group.end(), s) != group.end();
        });
    }

    // Tells whether a method has the stereotype of an accessor: get, predicate, property, or void-accessor
    //
    bool isAccessor(const stereotypeList& labels) {
        return hasAny(labels, {stereotype::get, stereotype::predicate, stereotype::property, stereotype::voidAccessor});
    }

    // Tells whether a call to a method with these stereotypes leaves the object as it is: the method is an accessor and no mutator,
    // or a degenerate method, which neither reads nor changes the object
    //
    bool keepsObject(const stereotypeList& labels) {
        if (hasAny(labels, {stereotype::set, stereotype::command, stereotype::nonVoidCommand})) return false;
        return isAccessor(labels) || hasAny(labels, {stereotype::incidental, stereotype::stateless, stereotype::empty});
    }

    // What a call does to the object or the field that it calls a method on, given the stereotypes of the methods that the call may reach
    //
    struct callEffect {
        bool    reads{false};       // A method that the call may reach is an accessor
        bool    changes{false};     // A method that the call may reach is no accessor or degenerate method, or no method with stereotypes fits
    };

    // Gets what a call does, where a write through the result of an accessor, as in at(i) = x, changes the object
    //
    callEffect effectOf(const methodCall& call, const std::vector<stereotypeList>& labels) {
        callEffect effect;
        effect.changes = call.callees.empty();
        for (std::size_t callee : call.callees) {
            effect.reads   = effect.reads || isAccessor(labels[callee]);
            effect.changes = effect.changes || !keepsObject(labels[callee]);
        }
        effect.changes = effect.changes || (effect.reads && call.resultWritten);
        return effect;
    }

    // Gets the facts of a method in which each call counts by what it does: a call to an accessor of the type reads a field,
    // a call that only reaches degenerate methods counts like a call of a free function, and only a call that may change the object stays a call
    // A call on a field reads the field already, as its name does
    //
    methodFacts withCallEffects(methodFacts f, const std::vector<stereotypeList>& labels) {
        for (std::vector<methodCall>* calls : {&f.typeMethodCalls, &f.callsOnFields}) {
            std::vector<methodCall> changing;
            for (methodCall& call : *calls) {
                const callEffect effect = effectOf(call, labels);
                if (effect.reads && calls == &f.typeMethodCalls) f.fieldUsed = true;
                if (effect.changes)    changing.push_back(std::move(call));
                else if (!effect.reads) ++f.callsOnFreeFunctions;
            }
            *calls = std::move(changing);
        }
        return f;
    }
}

// Classifies a method from its facts and returns its stereotypes in output order
// The calls to methods of the type and on fields in the facts are the calls that may change the object, which classifyMethods() keeps
// after the first round, while a call to an accessor of the type reads a field and a call to a degenerate method counts like a call of a free function
//
stereotypeList classifyMethod(const methodFacts& f) {
    // constructor copy-constructor destructor
    //
    if      (f.constructorOrDestructor == "destructor"      ) return {stereotype::destructor};
    else if (f.constructorOrDestructor == "copy-constructor") return {stereotype::copyConstructor};
    else if (!f.constructorOrDestructor.empty()             ) return {stereotype::constructor};

    // empty
    //
    // 1] Method has no statements except for comments
    // Any return type, const or not (C++)
    //
    if (f.nonCommentStatements == 0) return {stereotype::empty};

    stereotypeList labels;
    bool isVoidPointer      = isVoidPointerType(f.language, f.declaredReturnType);
    int  callsOnTypeMethods = static_cast<int>(f.typeMethodCalls.size());
    int  callsOnFields      = static_cast<int>(f.callsOnFields.size());

    // The conditions of set and command, which tell whether the method changes the state of the object
    // A write of the whole object, as *this = other, changes more than one field
    bool noFieldChanged  = f.fieldsModified == 0 && !f.objectReplaced;
    bool setsOneField    = callsOnTypeMethods == 0 && f.fieldsModified == 1 && callsOnFields == 0 && !f.objectReplaced;
    bool case1           = f.fieldsModified > 1 || f.objectReplaced;
    bool case2           = f.fieldsModified == 1 && (callsOnTypeMethods + callsOnFields) > 0;
    bool case3           = f.fieldsModified == 0 && (callsOnFields > 0 || callsOnTypeMethods > 0);
    bool isMutable       = f.isConst && case1;  // A const method changes only mutable fields (C++)
    bool commands        = (case1 || case2 || case3) && (!f.isConst || isMutable);
    bool isNonVoidReturn = isNonVoidType(f.language, f.returnType, f.declaredReturnType);

    // get
    //
    // 1] Contains at least one simple return expression that
    //     returns a field (e.g., return a;) or the value to a field (e.g., return *a; or return **a; ... etc)
    //     The field ( a ) can be of any data type (e.g., primitive, non-primitive, pointer, reference, etc)
    // 2] Or returns the object itself (e.g., return this; or return *this;) and changes no state, which a set or a command does
    // Any return type, const or not (C++)
    //
    if (f.simpleReturn || (f.returnsObject && !setsOneField && !commands)) labels.push_back(stereotype::get);

    // predicate
    //
    // 1] Return type is Boolean (e.g., bool, or Boolean in C# and Java)
    // 2] Contains at least one complex return expression (e.g., return a+5;)
    // 3] Uses a field in an expression or calls an accessor of the type
    // Const or not (C++)
    //
    // Returning "this" by itself is not a complex return (e.g., return this;)
    //
    if (isBooleanType(f.language, f.returnType) && f.complexReturn && f.fieldUsed)
        labels.push_back(stereotype::predicate);

    // property
    //
    // 1] Return type is known and is not void or Boolean, where a pointer to void (void*) counts as a value
    // 2] Contains at least one complex return statement (e.g., return a+5;)
    // 3] Uses a field in an expression or calls an accessor of the type
    // Const or not (C++)
    //
    // Returning "this" by itself is not a complex return (e.g., return this;)
    //
    if (isNonVoidType(f.language, f.returnType, f.declaredReturnType) && !isBooleanType(f.language, f.returnType) &&
        !f.returnType.empty() && f.complexReturn && f.fieldUsed)
        labels.push_back(stereotype::property);

    // void-accessor
    //
    // 1] Return type is void (also Void in C# and Java), where a pointer to void (void*) is not void
    // 2] Contains at least one parameter that is passed by non-const reference and is assigned a value
    // 3] Uses a field in an expression or calls an accessor of the type
    // Const or not (C++)
    //
    // The "this" keyword by itself is considered (e.g., p = this or p = *this)
    //  as an accessor to the state of the object where 'p' is passed by reference
    //
    if (f.parameterRefModified && !isNonVoidType(f.language, f.returnType, f.declaredReturnType) && f.fieldUsed)
        labels.push_back(stereotype::voidAccessor);

    // set
    //
    // 1] Only one field is changed and there are no calls on fields
    // 2] No calls to methods in type
    // 3] The whole object is not written (e.g., *this = other;)
    // Any return type, const or not (C++), so a const method that changes a mutable field is a set
    //
    // The "this" keyword by itself is considered (e.g., this["index"] = value; for indexers in C# or this./->dataMember = value)
    //
    if (setsOneField)
        labels.push_back(stereotype::set);

    // command
    //
    // Method has a void return type (also Void in C# and Java) or returns the object itself (e.g., return *this;)
    // Method is not const or const but has mutable fields (C++ only), which only case 1 accepts
    // Cases:
    //   Case 1: More than one field is modified, or the whole object is written (e.g., *this = other; or this = other; in a C# struct)
    //
    //   Case 2: one field is modified and
    //            there is at least one call on a field or
    //            at least one function call to other methods (except constructor calls) in type
    //   Case 3: zero fields are modified and
    //            there is at least one call on a field or
    //            at least one function call to other methods (except constructor calls) in type
    //
    // The "this" keyword by itself is considered (e.g., this["index"] = value; for indexers in C# or this./->dataMember = value)
    //
    // non-void-command
    //   Method return type is not void, where Boolean and a pointer to void (void*) count as values, and the method does not return the object itself
    //
    if (commands)
        labels.push_back(isNonVoidReturn && !f.returnsObject ? stereotype::nonVoidCommand : stereotype::command);

    // factory
    //
    // 1] Factories must include a non-primitive type in their return type
    //      and their return expression must be a local variable, parameter, or field, that
    //      call a constructor call or has a return expression with a constructor call (e.g., new)
    // Const or not (C++)
    //
    if (f.nonPrimitiveReturnType && (f.newReturned || f.variableCreatedWithNewAndReturned))
        labels.push_back(stereotype::factory);

    // wrapper
    //
    // 1] No fields are modified, the whole object included (e.g., *this = other;)
    // 2] No calls to methods in type
    // 3] No calls on fields
    // 4] No calls to methods of other types (e.g., a.foo() where a is a parameter or a local)
    // 5] Has at least one free function call
    // Constructor calls using the 'new' operator are not considered
    //
    // controller
    //
    // 1] No fields are modified, the whole object included (e.g., *this = other;)
    // 2] No calls to methods in type
    // 3] No calls on fields
    // 4] Has at least one call to other type methods or mutates a parameter or a local that is non-primitive
    // A wrapper is not a controller
    //
    // collaborator
    //
    // 1] It must use at least 1 non-primitive type (not of this type) or return a pointer to void (void*)
    // 2] Type could be a parameter, local variable, return type, or a field
    // A wrapper or a controller is not a collaborator
    //
    // The three rules read no const (C++), and only collaborator reads the return type
    //
    if (noFieldChanged && callsOnTypeMethods == 0 && callsOnFields == 0 &&
        f.callsToOtherTypeMethods == 0 && f.callsOnFreeFunctions > 0)
        labels.push_back(stereotype::wrapper);

    else if (noFieldChanged && callsOnTypeMethods == 0 && callsOnFields == 0 &&
             (f.callsToOtherTypeMethods > 0 || f.nonPrimitiveLocalOrParameterModified))
        labels.push_back(stereotype::controller);

    else if (f.nonPrimitiveFieldExternal || f.nonPrimitiveLocalExternal ||
             f.nonPrimitiveParameterExternal || f.nonPrimitiveReturnTypeExternal || isVoidPointer)
        labels.push_back(stereotype::collaborator);

    // incidental
    //
    // 1] Method contains at least one non-comment statement (i.e., method is not empty)
    // 2] No fields are used or modified (including no use of keyword "this" by itself)
    // 3] No calls of any kind
    // Any return type, const or not (C++)
    //
    bool noCalls = callsOnTypeMethods == 0 && callsOnFields == 0 && f.newConstructorCalls == 0 &&
                   f.callsToOtherTypeMethods == 0 && f.callsOnFreeFunctions == 0;

    if (!f.fieldUsed && noCalls)
        labels.push_back(stereotype::incidental);

    // stateless
    //
    // 1] Method contains at least one non-comment statement (i.e., method is not empty)
    // 2] No fields are used or modified (including no use of keyword "this" by itself)
    // 3] No calls to methods in type
    // 4] No calls on fields
    // 5] Has at least one call to other type methods (including constructor calls) or to a free function
    // Any return type, const or not (C++)
    //
    if (!f.fieldUsed && callsOnTypeMethods == 0 && callsOnFields == 0 &&
        (f.callsOnFreeFunctions > 0 || f.callsToOtherTypeMethods > 0 || f.newConstructorCalls > 0))
        labels.push_back(stereotype::stateless);

    // unclassified
    //
    // No stereotype found
    //
    if (labels.empty()) labels.push_back(stereotype::unclassified);
    return labels;
}

// Classifies the methods of the run in their order, where each call counts by the stereotypes of the methods that it may reach
// Each round classifies again the callers of the methods whose stereotypes the round before changed, from the stereotypes of that round,
// so the result does not depend on the order of the methods
//
std::vector<stereotypeList> classifyMethods(const std::vector<methodFacts>& methods) {
    std::vector<stereotypeList> labels;
    for (const methodFacts& f : methods) labels.push_back(classifyMethod(f));

    std::vector<std::vector<std::size_t>> callers(methods.size());
    std::vector<std::size_t> pending;
    for (std::size_t i = 0; i < methods.size(); ++i) {
        for (const std::vector<methodCall>* calls : {&methods[i].typeMethodCalls, &methods[i].callsOnFields})
            for (const methodCall& call : *calls)
                for (std::size_t callee : call.callees) callers[callee].push_back(i);
        if (!methods[i].typeMethodCalls.empty() || !methods[i].callsOnFields.empty()) pending.push_back(i);
    }

    // A const method that changes a mutable field turns into a set once its calls count as reads, so the stereotypes of a cycle of calls may alternate
    std::vector<bool> queued(methods.size(), false);
    for (int round = 0; round < methodRoundLimit && !pending.empty(); ++round) {
        std::vector<std::pair<std::size_t, stereotypeList>> changed;
        for (std::size_t i : pending) {
            stereotypeList next = classifyMethod(withCallEffects(methods[i], labels));
            if (next != labels[i]) changed.emplace_back(i, std::move(next));
        }

        pending.clear();
        for (auto& [method, next] : changed) {
            labels[method] = std::move(next);
            for (std::size_t caller : callers[method]) {
                if (queued[caller]) continue;
                queued[caller] = true;
                pending.push_back(caller);
            }
        }
        for (std::size_t caller : pending) queued[caller] = false;
    }
    return labels;
}

// Counts the methods of a type by group of stereotypes, given the stereotypes of each method other than constructors and destructors
//
typeCounts countTypeStereotypes(const std::vector<stereotypeList>& methods) {
    typeCounts c;
    c.methods = static_cast<int>(methods.size());
    for (const stereotypeList& labels : methods) {
        auto inGroup = [&labels](std::initializer_list<stereotype> group) { return hasAny(labels, group) ? 1 : 0; };
        int collaborational = inGroup({stereotype::controller, stereotype::collaborator, stereotype::wrapper});

        c.getters                 += inGroup({stereotype::get});
        c.accessors               += inGroup({stereotype::get, stereotype::predicate, stereotype::property, stereotype::voidAccessor});
        c.accessorsBesidesGet     += inGroup({stereotype::predicate, stereotype::property, stereotype::voidAccessor});
        c.setters                 += inGroup({stereotype::set});
        c.commands                += inGroup({stereotype::command, stereotype::nonVoidCommand});
        c.mutators                += inGroup({stereotype::set, stereotype::command, stereotype::nonVoidCommand});
        c.controllers             += inGroup({stereotype::controller});
        c.collaboratorsOrWrappers += inGroup({stereotype::collaborator, stereotype::wrapper});
        c.collaborators           += collaborational;
        c.factories               += inGroup({stereotype::factory});
        c.degenerates             += inGroup({stereotype::incidental, stereotype::stateless, stereotype::empty});
        c.getsOrSets              += inGroup({stereotype::get, stereotype::set});
        c.getsSetsOrCommands      += inGroup({stereotype::get, stereotype::set, stereotype::command, stereotype::nonVoidCommand});
        c.getsSetsOrDegenerates   += inGroup({stereotype::get, stereotype::set,
                                              stereotype::incidental, stereotype::stateless, stereotype::empty});
        if (collaborational == 0) ++c.nonCollaborators;
    }
    return c;
}

// Summarizes the numbers of methods of the types of a run, with the number of methods that a large type exceeds
//
methodCountSummary summarizeMethodCounts(const std::vector<int>& methodCounts) {
    methodCountSummary summary;
    summary.types = static_cast<int>(methodCounts.size());
    if (methodCounts.empty()) return summary;

    const long long types   = summary.types;
    long long       sum     = 0;
    long long       squares = 0;
    for (int count : methodCounts) {
        sum     += count;
        squares += static_cast<long long>(count) * count;
    }

    // The square of the number of types times the variance is a whole number, which keeps the threshold exact
    const long long spread = types * squares - sum * sum;
    long long       root   = static_cast<long long>(std::sqrt(static_cast<double>(spread)));
    while (root * root > spread) --root;
    while ((root + 1) * (root + 1) <= spread) ++root;

    summary.mean              = static_cast<double>(sum) / static_cast<double>(types);
    summary.standardDeviation = std::sqrt(static_cast<double>(spread)) / static_cast<double>(types);

    // A number of methods is whole, so it exceeds the mean plus the standard deviation when it exceeds their sum rounded down
    summary.largeTypeThreshold = static_cast<int>((sum + root) / types);
    return summary;
}

// Classifies a type from the numbers of its methods by group of stereotypes and returns its stereotypes in output order
//
stereotypeList classifyType(const typeCounts& c, int methodsPerTypeThreshold) {
    stereotypeList labels;

    // The entity rules need at least one method, so the ratio never compares zero with zero
    bool atMostTwoCollaboratorsPerOther = c.collaborators <= 2 * c.nonCollaborators;

    // Entity
    //
    // A method labeled get and property also counts as an accessor besides get
    if (c.accessorsBesidesGet != 0 && c.commands != 0 && atMostTwoCollaboratorsPerOther && c.controllers == 0)
        labels.push_back(stereotype::entity);

    // Minimal Entity
    //
    if (c.methods - c.getsSetsOrCommands == 0 && c.getters != 0 && c.setters != 0 && c.commands != 0 &&
        atMostTwoCollaboratorsPerOther)
        labels.push_back(stereotype::minimalEntity);

    // Data Provider
    //
    if (c.accessors > 2 * c.mutators && c.accessors > 2 * (c.controllers + c.factories))
        labels.push_back(stereotype::dataProvider);

    // Commander
    //
    if (c.mutators > 2 * c.accessors && c.mutators > 2 * (c.controllers + c.factories))
        labels.push_back(stereotype::commander);

    // Boundary
    //
    if (c.collaborators > c.nonCollaborators && 2 * c.factories < c.methods && 3 * c.controllers < c.methods)
        labels.push_back(stereotype::boundary);

    // Factory
    //
    if (3 * c.factories > 2 * c.methods)
        labels.push_back(stereotype::factory);

    // Controller
    //
    if (3 * (c.controllers + c.factories) > 2 * c.methods && (c.accessors != 0 || c.mutators != 0))
        labels.push_back(stereotype::controller);

    // Pure Controller
    //
    if (c.controllers + c.factories != 0 && c.accessors + c.mutators + c.collaboratorsOrWrappers == 0 && c.controllers != 0)
        labels.push_back(stereotype::pureController);

    // Large
    //
    auto inLargeRange = [&c](int part) { return c.methods < 5 * part && 3 * part < 2 * c.methods; };
    if (inLargeRange(c.accessors + c.mutators) && inLargeRange(c.controllers + c.factories) &&
        c.factories != 0 && c.controllers != 0 && c.accessors != 0 && c.mutators != 0 && c.methods > methodsPerTypeThreshold)
        labels.push_back(stereotype::large);

    // Lazy
    //
    if (c.getters + c.setters != 0 && 3 * c.degenerates > c.methods &&
        5 * (c.methods - c.getsSetsOrDegenerates) <= c.methods)
        labels.push_back(stereotype::lazy);

    // Degenerate
    //
    if (2 * c.degenerates > c.methods)
        labels.push_back(stereotype::degenerate);

    // Data
    //
    if (c.methods - c.getsOrSets == 0 && c.getters + c.setters != 0)
        labels.push_back(stereotype::data);

    // Small
    //
    if (0 < c.methods && c.methods < 3)
        labels.push_back(stereotype::small);

    // Empty (Considered degenerate)
    //
    if (c.methods == 0)
        labels.push_back(stereotype::empty);

    if (labels.empty()) labels.push_back(stereotype::unclassified);
    return labels;
}

// Classifies a free function from its facts and returns its stereotypes in output order
//
stereotypeList classifyFreeFunction(const freeFunctionFacts& f) {
    // main
    //
    // An entry point of a program (e.g., main, Main, wmain, WinMain, wWinMain, _tmain, or _tWinMain)
    //
    if (std::find(std::begin(entryPointNames), std::end(entryPointNames), f.name) != std::end(entryPointNames))
        return {stereotype::main};

    // empty
    //
    // Has no statements
    //
    if (f.nonCommentStatements == 0) return {stereotype::empty};

    stereotypeList labels;
    bool derivedFromParameters = f.parameterUsed && (f.parameterComplexReturn || f.parameterReturned);

    // predicate
    //
    // Returns a bool derived from the parameters
    //
    if (isBooleanType(f.language, f.returnType) && derivedFromParameters)
        labels.push_back(stereotype::predicate);

    // property
    //
    // Returns a non-bool derived from the parameters
    //
    // The test for an empty return type handles the case of a function that has no return type
    //
    if (isNonVoidType(f.language, f.returnType, f.declaredReturnType) && !isBooleanType(f.language, f.returnType) &&
        !f.returnType.empty() && derivedFromParameters)
        labels.push_back(stereotype::property);

    // global-command
    //
    // Modifies a global or a static variable
    //
    if (f.globalOrStaticVariableModified) labels.push_back(stereotype::globalCommand);

    // command
    //
    // Modifies a parameter passed by reference
    //
    if (f.parameterRefModified) labels.push_back(stereotype::command);

    // factory
    //
    // Creates and returns a 'new' locally created object
    // Constructor calls that are not using the 'new' operator are not considered
    //
    if (f.nonPrimitiveReturnType && (f.newReturned || f.variableCreatedWithNewAndReturned))
        labels.push_back(stereotype::factory);

    // literal
    //
    // Does not read or change parameters, globals, or statics
    //
    if (!f.parameterUsed && !f.globalUsed) labels.push_back(stereotype::literal);

    // wrapper
    //
    // Does not change parameters passed by reference. Has at least one call to other type methods or to a free function
    // Constructor calls using the 'new' operator are not considered
    //
    if (!f.parameterRefModified && f.calls > 0) labels.push_back(stereotype::wrapper);

    if (labels.empty()) labels.push_back(stereotype::unclassified);
    return labels;
}
