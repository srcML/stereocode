// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModelCollection.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
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
extern typeTokens                    TYPE_TOKENS;  
extern bool                          IS_VERBOSE;

classModelCollection::classModelCollection (srcml_archive* archive, srcml_archive* outputArchive,
                                            std::vector<srcml_unit*>& units, 
                                            const std::string& inputFile, const std::string& outputFile, 
                                            bool outputTxtReport, bool outputCsvReport, bool reDocComment) {  
    PRIMITIVES.createPrimitiveList();
    IGNORED_CALLS.createCallList();
    TYPE_TOKENS.createTokenList();

    if (IS_VERBOSE) {
        PRIMITIVES.outputPrimitives();
        IGNORED_CALLS.outputCalls();
        TYPE_TOKENS.outputTokens();
    }
        
    // Collects class info + methods defined internally to a class
    findClassInfo(archive, units); 

    // Collects free functions and methods defined externally to a class (C++ only)
    findFreeFunctions(archive, units); 

    // Finds inherited attributes for each class
    for (auto& pair : classCollection) {
        findInheritedAttributes(pair.second);
        pair.second.setInherited(true);
        for (auto& pairS : classCollection)
            pairS.second.setVisited(false);
    } 

    // Resets inheritance and build signatures for findInheritedMethod()
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
             m.findMethodData(pair.second.getAttribute(), pair.second.getMethodSignatures(), 
                              pair.second.getInheritedMethodSignatures(), pair.second.getName()[3]);                         
    }

    // Compute method and stereotypes here
    for (auto& pair : classCollection) {
        pair.second.computeMethodStereotype();
        pair.second.computeClassStereotype();
    }

    for (auto& f : freeFunctions) 
        f.findFreeFunctionData();
    
    computeFreeFunctionsStereotypes();

    // Optional TXT report file
    std::string InputFileNoExt = inputFile.substr(0, inputFile.size() - 4);
    if (outputTxtReport) {
        std::ofstream reportFile(InputFileNoExt + ".stereotypes.txt");
        std::stringstream stringStream;
        for (auto& pair : classCollection)
            outputTxtReportFile(stringStream, pair.second);
        reportFile << stringStream.str();
        reportFile.close();        
    }

    // Optional CSV report file
    if (outputCsvReport) {
        std::ofstream out;
        out.open(InputFileNoExt + ".stereotypes.csv");
        out << "Class Name,Class Stereotype,Method Name,Method Stereotype" << '\n';
        for (auto& pair : classCollection)
            outputCsvReportFile(out, pair.second);        
 
        out.close();
    }

    if (IS_VERBOSE) 
        outputCsvVerboseReportFile(InputFileNoExt);
    
            
    // Generate the stereotyped XML archive
    std::map<int, srcml_unit*> transformedUnits;
    std::unordered_map<int, srcml_transform_result*> results;
    std::vector<std::thread> threads;
    std::mutex mu;
    for (size_t i = 0; i < units.size(); i++)
        threads.push_back(std::thread(&classModelCollection::outputWithStereotypes, this, 
                                      units[i], std::ref(transformedUnits), i,  std::ref(XPATH_LIST[i]), std::ref(results), std::ref(mu)));
    
    for (std::thread& thread : threads) 
        if (thread.joinable())
            thread.join();
 
    for (const auto& pair : transformedUnits) 
        srcml_archive_write_unit(outputArchive, pair.second); 
    
    // Clean
    for (auto& pair : results)  
        srcml_transform_free(pair.second);   
    
    for (size_t i = 0; i < units.size(); i++)
        srcml_unit_free(units[i]);  

    srcml_archive_close(outputArchive);
    srcml_archive_free(outputArchive);   
    srcml_archive_close(archive);

    units.clear();

    // Annotate as comments
    if (reDocComment){
        std::string temp = outputFile+".temp.xml";
        outputArchive = srcml_archive_create();
        srcml_archive_write_open_filename(outputArchive, temp.c_str());
        srcml_archive_register_namespace(outputArchive, "st", "http://www.srcML.org/srcML/stereotype"); 
        srcml_archive_read_open_filename(archive, outputFile.c_str());  
        srcml_unit* unit = srcml_archive_read_unit(archive);

        // Read all units in an archive
        while (unit){
            units.push_back(unit);
            unit = srcml_archive_read_unit(archive);
        }

        for (size_t i = 0; i < units.size(); i++)
            outputAsComments(units[i], outputArchive);

        std::filesystem::remove(outputFile);
        std::filesystem::rename(temp, outputFile);

        srcml_archive_close(archive); 
        srcml_archive_close(outputArchive);
        srcml_archive_free(outputArchive);   
    }
    srcml_archive_free(archive);
    
}

// Finds classes in an archive
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
//  different # of generic parameters to exist. For example, foo<T> and foo<T, T1> are valid
//
void classModelCollection::findClassInfo(srcml_archive* archive, std::vector<srcml_unit*> units) {
    for (size_t j = 0; j < units.size(); j++) {
        std::string unitLanguage = srcml_unit_get_language(units[j]);   
        if (unitLanguage == "C++" || unitLanguage == "C#" || unitLanguage == "Java") {
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"class").c_str()); 

            srcml_transform_result* result = nullptr;
            srcml_unit_apply_transforms(archive, units[j], &result);
            int n = srcml_transform_get_unit_size(result);
            srcml_unit* resultUnit = nullptr;
            for (int i = 0; i < n; i++) {    
                resultUnit = srcml_transform_get_unit(result, i);
                srcml_archive* classArchive = srcml_archive_create();
                char* unparsed = nullptr;
                size_t size = 0;
                srcml_archive_write_open_memory(classArchive, &unparsed, &size);
                srcml_archive_write_unit(classArchive, resultUnit);
                srcml_archive_close(classArchive);
                srcml_archive_free(classArchive);
  
                classArchive = srcml_archive_create();
                srcml_archive_read_open_memory(classArchive, unparsed, size);
                srcml_unit* unit = srcml_archive_read_unit(classArchive);
                
                std::string classXpath = XPATH_TRANSFORMATION.getXpath(unitLanguage,"class") + "[" + std::to_string(i + 1) + "]";
                classModel c(classArchive, unit, unitLanguage); 
                
                // Needed for partial classes in C#
                if (classCollection.find(c.getName()[1]) != classCollection.end())
                    classCollection.at(c.getName()[1]).findClassData(classArchive, unit, freeFunctions, classXpath, j);
                else {
                    c.findClassData(classArchive, unit, freeFunctions, classXpath, j);      
                    classCollection.insert({c.getName()[1], c});  
                }                 

                // Needed for inheritance in Java and C#
                if (unitLanguage != "C++") classGenerics.insert({c.getName()[2], c.getName()[1]}); 
                
                free(unparsed);
                srcml_unit_free(unit);
                srcml_archive_close(classArchive);
                srcml_archive_free(classArchive);            
            }   
            srcml_transform_free(result);
            srcml_clear_transforms(archive);   
        }   
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
//      Method could belong to a namespace
//          namespaceA::MyClass<int>::Foo(){}
//      Method could belong to a normal class
//          MyClass::Foo(){}
//      Function could be a free function (including friend functions and static methods)
//          Foo(){}, externalClass::Foo(){}, namespaceA::Foo(){}, operator<<(){}
//
void classModelCollection::findFreeFunctions(srcml_archive* archive, std::vector<srcml_unit*> units) {
    for (size_t j = 0; j < units.size(); j++){
        std::string unitLanguage = srcml_unit_get_language(units[j]); 
        if (unitLanguage == "C++") {     
            srcml_append_transform_xpath(archive, XPATH_TRANSFORMATION.getXpath(unitLanguage,"free_function").c_str());
            srcml_transform_result* result = nullptr;
            srcml_unit_apply_transforms(archive, units[j], &result);
            int n = srcml_transform_get_unit_size(result);  

            srcml_unit* resultUnit = nullptr;
            for (int i = 0; i < n; i++) {
                resultUnit = srcml_transform_get_unit(result, i);
                srcml_archive* methodArchive = srcml_archive_create();
                char* unparsed = nullptr;
                size_t size = 0;
                srcml_archive_write_open_memory(methodArchive, &unparsed, &size);
                srcml_archive_write_unit(methodArchive, resultUnit);
                srcml_archive_close(methodArchive);
                srcml_archive_free(methodArchive);

                methodArchive = srcml_archive_create();
                srcml_archive_read_open_memory(methodArchive, unparsed, size);
                srcml_unit* methodUnit = srcml_archive_read_unit(methodArchive);
                
                std::string functionXpath =  "(" + XPATH_TRANSFORMATION.getXpath(unitLanguage,"free_function") + ")[" + std::to_string(i + 1) + "]";
                methodModel function(methodArchive, methodUnit, functionXpath, unitLanguage, "", j);
                if (function.IsStatic())
                    freeFunctions.push_back(function);   
                else {
                    // Removes namespaces if any
                    std::string functionName = function.getName();  
                    removeNamespace(functionName, false, "C++");

                    // Get the class name (if any). Else, it is a free function
                    size_t isClassName = functionName.find("::");
                    if (isClassName != std::string::npos) { // Class found, it is a method       
                        std::string className = functionName.substr(0, isClassName); 
                        auto resultS = classCollection.find(className);
                        if (resultS != classCollection.end())      
                            resultS->second.addMethod(function);                         
                        else { // Case specialized template method belongs to the generic template class
                            className = className.substr(0, className.find("<"));
                            resultS = classCollection.find(className);
                            if (resultS != classCollection.end()) 
                                resultS->second.addMethod(function);                       
                            else 
                                freeFunctions.push_back(function);                                   
                        }
                    }
                    else 
                        freeFunctions.push_back(function);   
                }              
                free(unparsed); 
                srcml_unit_free(methodUnit);
                srcml_archive_close(methodArchive);
                srcml_archive_free(methodArchive);
            }
            srcml_transform_free(result);
            srcml_clear_transforms(archive);            
        }
    }
}

// Finds inherited attributes 
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
void classModelCollection::findInheritedAttributes(classModel& c) {   
    const std::string& unitLanguage = c.getUnitLanguage();
    c.setVisited(true); 
    const std::unordered_map<std::string, std::string>& parentClassName =  c.getParentClassName();

    for (const auto& pair : parentClassName){
        std::string parClassName = pair.first;
        auto result = classCollection.find(parClassName);
        if (result != classCollection.end()) {
            if (result->second.HasInherited() && !result->second.IsVisited()) {
                c.inheritAttribute(result->second.getNonPrivateAndInheritedAttribute(), pair.second); 
                result->second.setVisited(true);
            }
                
            else if (!result->second.IsVisited()) {
                findInheritedAttributes(result->second);                     
                c.inheritAttribute(result->second.getNonPrivateAndInheritedAttribute(), pair.second);  
            }
        }       
        else {
            if (unitLanguage == "C++") {
                parClassName = parClassName.substr(0, parClassName.find("<"));
                result = classCollection.find(parClassName);
                if (result != classCollection.end()) {
                    if (result->second.HasInherited() && !result->second.IsVisited()) {
                        c.inheritAttribute(result->second.getNonPrivateAndInheritedAttribute(), pair.second); 
                        result->second.setVisited(true);
                    }
                        
                    else if (!result->second.IsVisited()) {
                        findInheritedAttributes(result->second);                     
                        c.inheritAttribute(result->second.getNonPrivateAndInheritedAttribute(), pair.second);  
                    }
                }              
            }
            else {  
                removeBetweenComma(parClassName, true);
                auto resultG = classGenerics.find(parClassName);
                if (resultG != classGenerics.end()) {
                    auto resultM = classCollection.find(resultG->second);
                    if (resultM != classCollection.end()) {
                        if (resultM->second.HasInherited() && !resultM->second.IsVisited()) {
                            c.inheritAttribute(resultM->second.getNonPrivateAndInheritedAttribute(), pair.second); 
                            resultM->second.setVisited(true);
                        }
                        else if (!resultM->second.IsVisited()) {
                            findInheritedAttributes(resultM->second);                     
                            c.inheritAttribute(resultM->second.getNonPrivateAndInheritedAttribute(), pair.second);  
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
            if (result->second.HasInherited() && !result->second.IsVisited()) {
                c.appendInheritedMethod(result->second.getMethodSignatures(), result->second.getInheritedMethodSignatures()); 
                result->second.setVisited(true);
            }
                
            else if (!result->second.IsVisited()) {
                findInheritedMethods(result->second);                     
                c.appendInheritedMethod(result->second.getMethodSignatures(), result->second.getInheritedMethodSignatures());  
            }
        }       
        else {
            if (unitLanguage == "C++") {
                parClassName = parClassName.substr(0, parClassName.find("<"));
                result = classCollection.find(parClassName);
                if (result != classCollection.end()) {
                    if (result->second.HasInherited() && !result->second.IsVisited()) {
                        c.appendInheritedMethod(result->second.getMethodSignatures(), result->second.getInheritedMethodSignatures()); 
                        result->second.setVisited(true);
                    }
                        
                    else if (!result->second.IsVisited()) {
                        findInheritedMethods(result->second);                     
                        c.appendInheritedMethod(result->second.getMethodSignatures(), result->second.getInheritedMethodSignatures()); 
                    }
                }              
            }
            else {  
                removeBetweenComma(parClassName, true);
                auto resultG = classGenerics.find(parClassName);
                if (resultG != classGenerics.end()) {
                    auto resultM = classCollection.find(resultG->second);
                    if (resultM != classCollection.end()) {
                        if (resultM->second.HasInherited() && !resultM->second.IsVisited()) {
                            c.appendInheritedMethod(resultM->second.getMethodSignatures(), resultM->second.getInheritedMethodSignatures());  
                            resultM->second.setVisited(true);
                        }
                        else if (!resultM->second.IsVisited()) {
                            findInheritedMethods(resultM->second);                     
                            c.appendInheritedMethod(resultM->second.getMethodSignatures(), resultM->second.getInheritedMethodSignatures());   
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
        {"command", 0},
        {"boundary", 0},
        {"factory", 0},
        {"control", 0},
        {"pure-control", 0},
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

    int total = 0;

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
        outS << "Class Stereotype,Stereotype Count" <<'\n';
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
     
    int factory = methodStereotypes["factory"];

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
    outC << "Total" << "," << total << '\n';

    outU.close();
    outV.close();
    outM.close();     
    outS.close();
    outC.close();
}

// Optional TXT report file containing stereotype information
// Format:
//
// Class Name:              Class Stereotype:
// ...                      ...
// Method Name:             Method Stereotype:
// ...                      ...
//---------------------------------------------------------------
// Class Name:              Class Stereotype:
// ...                      ...
// Method Name:             Method Stereotype:
// ...                      ...
//
void classModelCollection::outputTxtReportFile(std::stringstream& stringStream, classModel& c) {
    const int WIDTH = 70;
    const std::string line(WIDTH * 2, '-');
    const std::vector<methodModel>& methods = c.getMethods();
    
    auto setw_width = std::setw(WIDTH);
    std::string className = c.getName()[1];
    stringStream << std::left << setw_width << "Class Name:" << setw_width << "Class Stereotype:" << '\n';
    stringStream << std::left << setw_width <<  className << setw_width << c.getStereotype() << "\n\n";
    stringStream << std::left << setw_width << "Method Name:" << setw_width << "Method Stereotype:" << '\n';
    for (const auto& m : methods) {
        std::string methodName = m.getName();
        stringStream << std::left << setw_width << methodName; 
        stringStream << setw_width << m.getStereotype() << '\n';
    }
    stringStream << line << '\n'; 
}

// Optional CSV report file containing stereotype information
//
void classModelCollection::outputCsvReportFile(std::ofstream& out, classModel& c) {
    const std::string& classStereotype =  c.getStereotype(); 

    std::string className = c.getName()[1];
    className = "\"" + className + "\"";
    
    const std::vector<methodModel>& method = c.getMethods();
    for (const auto& m : method) {
        std::string methodName = m.getName();
        methodName = "\"" + methodName + "\"";

        const std::string& methodStereotype = m.getStereotype();

        out << className << "," << "\"" + classStereotype + "\"" << "," << methodName << ",";
        out << "\"" + methodStereotype + "\"" <<  "," << '\n';          
    }
}

//  Add in stereotype attribute on <class> and <function>
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


void classModelCollection::computeFreeFunctionsStereotypes() {
    for (methodModel& f : freeFunctions) {
        std::string methodName = f.getName();
        // main
        if (methodName == "main" || methodName == "Main")
            f.setStereotype("main");
        // empty
        else if (f.IsEmpty()) 
                f.setStereotype("empty");
        else {
            // predicate
            bool returnType = false;
            const std::string& returnTypeParsed = f.getReturnTypeParsed();
            const std::string& unitLanguage = f.getUnitLanguage();

            if (unitLanguage == "C++")
                returnType = (returnTypeParsed == "bool");
            else if (unitLanguage == "C#")
                returnType = (returnTypeParsed == "bool") || 
                            (returnTypeParsed == "Boolean");
            else if (unitLanguage == "Java")
                returnType = (returnTypeParsed == "boolean");

            bool hasComplexReturnExpr = f.IsParameterNotReturned();
            bool isParamaterUsed = f.IsParameterUsed();

            if (returnType && hasComplexReturnExpr && isParamaterUsed)
                f.setStereotype("predicate"); 

            // property
            returnType = false;
            if (unitLanguage == "C++")
                returnType = (returnTypeParsed != "bool" && returnTypeParsed != "void" && returnTypeParsed != "");
            else if (unitLanguage == "C#")
                returnType = (returnTypeParsed != "bool" && returnTypeParsed != "Boolean" &&
                            returnTypeParsed != "void" && returnTypeParsed != "Void" && returnTypeParsed != "");
            else if (unitLanguage == "Java")
                returnType = (returnTypeParsed != "boolean" && returnTypeParsed != "void" && 
                            returnTypeParsed != "Void" && returnTypeParsed != "");

            if (returnType && hasComplexReturnExpr && isParamaterUsed)
                f.setStereotype("property"); 
            
            // factory
            if(f.IsFactory() || f.IsStrictFactory())
                f.setStereotype("factory");   

            // global-command
            bool globalOrStaticChanged = f.IsGlobalOrStaticChanged();
            if (globalOrStaticChanged)
                f.setStereotype("global-command");
            
            // command
            bool parameterModified = f.IsParameterRefChanged();
            if (parameterModified && !globalOrStaticChanged)
                f.setStereotype("command");

            // literal
            if (!isParamaterUsed)
                f.setStereotype("literal");

            // wrapper
            
            bool hasCalls = (f.getFunctionCalls().size() + f.getMethodCalls().size()) > 0;

            if (!parameterModified && hasCalls)
                f.setStereotype("wrapper");



            // empty
            if (f.getStereotype() == "") 
                f.setStereotype("unclassified");
        }
        XPATH_LIST[f.getUnitNumber()].insert({f.getXpath(), f.getStereotype()});
    }
}