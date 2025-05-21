// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModelCollection.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "ClassModelCollection.hpp"

extern XPathBuilder                  XPATH_TRANSFORMATION;  
extern std::unordered_map
       <int, std::unordered_map
       <std::string, std::string>>   XPATH_LIST;   
extern primitiveTypes                PRIMITIVES;
extern ignorableCalls                IGNORED_CALLS;
extern typeSpecifiers                TYPE_SPECIFIERS;  
extern bool                          IS_VERBOSE;
extern bool                          FREE_FUNCTION;

classModelCollection::classModelCollection(srcml_archive* archive, srcml_archive* outputArchive,
                                                    const std::string& inputFile, const std::string& outputFile, 
                                                    bool outputTxtReport, bool outputCsvReport, bool reDocComment) {  
    PRIMITIVES.createPrimitiveList();
    IGNORED_CALLS.createCallList();
    TYPE_SPECIFIERS.createSpecifierList();

    if (IS_VERBOSE) {
        PRIMITIVES.outputPrimitives();
        IGNORED_CALLS.outputCalls();
        TYPE_SPECIFIERS.outputSpecifiers();
    }
        
    // Analyze one unit at a time
    srcml_unit* unit = srcml_archive_read_unit(archive);
    int unitNumber = 1; // Count starts at 1 in XPath
    while (unit){
        findClassInfo(archive, unit, unitNumber); 
        findFreeFunctions(archive, unit, unitNumber);

        srcml_unit_free(unit); 
        ++unitNumber;
        unit = srcml_archive_read_unit(archive);
    }

    // Performed after the collection of all classes and free functions
    analyzeFreeFunctions();

    // Finds inherited data members
    for (auto& pair : classCollection) {
        if (pair.second.getName()[1] == "LexerStylerArray") {
            std::cout << "LexerStylerArray" << std::endl;
        }
        findInheritedDataMembers(pair.second);
        pair.second.setInherited(true);
        for (auto& pairS : classCollection)
            pairS.second.setVisited(false);
    } 

    // Resets inheritance and build signatures for findInheritedMethods()
    for (auto& pair : classCollection) {
        pair.second.setInherited(false); 
        pair.second.buildMethodSignature();
    }
        
    // Finds inherited methods
    for (auto& pair : classCollection) {
        findInheritedMethods(pair.second);
        pair.second.setInherited(true);
        for (auto& pairS : classCollection)
            pairS.second.setVisited(false);
    } 

    // Analyze all methods for each class
    for (auto& pair : classCollection) {
        std::vector<methodModel>& methods = pair.second.getMethods();
        for (auto& m : methods)
             m.findData(pair.second.getDataMembers(), pair.second.getMethodSignatures(), pair.second.getName()[3]);                         
    }

    // Compute stereotypes here
    stereotypes stereotypesObj;
    stereotypesObj.computeMethodStereotypes (classCollection);
    stereotypesObj.computeClassStereotypes  (classCollection);

    // Analyze and compute stereotypes for free functions
    if (FREE_FUNCTION) {
        for (auto& f : freeFunctions) f.findFreeFunctionData();
        stereotypesObj.computeFreeFunctionsStereotypes(freeFunctions);
    }
    
    // Optional TXT report file
    std::string InputFileNoExt = inputFile.substr(0, inputFile.size() - 4);
    if (outputTxtReport) {
        std::ofstream reportFile(InputFileNoExt + ".stereotypes.txt");
        std::stringstream stringStream;
        for (auto& pair : classCollection)  outputTxtReportFile(stringStream, &pair.second);
        reportFile << stringStream.str();
        reportFile.close();         

        stringStream.str("");
        stringStream.clear();
        reportFile.open(InputFileNoExt + ".free_functions_stereotypes.txt");
        outputTxtReportFile(stringStream, nullptr);
        reportFile << stringStream.str();
        reportFile.close();   
    }

    // Optional CSV report file
    if (outputCsvReport) {
        std::ofstream out;
        out.open(InputFileNoExt + ".stereotypes.csv");
        out << "Class Name,Class Stereotype,Method Name,Method Stereotype" << '\n';
        for (auto& pair : classCollection) outputCsvReportFile(out, &pair.second);        
        out.close();

        out.open(InputFileNoExt + ".free_functions_stereotypes.csv");
        out << "Free Function Name,Free Function Stereotype" << '\n';
        outputCsvReportFile(out, nullptr);        
        out.close();
    }

    // Optional verbose output
    if (IS_VERBOSE) outputCsvVerboseReportFile(InputFileNoExt);
    
    // Generate the stereotyped XML archive
    std::map<int, srcml_unit*> transformedUnits;
    std::unordered_map<int, srcml_transform_result*> results;

    std::vector<std::thread> threads;
    std::vector<srcml_unit*> units;
    std::mutex mu;

    unsigned int unitNumberCount = 1;
    unsigned int threadPoolCount = 0;
    unsigned int nthreads = std::thread::hardware_concurrency();

    // Read all units in the archive again for output generation
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    archive = srcml_archive_create();
    srcml_archive_read_open_filename(archive, inputFile.c_str()); 
    unit = srcml_archive_read_unit(archive);

    while (unit){
        while ((threadPoolCount < nthreads) && unit) {
            units.push_back(unit);
            threads.push_back(std::thread(&classModelCollection::outputWithStereotypes, this, 
                                        unit, std::ref(transformedUnits), unitNumberCount,  
                                        std::ref(XPATH_LIST[unitNumberCount]), std::ref(results), std::ref(mu)));

            unit = srcml_archive_read_unit(archive);

            ++unitNumberCount;
            ++threadPoolCount;
        }

        for (std::thread& thread : threads) if (thread.joinable()) thread.join();
        threads.clear();

        // Write output
        for (const auto& pair : transformedUnits) srcml_archive_write_unit(outputArchive, pair.second); 
        transformedUnits.clear();

        // Clean
        for (auto& pair : results) srcml_transform_free(pair.second);  
        results.clear();

        for (auto& u : units) srcml_unit_free(u); 
        units.clear();

        threadPoolCount = 0;       
    }

    srcml_archive_close(outputArchive);
    srcml_archive_free(outputArchive);   
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    // Annotate as comments
    if (reDocComment){
        std::string temp = outputFile + ".temp.xml";

        archive = srcml_archive_create();
        srcml_archive_read_open_filename(archive, outputFile.c_str());  

        outputArchive = srcml_archive_create();
        srcml_archive_write_open_filename(outputArchive, temp.c_str());
        srcml_archive_register_namespace(outputArchive, "st", "http://www.srcML.org/srcML/stereotype"); 

        unit = srcml_archive_read_unit(archive);

        while (unit) {
            outputAsComments(unit, outputArchive);
            srcml_unit_free(unit);
            unit = srcml_archive_read_unit(archive);
        }

        std::filesystem::remove(outputFile);
        std::filesystem::rename(temp, outputFile);

        srcml_archive_close(archive); 
        srcml_archive_close(outputArchive);
        srcml_archive_free(outputArchive);   
    }
}

// Finds classs in an archive
//
// In C++, class names are usually in the form of:
//      myClass  
// or  
//      myClass<type1, type2, ...> for a specialized templated class from myClass
//
// In C# and Java:
//      myClass
// or
//      myClass<T, G, ... > for a generic class
//
// Unlike C++, C# and Java can allow multiple classes with the same name but 
//  different number of generic parameters to exist
// For example, foo<T> and foo<T, T1> are valid
//
// C++:
//  Unions can be declared anonymous (union without a name) 
//  Anonymous unions can only be declared inside a class, struct, or a namespace and cannot have methods 
//   and their data members are accessed directly as part of the enclosing scope
//   so, data members that are defined inside anonymous unions are considered and the anonymous union itself is not considered
//   also, anonymous unions that are defined in a namespace are not considered because their data members are basically globals
//  Classes can be declared without a name (anonymous classes) require you to simultaneously create an instance of it during its definition
//   so, these are treated as normal classes
// C#:
//  Allows static classes to be declared, and these must contain only static data members
//   They are ignored and their methods are collected as free functions
//  Nested classes, struct, or interfaces are ignored
// Java:
//  enums in Java can contain methods and data members
//  Static classes are allowed, but these can only be nested within other classes, interfaces, or enums
//   Static classes in java can contain non-static data members or methods
//   They are ignored (since they are nested) and their methods (only if static) are collected as free functions
//  Anonymous classes (classes without names and are nested as instances) are ignored
void classModelCollection::findClassInfo(srcml_archive* archive, srcml_unit* unit, int unitNumber) {
    std::string unitLanguage = srcml_unit_get_language(unit);
    if (unitLanguage == "C") { unitLanguage = "C++"; } // Quick hack. C is a simple case of C++, so we can process it as C++.
    if (unitLanguage == "C++" || unitLanguage == "C#" || unitLanguage == "Java") {
        srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage, "class").c_str()); 

        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);
        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; i++) {    
            resultUnit = srcml_transform_get_unit(result, i);
            srcml_archive* classArchive = srcml_archive_create();
            srcml_archive_register_namespace(classArchive, "pos", "http://www.srcML.org/srcML/position"); // Needed for input with positions enabled

            char* unparsed = nullptr;
            std::size_t size = 0;
            srcml_archive_write_open_memory(classArchive, &unparsed, &size);
            srcml_archive_write_unit(classArchive, resultUnit);
            srcml_archive_close(classArchive);
            srcml_archive_free(classArchive);

            classArchive = srcml_archive_create();
            srcml_archive_read_open_memory(classArchive, unparsed, size);
            srcml_unit* unitClass = srcml_archive_read_unit(classArchive);
            std::string classXpath = "(" + XPATH_TRANSFORMATION.getXpath(unitLanguage, "class") + ")[" + std::to_string(i + 1) + "]";
            classModel c(classArchive, unitClass, unitLanguage); 

            // Needed for partial classs in C#
            if (classCollection.find(c.getName()[1]) != classCollection.end())
                // Append the partial class data to the existing partial class
                classCollection.at(c.getName()[1]).findData(classArchive, unitClass, classXpath, unitNumber);
            else {
                c.findData(classArchive, unitClass, classXpath, unitNumber);      
                classCollection.insert({c.getName()[1], c});  
            }                 

            // Needed for inheritance in Java and C#
            if (unitLanguage != "C++") classGenerics.insert({c.getName()[2], c.getName()[1]}); 
            
            free(unparsed);
            srcml_unit_free(unitClass);
            srcml_archive_close(classArchive);
            srcml_archive_free(classArchive);            
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
//      Method (Foo) could belong to a specialized templated class: 
//          template<typename T> class MyClass{}; // Generic templated class.
//          template<optional> class MyClass<int>{}; // Specialized templated class from the generic one.
//          void MyClass<int>::Foo(){} // Belongs to the specialized templated class. 
//      Method could belong to the generic templated class
//          template<typename T> class MyClass{}; // Generic templated class.
//          template<typename T> void MyClass<T>::Foo(){} // Generic templated method belongs to generic templated class.        
//          template<optional>   void MyClass<int>::Foo(){} // Specialized templated method belongs to a generic templated class.
//           If a specialized templated class is defined for <int> then the previous method won't be allowed. It must be replaced with:
//           void MyClass<int>::Foo(){} 
//      Method could be contained in a namespace
//          namespace::MyClass<int>::Foo(){}
//      Method could belong to a normal class
//          MyClass::Foo(){}
//      Function could be a free function (including normal free functions, friend functions, static methods, methods defined for external classes)
//          Foo(){}, namespace::Foo(){}, static Foo(){}, externalClass::Foo(){}, 
//
void classModelCollection::findFreeFunctions(srcml_archive* archive, srcml_unit* unit, int unitNumber) {
    std::string unitLanguage = srcml_unit_get_language(unit); 
    if (unitLanguage == "C") { unitLanguage = "C++"; }
    if (unitLanguage == "C++" || unitLanguage == "C#" || unitLanguage == "Java") {
        srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"free_function").c_str());
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);  

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; i++) {
            resultUnit = srcml_transform_get_unit(result, i);
            srcml_archive* methodArchive = srcml_archive_create();
            srcml_archive_register_namespace(methodArchive, "pos", "http://www.srcML.org/srcML/position");
            
            char* unparsed = nullptr;
            std::size_t size = 0;
            srcml_archive_write_open_memory(methodArchive, &unparsed, &size);
            srcml_archive_write_unit(methodArchive, resultUnit);
            srcml_archive_close(methodArchive);
            srcml_archive_free(methodArchive);

            methodArchive = srcml_archive_create();
            srcml_archive_read_open_memory(methodArchive, unparsed, size);
            srcml_unit* methodUnit = srcml_archive_read_unit(methodArchive);

            std::string functionXpath =  "(" + XPATH_TRANSFORMATION.getXpath(unitLanguage,"free_function") + ")[" + std::to_string(i + 1) + "]";
            methodModel function(methodArchive, methodUnit, functionXpath, unitLanguage, "", unitNumber);

            freeFunctions.push_back(function);

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
void classModelCollection::analyzeFreeFunctions() {
    for (std::vector<methodModel>::iterator function = freeFunctions.begin(); function != freeFunctions.end();) {
        if (function->getName() == "LexerStylerArray::addLexerStyler") {
            std::cout << "addLexerStyler" << std::endl;
        }
        if (function->getUnitLanguage() == "C++") {
            // Removes namespaces if any
            std::string functionName = function->getName();  
            removeNamespace(functionName, "C++", false);

            // Get the class name (if any). Else, it is a free function
            std::size_t isClassName = functionName.find("::");
            if (isClassName != std::string::npos) { // Class found, it is a method       
                std::string className = functionName.substr(0, isClassName); 
                auto result = classCollection.find(className);
                if (result != classCollection.end()) {
                    result->second.addMethod(*function);
                    function = freeFunctions.erase(function);
                }                          
                else { // Case specialized template method belongs to the generic template class
                    className = className.substr(0, className.find("<"));
                    result = classCollection.find(className);
                    if (result != classCollection.end())  {
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


// Finds inherited data members 
// In C++, you can inherit from a specialized templated class or
//  you can specialize the inheritance itself from the generic class, or
//  you can inherit from the generic class itself.
// For example:
//  myClass --> childClass : myClass<T> or childClass : myClass<int>
//  specializedClass<int> --> childClass : specializedClass<int>
//
// In Java and C#, you can inherit from the generic class or specialize the inheritance.
// For example:
//  myClass<T1, T2> --> childClass : myClass<T1, T2> or childClass : myClass<int, double>
//
void classModelCollection::findInheritedDataMembers(classModel& c) {
    const std::string& unitLanguage = c.getUnitLanguage();
    c.setVisited(true); 
    const std::unordered_map<std::string, std::string>& parentClassName =  c.getParentClassName();

    for (const auto& pair : parentClassName){
        std::string parClassName = pair.first;
        auto result = classCollection.find(parClassName);
        if (result != classCollection.end()) {
            if (result->second.isInherited() && !result->second.isVisited()) {
                c.appendInheritedDataMembers(result->second.getDataMembers()); 
                result->second.setVisited(true);
            }
                
            else if (!result->second.isVisited()) {
                findInheritedDataMembers(result->second);
                c.appendInheritedDataMembers(result->second.getDataMembers());
            }
        }       
        else {
            if (unitLanguage == "C++") {
                parClassName = parClassName.substr(0, parClassName.find("<"));
                result = classCollection.find(parClassName);
                if (result != classCollection.end()) {
                    if (result->second.isInherited() && !result->second.isVisited()) {
                        c.appendInheritedDataMembers(result->second.getDataMembers());
                        result->second.setVisited(true);
                    }
                        
                    else if (!result->second.isVisited()) {
                        findInheritedDataMembers(result->second);
                        c.appendInheritedDataMembers(result->second.getDataMembers());
                    }
                }              
            }
            else {  
                removeBetweenComma(parClassName, true);
                auto resultG = classGenerics.find(parClassName);
                if (resultG != classGenerics.end()) {
                    auto resultM = classCollection.find(resultG->second);
                    if (resultM != classCollection.end()) {
                        if (resultM->second.isInherited() && !resultM->second.isVisited()) {
                            c.appendInheritedDataMembers(resultM->second.getDataMembers());
                            resultM->second.setVisited(true);
                        }
                        else if (!resultM->second.isVisited()) {
                            findInheritedDataMembers(resultM->second);
                            c.appendInheritedDataMembers(resultM->second.getDataMembers());
                        }
                    }
                }
            }      
        }         
    }
}

// Finds inherited methods
//
void classModelCollection::findInheritedMethods(classModel& c) {   
    const std::string& unitLanguage = c.getUnitLanguage();
    c.setVisited(true); 
    const std::unordered_map<std::string, std::string>& parentClassName =  c.getParentClassName();

    for (const auto& pair : parentClassName){
        std::string parClassName = pair.first;
        auto result = classCollection.find(parClassName);
        if (result != classCollection.end()) {
            if (result->second.isInherited() && !result->second.isVisited()) {
                c.appendInheritedMethod(result->second.getMethodSignatures()); 
                result->second.setVisited(true);
            }
                
            else if (!result->second.isVisited()) {
                findInheritedMethods(result->second);                     
                c.appendInheritedMethod(result->second.getMethodSignatures());  
            }
        }       
        else {
            if (unitLanguage == "C++") {
                parClassName = parClassName.substr(0, parClassName.find("<"));
                result = classCollection.find(parClassName);
                if (result != classCollection.end()) {
                    if (result->second.isInherited() && !result->second.isVisited()) {
                        c.appendInheritedMethod(result->second.getMethodSignatures()); 
                        result->second.setVisited(true);
                    }
                        
                    else if (!result->second.isVisited()) {
                        findInheritedMethods(result->second);                     
                        c.appendInheritedMethod(result->second.getMethodSignatures()); 
                    }
                }              
            }
            else {  
                removeBetweenComma(parClassName, true);
                auto resultG = classGenerics.find(parClassName);
                if (resultG != classGenerics.end()) {
                    auto resultM = classCollection.find(resultG->second);
                    if (resultM != classCollection.end()) {
                        if (resultM->second.isInherited() && !resultM->second.isVisited()) {
                            c.appendInheritedMethod(resultM->second.getMethodSignatures());  
                            resultM->second.setVisited(true);
                        }
                        else if (!resultM->second.isVisited()) {
                            findInheritedMethods(resultM->second);                     
                            c.appendInheritedMethod(resultM->second.getMethodSignatures());   
                        }
                    }
                }
            }      
        }         
    }
}

// Generates other CSV report files containing stereotype information
// This includes method_view (e.g., get set ... etc)
// This includes class view (e.g., entity control ... etc)
// This includes unique_method_view (e.g., 'get collaborator' ... etc)
// This includes unique_class_view (e.g., 'entity control' ... etc)
// This includes category_view (e.g., accessors, mutators ... etc) 
//
void classModelCollection::outputCsvVerboseReportFile(const std::string& InputFileNoExt) {
    std::unordered_map<std::string, int>            uniqueMethodStereotypesView;  
    std::unordered_map<std::string, int>            uniqueClassStereotypesView;  

    std::unordered_map<std::string, int> classStereotypes = {
        {"entity", 0},
        {"minimal-entity", 0},
        {"data-provider", 0},
        {"commander", 0},
        {"boundary", 0},
        {"factory", 0},
        {"controller", 0},
        {"pure-controller", 0},
        {"large-class", 0},
        {"lazy-class", 0},
        {"degenerate", 0},
        {"data-class", 0},
        {"small-class", 0},
        {"unclassified", 0},
        {"empty", 0},
    };

    std::unordered_map<std::string, int> methodStereotypes = {
        {"get", 0},
        {"predicate", 0},
        {"property", 0},
        {"void-accessor", 0},
        {"set", 0},
        {"command", 0},
        {"non-void-command", 0},
        {"collaborator", 0},
        {"controller", 0},
        {"wrapper", 0},
        {"constructor", 0},
        {"copy-constructor", 0},
        {"destructor", 0},
        {"factory", 0},
        {"incidental", 0},
        {"stateless", 0},  
        {"empty", 0},
        {"unclassified", 0},
    };

    for (auto& pair : classCollection) {
        uniqueClassStereotypesView[pair.second.getStereotype()]++; 
        for (const std::string& s : pair.second.getStereotypeList()) 
            classStereotypes[s]++;   
        
        const std::vector<methodModel>& method = pair.second.getMethods();     
        for (const auto& m : method) {         
            uniqueMethodStereotypesView[m.getStereotype()]++;

            for (const std::string& s : m.getStereotypeList())
                methodStereotypes[s]++;           
        }
    }

    std::ofstream outU, outV, outM, outS, outC;
    outU.open(InputFileNoExt + ".unique_method_view.csv");
    outV.open(InputFileNoExt + ".unique_class_view.csv");
    outM.open(InputFileNoExt + ".method_view.csv");
    outS.open(InputFileNoExt + ".class_view.csv");
    outC.open(InputFileNoExt + ".category_view.csv");

    
    // Needed to print stereotypes in this order
    std::vector<std::string> method_ordered_keys = {
        "get", "predicate", "property", "void-accessor", "set", "command", "non-void-command", 
        "collaborator", "controller", "wrapper", "constructor", "copy-constructor", "destructor", "factory", 
        "incidental", "stateless", "empty", "unclassified"
    };
    
    std::vector<std::string> class_ordered_keys = {
        "entity", "minimal-entity", "data-provider", "commander", "boundary", "factory", 
        "controller", "pure-controller", "large-class", "lazy-class", "degenerate", "data-class", 
        "small-class", "empty", "unclassified"
    };

    int total = 0;
    // Unique Method View
    if (outU.is_open()) {
        outU << "Unique Method Stereotype,Method Count" <<'\n';
        for (auto& pair : uniqueMethodStereotypesView){
            outU << pair.first << ",";
            outU << pair.second << '\n';
            total += pair.second;
        }
        outU << "Total" << "," << total;
    }

    // Unique Class View
    if (outV.is_open()) {   
        outV << "Unique Class Stereotype,Class Count" <<'\n';
        total = 0;
        for (auto& pair : uniqueClassStereotypesView){
            outV << pair.first << ",";
            outV << pair.second << '\n';
            total += pair.second;
        }
        outV << "Total" << "," << total;
    }

    // Method View
    //
    if (outM.is_open()) { 
        outM << "Method Stereotype,Stereotype Count" <<'\n';
        total = 0;
        for (const auto& key : method_ordered_keys) {
            outM << key << ",";
            outM << methodStereotypes[key] << '\n';
            total += methodStereotypes[key];
        }
        outM << "Total" << "," << total;
    }

    // Class View
    //
    if (outS.is_open()) {
        outS << "Class Stereotype,Class Count" <<'\n';
        total = 0;
        for (const auto& key : class_ordered_keys) {
            outS << key << ",";
            outS << classStereotypes[key] << '\n';
            total += classStereotypes[key];
        }
        outS << "Total" << "," << total;
    }

    // Category view
    int getters = methodStereotypes["get"];
    int accessors = getters + methodStereotypes["predicate"] +
                    methodStereotypes["property"] +
                    methodStereotypes["void-accessor"];

    int setters = methodStereotypes["set"];     
    int commands = methodStereotypes["command"] + methodStereotypes["non-void-command"];           
    int mutators = setters + commands;

    int controllers = methodStereotypes["controller"];
    int collaborator =  methodStereotypes["collaborator"] + methodStereotypes["wrapper"]; 
    int collaborators = controllers + collaborator;
    
    int factory = methodStereotypes["factory"] + methodStereotypes["constructor"] + methodStereotypes["copy-constructor"] + methodStereotypes["destructor"];

    int degenerates = methodStereotypes["incidental"] + methodStereotypes["stateless"] + methodStereotypes["empty"]; 

    int unclassified = methodStereotypes["unclassified"];

    total = accessors + mutators + factory + collaborators + degenerates + unclassified;
    outC << "Stereotype Category,Stereotype Count" <<'\n';
    outC << "Accessors" << "," << accessors << '\n';
    outC << "Mutators" << "," << mutators << '\n';
    outC << "Creational" << "," << factory << '\n';
    outC << "Collaborational" << "," << collaborators << '\n';
    outC << "Degenerate" << "," << degenerates << '\n';
    outC << "Unclassified" << "," << unclassified << '\n';
    outC << "Total" << "," << total;

    outU.close();
    outV.close();
    outM.close();     
    outS.close();
    outC.close();
}

// Optional TXT report file containing stereotype information
// Format:
//
// Class Name:                  Class Stereotype:
// ...                          ...
// Method Name:                 Method Stereotype:
// ...                          ...
//---------------------------------------------------------------
// Class Name:                  Class Stereotype:
// ...                          ...
// Method Name:                 Method Stereotype:
// ...                          ...
//
void classModelCollection::outputTxtReportFile(std::stringstream& stringStream, classModel* c) {
    const int WIDTH = 70;
    const std::string line(WIDTH * 2, '-');
    auto setw_width = std::setw(WIDTH);
    std::vector<methodModel>* methods = &freeFunctions;

    if (c != nullptr) {
        const std::string& className = c->getName()[1];
        const std::string& classStereotype = c->getStereotype();
        stringStream << std::left << setw_width << "Class Name:" << setw_width << "Class Stereotype:" << '\n';
        stringStream << std::left << setw_width <<  className << setw_width << classStereotype << "\n\n";
        stringStream << std::left << setw_width << "Method Name:" << setw_width << "Method Stereotype:" << '\n';
        methods = &c->getMethods();
    }
    else 
        stringStream << std::left << setw_width << "Free Function Name:" << setw_width << "Free Function Stereotype:" << '\n';

    
    for (const auto& m : *methods) {
        std::string methodName = m.getName();
        stringStream << std::left << setw_width << methodName; 
        stringStream << setw_width << m.getStereotype() << '\n';
    }
    stringStream << line << '\n'; 
}

// Optional CSV report file containing stereotype information
//
void classModelCollection::outputCsvReportFile(std::ofstream& out, classModel* c) {
    std::vector<methodModel>* methods = &freeFunctions;

    std::string classInfo;
    if (c != nullptr) {
        classInfo = "\"" + c->getName()[1] + "\",\"" + c->getStereotype() + "\"";
        methods = &c->getMethods();
    }

    for (const auto& m : *methods) {
        std::string methodName = "\"" + m.getName() + "\"";
        if (c != nullptr) out << classInfo << ",";
        out << methodName << "," << "\"" + m.getStereotype() + "\"" << '\n';          
    }
}

//  Add in stereotypes on <class> and <function>
//  Example: <function st:stereotype="get"> ... </function>
//           <class st:stereotype="boundary"> ... ></class>
//
void classModelCollection::outputWithStereotypes(srcml_unit* unit, std::map<int, srcml_unit*>& transformedUnits,
                                                int unitNumber, const std::unordered_map<std::string, std::string>& xpathPair,
                                                std::unordered_map<int, srcml_transform_result*>& results, std::mutex& mu) {  
        srcml_archive* archive = srcml_archive_create();
        bool transform = false;
        for (auto& pair : xpathPair) { 
            srcml_append_transform_xpath_attribute(archive, pair.first.c_str(), "st",
                                    "http://www.srcML.org/srcML/stereotype",
                                    "stereotype", pair.second.c_str());             
            transform = true;               
        }  
        if (transform) {
            srcml_transform_result* result = nullptr; 
            srcml_unit_apply_transforms(archive, unit, &result);
            srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);  
            {
                std::lock_guard<std::mutex> guard(mu);
                transformedUnits.insert({unitNumber, resultUnit});
                results.insert({unitNumber, result});   
            }
        }
        else {
            std::lock_guard<std::mutex> guard(mu);
            transformedUnits.insert({unitNumber, unit});
        }
             
        srcml_clear_transforms(archive); 
        srcml_archive_free(archive);
}

// Inserts the stereotype as a comment before each function or class tag
// For example, /** @stereotype get */
// last_ws is used to preserve to the whitespace that precedes each function or class
//
void classModelCollection::outputAsComments(srcml_unit* unit, srcml_archive* outputArchive) {
    std::string xslt = R"**(<xsl:stylesheet
    xmlns="http://www.srcML.org/srcML/src"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:src="http://www.srcML.org/srcML/src" 
    xmlns:cpp="http://www.srcML.org/srcML/cpp"
    xmlns:st="http://www.srcML.org/srcML/stereotype"
    xmlns:func="http://exslt.org/functions"
    extension-element-prefixes="func"
    exclude-result-prefixes="src st cpp"
    version="1.0">
    
    <xsl:output method="xml" version="1.0" encoding="UTF-8" standalone="yes"/>
    <xsl:template match="@*|node()"><xsl:copy><xsl:apply-templates select="@*|node()"/></xsl:copy></xsl:template>
 
    <func:function name="src:last_ws">
        <xsl:param name="s"/>
        <xsl:choose>
        <xsl:when test="contains($s, '&#xa;')">
            <func:result select="src:last_ws(substring-after($s, '&#xa;'))"/>
        </xsl:when>
        <xsl:otherwise>
            <func:result select="$s"/>
        </xsl:otherwise>
        </xsl:choose>
    </func:function>
    
    <xsl:template match="*[@st:stereotype]">
        <comment type="block">
            <xsl:text>/** @Stereotype </xsl:text>
            <xsl:value-of select="@st:stereotype"/>
            <xsl:text> */</xsl:text>
        </comment>
        <xsl:text>&#xa;</xsl:text>
        <xsl:value-of select="src:last_ws(preceding-sibling::text()[1])"/>
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
    </xsl:template>
    </xsl:stylesheet>
    )**"; 

    srcml_archive* archive = srcml_archive_create();
    srcml_transform_result* result = nullptr; 

    srcml_append_transform_xslt_memory(archive, xslt.c_str(), xslt.size());           
    srcml_unit_apply_transforms(archive, unit, &result);

    srcml_unit* resultUnit = srcml_transform_get_unit(result, 0);  
    srcml_archive_write_unit(outputArchive, resultUnit); 
    
    srcml_transform_free(result);  
    srcml_clear_transforms(archive); 
    srcml_archive_free(archive);
}

