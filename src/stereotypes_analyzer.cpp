// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotypes_analyzer.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#include "stereotypes_analyzer.hpp"
#include "output/annotated_unit.hpp"
#include "output/csv_report.hpp"
#include "helper_functions.hpp"
#include "archive/input_archive.hpp"
#include "ordered_pool.hpp"
#include "archive/output_archive.hpp"
#include "model/qualified_name.hpp"
#include "archive/query_engine.hpp"
#include "archive/source_language.hpp"
#include "archive/srcml_handles.hpp"
#include "rules/stereotype_labels.hpp"
#include "output/txt_report.hpp"

#include <algorithm>
#include <future>
#include <iostream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {
    // Units per worker that wait for a worker, so that every worker finds a unit while few units wait in memory
    constexpr std::size_t queuedPerWorker = 2;
    // Annotated units per worker that wait for their turn in the order of the units, which bounds the memory of the output
    constexpr std::size_t writesPerWorker = 8;

    // Gets the language whose XPaths apply to a unit (C units use the C++ XPaths), or an empty string when none apply
    //
    std::string analysisLanguage(srcml_unit* unit) {
        // A unit without a language is not analyzed and is written without stereotypes
        const char* language = srcml_unit_get_language(unit);
        if (!language) return "";

        return analysisLanguageOf(language);
    }

    // Tells whether srcML parsed a unit as C
    //
    bool isCUnit(srcml_unit* unit) {
        const char* language = srcml_unit_get_language(unit);
        return language && std::string_view(language) == "C";
    }

    // Gets the srcML of a unit, which libsrcml may read from the archive at the first access, so the reading thread calls this function
    //
    const char* readSrcml(srcml_unit* unit, int unitNumber) {
        const char* srcml = srcml_unit_get_srcml(unit);
        if (!srcml) throw std::runtime_error("cannot read the srcML of unit " + std::to_string(unitNumber));
        return srcml;
    }

    // Gets the key of a C# partial type, which its parts share: its namespaces, the types around it, its name, and its numbers of generic parameters
    //
    std::string partialTypeKey(const typeModel& type) {
        std::string key;
        for (const std::string& part : type.getScope()) key += part + ".";
        return key + genericSkeleton(type.getName()[1]);
    }

    // Gets the path of a type, the namespaces or the package around it and then the parts of its name without generic arguments, outermost first,
    // which is the scope of the names that its members write, where its nested types and member aliases come first
    //
    std::vector<std::string> pathOf(const typeModel& type) {
        std::vector<std::string> path = type.getScope();
        for (const std::string& part : splitQualifiedName(type.getName()[1], type.getUnitLanguage())) path.push_back(withoutGenericArguments(part));
        return path;
    }

    // Gets the namespaces or the package and the types around a type, outermost first, in which the names of its parents are looked up
    //
    std::vector<std::string> lookupScope(const typeModel& type) {
        std::vector<std::string> scope = pathOf(type);
        scope.pop_back();
        return scope;
    }
}

// Prepares the XPaths and the lists of primitive types, ignorable calls, and specifiers
//
stereotypesAnalyzer::stereotypesAnalyzer(const options& settings_) : settings{settings_}, tables{settings_} {}

// Computes the stereotypes of the input archive and writes the requested CSV reports and srcML output archive
//
void stereotypesAnalyzer::run() {
    archivePtr input = openInputArchive(settings.inputFile);

    // Created before the analysis, so that an output that cannot be written stops the run at once
    std::optional<outputArchive> output;
    if (!settings.disableSrcmlOutput) output.emplace(settings.outputFile, input.get());
    std::optional<csvReports> reports;
    if (settings.csvReport || settings.verboseReport) reports.emplace(settings.outputFile, settings.freeFunctions, settings.verboseReport);
    std::optional<txtReports> textReports;
    if (settings.txtReport) textReports.emplace(settings.outputFile, settings.freeFunctions);

    // The check reads the whole input on a thread of its own, so that it delays no error before the analysis
    std::future<void> wellFormed = std::async(std::launch::async, checkWellFormed, settings.inputFile);

    // Verbose output
    if (settings.verboseReport) {
        tables.primitiveTypes.outputPrimitives();
        tables.ignorableCalls.outputIgnorableCalls();
        tables.typeSpecifiers.outputSpecifiers();
        tables.fieldQueries.outputQueryMethods();
    }

    int analyzedUnits = 0;
    try {
        analyzedUnits = findModels(input.get());
    }
    catch (...) {
        // Damage that the check finds explains a failed read better than the error of the read
        wellFormed.get();
        throw;
    }
    if (analyzedUnits == 0) std::cerr << "Warning: " << settings.inputFile << " holds no unit in C, C++, C#, Java, or Python\n";
    input.reset();
    wellFormed.get();
    indexTypes();
    confirmReturnTypeMacros();
    declareMethodMembers();

    // Performed after the collection of all types and free functions
    analyzeFreeFunctions();

    // Build signatures for findInheritedDataMembers()
    for (typeModel& type : types) type.buildMethodSignature();

    // Finds unknown parents and inherited data members, with the types of the archive that each type names as parents
    std::vector<inheritance> walk(types.size(), inheritance::pending);
    std::vector<std::vector<std::size_t>> parentTypes(types.size());
    for (std::size_t position = 0; position < types.size(); ++position) findInheritedDataMembers(position, walk, parentTypes);

    // An enclosing type comes before its inner classes, so it holds the data members of its own enclosing instances by then
    for (std::size_t position = 0; position < types.size(); ++position)
        if (enclosingTypes[position]) types[position].appendEnclosingDataMembers(types[*enclosingTypes[position]]);

    addGlobalStaticImports();
    addAncestorStaticHolders();
    findHolderFunctions();

    // Analyze all methods for each type
    findCreatedObjects();
    for (typeModel& type : types) type.findDataAfterCollection(globals, archiveNames[type.getUnitLanguage()]);
    
    // Compute method and type stereotypes, where the calls of each method reach the methods of its type, of its ancestors, and of the types of its fields
    stereotypeRules stereotypes(settings.largeClassThreshold);
    const calledMethods called(types, std::move(parentTypes), enclosingTypes,
                               [this](std::size_t type, const std::string& name, int unitNumber, bool throughPointer) {
                                   return findWrittenTypes(type, name, unitNumber, throughPointer);
                               }, staticMethods, extensions);
    stereotypes.computeMethodStereotypes(types, called);

    // Only the types that the run labels count toward the large-class threshold
    std::vector<typeModel*> labeledTypes;
    for (typeModel& type : types)
        if (isLabeled(type)) labeledTypes.push_back(&type);
    stereotypes.computeTypeStereotypes(labeledTypes);
    if (settings.verboseReport) stereotypes.outputLargeClassThreshold();

    // Analyze and compute stereotypes for free functions
    if (settings.freeFunctions) {
        for (auto& f : freeFunctions) f.findDataFreeFunctionAfterCollection(globals, valueTypes, archiveNames[f.getUnitLanguage()]);
        stereotypes.computeFreeFunctionStereotypes(freeFunctions);
    }

    if (reports) outputReport(*reports);
    if (textReports) outputReport(*textReports);
    if (output) {
        outputStereotypes(*output);
        output->close();
    }
    if (reports) reports->close();
    if (textReports) textReports->close();

    // Each output replaces its file only once every output is complete, and the reports go first, since -o may name the input
    if (reports) reports->publish();
    if (textReports) textReports->publish();
    if (output) output->publish();
}

// Finds the types and free functions of every unit on the worker threads, merges them in the order of the units, and gives the number of analyzed units
// The merged models are thus the same for any number of workers
//
int stereotypesAnalyzer::findModels(srcml_archive* input) {
    // A compiled XPath is not read only, so each worker compiles an engine of its own at its first job
    std::vector<std::optional<queryEngine>> engines(settings.threads);

    // Besides the engine of its worker, a job reads only its unit and the tables
    // The pool comes after the engines, so that its workers end before the engines
    const languageTables& sharedTables = tables;
    orderedPool<unitModels> pool(settings.threads);
    int unitNumber = 0;
    int analyzedUnits = 0;
    for (unitPtr unit(srcml_archive_read_unit(input)); unit; unit.reset(srcml_archive_read_unit(input))) {
        ++unitNumber;

        const std::string unitLanguage = analysisLanguage(unit.get());
        if (unitLanguage.empty()) continue;
        ++analyzedUnits;

        // The job owns the unit, which holds the srcML
        const char* srcml = readSrcml(unit.get(), unitNumber);
        const bool cUnit  = isCUnit(unit.get());
        // The models merge in the order of the units once ready, so a slow unit holds up the merge but not the workers
        while (pool.nextReady()) mergeModels(pool.next());
        pool.waitForRoom(queuedPerWorker * settings.threads);
        pool.submit([&engines, &sharedTables, srcmlOwner = std::move(unit), srcml, unitLanguage, cUnit, unitNumber](std::size_t worker) {
            std::optional<queryEngine>& engine = engines[worker];
            if (!engine) engine.emplace(sharedTables.queries);
            return findUnitModels(*engine, sharedTables, srcml, unitLanguage, cUnit, unitNumber);
        });
    }
    while (pool.pending() > 0) mergeModels(pool.next());
    return analyzedUnits;
}

// Adds the types, free functions, globals, and aliases of the next unit, where the parts of a C# partial type in one namespace form one type
//
void stereotypesAnalyzer::mergeModels(unitModels models) {
    std::vector<std::size_t> positions;
    for (std::size_t i = 0; i < models.types.size(); ++i) {
        typeModel& type = models.types[i];
        if (type.isPartial()) {
            const auto [part, added] = partialTypes.try_emplace(partialTypeKey(type), types.size());
            if (!added) {
                types[part->second].mergeData(type);
                positions.push_back(part->second);
                continue;
            }
        }
        const std::optional<std::size_t> enclosing = models.enclosingTypes[i];
        enclosingTypes.push_back(enclosing ? std::optional<std::size_t>(positions[*enclosing]) : std::nullopt);
        positions.push_back(types.size());
        types.push_back(std::move(type));
    }
    std::move(models.freeFunctions.begin(), models.freeFunctions.end(), std::back_inserter(freeFunctions));
    std::move(models.freeFunctionScopes.begin(), models.freeFunctionScopes.end(), std::back_inserter(freeFunctionScopes));
    std::move(models.freeFunctionDirectives.begin(), models.freeFunctionDirectives.end(), std::back_inserter(freeFunctionDirectives));
    globals.merge(models.globals);
    globalStaticImports.insert(models.globalStaticImports.begin(), models.globalStaticImports.end());
    for (const staticMethod& method : models.staticMethods) {
        staticMethods[method.holder][method.name].push_back(method.arguments);
        // A call on an object passes the object as the first argument
        const argumentRange& arguments = method.arguments;
        if (!method.extendedType.empty() && arguments.maximum > 0)
            extensions[method.holder.substr(0, method.holder.find(' ') + 1) + method.name].push_back(
                {method.extendedType, {std::max(arguments.minimum - 1, 0), arguments.maximum - 1, arguments.variadic}});
    }
    valueTypes.insert(models.valueTypes.begin(), models.valueTypes.end());
    cClassNames.insert(models.cClassNames.begin(), models.cClassNames.end());
    declaredTypeNames.merge(models.declaredTypeNames);
    for (indexedAlias& alias : models.aliases) typeNames.addAlias(std::move(alias));
    for (typeImport& import : models.typeImports) typeNames.addImport(std::move(import));
    if (!models.warning.empty()) std::cerr << "Warning: " << models.warning << '\n';
}

// Indexes the names of the types, whose positions in the index are their positions in types
//
void stereotypesAnalyzer::indexTypes() {
    for (const typeModel& type : types) {
        const std::vector<std::string> parts = splitQualifiedName(type.getName()[1], type.getUnitLanguage());

        indexedType entry;
        entry.language   = type.getUnitLanguage();
        entry.path       = pathOf(type);
        entry.arguments  = genericArguments(parts.back());
        entry.arity      = genericArity(parts.back());
        entry.unitNumber = type.getLocations().front().unitNumber;
        entry.fileName   = type.getPartFileNames().front();
        entry.position   = type.getLocations().front().path;
        typeNames.add(std::move(entry));
    }
}

// Keeps in the C++ return types the names spelled as macros that a type or an alias of the archive has, which only the names of every unit tell
//
void stereotypesAnalyzer::confirmReturnTypeMacros() {
    for (typeModel& type : types)
        for (functionModel& method : type.getMethods()) method.confirmReturnTypeMacros(declaredTypeNames);
    for (functionModel& function : freeFunctions) function.confirmReturnTypeMacros(declaredTypeNames);
}

// Finds the data members that the methods of each type of a dynamic language declare, some with the type of an object that a call creates,
// where the names of the classes and the free functions of the archive tell a class from a function in a language without new
// The parent types take over these data members, so the step comes before the inherited data members
//
void stereotypesAnalyzer::declareMethodMembers() {
    for (const typeModel& type : types) archiveNames[type.getUnitLanguage()].classes.insert(type.getName()[3]);
    for (const functionModel& function : freeFunctions)
        if (languageOf(function.getUnitLanguage()).callsCreate) archiveNames[function.getUnitLanguage()].functions.insert(function.getName()[3]);

    for (typeModel& type : types)
        if (languageOf(type.getUnitLanguage()).dynamic) type.declareMethodMembers(archiveNames[type.getUnitLanguage()], globals);
}

// Finds the calls that create an object in a language without new, once each type knows the methods that it inherits,
// since a call through the object names a method before it names a class
//
void stereotypesAnalyzer::findCreatedObjects() {
    for (typeModel& type : types)
        if (languageOf(type.getUnitLanguage()).callsCreate) type.findCreatedObjects(archiveNames[type.getUnitLanguage()], globals);

    // A free function has no type, and so no method and no variable of a type
    const std::unordered_set<std::string> none;
    for (functionModel& function : freeFunctions)
        if (languageOf(function.getUnitLanguage()).callsCreate) function.findCreatedObjects({archiveNames[function.getUnitLanguage()], globals, none, none});
}

// Finds the type of a C++ method defined outside its type, as the qualifier of its name, the namespaces around the definition,
// and the using directives in effect at the definition name it
// The definition of a static method that the type declares stays a free function
//
std::optional<std::size_t> stereotypesAnalyzer::findDefinitionType(const functionModel& function, const std::vector<std::string>& scope,
                                                                   const std::vector<usingDirective>& directives) const {
    std::vector<std::string> qualifier = splitQualifiedName(function.getName()[1], "C++");
    if (qualifier.size() < 2) return std::nullopt;
    const std::string methodName = withoutGenericArguments(qualifier.back());
    qualifier.pop_back();

    const std::optional<elementLocation>& location = function.getLocation();
    typeReference reference = referenceOf(qualifier, "C++", scope, location ? location->unitNumber : 0, function.getFileName());
    reference.directives = directives;
    if (location) reference.position = location->path;
    const std::optional<std::size_t> type = typeNames.find(reference);
    if (type && declaresStaticMethod(*type, methodName, function)) return std::nullopt;
    return type;
}

// Tells whether a C++ type declares a static method with the name and the parameter types of a definition outside the type,
// where the two signatures write each type alike or one writes an alias in place of the type that the alias stands for
// The files of a declaration and its definition may resolve the name of an alias apart, while a type written alike is the same type
// A name that the type gives only to static methods names one, whatever srcML makes of the parameters of the definition
//
bool stereotypesAnalyzer::declaresStaticMethod(std::size_t position, const std::string& methodName, const functionModel& definition) const {
    const typeModel& type = types[position];
    const auto [first, last] = type.getStaticMethodSignatures().equal_range(methodName);
    if (first == last) return false;
    if (!type.namesOwnMethod(methodName)) return true;

    const std::string written = typeListText(definition.getParameterTypes());
    const std::string defined = spelledTypes(definition.getParameterTypes(), type, definition.getLocation().value_or(elementLocation{}),
                                             definition.getFileName());
    return std::any_of(first, last, [&](const auto& declaration) {
        return typeListText(declaration.second) == written ||
               spelledTypes(declaration.second, type, type.getLocations().front(), type.getPartFileNames().front()) == defined;
    });
}

// Gets the text of the parameter types of a C++ signature at a place of a file, where each name of an alias gives way to the type
// that the alias stands for, as (constint&) of (const Count&) after typedef int Count;
// A name after the qualifier of a definition outside its type resolves in the scope of the type, as a name in the type does
//
std::string stereotypesAnalyzer::spelledTypes(std::vector<writtenType> parameterTypes, const typeModel& type, const elementLocation& place,
                                              const std::string& fileName) const {
    typeReference signature = referenceOf({}, "C++", pathOf(type), place.unitNumber, fileName);
    signature.position = place.path;
    for (writtenType& parameterType : parameterTypes) parameterType = typeNames.withoutAliases(parameterType, signature);
    return typeListText(parameterTypes);
}

// Finds the types of the archive that a type name written in the code of a type refers to equally well, such as the type of a field of the type,
// also through aliases, where a call through -> reaches the type that an alias of a smart pointer points to
// The name is written in the part of the type in a unit, which for a C# partial type has the using alias directives of its own file,
// and otherwise in the first part, as for the name of the type of an inherited field
//
std::vector<std::size_t> stereotypesAnalyzer::findWrittenTypes(std::size_t position, const std::string& typeName, int unitNumber, bool throughPointer) const {
    const typeModel& type = types[position];
    const std::vector<elementLocation>& locations = type.getLocations();
    const auto inUnit = std::find_if(locations.begin(), locations.end(), [unitNumber](const elementLocation& part) { return part.unitNumber == unitNumber; });
    const std::size_t part = inUnit == locations.end() ? 0 : static_cast<std::size_t>(inUnit - locations.begin());

    const std::vector<std::string> parts = splitQualifiedName(typeName, type.getUnitLanguage());
    typeReference reference = referenceOf(parts, type.getUnitLanguage(), pathOf(type), locations[part].unitNumber, type.getPartFileNames()[part]);
    reference.position = locations[part].path;
    return typeNames.findThroughAliases(reference, throughPointer);
}

// Moves the C++ methods, constructors, and destructors defined outside their types from the free functions to their types
// The definitions for a class that only the C markup of a header holds are unclassified, and the constructors and destructors
// of other types outside the archive get no stereotype
//
void stereotypesAnalyzer::analyzeFreeFunctions() {
    std::vector<functionModel> remaining;
    for (std::size_t i = 0; i < freeFunctions.size(); ++i) {
        functionModel& function = freeFunctions[i];
        if (function.getUnitLanguage() == "C++") {
            if (const std::optional<std::size_t> type = findDefinitionType(function, freeFunctionScopes[i], freeFunctionDirectives[i])) {
                types[*type].addMethod(std::move(function));
                continue;
            }
            if (isCClassMethod(function)) {
                function.setStereotype(std::string(stereotypeName(stereotype::unclassified)));
                cClassMethods.push_back(std::move(function));
                continue;
            }
        }
        if (function.getConstructorOrDestructor().empty()) remaining.push_back(std::move(function));
    }
    freeFunctions = std::move(remaining);
    freeFunctionScopes.clear();
    freeFunctionDirectives.clear();
}

// Lets each C# type and free function reach, without their names, the static fields of the types of the global static imports of any unit
//
void stereotypesAnalyzer::addGlobalStaticImports() {
    for (typeModel& type : types)
        if (type.getUnitLanguage() == "C#") type.addStaticImports(globalStaticImports);
    for (functionModel& function : freeFunctions)
        if (function.getUnitLanguage() == "C#") function.addStaticHolders(globalStaticImports);
}

// Lets each type and free function reach, without their names, the static fields of the ancestors of the types around it and of its static imports
// These types are known only by name, so each takes the ancestors of every type of its language with that name
//
void stereotypesAnalyzer::addAncestorStaticHolders() {
    std::unordered_map<std::string, std::unordered_set<std::string>> ancestors;
    for (const typeModel& type : types) {
        std::unordered_set<std::string>& names = ancestors[type.getUnitLanguage() + ' ' + type.getName()[3]];
        for (const std::vector<std::string>& parent : type.getParentNames())          names.insert(parent[3]);
        for (const std::vector<std::string>& parent : type.getInheritedParentNames()) names.insert(parent[3]);
    }
    auto ancestorsOf = [&ancestors](const std::string& language, const std::unordered_set<std::string>& holders) {
        std::unordered_set<std::string> reached;
        for (const std::string& holder : holders) {
            const auto found = ancestors.find(language + ' ' + holder);
            if (found != ancestors.end()) reached.insert(found->second.begin(), found->second.end());
        }
        return reached;
    };

    for (typeModel& type : types) type.addStaticHolders(ancestorsOf(type.getUnitLanguage(), type.getStaticHolders()));
    for (functionModel& function : freeFunctions) function.addStaticHolders(ancestorsOf(function.getUnitLanguage(), function.getStaticHolders()));
}

// Gives each type of a language without free functions the names that a call without an object may name besides its own methods, which are
// the static methods of the type and its ancestors, their methods that srcML misreads, and the methods, static or not, of its static holders,
// and tells whether one of its static imports names a type outside the archive
// The holders are known only by name, so each stands for every type of its language with that name
//
void stereotypesAnalyzer::findHolderFunctions() {
    std::unordered_map<std::string, std::unordered_set<std::string>> functions;
    for (const auto& [holder, methods] : staticMethods)
        for (const auto& method : methods) functions[holder].insert(method.first);
    for (const typeModel& type : types) {
        if (!languageOf(type.getUnitLanguage()).methodsOnly) continue;
        std::unordered_set<std::string>& names = functions[type.getUnitLanguage() + ' ' + type.getName()[3]];
        names.merge(type.methodNames());
        names.insert(type.getMisreadMethods().begin(), type.getMisreadMethods().end());
    }

    for (typeModel& type : types) {
        const std::string& language = type.getUnitLanguage();
        if (!languageOf(language).methodsOnly) continue;

        std::unordered_set<std::string> holders = type.getStaticHolders();
        holders.insert(type.getName()[3]);
        for (const auto* parents : {&type.getParentNames(), &type.getInheritedParentNames()})
            for (const std::vector<std::string>& parent : *parents) holders.insert(parent[3]);

        std::unordered_set<std::string> reached;
        for (const std::string& holder : holders) {
            const auto found = functions.find(language + ' ' + holder);
            if (found != functions.end()) reached.insert(found->second.begin(), found->second.end());
        }
        const std::unordered_set<std::string>& imports = type.getStaticImports();
        const bool outsideImport = std::any_of(imports.begin(), imports.end(), [&](const std::string& imported) {
            return functions.count(language + ' ' + imported) == 0;
        });
        type.setHolderFunctions(std::move(reached), outsideImport);
    }
}

// Tells whether the reports and the srcML output show the labels of a type, which for an interface needs -i and for an enum -e
//
bool stereotypesAnalyzer::isLabeled(const typeModel& type) const {
    return (settings.interfaces || type.getStructure() != "interface") && (settings.enums || type.getStructure() != "enum");
}

// Tells whether a C++ definition belongs to a class that only the C markup of a header holds
//
bool stereotypesAnalyzer::isCClassMethod(const functionModel& function) const {
    const std::vector<std::string> parts = splitQualifiedName(function.getName()[1], "C++");
    return parts.size() > 1 && cClassNames.count(withoutGenericArguments(parts[parts.size() - 2])) > 0;
}

// Finds the type that a parent of a type refers to, which is never the type itself
// Each part of a C# partial type resolves the parents that it names from its own file
//
std::optional<std::size_t> stereotypesAnalyzer::findParentType(std::size_t position, std::size_t parent) const {
    const typeModel& type = types[position];
    const std::size_t part = type.getParentParts()[parent];
    const std::vector<std::string> parts = splitQualifiedName(type.getParentNames()[parent][1], type.getUnitLanguage());
    const typeReference reference = referenceOf(parts, type.getUnitLanguage(), lookupScope(type), type.getLocations()[part].unitNumber, type.getPartFileNames()[part]);
    return typeNames.find(reference, position);
}

// Finds inherited fields and methods
// For C++, we do need to check which fields or methods are accessible in inheritance chain (e.g., using 'private' specifier for inheritance)
//   this is because the first child (call it X) that inherits from the parent type will have access to all the parent's data members, including the private ones,
//   and if 'private' specifier is used for inheritance, then everything inherited will simply become private in the child type, so the first child will still have access to all the parent's data members, 
//   but any other later child that inherits from X will not have access to any of the X's data members since they are all private in X
//   Therefore, we could just collect everything across the entire hierarchy and then during analysis, methods will only access data members that are not private in the parent type, 
//   so even if these private data members are collected, they will simply not be accessed in the child type or any of its children anyway.
//
// A depth-first walk completes every parent before its child takes the parent's members, so the result is the same in any order of the types
// A parent on the current path closes a cycle of names and gives the members it holds so far
//
void stereotypesAnalyzer::findInheritedDataMembers(std::size_t position, std::vector<inheritance>& walk, std::vector<std::vector<std::size_t>>& parentTypes) {
    if (walk[position] != inheritance::pending) return;
    walk[position] = inheritance::onPath;

    for (std::size_t parentNumber = 0; parentNumber < types[position].getParentNames().size(); ++parentNumber) {
        const std::optional<std::size_t> parent = findParentType(position, parentNumber);
        if (!parent) {
            types[position].setUnknownParent(true);
            continue;
        }
        findInheritedDataMembers(*parent, walk, parentTypes);
        types[position].appendInheritedDataMembers(types[*parent]);
        parentTypes[position].push_back(*parent);
    }
    walk[position] = inheritance::done;
}

// Writes the rows of the types and, with free functions, the rows of the free functions to the CSV or text reports
//
template <typename report>
void stereotypesAnalyzer::outputReport(report& reports) const {
    std::vector<const typeModel*> labeledTypes;
    for (const typeModel& type : types)
        if (isLabeled(type)) labeledTypes.push_back(&type);

    reports.writeTypes(labeledTypes);
    if (settings.freeFunctions) {
        reports.writeFreeFunctions(freeFunctions);
        reports.writeFreeFunctions(cClassMethods);
    }
}

// Writes every unit of the input archive, with the stereotype attributes of its labeled elements, to the output archive
// The workers annotate the units, and the units go to the output archive in the order of the input archive
//
void stereotypesAnalyzer::outputStereotypes(outputArchive& output) const {
    // Labels of the types, methods, and free functions by unit
    std::unordered_map<int, std::vector<stereotypeAnnotation>> annotations;
    auto addLabel = [&annotations](const elementLocation& location, const std::string& stereotypes) {
        annotations[location.unitNumber].push_back({location.path, stereotypes});
    };
    for (const typeModel& type : types) {
        if (!isLabeled(type)) continue;
        for (const elementLocation& location : type.getLocations()) addLabel(location, type.getStereotypesString());
        for (const functionModel& method : type.getMethods())
            if (method.getLocation()) addLabel(*method.getLocation(), method.getStereotypesString());
    }
    for (const functionModel& method : cClassMethods)
        if (method.getLocation()) addLabel(*method.getLocation(), method.getStereotypesString());
    if (settings.freeFunctions) {
        for (const functionModel& function : freeFunctions)
            if (function.getLocation()) addLabel(*function.getLocation(), function.getStereotypesString());
    }

    archivePtr input = openInputArchive(settings.inputFile);
    orderedPool<annotatedUnit> pool(settings.threads);
    int unitNumber = 0;
    for (unitPtr unit(srcml_archive_read_unit(input.get())); unit; unit.reset(srcml_archive_read_unit(input.get()))) {
        readSrcml(unit.get(), ++unitNumber);

        std::vector<stereotypeAnnotation> unitAnnotations;
        const auto labeled = annotations.find(unitNumber);
        if (labeled != annotations.end()) unitAnnotations = std::move(labeled->second);

        while (pool.nextReady() || pool.pending() >= writesPerWorker * settings.threads) output.write(pool.next());
        pool.waitForRoom(queuedPerWorker * settings.threads);
        pool.submit([inputUnit = std::move(unit), labels = std::move(unitAnnotations), comments = settings.comments](std::size_t) mutable {
            return annotatedUnit(std::move(inputUnit), labels, comments);
        });
    }
    while (pool.pending() > 0) output.write(pool.next());
}
