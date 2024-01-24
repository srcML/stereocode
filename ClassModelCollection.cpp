// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModelCollection.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "ClassModelCollection.hpp"

classModelCollection::classModelCollection (srcml_archive* archive, srcml_archive* outputArchive, 
                                            std::vector<srcml_unit*> units, std::ofstream& reportFile, 
                                            std::string inputFile, bool outputReport, bool outputViews) {
    std::string InputFileNoExt = "";
    std::ofstream out, outU, outV, outM, outS, outC;
    if (outputViews) {
        InputFileNoExt = inputFile.substr(0, inputFile.size() - 4) + "_";
        out.open(InputFileNoExt + "method_class_stereotypes.csv");
        outU.open(InputFileNoExt + "unique_method_view.csv");
        outV.open(InputFileNoExt + "unique_class_view.csv");
        outM.open(InputFileNoExt + "method_view.csv");
        outS.open(InputFileNoExt + "class_view.csv");
        outC.open(InputFileNoExt + "category_view.csv");
    }
        
    findClassInfo(archive, units); // Collects class info + methods defined internally 
    findFreeFunction(archive, units); // Coo
    for (auto& pair : classCollection) {
        findInheritedAttribute(pair.second);
        pair.second.setInherited(true);
        for (auto& pairS : classCollection)
            pairS.second.setVisited(false);
    } 
    bool headLine = false;
    for (auto it = classCollection.begin(); it != classCollection.end();) {
        for (auto& pairM : it->second.getMethod()) {
            pairM.findMethodData(it->second.getAttribute(), it->second.getParentClassName(),
                                 it->second.getNameParsed());
        }       
        it->second.ComputeMethodStereotype();
        it->second.ComputeClassStereotype();

        if (outputReport)
            outputReportFile(reportFile, it->second);       
        
        if (outputViews)
            allView(out, it->second, headLine); 

        it = classCollection.erase(it);
        if (!headLine) headLine = true;
    }

    if (outputViews) { 
        method_class_unique_views(outU, outV, outM, outS, outC);
        out.close();
        outU.close();
        outV.close();
        outM.close();     
        outS.close();
        outC.close();
    }

    outputWithStereotypes(archive, outputArchive, units);
}

// Finds classes in an archive
// Duplicate classes (classes with the same name) are not combined.
// Ignores nested classes including anonymous classes in Java
// C++ could have a nested class defined externally to its parent
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
void classModelCollection::findClassInfo(srcml_archive* archive, std::vector<srcml_unit*> units) {
    for (size_t j = 0; j < units.size(); j++) {
        std::string unitLanguage = srcml_unit_get_language(units[j]);   
        if (unitLanguage == "C++" || unitLanguage == "C#" || unitLanguage == "Java"){
            // Primitives change depending on the language of the unit
            PRIMITIVES.setLanguage(unitLanguage); 
            srcml_append_transform_xpath(archive, "//src:class[not(ancestor::src:class) and not(src:name/src:operator='::')]"); 

            srcml_transform_result* result = nullptr;
            srcml_unit_apply_transforms(archive, units[j], &result);
            int n = srcml_transform_get_unit_size(result);
            srcml_unit* resultUnit = nullptr;
            for (int i = 0; i < n; ++i) {    
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

                classModel c(classArchive, unit); 
                std::string className = c.getName();
                trimWhitespace(className);        
                std::string classXpath = "//src:class[not(ancestor::src:class) and not(src:name/src:operator='::')][" + std::to_string(i + 1) + "]"; 

                if (unitLanguage != "C++") {
                    // Needed for inheritance in Java and C#
                    std::string classNameCommaRemoved = className;
                    removeBetweenComma(className);
                    classGeneric.insert({classNameCommaRemoved, className}); 
                }

                // Only insert if class doesn't exists yet
                auto resultS = classCollection.insert({className, c}); 
                
                // True if class doesn't exist and was inserted
                if (resultS.second) 
                    resultS.first->second.findClassData(classArchive, unit, classXpath, unitLanguage, j); 
                
                // Class exists already and could be a partial class (C# only) or a duplicate
                else
                    resultS.first->second.findClassData(classArchive, unit, classXpath, unitLanguage, j);               
                
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
void classModelCollection::findFreeFunction(srcml_archive* archive, std::vector<srcml_unit*> units){
    for (size_t j = 0; j < units.size(); j++){
        std::string unitLanguage = srcml_unit_get_language(units[j]); 
        if (unitLanguage == "C++"){      
            srcml_append_transform_xpath(archive, "//src:function[not(ancestor::src:class)]");
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

                std::string functionXpath = "//src:function[not(ancestor::src:class)][" + std::to_string(i+1) + "]";

                methodModel function(methodArchive, methodUnit, functionXpath, unitLanguage, j);

                // Removes namespaces if any
                std::string functionName = function.getName();
                removeNamespace(functionName, false, "C++");
                trimWhitespace(functionName);    

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
                            freeFunction.push_back(functionXpath);                                   
                    }
                }
                else {
                    // Method could be a friend function
                    if (!isFriendFunction(function)) 
                        freeFunction.push_back(functionXpath);                
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

// Finds inherited attributes for classes that inherit
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
    const std::unordered_set<std::string>& parentClassName =  c.getParentClassName();

    for (std::string parClassName : parentClassName){
        removeNamespace(parClassName, true, unitLanguage);
        trimWhitespace(parClassName); 
        auto result = classCollection.find(parClassName);
        if (result != classCollection.end()) {
            if (!result->second.HasInherited() && !result->second.IsVisited()) {
                if (result->second.getParentClassName().size() > 0) 
                    findInheritedAttribute(result->second);            
            }  
            c.appendInheritedAttribute(result->second.getNonPrivateAttribute());       
        }       
        else {
            if (unitLanguage == "C++") {
                parClassName = parClassName.substr(0, parClassName.find("<"));
                result = classCollection.find(parClassName);
                if (result != classCollection.end()) {
                    if (!result->second.HasInherited() && !result->second.IsVisited()) {
                        if (result->second.getParentClassName().size() > 0) 
                            findInheritedAttribute(result->second);    
                    }   
                    c.appendInheritedAttribute(result->second.getNonPrivateAttribute());
                }              
            }
            else {  
                removeBetweenComma(parClassName);
                auto resultG = classGeneric.find(parClassName);
                if (resultG != classGeneric.end()) {
                    auto resultM = classCollection.find(resultG->second);
                    if (resultM != classCollection.end()) {
                        if (!resultM->second.HasInherited() && !resultM->second.IsVisited()) {
                            if (resultM->second.getParentClassName().size() > 0) 
                                findInheritedAttribute(resultM->second);    
                        }  
                        c.appendInheritedAttribute(resultM->second.getNonPrivateAttribute());
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
// If two classes declare the same friend function, the friend function can be used with both classes    
//
bool classModelCollection::isFriendFunction(methodModel& function) {
    std::string functionSignature = function.getReturnType();
    functionSignature += function.getName() + "(";
    const std::vector<Variable>& parameter = function.getParameterOrdered();

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

//  Copy unit and add in stereotype attribute on <class> and <function>
//  Example: <function st:stereotype="get"> ... </function>
//           <class st:stereotype="boundary"> ... ></class>
//
void classModelCollection::outputWithStereotypes(srcml_archive* archive, srcml_archive* outputArchive, std::vector<srcml_unit*> units) {   
    for (size_t i = 0; i < units.size(); i++){ 
        bool isTransform = false;
        for (const auto& pairV : xpathList[i]) {  
            srcml_append_transform_xpath_attribute(archive, pairV.first.c_str(), "st",
                                    "http://www.srcML.org/srcML/stereotype",
                                    "stereotype", pairV.second.c_str());
            isTransform = true;
            
        }  
        if (isTransform){
            srcml_transform_result* result = nullptr; 
            srcml_unit_apply_transforms(archive, units[i], &result);  
            srcml_unit* tempUnit = srcml_transform_get_unit(result, 0);
            srcml_archive_write_unit(outputArchive, tempUnit);
            
            srcml_transform_free(result); 
            srcml_clear_transforms(archive);   
        }
        else{
            srcml_archive_write_unit(outputArchive, units[i]);
            srcml_clear_transforms(archive);  
        } 
    }
}

// Outputs a report file for a class (tab separated)
// class name || method || stereotypes
//
void classModelCollection::outputReportFile(std::ofstream& out, classModel& c) {
    const int WIDTH = 70;
    const std::string line(WIDTH*2, '-');
    if (out.is_open()) {
        const  std::vector<methodModel>& method = c.getMethod();
        out << std::left << std::setw(WIDTH) << "Class name:" << std::setw(WIDTH) << "Class stereotype:" << std::endl;
        out << std::left << std::setw(WIDTH)  <<  c.getName() << std::setw(WIDTH) << c.getStereotype() << std::endl << std::endl;  
        out << std::left << std::setw(WIDTH) << "Method name:" << std::setw(WIDTH) << "Method stereotype:" << std::endl; 
        for (const auto& m : method) {
            std::string methodName = m.getName();
            WStoBlank(methodName);
            out << std::left << std::setw(WIDTH) << methodName; 
            out << std::setw(WIDTH) << m.getStereotype() << std::endl; 
        }
        out << line << std::endl;
    }
}

// Used to generate optional views for the distribution of method and class stereotypes
// These views are generated in .csv files
//
void classModelCollection::method_class_unique_views(std::ofstream& outU, std::ofstream& outV, 
                                                     std::ofstream& outM, std::ofstream& outS, std::ofstream& outC) {
    int total = 0;

    // Unique Method View
    if (outU.is_open()) {
        outU << "Unique Method Stereotype,Method Count" <<std::endl;
        for (auto& pair : uniqueMethodStereotypesView){
            outU << pair.first << ",";
            outU << pair.second << std::endl;
            total += pair.second;
        }
        outU << "Total" << "," << total;
    }

    // Unique Class View
    if (outV.is_open()) {   
        outV << "Unique Class Stereotype,Class Count" <<std::endl;
        total = 0;
        for (auto& pair : uniqueClassStereotypesView){
            outV << pair.first << ",";
            outV << pair.second << std::endl;
            total += pair.second;
        }
        outV << "Total" << "," << total;
    }

    // Method View
    //
    if (outM.is_open()) { 
        outM << "Method Stereotype,Method Count" <<std::endl;
        total = 0;
        for (auto& pair : methodStereotypesView){
            outM << pair.first << ",";
            outM << pair.second << std::endl;
            total += pair.second;
        }
        outM << "Total" << "," << total;
    }

    // Class View
    //
    if (outS.is_open()) {
        outS << "Class Stereotype,Class Count" <<std::endl;
        total = 0;
        for (auto& pair : classStereotypesView){
            outS << pair.first << ",";
            outS << pair.second << std::endl;
            total += pair.second;
        }
        outS << "Total" << "," << total;
    }

    // Category view
    //
    int getters = methodStereotypesView["get"] + methodStereotypesView["non-const-get"];
    int accessors = getters + methodStereotypesView["predicate"] + methodStereotypesView["non-const-predicate"] +
                    methodStereotypesView["property"] + methodStereotypesView["non-const-property"] +
                    methodStereotypesView["accessor"] + methodStereotypesView["non-const-accessor"]; 

    int setters = methodStereotypesView["set"];     
    int commands = methodStereotypesView["command"] + methodStereotypesView["non-void-command"];           
    int mutators = setters + commands;

    int controllers = methodStereotypesView["controller"];
    int collaborator =  methodStereotypesView["collaborator"]; 
    int collaborators = controllers + collaborator;
     
    int factory = methodStereotypesView["factory"];

    int degenerates = methodStereotypesView["empty"] + methodStereotypesView["stateless"] + methodStereotypesView["wrapper"]; 

    int unclassified = methodStereotypesView["unclassified"];

    total = accessors + mutators + factory + collaborators + degenerates + unclassified;
    outC << "Stereotype Category,Method Count" <<std::endl;
    outC << "Accessors" << "," << accessors << std::endl;
    outC << "Mutators" << "," << mutators << std::endl;
    outC << "Creational" << "," << factory << std::endl;
    outC << "Collaborational" << "," << collaborators << std::endl;
    outC << "Degenerate" << "," << degenerates << std::endl;
    outC << "Unclassified" << "," << unclassified << std::endl;
    outC << "Total" << "," << total << std::endl;
}


// Used to generate optional views for the distribution of method and class stereotypes
// These views are generated in .csv files
//
void classModelCollection::allView(std::ofstream& out, classModel& c, bool headLine) {
    if (!headLine) out << "Class Name,Class Stereotype,Method Name,Method Stereotype,Method Stereotype Count" << std::endl;
    if (out.is_open()) {   
        std::string classStereotype =  c.getStereotype(); 
        uniqueClassStereotypesView[classStereotype]++; // for class unique View 

        for (const std::string& s : c.getStereotypeList()) // For class view
            classStereotypesView[s]++;   

        std::string className = c.getName();
        WStoBlank(className);
        commaConversion(className);
        
        const  std::vector<methodModel>& method = c.getMethod();
        
        for (const auto& m : method) {
            std::string methodName = m.getName();
            WStoBlank(methodName);
            commaConversion(methodName);

            std::string methodStereotype = m.getStereotype();
            uniqueMethodStereotypesView[methodStereotype]++;

            for (const std::string& s : m.getStereotypeList())
                methodStereotypesView[s]++;    

            out << className << "," << classStereotype << "," << methodName << ",";
            out << methodStereotype <<  ",";
            out << m.getStereotypeList().size() << std::endl;          
        }
    }
}



