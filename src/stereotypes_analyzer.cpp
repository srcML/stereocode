// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotypes_analyzer.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#include "stereotypes_analyzer.hpp"
#include "stereotype_rules.hpp"
#include "primitives.hpp"
#include "calls.hpp"
#include "specifiers.hpp"
#include "xpath_generator.hpp"
#include "helper_functions.hpp"

#include <fstream>
#include <srcml.h>
#include <thread>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <cstring>


XPathGenerator                     XPATH_GENERATOR;

extern std::unordered_map
       <int, std::unordered_map
       <std::string, std::string>> XPATH_LIST;  

extern primitives                  PRIMITIVES;
extern calls                       CALLS;
extern specifiers                  SPECIFIERS;

extern bool                        IS_VERBOSE;
extern bool                        FREE_FUNCTION;
extern bool                        CSV_REPORT;

srcml_archive*                     archive{srcml_archive_create()};
srcml_unit*                        unit{nullptr};
srcml_archive*                     typeArchive{nullptr};
srcml_unit*                        typeUnit{nullptr};
srcml_archive*                     methodArchive{nullptr};
srcml_unit*                        methodUnit{nullptr};


stereotypesAnalyzer::stereotypesAnalyzer(const std::string& inputFile, const std::string& outputFile) {
    bool error{false};

    // Open input archive
    if (srcml_archive_read_open_filename(archive, inputFile.c_str())) {
        std::cerr << "Error: File not found: " << inputFile << '\n';
        error = true;
    }

    // Cloning is needed if input archive has extra namespaces (e.g., pos)
    srcml_archive* outputArchive = srcml_archive_clone(archive);

    // Open output archive
    if (srcml_archive_write_open_filename(outputArchive, outputFile.c_str())) {
        std::cerr << "Error opening: " << outputFile << std::endl;
        error = true;
    }

    // Error handling
    if (error) {
        srcml_archive_close(archive);
        srcml_archive_close(outputArchive);
        srcml_archive_free(archive);
        srcml_archive_free(outputArchive);
        exit(1);
    }

    // Prepare XPaths, primitives, ignored calls, and specifiers
    XPATH_GENERATOR.generateXPathList(); 
    PRIMITIVES.initializePrimitivesList();
    CALLS.initializeIgnorableCalls();
    SPECIFIERS.initializeSpecifiersList();

    // Verbose output
    if (IS_VERBOSE) {
        PRIMITIVES.outputPrimitives();
        CALLS.outputIgnorableCalls();
        SPECIFIERS.outputSpecifiers();
    }

    // Perform data extraction
    unit = srcml_archive_read_unit(archive);

    int unitNumber = 1;
    while (unit) {
        findTypeInfo(unit, unitNumber);
        findFreeFunctions(unit, unitNumber);

        srcml_unit_free(unit);
        ++unitNumber;
        unit = srcml_archive_read_unit(archive);
    }
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    // Appends duplicate types
    analyzeDuplicates();
    
    // Performed after the collection of all types and free functions
    analyzeFreeFunctions();

    // Build signatures for findInheritedDataMembers()
    for (auto& pair : types) pair.second.buildMethodSignature();
    
    // Finds inherited data members
    for (auto& pair : types) {
        findInheritedDataMembers(pair.second);
        pair.second.setInherited(true);
        for (auto& pairS : types) pairS.second.setVisited(false);
    } 

    // Analyze all methods for each type
    for (auto& pair : types) pair.second.findDataAfterCollection();
    
    // Compute method and type stereotypes
    stereotypeRules stereotypes;
    stereotypes.computeMethodStereotypes (types);
    stereotypes.computeTypeStereotypes   (types);

    // Analyze and compute stereotypes for free functions
    if (FREE_FUNCTION) {
        for (auto& f : freeFunctions) f.findDataFreeFunctionAfterCollection();
        stereotypes.computeFreeFunctionStereotypes(freeFunctions);
    }
    
    // Optional TXT report file
    std::string InputFileNoExt = inputFile.substr(0, inputFile.size() - 4);

    // Optional CSV report file
    if (CSV_REPORT) {
        std::string header = "type_name,type_stereotype,method_name,method_stereotype";
        std::ofstream out;
        out.open(InputFileNoExt + ".stereotypes.csv");
        out << header << '\n';
        for (auto& pair : types) outputStereotypesAsCSV(out, &pair.second, false);        
        out.close();

        header = "type_name,type_stereotype,free_function_name,free_function_stereotype";
        out.open(InputFileNoExt + ".free_functions_stereotypes.csv");
        out << header << '\n';
        outputStereotypesAsCSV(out, nullptr, true);        
        out.close();
    }

    // Verbose output
    if (IS_VERBOSE) {
        std::string header = "file_name,type_name,type_stereotype,type_parents,type_inherited_parents,function_signatures,"
                             "inherited_function_signatures,function_name,function_stereotype,function_parameters_list,"
                             "function_return_type,function_specifiers,function_internal_calls,function_attributes_annotations,"
                             "field_used,fields_modified,calls_on_fields";
        std::ofstream out;
        out.open(InputFileNoExt + ".stereotypes_verbose.csv");
        out << header << '\n';
        for (auto& pair : types) outputStereotypesAsCSV(out, &pair.second, false);        
        out.close();

        out.open(InputFileNoExt + ".free_functions_stereotypes_verbose.csv");
        out << header << '\n';
        outputStereotypesAsCSV(out, nullptr, true);        
        out.close();
    }

    // Generate the stereotyped XML archive
    archive = srcml_archive_create();
    srcml_archive_read_open_filename(archive, inputFile.c_str());
    unit = srcml_archive_read_unit(archive);

    std::vector<srcml_unit*> inputUnits;
    std::map<int, srcml_unit*> outputUnits;
    std::unordered_map<int, srcml_transform_result*> resultUnits;
    
    std::vector<std::thread> threads;
    std::mutex mu;

    unsigned int nthreads = std::thread::hardware_concurrency();

    unitNumber = 1;
    while(unit) {
        unsigned int threadPoolCount = 0;
        while ((threadPoolCount < nthreads) && unit) {
            inputUnits.push_back(unit);

            threads.emplace_back(std::thread(&stereotypesAnalyzer::outputWorker, this, unit, unitNumber, std::ref(outputUnits), std::ref(XPATH_LIST[unitNumber]), std::ref(resultUnits), std::ref(mu)));
            ++unitNumber;
            ++threadPoolCount;

            unit = srcml_archive_read_unit(archive);
        }
        for (std::thread& thread : threads) if (thread.joinable()) thread.join();
        threads.clear();

        // Registering the stereotype namespace for unit
        for (const auto& pair : outputUnits) {
            srcml_unit_register_namespace(pair.second, "st", "http://www.srcML.org/srcML/stereotype");
            srcml_archive_write_unit(outputArchive, pair.second);
        }
    
        // Clean
        for (const auto& pair : resultUnits) srcml_transform_free(pair.second);
        for (const auto& inputUnit : inputUnits) srcml_unit_free(inputUnit); 
        inputUnits.clear();
        outputUnits.clear();
        resultUnits.clear();
    }
    srcml_archive_close(archive);
    srcml_archive_free(archive);
    srcml_archive_close(outputArchive);
    srcml_archive_free(outputArchive);
}


// Finds types in an archive
//
// In C++, type names are usually in the form of:
//      myType  
// or  
//      myType<type1, type2, ...> for a specialized templated type from myType
//
// In C# and Java:
//      myType
// or
//      myType<T, G, ... > for a generic type
//
// Unlike C++, C# and Java can allow multiple types with the same name but 
//  different number of generic parameters to exist
// For example, foo<T> and foo<T, T1> are valid
//
// C++:
//  Unions can be declared anonymous (union without a name) 
//  Anonymous unions can only be declared inside a type, struct, or a namespace and cannot have methods 
//   and their data members are accessed directly as part of the enclosing scope
//   so, data members that are defined inside anonymous unions are considered and the anonymous union itself is not considered
//   also, anonymous unions that are defined in a namespace are not considered because their data members are basically globals
//  Types can be declared without a name (anonymous types) require you to simultaneously create an instance of it during its definition
//   so, these are treated as normal types
// C#:
//  Allows static types to be declared, and these must contain only static data members
//   They are ignored and their methods are collected as free functions
//  Nested classes, struct, or interfaces are ignored
// Java:
//  enums in Java can contain methods and data members
//  Static classes are allowed, but these can only be nested within other classes, interfaces, or enums
//   Static classes in java can contain non-static data members or methods
//   They are ignored (since they are nested) and their methods (only if static) are collected as free functions
//  Anonymous classes (classes without names and are nested as instances) are ignored
//
void stereotypesAnalyzer::findTypeInfo(srcml_unit* inputUnit, int unitNumber) {
    std::string unitLanguage = srcml_unit_get_language(inputUnit);
    if (unitLanguage == "C") { unitLanguage = "C++"; }
    if (unitLanguage == "C++" || unitLanguage == "C#" || unitLanguage == "Java") {
        srcml_append_transform_xpath(archive, XPATH_GENERATOR.getXPathList(unitLanguage, "type").c_str()); 
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, inputUnit, &result);
        int n = srcml_transform_get_unit_size(result);
        srcml_unit* resultUnit = nullptr;
        
        for (int i = 0; i < n; i++) {
            resultUnit = srcml_transform_get_unit(result, i);

            typeArchive = srcml_archive_clone(archive); // Creates an archive internally
            
            char* unparsed = nullptr;
            std::size_t size = 0;
            srcml_archive_write_open_memory(typeArchive, &unparsed, &size);
            srcml_archive_write_unit(typeArchive, resultUnit);
            srcml_archive_close(typeArchive);
            srcml_archive_free(typeArchive);

            typeArchive = srcml_archive_create();
            srcml_archive_read_open_memory(typeArchive, unparsed, size);
            typeUnit = srcml_archive_read_unit(typeArchive);

            std::string typeXpath = "(" + XPATH_GENERATOR.getXPathList(unitLanguage, "type") + ")[" + std::to_string(i + 1) + "]"; 

            typeModel type(unitLanguage); 
            type.findData(typeXpath, unitNumber);   
            const std::string& typeNameParsed = type.getName()[1];

            // Needed for inheritance in Java and C#
            if (unitLanguage != "C++") genericTypes.insert({type.getName()[2], type.getName()[1]}); 

            if (types.find(typeNameParsed) != types.end()) 
                duplicateTypes.push_back(std::move(type));
            else      
                types.insert({typeNameParsed, std::move(type)});
            
            free(unparsed);
            srcml_unit_free(typeUnit);
            srcml_archive_close(typeArchive);
            srcml_archive_free(typeArchive); 
         }   
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
    }    
}

// C++ only
//
// Finds free functions as well as methods defined externally
//
// Cases for functions and methods (defined externally)
//   Cases:
//      Method (Foo) could belong to a specialized templated type: 
//          template<typename T> type myType{}; // Generic templated type.
//          template<optional> type myType<int>{}; // Specialized templated type from the generic one.
//          void myType<int>::Foo(){} // Belongs to the specialized templated type. 
//      Method could belong to the generic templated type
//          template<typename T> type myType{}; // Generic templated type.
//          template<typename T> void myType<T>::Foo(){} // Generic templated method belongs to generic templated type.        
//          template<optional>   void myType<int>::Foo(){} // Specialized templated method belongs to a generic templated type.
//           If a specialized templated type is defined for <int> then the previous method won't be allowed. It must be replaced with:
//           void myType<int>::Foo(){} 
//      Method could be contained in a namespace
//          namespace::myType<int>::Foo(){}
//      Method could belong to a normal type
//          myType::Foo(){}
//      Function could be a free function (including normal free functions, friend functions, static methods, methods defined for external types)
//          Foo(){}, namespace::Foo(){}, static Foo(){}, externalType::Foo(){}, 
//
void stereotypesAnalyzer::findFreeFunctions(srcml_unit* inputUnit, int unitNumber) {
    std::string unitLanguage = srcml_unit_get_language(inputUnit); 
    if (unitLanguage == "C") { unitLanguage = "C++"; }
    if (unitLanguage == "C++" || unitLanguage == "C#" || unitLanguage == "Java") {
        srcml_append_transform_xpath(archive, XPATH_GENERATOR.getXPathList(unitLanguage,"free_function").c_str());
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, inputUnit, &result);
        int n = srcml_transform_get_unit_size(result);  
        srcml_unit* resultUnit = nullptr;
    
        for (int i = 0; i < n; i++) {
            resultUnit = srcml_transform_get_unit(result, i);

            methodArchive = srcml_archive_clone(archive);
            
            char* unparsed = nullptr;
            std::size_t size = 0;
            srcml_archive_write_open_memory(methodArchive, &unparsed, &size);
            srcml_archive_write_unit(methodArchive, resultUnit);
            srcml_archive_close(methodArchive);
            srcml_archive_free(methodArchive);
            
            methodArchive = srcml_archive_create();
            srcml_archive_read_open_memory(methodArchive, unparsed, size);
            methodUnit = srcml_archive_read_unit(methodArchive);

            std::string functionXpath =  "(" + XPATH_GENERATOR.getXPathList(unitLanguage, "free_function") + ")[" + std::to_string(i + 1) + "]";
            functionModel function(functionXpath, unitLanguage, "", "", unitNumber, false);
            
            freeFunctions.push_back(std::move(function));

            free(unparsed);
            srcml_unit_free(methodUnit);
            srcml_archive_close(methodArchive);
            srcml_archive_free(methodArchive); 
        }
        srcml_transform_free(result);
        srcml_clear_transforms(archive); 
    }
}

// Analyzes free functions to determine externally defined methods
//
void stereotypesAnalyzer::analyzeFreeFunctions() {
    for (std::vector<functionModel>::iterator function = freeFunctions.begin(); function != freeFunctions.end();) {
        if (function->getUnitLanguage() == "C++") {
            // Removes namespaces if any
            std::string functionName = function->getName();  
            helperFunctions::removeNamespace(functionName, "C++", false);

            // Get the type name (if any). Else, it is a free function
            std::size_t isTypeName = functionName.find("::");
            if (isTypeName != std::string::npos) { // Type found, it is a method       
                std::string typeName = functionName.substr(0, isTypeName); 
                auto result = types.find(typeName);
                if (result != types.end()) {
                    result->second.addMethod(*function);
                    function = freeFunctions.erase(function);
                }                          
                else { // Case specialized template method belongs to the generic template type
                    typeName = typeName.substr(0, typeName.find("<"));
                    result = types.find(typeName);
                    if (result != types.end())  {
                        result->second.addMethod(*function);
                        function = freeFunctions.erase(function);
                    } 
                    else ++function;                                                  
                }
            }
            else ++function;
        }
        else ++function;
    }
}

// Appends duplicates types (e.g., partial classes in C#)
// 
void stereotypesAnalyzer::analyzeDuplicates() {
    for (auto& duplicateType : duplicateTypes) { 
        std::string key = duplicateType.getName()[1];
        auto it = types.find(key);
        if (it != types.end()) it->second.mergeData(duplicateType);
        else types.insert({key, std::move(duplicateType)});
    }
}

// Finds inherited fields and methods
// In C++, you can inherit from a specialized templated type or
//  you can specialize the inheritance itself from the generic type, or
//  you can inherit from the generic type itself.
// For example:
//  myType --> childType : myType<T> or childType : myType<int>
//  specializedType<int> --> childType : specializedType<int>
//
// In Java and C#, you can inherit from the generic type or specialize the inheritance.
// For example:
//  myType<T1, T2> --> childType : myType<T1, T2> or childType : myType<int, double>
//
void stereotypesAnalyzer::findInheritedDataMembers(typeModel& type) {
    type.setVisited(true); 
    // A copy here since parentNames will append the inherited ones
    std::unordered_set<std::string> parentTypeNames =  type.getParentNames();

    for (std::string parentTypeName : parentTypeNames){
        auto result = types.find(parentTypeName);
        if (result != types.end()) {
            // Checking for ( isVisited ) is needed since even if ( isInherited ) is true, we might reach
            //  this type multiple times from ( type ), and we do not want to append it multiple times 
            if (result->second.isInherited() && !result->second.isVisited()) {
                type.appendInheritedDataMembers(result->second.getFields(), result->second.getMethodSignatures(), result->second.getParentNames(),
                                                result->second.getInheritedFields(), result->second.getInheritedMethodSignatures(), result->second.getInheritedParentNames()); 
                result->second.setVisited(true);
            }
                
            else if (!result->second.isVisited()) {
                findInheritedDataMembers(result->second);
                type.appendInheritedDataMembers(result->second.getFields(), result->second.getMethodSignatures(), result->second.getParentNames(),
                                                result->second.getInheritedFields(), result->second.getInheritedMethodSignatures(), result->second.getInheritedParentNames()); 
            }
        }       
        else {
            if (type.getUnitLanguage() == "C++") {
                parentTypeName = parentTypeName.substr(0, parentTypeName.find("<"));
                result = types.find(parentTypeName);
                if (result != types.end()) {
                    if (result->second.isInherited() && !result->second.isVisited()) {
                        type.appendInheritedDataMembers(result->second.getFields(), result->second.getMethodSignatures(), result->second.getParentNames(),
                                                        result->second.getInheritedFields(), result->second.getInheritedMethodSignatures(), result->second.getInheritedParentNames()); 
                        result->second.setVisited(true);
                    }
                        
                    else if (!result->second.isVisited()) {
                        findInheritedDataMembers(result->second);
                        type.appendInheritedDataMembers(result->second.getFields(), result->second.getMethodSignatures(), result->second.getParentNames(),
                                                        result->second.getInheritedFields(), result->second.getInheritedMethodSignatures(), result->second.getInheritedParentNames()); 
                    }
                }              
            }
            else {  
                helperFunctions::removeBetweenComma(parentTypeName, true);
                auto resultG = genericTypes.find(parentTypeName);
                if (resultG != genericTypes.end()) {
                    auto resultM = types.find(resultG->second);
                    if (resultM != types.end()) {
                        if (resultM->second.isInherited() && !resultM->second.isVisited()) {
                            type.appendInheritedDataMembers(resultM->second.getFields(), resultM->second.getMethodSignatures(), resultM->second.getParentNames(),
                                                            resultM->second.getInheritedFields(), resultM->second.getInheritedMethodSignatures(), resultM->second.getInheritedParentNames()); 
                            resultM->second.setVisited(true);
                        }
                        else if (!resultM->second.isVisited()) {
                            findInheritedDataMembers(resultM->second);
                            type.appendInheritedDataMembers(resultM->second.getFields(), resultM->second.getMethodSignatures(), resultM->second.getParentNames(),
                                                            resultM->second.getInheritedFields(), resultM->second.getInheritedMethodSignatures(), resultM->second.getInheritedParentNames()); 
                        }
                    }
                }
            }      
        }         
    }
}

// Outputs a CSV report file containing stereotype information and meta data
//  
void stereotypesAnalyzer::outputStereotypesAsCSV(std::ofstream& csvFile, typeModel* type, bool isFreeFunction) {
    const std::vector<functionModel>* functionsPtr = &type->getMethods();

    if (isFreeFunction) functionsPtr = &freeFunctions;
    
    if (IS_VERBOSE) {
        for (const functionModel& function : *functionsPtr) {
            std::string typeName = isFreeFunction ? "N/A" : (type->getName().size() > 0 ? type->getName()[0] : "N/A");
            std::string typeStereotype = isFreeFunction ? "N/A" : type->getStereotypesString();
            std::string typeParents = isFreeFunction ? "N/A" : (type->getParentsString().size() > 0 ? type->getParentsString() : "N/A");
            std::string typeInheritedParents = isFreeFunction ? "N/A" : (type->getInheritedParentsString().size() > 0 ? type->getInheritedParentsString() : "N/A");
            std::string signatures = isFreeFunction ?  (function.getNameSignature().empty() ? "N/A" :  function.getNameSignature()) : (type->getMethodSignatures().size() > 0 ? type->getMethodSignaturesString() : "N/A");
            std::string inheritedSignatures = isFreeFunction ? "N/A" : (type->getInheritedMethodSignatures().size() > 0 ? type->getInheritedMethodSignaturesString() : "N/A");
            std::string functionAttributesOrAnnotations = function.getUnitLanguage() == "C++" ? "N/A" : (function.getAttributesOrAnnotations().size() > 0 ? function.getAttributesOrAnnotationsString() : "N/A");
            std::string specifiers = function.getSpecifiers().size() > 0 ? function.getSpecifiersString() : "N/A";
            std::string returnType = function.getReturnType().getType().empty() ? "N/A" : function.getReturnType().getType();
            std::string parametersList = function.getParametersList().empty() ? "N/A" : function.getParametersList();
            std::string internalCalls = isFreeFunction ? "N/A" : (function.getFunctionCalls().empty() ? "N/A" : function.getFunctionCallsString());
            std::string isFieldUsed = function.isFieldUsed() ? "True" :  "False";
            std::string isFieldModified = function.getFieldsModifiedCount() > 0 ? "True" :  "False";
            std::string isCallOnField = function.getMethodCalls().size() > 0 ? "True" :  "False";
            std::string functionName = function.getName().empty() ? "N/A" : function.getName();

            csvFile << helperFunctions::escapeCSV(function.getFileName()) << ","
                    << helperFunctions::escapeCSV(typeName) << ","
                    << helperFunctions::escapeCSV(typeStereotype) << ","
                    << helperFunctions::escapeCSV(typeParents) << ","
                    << helperFunctions::escapeCSV(typeInheritedParents) << ","
                    << helperFunctions::escapeCSV(signatures) << ","
                    << helperFunctions::escapeCSV(inheritedSignatures) << ","
                    << helperFunctions::escapeCSV(functionName) << ","
                    << helperFunctions::escapeCSV(function.getStereotypesString()) << ","
                    << helperFunctions::escapeCSV(parametersList) << ","
                    << helperFunctions::escapeCSV(returnType) << ","
                    << helperFunctions::escapeCSV(specifiers) << ","
                    << helperFunctions::escapeCSV(internalCalls) << ","
                    << helperFunctions::escapeCSV(functionAttributesOrAnnotations) << ","
                    << helperFunctions::escapeCSV(isFieldUsed) << ","
                    << helperFunctions::escapeCSV(isFieldModified) << ","
                    << helperFunctions::escapeCSV(isCallOnField) << "\n";
        }
    }
    else {
            for (const functionModel& function : *functionsPtr) {
            std::string typeName = isFreeFunction ? "N/A" : (type->getName().size() > 0 ? type->getName()[0] : "N/A");
            std::string typeStereotype = isFreeFunction ? "N/A" : type->getStereotypesString();

            csvFile << helperFunctions::escapeCSV(typeName) << ","
                    << helperFunctions::escapeCSV(typeStereotype) << ","
                    << helperFunctions::escapeCSV(function.getName()) << ","
                    << helperFunctions::escapeCSV(function.getStereotypesString()) << "\n";
        }
    }
}

// Thread worker
//
void stereotypesAnalyzer::outputWorker(srcml_unit* inputUnit, int unitNumber, std::map<int, srcml_unit*>& outputUnits,
                                       const std::unordered_map<std::string, std::string>& xpathPair,
                                       std::unordered_map<int, srcml_transform_result*>& resultUnits, std::mutex& mu) {
    outputStereotypes(inputUnit, unitNumber, outputUnits, xpathPair, resultUnits, mu);
}

//  Add in stereotypes on <type> and <function>
//  Example: <function st:stereotype="get"> ... </function>
//           <type st:stereotype="boundary"> ... ></type>
//
void stereotypesAnalyzer::outputStereotypes(srcml_unit* inputUnit, int unitNumber, std::map<int, srcml_unit*>& outputUnits,
                                            const std::unordered_map<std::string, std::string>& xpathPair,
                                            std::unordered_map<int, srcml_transform_result*>& resultUnits, std::mutex& mu) {  
    // If there are no stereotypes to apply, then exit
    if (xpathPair.empty()) {
        std::lock_guard<std::mutex> guard(mu);
        outputUnits.insert({unitNumber, inputUnit});
        return;
    }
    srcml_archive* transformationArchive{srcml_archive_create()};

    for (const auto& pair : xpathPair) srcml_append_transform_xpath_attribute(transformationArchive, pair.first.c_str(), "st", "http://www.srcML.org/srcML/stereotype", "stereotype", pair.second.c_str());

    srcml_transform_result* resultUnit = nullptr; 
    srcml_unit_apply_transforms(transformationArchive, inputUnit, &resultUnit);

    // Note: ( outputUnit ) is owned by ( resultUnit ), so we must store ( resultUnit ) later.
    srcml_unit* outputUnit = srcml_transform_get_unit(resultUnit, 0);  
    std::lock_guard<std::mutex> guard(mu);
    outputUnits.insert({unitNumber, outputUnit});
    resultUnits.insert({unitNumber, resultUnit});   

    srcml_clear_transforms(transformationArchive);
    srcml_archive_free(transformationArchive);
}
