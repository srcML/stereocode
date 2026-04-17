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
#include <algorithm>

XPathGenerator                     XPATH_GENERATOR;

extern std::unordered_map
       <int, std::unordered_map
       <std::string, std::string>> XPATH_LIST;  

extern primitives                  PRIMITIVES;
extern calls                       CALLS;
extern specifiers                  SPECIFIERS;

extern bool                        VERBOSE_REPORT;
extern bool                        FREE_FUNCTION;
extern bool                        CSV_REPORT;
extern bool                        DISABLE_SRCML_OUTPUT;

srcml_archive*                     archive{srcml_archive_create()};
srcml_unit*                        unit{nullptr};
srcml_archive*                     typeArchive{nullptr};
srcml_unit*                        typeUnit{nullptr};
srcml_archive*                     methodArchive{nullptr};
srcml_unit*                        methodUnit{nullptr};


stereotypesAnalyzer::stereotypesAnalyzer(const std::string& inputFile, const std::string& outputFile) {
    // Open input archive
    if (srcml_archive_read_open_filename(archive, inputFile.c_str())) {
        std::cerr << "Error: File not found: " << inputFile << '\n';
        srcml_archive_close(archive);
        srcml_archive_free(archive);
        exit(1);
    }

    // Cloning is needed if input archive has extra namespaces (e.g., pos)
    srcml_archive* outputArchive = srcml_archive_clone(archive);

    if (!DISABLE_SRCML_OUTPUT) {
        // Open output archive
        if (srcml_archive_write_open_filename(outputArchive, outputFile.c_str())) {
            std::cerr << "Error opening: " << outputFile << std::endl;
            srcml_archive_close(outputArchive);
            srcml_archive_free(outputArchive);
            exit(1);
        }
    }

    // Prepare XPaths, primitives, ignored calls, and specifiers
    XPATH_GENERATOR.generateXPathList(); 
    PRIMITIVES.initializePrimitivesList();
    CALLS.initializeIgnorableCalls();
    SPECIFIERS.initializeSpecifiersList();

    // Verbose output
    if (VERBOSE_REPORT) {
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
    
    // Finds unknown parents and inherited data members
    for (auto& pair : types) {
        findInheritedDataMembers(pair.second);
        for (auto& pairS : types) pairS.second.setVisited(false);
    } 

    // Analyze all methods for each type
    for (auto& pair : types) {
        pair.second.findDataAfterCollection();
    }
    
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

    // Verbose or CSV output file
    if (VERBOSE_REPORT || CSV_REPORT) {
        std::string header = "type_name,type_stereotype,function_name,function_stereotype";
        if (VERBOSE_REPORT) {
            // header = "type_file_name,type_name,type_stereotype,type_parents,type_inherited_parents,type_function_signatures,"
            //          "type_inherited_function_signatures,function_name,function_stereotype,function_line_number,function_signature,function_unit_language,function_parameters_list,"
            //          "function_return_type,function_specifiers,function_internal_calls,function_internal_call_to_type_signatures,function_attributes_annotations,function_field_used,function_fields_modified,function_calls_on_fields,function_source_code";
            header = "type_file_name,type_name,type_parents,type_unknown_parents,type_function_signatures,type_inherited_function_signatures,function_name,function_stereotype,function_line_number,function_signature,function_unit_language,function_parameters_list,"
                     "function_return_type,function_specifiers,function_external_calls,function_internal_calls,function_attributes,function_field_used,function_fields_modified,function_calls_on_fields";

        }

        std::ofstream out;
        out.open(InputFileNoExt + ".stereotypes.csv");
        out << header << '\n';
        for (auto& pair : types) outputStereotypesAsCSV(out, &pair.second, false);        
        out.close();

        out.open(InputFileNoExt + ".free_functions.stereotypes.csv");
        out << header << '\n';
        outputStereotypesAsCSV(out, nullptr, true);        
        out.close();
    }

    if (!DISABLE_SRCML_OUTPUT) {
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
        
        const char* inputUnitSrcml = srcml_unit_get_srcml(inputUnit);
        std::string header(inputUnitSrcml, std::strchr(inputUnitSrcml, '>') + 1);

        for (int i = 0; i < n; i++) {
            resultUnit = srcml_transform_get_unit(result, i);
            std::string resultUnitSrcml = header + srcml_unit_get_srcml(resultUnit) + "</unit>";

            typeArchive = srcml_archive_create();
            srcml_archive_read_open_memory(typeArchive, resultUnitSrcml.c_str(), resultUnitSrcml.size());
            typeUnit = srcml_archive_read_unit(typeArchive);

            std::string typeXpath = "(" + XPATH_GENERATOR.getXPathList(unitLanguage, "type") + ")[" + std::to_string(i + 1) + "]"; 

            typeModel type(unitLanguage); 
            type.findData(typeXpath, header, unitNumber);   
            const std::string& typeNameParsed = type.getName()[1];

            // Needed for inheritance in Java and C#
            if (unitLanguage != "C++") {
                genericTypes.insert({type.getName()[2], type.getName()[1]}); 
            }

            // Checks for partial classes in C# and adds them to a separate list to be analyzed later. 
            // For non-partial classes, adds them to the main types list
            if (types.find(typeNameParsed) != types.end()) {
                duplicateTypes.push_back(std::move(type));
            } else {
                types.insert({typeNameParsed, std::move(type)});
            }

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
    
        const char* inputUnitSrcml = srcml_unit_get_srcml(inputUnit);
        std::string header(inputUnitSrcml, std::strchr(inputUnitSrcml, '>') + 1);

        for (int i = 0; i < n; i++) {
            resultUnit = srcml_transform_get_unit(result, i);
            std::string resultUnitSrcml = header + srcml_unit_get_srcml(resultUnit) + "</unit>";

            methodArchive = srcml_archive_create();
            srcml_archive_read_open_memory(methodArchive, resultUnitSrcml.c_str(), resultUnitSrcml.size());
            methodUnit = srcml_archive_read_unit(methodArchive);

            std::string functionXpath =  "(" + XPATH_GENERATOR.getXPathList(unitLanguage, "free_function") + ")[" + std::to_string(i + 1) + "]";
            functionModel function(functionXpath, unitLanguage, "", "", unitNumber, HELPERS::extractFirstLineNumber(srcml_unit_get_srcml(resultUnit)), false);
            if (!function.getName().empty()) {
                freeFunctions.push_back(std::move(function));
            }

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
    // std::remove_if shifts all "kept" items to the front and returns the new end iterator
    auto newEnd = std::remove_if(freeFunctions.begin(), freeFunctions.end(), [this](functionModel& function) {
        if (function.getUnitLanguage() == "C++") {
            // Removes namespaces if any
            std::string functionName = function.getName()[1];  
            HELPERS::removeNamespace(functionName, "C++", false);

            // Get the type name (if any). Else, it is a free function
            std::size_t isTypeName = functionName.find("::");
            if (isTypeName != std::string::npos) { // Type found, it is a method       
                std::string typeName = functionName.substr(0, isTypeName); 
                auto result = types.find(typeName);
                
                if (result != types.end()) {
                    result->second.addMethod(std::move(function));
                    return true; // Return true to mark this element for removal from the vector
                }                          
                else { // Case specialized template method belongs to the generic template type
                    typeName = typeName.substr(0, typeName.find("<"));
                    result = types.find(typeName);
                    if (result != types.end())  {
                        result->second.addMethod(std::move(function));
                        return true; // Mark for removal
                    } 
                }
            }
        }
        return false; // Keep this element in freeFunctions
    });

    // Erase all the elements marked for removal
    freeFunctions.erase(newEnd, freeFunctions.end());
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
// In srcML, the name does not have the generics <>. But the parent names can have them.
// For example:
//  myType | childType : myType<T> | childType : myType<int>
//  specializedType<int> | childType : specializedType<int>
//
// In Java and C#, you can inherit from the generic type or specialize the inheritance.
// In srcML, the name and the parent names have the generics <>
// For example:
//  myType<T1, T2> | childType : myType<T1, T2> | childType : myType<int, double>
//
// For C++, we do need to check which fields or methods are accessible in inheritance chain (e.g., using 'private' specifier for inheritance)
//   this is because the first child (call it X) that inherits from the parent type will have access to all the parent's data members, including the private ones,
//   and if 'private' specifier is used for inheritance, then everything inherited will simply become private in the child type, so the first child will still have access to all the parent's data members, 
//   but any other later child that inherits from X will not have access to any of the X's data members since they are all private in X
//   Therefore, we could just collect everything across the entire hierarchy and then during analysis, methods will only access data members that are not private in the parent type, 
//   so even if these private data members are collected, they will simply not be accessed in the child type or any of its children anyway.
// 
void stereotypesAnalyzer::findInheritedDataMembers(typeModel& type) {
    type.setVisited(true); 

    // Try to match the name as is
    for (auto parentNamesVector : type.getParentNames()){
        std::string parentTypeName = parentNamesVector[1];
        auto result = types.find(parentTypeName);

        // Try matching name as is without whitespaces and namespaces for all languages
        if (result != types.end()) {
            // Checking for ( isVisited ) is needed since even if ( isInherited ) is true, we might reach
            //  this type multiple times from ( type ), and we do not want to append it multiple times 
            if (result->second.isInherited() && !result->second.isVisited()) {
                type.appendInheritedDataMembers(result->second.getFields(), result->second.getInheritedFields(), result->second.getMethodSignatures(), result->second.getInheritedMethodSignatures(),
                                                result->second.getDeclMethodSignatures(), result->second.getInheritedDeclMethodSignatures()); 
                result->second.setVisited(true);
            }
                
            else if (!result->second.isVisited()) {
                findInheritedDataMembers(result->second);
                type.appendInheritedDataMembers(result->second.getFields(), result->second.getInheritedFields(), result->second.getMethodSignatures(), result->second.getInheritedMethodSignatures(),
                                                result->second.getDeclMethodSignatures(), result->second.getInheritedDeclMethodSignatures()); 
            }
        }       
        else {
             // Try matching name as is without whitespaces, namespaces, and generics
            if (type.getUnitLanguage() == "C++") {
                parentTypeName = parentNamesVector[3];
                result = types.find(parentTypeName);
                if (result != types.end()) {
                    if (result->second.isInherited() && !result->second.isVisited()) {
                        type.appendInheritedDataMembers(result->second.getFields(), result->second.getInheritedFields(), result->second.getMethodSignatures(), result->second.getInheritedMethodSignatures(),
                                                        result->second.getDeclMethodSignatures(), result->second.getInheritedDeclMethodSignatures()); 
                        result->second.setVisited(true);
                    }
                        
                    else if (!result->second.isVisited()) {
                        findInheritedDataMembers(result->second);
                        type.appendInheritedDataMembers(result->second.getFields(), result->second.getInheritedFields(), result->second.getMethodSignatures(), result->second.getInheritedMethodSignatures(), 
                                                        result->second.getDeclMethodSignatures(), result->second.getInheritedDeclMethodSignatures()); 
                    }
                }  
                else {
                    // One of the parents is not defined in the code, 
                    // so we mark the type as having unknown parents
                    if (!type.hasUnknownParent()) {
                        type.setUnknownParent(true);
                    }
                }
            }
            else {  
                parentTypeName = parentNamesVector[2];
                auto resultG = genericTypes.find(parentTypeName);
                 // Try matching name as is without whitespaces, namespaces, and anything between the generics <>
                if (resultG != genericTypes.end()) {
                    auto resultM = types.find(resultG->second);
                    if (resultM != types.end()) {
                        if (resultM->second.isInherited() && !resultM->second.isVisited()) {
                            type.appendInheritedDataMembers(resultM->second.getFields(), resultM->second.getInheritedFields(), resultM->second.getMethodSignatures(), resultM->second.getInheritedMethodSignatures(),
                                                            resultM->second.getDeclMethodSignatures(), resultM->second.getInheritedDeclMethodSignatures()); 
                            resultM->second.setVisited(true);
                        }
                        else if (!resultM->second.isVisited()) {
                            findInheritedDataMembers(resultM->second);
                            type.appendInheritedDataMembers(resultM->second.getFields(), resultM->second.getInheritedFields(), resultM->second.getMethodSignatures(), resultM->second.getInheritedMethodSignatures(), 
                                                            resultM->second.getDeclMethodSignatures(), resultM->second.getInheritedDeclMethodSignatures()); 
                        }
                    }
                }
                else {
                    if (!type.hasUnknownParent()) {
                        type.setUnknownParent(true);
                    }
                }
            }      
        }         
    }
    if (!type.isInherited()) {
        type.setInherited(true);
    }
}

// // Finds unknown parents (parents that are not defined in the code) and marks the type as having unknown parents if any is found
// //
// void stereotypesAnalyzer::findUnknownParents(typeModel& type) {
//     for (const auto& parentTypeName : type.getParentNames()) {
//         if (types.find(parentTypeName[1]) == types.end()) {
//             type.setUnknownParent(true);
//             break;
//         }
//     }
// }

// Outputs a CSV report file containing stereotype information and meta data
//  
void stereotypesAnalyzer::outputStereotypesAsCSV(std::ofstream& csvFile, typeModel* type, bool isFreeFunction) {
    const std::vector<functionModel>* functionsPtr = &type->getMethods();

    if (isFreeFunction) functionsPtr = &freeFunctions;
    
    if (VERBOSE_REPORT) {
        for (const functionModel& function : *functionsPtr) {
            std::string temp;

            std::string typeName = isFreeFunction ? "N/A" : (type->getName().size() > 0 ? type->getName()[3] : "N/A");
            std::string typeParents = isFreeFunction ? "N/A" : (type->getParentNames().size() > 0 ? type->getParentsString() : "N/A");
            std::string typeUnknownParents = isFreeFunction ? "N/A" : (type->hasUnknownParent() ? "True" : "False");
            std::string signatures = isFreeFunction ?  (function.getNameSignature().first.empty() ? "N/A" :  function.getNameSignature().first + function.getNameSignature().second) : (type->getMethodSignatures().size() > 0 || type->getDeclMethodSignatures().size() > 0  ? type->getMethodSignaturesString() : "N/A");
            std::string inheritedSignatures = isFreeFunction ? "N/A" : (type->getInheritedMethodSignatures().size() > 0 || type->getInheritedDeclMethodSignatures().size() > 0 ? type->getInheritedMethodSignaturesString() : "N/A");
            std::string functionAttributes = function.getUnitLanguage() == "C++" ? "N/A" : (function.getAttributesOrAnnotations().size() > 0 ? function.getAttributesOrAnnotationsString() : "N/A");
            std::string specifiers = function.getSpecifiers().size() > 0 ? function.getSpecifiersString() : "N/A";
            std::string returnType = function.getReturnType().getType().empty() ? "N/A" : function.getReturnType().getType();
            std::string parametersList = function.getParametersList().empty() ? "N/A" : function.getParametersList();
            std::string internalCalls = function.getFunctionCalls().size() > 0 ? function.getCallsString(false) : "N/A";
            std::string externalCalls = function.getExternalCalls().size() > 0 ? function.getCallsString(true) : "N/A";
            std::string isFieldUsed = function.isFieldUsed() ? "True" :  "False";
            std::string isFieldModified = function.getFieldsModifiedCount() > 0 ? "True" :  "False";
            std::string isCallOnField = function.getMethodCalls().size() > 0 ? "True" :  "False";
            std::string functionName = function.getName().empty() ? "N/A" : function.getName()[3];
            std::string functionLineNumber = function.getLineNumber() != -1 ? std::to_string(function.getLineNumber()) : "N/A";
            std::string functionUnitLanguage = function.getUnitLanguage().empty() ? "N/A" : function.getUnitLanguage();
            std::string functionSignature = function.getNameSignature().first.empty() ? "N/A" : function.getNameSignature().first + function.getNameSignature().second;
            csvFile << HELPERS::escapeCSV(function.getFileName()) << ","
                    << HELPERS::escapeCSV(typeName) << ","
                    << HELPERS::escapeCSV(typeParents) << ","
                    << HELPERS::escapeCSV(typeUnknownParents) << ","
                    << HELPERS::escapeCSV(signatures) << ","
                    << HELPERS::escapeCSV(inheritedSignatures) << ","
                    << HELPERS::escapeCSV(functionName) << ","
                    << HELPERS::escapeCSV(function.getStereotypesString()) << ","
                    << HELPERS::escapeCSV(functionLineNumber) << ","
                    << HELPERS::escapeCSV(functionSignature) << ","
                    << HELPERS::escapeCSV(functionUnitLanguage) << ","
                    << HELPERS::escapeCSV(parametersList) << ","
                    << HELPERS::escapeCSV(returnType) << ","
                    << HELPERS::escapeCSV(specifiers) << ","
                    << HELPERS::escapeCSV(externalCalls) << ","
                    << HELPERS::escapeCSV(internalCalls) << ","
                    << HELPERS::escapeCSV(functionAttributes) << ","
                    << HELPERS::escapeCSV(isFieldUsed) << ","
                    << HELPERS::escapeCSV(isFieldModified) << ","
                    << HELPERS::escapeCSV(isCallOnField) << "\n";
        }
    }
    else {
            for (const functionModel& function : *functionsPtr) {
            std::string typeName = isFreeFunction ? "N/A" : (type->getName().size() > 0 ? type->getName()[0] : "N/A");
            std::string typeStereotype = isFreeFunction ? "N/A" : type->getStereotypesString();

            csvFile << HELPERS::escapeCSV(typeName) << ","
                    << HELPERS::escapeCSV(typeStereotype) << ","
                    << HELPERS::escapeCSV(function.getName()[1]) << ","
                    << HELPERS::escapeCSV(function.getStereotypesString()) << "\n";
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
