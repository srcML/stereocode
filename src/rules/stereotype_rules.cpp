// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_rules.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "rules/stereotype_rules.hpp"
#include "helper_functions.hpp"
#include "rules/stereotype_classifier.hpp"
#include "model/written_type.hpp"
#include "archive/source_language.hpp"

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>

// Takes the method threshold of -l for large types, or nothing to compute the threshold from the types of the run
//
stereotypeRules::stereotypeRules(std::optional<int> largeClassThreshold_) : largeClassThreshold{largeClassThreshold_} {}

namespace {
    // Gets the signature that tells a method of a type from the other methods, where a C# accessor takes the name of its property, such as Count.get,
    // or nothing for a function whose markup lacks a parameter list and for a method of a language without #if alternatives, where each
    // definition is a method of its own, as the methods that the bodies of the constants of a Java enum define under one name
    //
    std::optional<std::string> methodSignature(const functionModel& m) {
        if (!languageOf(m.getUnitLanguage()).preprocessor) return std::nullopt;
        if (!m.getPropertyName().empty()) return m.getPropertyName() + "." + m.getName()[1];
        if (m.getParametersList().empty()) return std::nullopt;
        return m.getName()[1] + typeListText(m.getParameterTypes()) + (m.isMethodConst() ? "const" : "");
    }

    // Reads the facts of a method of a type that the method stereotype rules use, with the methods of the run that its calls may reach
    //
    methodFacts readMethodFacts(const functionModel& m, std::size_t position, std::size_t type, const std::string& unitLanguage, const calledMethods& called) {
        methodFacts facts;
        facts.language                             = unitLanguage;
        facts.constructorOrDestructor              = m.getConstructorOrDestructor();
        facts.returnType                           = m.getReturnTypeParsed();
        facts.declaredReturnType                   = m.getReturnType().getType();
        HELPERS::removeWhitespace(facts.declaredReturnType);
        facts.fieldsModified                       = m.getFieldsModifiedCount();
        facts.callsOnFreeFunctions                 = m.getExternalFunctionCallsCount();
        for (const callModel& call : m.getMethodCalls()) {
            // The accessor of a record component reads the data member on which it is called, unless the code writes through its result
            if (!call.isResultWritten() && called.callsImplicitAccessor(type, call)) continue;
            std::vector<std::size_t> reached = called.ofCallOnField(type, position, call);
            // An extension method takes the data member as a static method does, as IsEmpty(basket) for basket.IsEmpty()
            if (reached.empty() && called.callsExtensionMethod(type, call)) {
                ++facts.callsOnFreeFunctions;
                continue;
            }
            facts.callsOnFields.push_back({std::move(reached), call.isResultWritten()});
        }
        for (const callModel& call : m.getFunctionCalls()) {
            if (called.callsStaticMethod(type, call)) ++facts.callsOnFreeFunctions;
            else facts.typeMethodCalls.push_back({called.ofCallToType(type, position, call), call.isResultWritten()});
        }
        facts.newConstructorCalls                  = static_cast<int>(m.getNewConstructorCalls().size());
        facts.callsToOtherTypeMethods              = m.getExternalMethodCallsCount();
        facts.nonCommentStatements                 = m.getNonCommentStatementsCount();
        facts.fieldUsed                            = m.isFieldUsed();
        facts.isConst                              = m.isMethodConst();
        facts.simpleReturn                         = m.hasSimpleReturn();
        facts.complexReturn                        = m.hasComplexReturn();
        facts.returnsObject                        = m.isObjectReturned();
        facts.objectReplaced                       = m.isObjectReplaced();
        facts.parameterRefModified                 = m.isParameterRefModified();
        facts.nonPrimitiveReturnType               = m.isNonPrimitiveReturnType();
        facts.newReturned                          = m.isNewReturned();
        facts.variableCreatedWithNewAndReturned    = m.isVariableCreatedAndReturnedWithNew();
        facts.nonPrimitiveLocalOrParameterModified = m.isNonPrimitiveLocalOrParameterModified();
        facts.nonPrimitiveFieldExternal            = m.isNonPrimitiveFieldExternal();
        facts.nonPrimitiveLocalExternal            = m.isNonPrimitiveLocalExternal();
        facts.nonPrimitiveParameterExternal        = m.isNonPrimitiveParamaterExternal();
        facts.nonPrimitiveReturnTypeExternal       = m.isNonPrimitiveReturnTypeExternal();
        return facts;
    }

    // Reads the stereotypes of the methods of a type, constructors and destructors excluded
    // A method defined in several #if or #else alternatives counts once, with the stereotypes of its first definition
    //
    std::vector<stereotypeList> readMethodStereotypes(const std::vector<functionModel>& methods) {
        std::vector<stereotypeList> stereotypes;
        std::unordered_set<std::string> signatures;
        for (const functionModel& m : methods) {
            if (!m.getConstructorOrDestructor().empty()) continue;

            const std::optional<std::string> signature = methodSignature(m);
            if (signature && !signatures.insert(*signature).second) continue;

            stereotypeList labels;
            for (const std::string& name : m.getStereotypes())
                if (std::optional<stereotype> label = stereotypeFromName(name)) labels.push_back(*label);
            stereotypes.push_back(labels);
        }
        return stereotypes;
    }

    // Tells whether a type is a class element, a Java record included, which no option adds to the run
    //
    bool isClassElement(const typeModel& type) {
        return type.getStructure() == "class" || type.getStructure() == "record";
    }

    // Reads the facts of a free function that the free-function stereotype rules use
    //
    freeFunctionFacts readFreeFunctionFacts(const functionModel& f) {
        freeFunctionFacts facts;
        facts.language                          = f.getUnitLanguage();
        facts.name                              = f.getName()[1];
        facts.returnType                        = f.getReturnTypeParsed();
        facts.declaredReturnType                = f.getReturnType().getType();
        HELPERS::removeWhitespace(facts.declaredReturnType);
        facts.calls                             = static_cast<int>(f.getMethodCalls().size() + f.getFunctionCalls().size() + f.getCallsOnResults().size());
        facts.nonCommentStatements              = f.getNonCommentStatementsCount();
        facts.parameterUsed                     = f.isParameterUsed();
        facts.globalUsed                        = f.isGlobalUsed();
        facts.parameterComplexReturn            = f.hasParameterComplexReturn();
        facts.parameterReturned                 = f.isParameterReturned();
        facts.parameterRefModified              = f.isParameterRefModified();
        facts.globalOrStaticVariableModified    = f.isGlobalOrStaticVariableModified();
        facts.nonPrimitiveReturnType            = f.isNonPrimitiveReturnType();
        facts.newReturned                       = f.isNewReturned();
        facts.variableCreatedWithNewAndReturned = f.isVariableCreatedAndReturnedWithNew();
        return facts;
    }
}

// Compute the method stereotypes of the run at once, since a call reads the stereotypes of the methods that it may reach
//
void stereotypeRules::computeMethodStereotypes(std::vector<typeModel>& typeCollection, const calledMethods& called) {
    std::vector<methodFacts> facts;
    for (std::size_t type = 0; type < typeCollection.size(); ++type) {
        for (const functionModel& m : typeCollection[type].getMethods())
            facts.push_back(readMethodFacts(m, facts.size(), type, typeCollection[type].getUnitLanguage(), called));
    }

    const std::vector<stereotypeList> labels = classifyMethods(facts);
    std::size_t position = 0;
    for (typeModel& type : typeCollection) {
        for (functionModel& m : type.getMethods()) {
            for (stereotype label : labels[position]) m.setStereotype(std::string(stereotypeName(label)));
            ++position;
        }
    }
}

// Compute the type stereotypes of the types that the run labels
// Without -l, the numbers of methods of the classes give the large-class threshold, so -t, -u, -i, and -e leave it unchanged,
// while a run without a class takes every labeled type
// Constructors and destructors are not considered in the computation of type stereotypes
//
void stereotypeRules::computeTypeStereotypes(const std::vector<typeModel*>& labeledTypes) {
    std::vector<typeCounts> counts;
    std::vector<int>        typeMethodCounts;
    std::vector<int>        classMethodCounts;
    for (const typeModel* type : labeledTypes) {
        counts.push_back(countTypeStereotypes(readMethodStereotypes(type->getMethods())));
        typeMethodCounts.push_back(counts.back().methods);
        if (isClassElement(*type)) classMethodCounts.push_back(counts.back().methods);
    }
    thresholdOverClasses = !classMethodCounts.empty();
    methodSummary = summarizeMethodCounts(thresholdOverClasses ? classMethodCounts : typeMethodCounts);

    const int threshold = largeClassThreshold.value_or(methodSummary.largeTypeThreshold);
    for (std::size_t i = 0; i < labeledTypes.size(); ++i) {
        for (stereotype label : classifyType(counts[i], threshold))
            labeledTypes[i]->setStereotype(typeStereotypeName(label, labeledTypes[i]->getStructure()));
    }
}

void stereotypeRules::computeFreeFunctionStereotypes(std::vector<functionModel>& freeFunctions) {
    for (functionModel& f : freeFunctions) {
        for (stereotype label : classifyFreeFunction(readFreeFunctionFacts(f))) {
            f.setStereotype(std::string(stereotypeName(label)));
        }
    }
}

// Prints the number of methods that a large type exceeds in the run, after computeTypeStereotypes()
//
void stereotypeRules::outputLargeClassThreshold() const {
    std::ostringstream description;
    if (largeClassThreshold) {
        description << "More than " << *largeClassThreshold << " methods, as given with -l";
    }
    else {
        description << std::fixed << std::setprecision(2) << "More than " << methodSummary.largeTypeThreshold
                    << " methods, the mean " << methodSummary.mean << " plus the standard deviation " << methodSummary.standardDeviation;
        if (thresholdOverClasses) description << " of the methods per class over the " << methodSummary.types << " classes of the run, rounded down";
        else                      description << " of the methods per type over the " << methodSummary.types << " labeled types of the run, which has no class, rounded down";
    }
    std::cerr << "---Large-Class Threshold---\n" << description.str() << "\n\n";
}
