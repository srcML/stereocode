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

classModelCollection::classModelCollection (srcml_archive* archive, srcml_archive* outputArchive, 
                                            std::vector<srcml_unit*>& units, 
                                            const std::string& inputFile, bool outputReport, bool outputViews) {  
    // Initialize the lists of specifiers
    createSpecifierList();
         
    // Collects class info + methods defined internally to a class
    findClassInfo(archive, units); 

    // Collects free functions, friend functions, and methods defined externally to a class
    findFreeFunction(archive, units); 

    // Finds inherited attributes for each class
    // Creates a set of method signatures for each class (Needed for call filtering)
    for (auto& pair : classCollection) {
        pair.second.buildMethodSignature();
        findInheritedAttribute(pair.second);
        pair.second.setInherited(true);
        for (auto& pairS : classCollection)
            pairS.second.setVisited(false);
    } 
    
    // Appends the inherited private attributes (C++ only)
    for (auto& pair : classCollection) {
        pair.second.setInherited(false); // Resets inheritance for findInheritedMethod()
        if (pair.second.getUnitLanguage() == "C++")
            pair.second.appendInheritedPrivateAttribute();   
    }

    // Finds inherited methods
    for (auto& pair : classCollection) {
        findInheritedMethod(pair.second);
        pair.second.setInherited(true);
        for (auto& pairS : classCollection)
            pairS.second.setVisited(false);
    } 

    // Analyze all methods for each class statically
    for (auto& pair : classCollection) {
        PRIMITIVES.setLanguage(pair.second.getUnitLanguage()); 
        IGNORED_CALLS.setLanguage(pair.second.getUnitLanguage());
        std::vector<methodModel>& methods = pair.second.getMethod();

        for (auto& m : methods)
             m.findMethodData(pair.second.getAttribute(), pair.second.getMethodSignature(), 
                              pair.second.getInheritedMethodSignature(), pair.second.getName()[3]);
    }

    // Compute method and stereotypes here
    for (auto& pair : classCollection) {
        pair.second.ComputeMethodStereotype();
        pair.second.ComputeClassStereotype();
    }

    // Optionally output a .txt report file
    if (outputReport) {
        std::ofstream reportFile(inputFile + ".report.txt");
        std::stringstream stringStream;
        for (auto& pair : classCollection)
            outputReportFile(stringStream, pair.second);
        reportFile << stringStream.str();
        reportFile.close();        
    }

    
    if (outputViews) {
        std::string InputFileNoExt = inputFile.substr(0, inputFile.size() - 4) + "_";
        std::ofstream out, outU, outV, outM, outS, outC;
        out.open(InputFileNoExt + "method_class_stereotypes.csv");
        out << "Class Name,Class Stereotype,Method Name,Method Stereotype,Method Stereotype Count" << '\n';
        for (auto& pair : classCollection)
            allView(out, pair.second);        
 
        outU.open(InputFileNoExt + "unique_method_view.csv");
        outV.open(InputFileNoExt + "unique_class_view.csv");
        outM.open(InputFileNoExt + "method_view.csv");
        outS.open(InputFileNoExt + "class_view.csv");
        outC.open(InputFileNoExt + "category_view.csv");
        method_class_unique_views(outU, outV, outM, outS, outC);
        out.close();
        outU.close();
        outV.close();
        outM.close();     
        outS.close();
        outC.close();
    }

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
    
    for (auto& pair : results)  
        srcml_transform_free(pair.second);   
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

                PRIMITIVES.setLanguage(unitLanguage); 
                std::string classXpath = XPATH_TRANSFORMATION.getXpath(unitLanguage,"class") + "[" + std::to_string(i + 1) + "]";

                classModel c(classArchive, unit, unitLanguage, classXpath, j);        

                // Needed for inheritance in Java and C#
                if (unitLanguage != "C++") classGeneric.insert({c.getName()[2], c.getName()[1]}); 
    
                classCollection.insert({c.getName()[1], c});  
                          
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
//      Function could be a friend or a free function
//          Foo(){}, externalClass::Foo(){}, namespaceA::Foo(){}, operator<<(){}
//
void classModelCollection::findFreeFunction(srcml_archive* archive, std::vector<srcml_unit*> units) {
    for (size_t j = 0; j < units.size(); j++){
        std::string unitLanguage = srcml_unit_get_language(units[j]); 
        if (unitLanguage == "C++") {     
            // std::string xpath = "//src:function[not(ancestor::src:class or ancestor::src:struct)]";
            std::string xpath = "//*[self::src:function or self::src:constructor or self::src:destructor][not(ancestor::src:class or ancestor::src:struct)]";
            srcml_append_transform_xpath(archive, xpath.c_str());
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
                
                std::string functionXpath =   "(" + xpath + ")[" + std::to_string(i + 1) + "]";
                methodModel function(methodArchive, methodUnit, functionXpath, unitLanguage, "", j);

                // Removes namespaces if any
                std::string functionName = function.getName();
                trimWhitespace(functionName);    
                removeNamespace(functionName, false, "C++");
 
                // Get the class name (if any). Else, it is a free function or a friend function name
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
                            freeFunction.push_back(function);                                   
                    }
                }
                else {
                    // Method could be a friend function
                    if(!isFriendFunction(function))
                        freeFunction.push_back(function);                
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
void classModelCollection::findInheritedAttribute(classModel& c) {   
    std::string unitLanguage = c.getUnitLanguage();
    c.setVisited(true); 
    const std::unordered_map<std::string, std::string>& parentClassName =  c.getParentClassName();

    for (const auto& pair : parentClassName){
        std::string parClassName = pair.first;
        auto result = classCollection.find(parClassName);
        if (result != classCollection.end()) {
            if (result->second.HasInherited() && !result->second.IsVisited()) {
                c.appendInheritedAttribute(result->second.getNonPrivateAttribute(), pair.second); 
                result->second.setVisited(true);
            }
                
            else if (!result->second.IsVisited()) {
                findInheritedAttribute(result->second);                     
                c.appendInheritedAttribute(result->second.getNonPrivateAttribute(), pair.second);  
            }
        }       
        else {
            if (unitLanguage == "C++") {
                parClassName = parClassName.substr(0, parClassName.find("<"));
                result = classCollection.find(parClassName);
                if (result != classCollection.end()) {
                    if (result->second.HasInherited() && !result->second.IsVisited()) {
                        c.appendInheritedAttribute(result->second.getNonPrivateAttribute(), pair.second); 
                        result->second.setVisited(true);
                    }
                        
                    else if (!result->second.IsVisited()) {
                        findInheritedAttribute(result->second);                     
                        c.appendInheritedAttribute(result->second.getNonPrivateAttribute(), pair.second);  
                    }
                }              
            }
            else {  
                removeBetweenComma(parClassName, true);
                auto resultG = classGeneric.find(parClassName);
                if (resultG != classGeneric.end()) {
                    auto resultM = classCollection.find(resultG->second);
                    if (resultM != classCollection.end()) {
                        if (resultM->second.HasInherited() && !resultM->second.IsVisited()) {
                            c.appendInheritedAttribute(resultM->second.getNonPrivateAttribute(), pair.second); 
                            resultM->second.setVisited(true);
                        }
                        else if (!resultM->second.IsVisited()) {
                            findInheritedAttribute(resultM->second);                     
                            c.appendInheritedAttribute(resultM->second.getNonPrivateAttribute(), pair.second);  
                        }
                    }
                }
            }      
        }         
    }
}

// Finds inherited methods
//
void classModelCollection::findInheritedMethod(classModel& c) {   

    std::string unitLanguage = c.getUnitLanguage();
    c.setVisited(true); 
    const std::unordered_map<std::string, std::string>& parentClassName =  c.getParentClassName();
    for (const auto& pair : parentClassName){
        std::string parClassName = pair.first;
        auto result = classCollection.find(parClassName);
        if (result != classCollection.end()) {
            if (result->second.HasInherited() && !result->second.IsVisited()) {
                c.appendInheritedMethod(result->second.getMethodSignature(), result->second.getInheritedMethodSignature()); 
                result->second.setVisited(true);
            }
                
            else if (!result->second.IsVisited()) {
                findInheritedMethod(result->second);                     
                c.appendInheritedMethod(result->second.getMethodSignature(), result->second.getInheritedMethodSignature());  
            }
        }       
        else {
            if (unitLanguage == "C++") {
                parClassName = parClassName.substr(0, parClassName.find("<"));
                result = classCollection.find(parClassName);
                if (result != classCollection.end()) {
                    if (result->second.HasInherited() && !result->second.IsVisited()) {
                        c.appendInheritedMethod(result->second.getMethodSignature(), result->second.getInheritedMethodSignature()); 
                        result->second.setVisited(true);
                    }
                        
                    else if (!result->second.IsVisited()) {
                        findInheritedMethod(result->second);                     
                        c.appendInheritedMethod(result->second.getMethodSignature(), result->second.getInheritedMethodSignature()); 
                    }
                }              
            }
            else {  
                removeBetweenComma(parClassName, true);
                auto resultG = classGeneric.find(parClassName);
                if (resultG != classGeneric.end()) {
                    auto resultM = classCollection.find(resultG->second);
                    if (resultM != classCollection.end()) {
                        if (resultM->second.HasInherited() && !resultM->second.IsVisited()) {
                            c.appendInheritedMethod(resultM->second.getMethodSignature(), resultM->second.getInheritedMethodSignature());  
                            resultM->second.setVisited(true);
                        }
                        else if (!resultM->second.IsVisited()) {
                            findInheritedMethod(resultM->second);                     
                            c.appendInheritedMethod(resultM->second.getMethodSignature(), resultM->second.getInheritedMethodSignature());   
                        }
                    }
                }
            }      
        }         
    }
}


// C++ only
// Determines whether a function is a friend by comparing its signature
//  to the list of friend function signatures in each class
// A friend function is not tied to a specific class; rather, it is associated with a specific declaration 
// If two classes  declare the same friend function, the friend function can be used with both classes
//
bool classModelCollection::isFriendFunction(methodModel& function) {

    function.findFriendData(); 

    std::string functionSignature = function.getReturnType();
    functionSignature += function.getName() + "(";
    const std::vector<variable>& parameter = function.getParameterOrdered();

    for (size_t i = 0; i < parameter.size(); i++) {
        functionSignature += parameter[i].getType();
        if (i < parameter.size() - 1) 
            functionSignature += ",";   
    }          
    functionSignature +=  ")";
    functionSignature += function.getConst();

    trimWhitespace(functionSignature);

    bool added = false;
    for (auto& pair : classCollection) {
        const std::unordered_set<std::string>& friendFuncDecl = pair.second.getFriendFunctionDecl();
        if (friendFuncDecl.find(functionSignature) != friendFuncDecl.end()) {
            pair.second.addMethod(function);
            if (!added) added = true;
        }  
    }
    return added;
}



// Outputs a .txt report file containing the stereotypes of all methods and classes
//
void classModelCollection::outputReportFile(std::stringstream& stringStream, classModel& c) {
    const int WIDTH = 70;
    const std::string line(WIDTH * 2, '-');
    const std::vector<methodModel>& method = c.getMethod();
    std::_Setw setw_width = std::setw(WIDTH);
    std::string className = c.getName()[1];
    WStoBlank(className);
    stringStream << std::left << setw_width << "Class name:" << setw_width << "Class stereotype:" << '\n';
    stringStream << std::left << setw_width <<  className << setw_width << c.getStereotype() << "\n\n";
    stringStream << std::left << setw_width << "Method name:" << setw_width << "Method stereotype:" << '\n';
    for (const auto& m : method) {
        std::string methodName = m.getName();
        WStoBlank(methodName);
        stringStream << std::left << setw_width << methodName; 
        stringStream << setw_width << m.getStereotype() << '\n';
    }
    stringStream << line << '\n'; 
}

// Used to generate optional views for the distribution of method and class stereotypes
// These views are generated in .csv files
//
void classModelCollection::method_class_unique_views(std::ofstream& outU, std::ofstream& outV, 
                                                     std::ofstream& outM, std::ofstream& outS, std::ofstream& outC) {
    int total = 0;

    std::vector<std::string> method_ordered_keys = {
        "get", "predicate", "property", "void-accessor", "set", "command", "non-void-command", 
        "collaborator", "controller", "constructor", "copy-constructor", "destructor", "factory", 
        "empty", "stateless", "wrapper", "unclassified"
    };
    
    std::vector<std::string> class_ordered_keys = {
        "entity", "minimal-entity", "data-provider", "command", "boundary", "factory", 
        "control", "pure-control", "large-class", "lazy-class", "degenerate", "data-class", 
        "small-class", "unclassified", "empty"
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
        outM << "Method Stereotype,Method Count" <<'\n';
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
    int collaborator =  methodStereotypes["collaborator"]; 
    int collaborators = controllers + collaborator;
     
    int factory = methodStereotypes["factory"];

    int degenerates = methodStereotypes["empty"] + methodStereotypes["stateless"] + methodStereotypes["wrapper"]; 

    int unclassified = methodStereotypes["unclassified"];

    total = accessors + mutators + factory + collaborators + degenerates + unclassified;
    outC << "Stereotype Category,Method Count" <<'\n';
    outC << "Accessors" << "," << accessors << '\n';
    outC << "Mutators" << "," << mutators << '\n';
    outC << "Creational" << "," << factory << '\n';
    outC << "Collaborational" << "," << collaborators << '\n';
    outC << "Degenerate" << "," << degenerates << '\n';
    outC << "Unclassified" << "," << unclassified << '\n';
    outC << "Total" << "," << total << '\n';
}


// Generate an all-one-view for all method and class stereotypes as a .csv file
// Count the stereotypes to generate the other views
//
void classModelCollection::allView(std::ofstream& out, classModel& c) {
    if (out.is_open()) {   
        const std::string& classStereotype =  c.getStereotype(); 
        uniqueClassStereotypesView[classStereotype]++; // for class unique View 

        for (const std::string& s : c.getStereotypeList()) // For class view
            classStereotypes[s]++;   

        std::string className = c.getName()[1];
        WStoBlank(className);
        className = "\"" + className + "\"";
        
        const std::vector<methodModel>& method = c.getMethod();
        
        for (const auto& m : method) {
            std::string methodName = m.getName();
            WStoBlank(methodName);
            methodName = "\"" + methodName + "\"";
            
            const std::string& methodStereotype = m.getStereotype();
            uniqueMethodStereotypesView[methodStereotype]++;

            for (const std::string& s : m.getStereotypeList())
                methodStereotypes[s]++;    

            out << className << "," << classStereotype << "," << methodName << ",";
            out << methodStereotype <<  ",";
            out << m.getStereotypeList().size() << '\n';          
        }
    }
}

//  Add in stereotype attribute on <class> and <function>
//  Example: <function st:stereotype="get"> ... </function>
//           <class st:stereotype="boundary"> ... ></class>
//
void classModelCollection::outputWithStereotypes(srcml_unit* unit, std::map<int, srcml_unit*>& transformedUnits,
                                                int unitNumber, const std::unordered_map<std::string, std::string>& xpathPair,
                                                std::unordered_map<int, srcml_transform_result*>& results, std::mutex& mu) {   
    // std::string xsltStart = R"**(<xsl:stylesheet
    // xmlns="http://www.srcML.org/srcML/src"
    // xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    // xmlns:src="http://www.srcML.org/srcML/src" 
    // xmlns:cpp="http://www.srcML.org/srcML/cpp"
    // xmlns:st="http://www.srcML.org/srcML/stereotype"
    // xmlns:func="http://exslt.org/functions"
    // extension-element-prefixes="func"
    // exclude-result-prefixes="src st cpp"
    // version="1.0">
    
    // <xsl:output method="xml" version="1.0" encoding="UTF-8" standalone="yes"/>
    // <xsl:template match="@*|node()"><xsl:copy><xsl:apply-templates select="@*|node()"/></xsl:copy></xsl:template>)**";
 
    // xsltStart += R"**(
    // <func:function name="src:last_ws">
    //     <xsl:param name="s"/>
    //     <xsl:choose>
    //     <xsl:when test="contains($s, '&#xa;')">
    //         <func:result select="src:last_ws(substring-after($s, '&#xa;'))"/>
    //     </xsl:when>
    //     <xsl:otherwise>
    //         <func:result select="$s"/>
    //     </xsl:otherwise>
    //     </xsl:choose>
    // </func:function>)**";

    // for (size_t i = 0; i < units.size(); i++){ 
    //     bool transform = false;
    //     std::string xsltMiddle;
    //     for (const auto& pair : xpathList[i]) { 

    //         xsltMiddle += R"**(
    //         <xsl:template match=")**" + pair.first + R"**(">
    //         <comment type="block">/** @stereotype )**" + pair.second + R"**( */</comment>
    //         <xsl:text>&#xa;</xsl:text>
    //         <xsl:value-of select="src:last_ws(preceding-sibling::text()[1])"/>
    //         <xsl:copy><xsl:apply-templates select="@*|node()"/></xsl:copy>
    //         </xsl:template>)**";      
    //         if (!transform) transform = true;              
    //     }  
    //     if (transform){
    //         std::string xsltEnd = R"**(</xsl:stylesheet>)**";
    //         std::string xslt = xsltStart + xsltMiddle + xsltEnd;
    //         srcml_append_transform_xslt_memory(archive, xslt.c_str(), xslt.size());
            
    //         srcml_transform_result* result = nullptr; 
    //         srcml_unit_apply_transforms(archive, units[i], &result);  
    //         srcml_unit* unit = srcml_transform_get_unit(result, 0);
    //         srcml_archive_write_unit(outputArchive, unit);
            
    //         srcml_transform_free(result);              
    //     }
    //     else {
    //         srcml_archive_write_unit(outputArchive, units[i]);  
    //     } 
    //     srcml_clear_transforms(archive); 
    // }

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
