// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModelCollection.cpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "ClassModelCollection.hpp"

classModelCollection::classModelCollection (srcml_archive* archive, std::vector<srcml_unit*> units) : classModelCollection() {
    findClassInfo(archive, units); 
    findFreeFunction(archive, units); 
    findInheritedAttribute();

    for (auto& pair : classCollection){
        pair.second.ComputeMethodStereotype();
        pair.second.ComputeClassStereotype();
    }
}

// Finds all classes in an archive and performs analysis on these classes
//
// In C++, class names are usually in the form of:
//      myClass  
// or  
//      myClass<type1, type2, ...> for a specialized templated class from myClass
// In C++, a specialized templated class can exist if there is a generic one
//
// In C# and Java, there is no concept of a specialized templated class, but one could define:
//      myClass
// or
//      myClass<T, G, ... >
// Where these two are different classes. First is a normal class and second is a generic 
//
void classModelCollection::findClassInfo(srcml_archive* archive, std::vector<srcml_unit*> units){
    for (size_t j = 0; j < units.size(); j++){
        std::string lang = srcml_unit_get_language(units[j]);    
        if (lang == "C++" || lang == "C#" || lang == "Java"){
            PRIMITIVES.setLanguage(lang); // Primitives change depending on the language of the unit
            srcml_append_transform_xpath(archive, "//src:class[not(ancestor::src:class)]"); 
            srcml_transform_result* result = nullptr;
            srcml_unit_apply_transforms(archive, units[j], &result);
            int n = srcml_transform_get_unit_size(result);
            srcml_unit* resultUnit = nullptr;
            for (int i = 0; i < n; ++i) {    
                resultUnit = srcml_transform_get_unit(result, i);
                srcml_archive* temp = srcml_archive_create();

                char* unparsed = nullptr;
                size_t size = 0;
                srcml_archive_write_open_memory(temp, &unparsed, &size);
                srcml_archive_write_unit(temp, resultUnit);
                srcml_archive_close(temp);
                srcml_archive_free(temp);
                    
                temp = srcml_archive_create();
                srcml_archive_read_open_memory(temp, unparsed, size);
                srcml_unit* unit = srcml_archive_read_unit(temp); // This unit is a class only, not a whole unit
             
                classModel c(temp, unit, lang); // Builds class model with basic analysis
                std::string className = trimWhitespace(c.getClassName());        
                std::string classXpath = "//src:class[not(ancestor::src:class)][" + std::to_string(i + 1) + "]"; 

                if (lang != "C++")
                    classGeneric.insert({className.substr(0, className.find("<")), className}); // Needed for inheritance in Java and C#

                auto result = classCollection.insert({className, c}); // Only insert if class doesn't exists yet
                if (result.second) // True if class doesn't exist and was inserted
                    result.first->second.findClassData(temp, unit, classXpath, j); // Do in-depth class analysis
                else { // Class exists already
                    if (lang == "C#" && c.getIsPartial())  // Class could be a partial class
                        result.first->second.findClassData(temp, unit, classXpath, j);               
                }                                             
                free(unparsed);
                srcml_unit_free(unit);
                srcml_archive_close(temp);
                srcml_archive_free(temp);           
            }   
            srcml_transform_free(result);
            srcml_clear_transforms(archive);   
        }   
    }
}

// C++ only
//
// Finds free functions as well as methods defined externally
// Free functions could also have :: in their names
//  They could belong to a namespace or to an external class (class not defined in the archive),
//   so classes need to be found first 
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
//          Foo(){}, namespaceA::Foo(){}, operator<<(){}, 
//
void classModelCollection::findFreeFunction(srcml_archive* archive, std::vector<srcml_unit*> units){
    PRIMITIVES.setLanguage("C++"); 
    for (size_t j = 0; j < units.size(); j++){
        std::string lang = srcml_unit_get_language(units[j]); 
        if (lang == "C++"){
            std::string lang = srcml_unit_get_language(units[j]);        
            srcml_append_transform_xpath(archive, "//src:function[not(ancestor::src:class)]");
            srcml_transform_result* result = nullptr;
            srcml_unit_apply_transforms(archive, units[j], &result);
            int n = srcml_transform_get_unit_size(result);  

            srcml_unit* resultUnit = nullptr;
            for (int i = 0; i < n; i++) {
                resultUnit = srcml_transform_get_unit(result, i);
                
                srcml_archive* unitArchive = srcml_archive_create();
                char* unparsed = nullptr;
                size_t size = 0;
                srcml_archive_write_open_memory(unitArchive, &unparsed, &size);
                srcml_archive_write_unit(unitArchive, resultUnit);
                srcml_archive_close(unitArchive);
                srcml_archive_free(unitArchive);
                
                unitArchive = srcml_archive_create();
                srcml_archive_read_open_memory(unitArchive, unparsed, size);
                srcml_unit* unit = srcml_archive_read_unit(unitArchive);

                methodModel function(unitArchive, unit); // Builds function model and performs basic analysis on it

                std::string xpathS = "//src:function[not(ancestor::src:class)][" + std::to_string(i+1) + "]";

                // Method names are in the form of:
                //      MyClass<int>::Foo, namespaceA::MyClass<int>::Foo, MyClass::Foo, 
                // Free and friend functions are in the form:
                //      Foo, operator<<
                std::string functionName = trimWhitespace(removeNamespace(function.getName(), false, "C++")); // Removes namespaces if any

                // Get the class name (if any). Else, it is a free function or a friend function name
                size_t isClassName = functionName.find("::");
                if (isClassName != std::string::npos){ // Class found, it is a method       
                    std::string className = functionName.substr(0, isClassName); 
                    if (classCollection.find(className) != classCollection.end()) { 
                        // To avoid adding the method to a class of a different language
                        if (classCollection[className].getUnitLanguage() == "C++") 
                            // Add method and perform the rest of the analysis         
                            classCollection[className].addMethod(unitArchive, unit, function, xpathS, j); 
                    }          
                    else { // Case specialized template method belongs to the generic template class
                        className = className.substr(0, className.find("<"));
                        if (classCollection.find(className) != classCollection.end()){
                            if (classCollection[className].getUnitLanguage() == "C++")   
                                classCollection[className].addMethod(unitArchive, unit, function, xpathS, j);         
                        }
                        else{
                            freeFunction.push_back(xpathS);
                        }
                        
                    }
                }
                else{
                    if (!isFriendFunction(unitArchive, unit, function, xpathS, j)) // Method could be a friend function
                        freeFunction.push_back(xpathS);                
                }
                free(unparsed); 
                srcml_unit_free(unit);
                srcml_archive_close(unitArchive);
                srcml_archive_free(unitArchive);
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
//  myClass --> childClass from myClass<T> or childClass from myClass<int>
//  specializedClass<int> --> childClass from specializedClass<int>
//
// In Java and C#, you can inherit from the generic class or specialize the inheritance.
// For example:
//  myClass<T> --> childClass from myClass<T> or childClass from myClass<int>
//
void classModelCollection::findInheritedAttribute(){
    for (auto& pair : classCollection){
        std::string lang = pair.second.getUnitLanguage();
        std::vector<std::string> parentClassName =  pair.second.getParentClassName();
        for (size_t i = 0; i < parentClassName.size(); i++){
            std::string parClassName = trimWhitespace(removeNamespace(parentClassName[i], true, lang));        
            if (classCollection.find(parClassName) != classCollection.end())
                pair.second.appendInheritedAttribute(classCollection[parClassName].getNonPrivateAttribute());
            else {
                parClassName = parClassName.substr(0, parClassName.find("<"));
                if (lang == "Java" || lang == "C#"){
                    if (classGeneric.find(parClassName) != classGeneric.end()) {
                        pair.second.appendInheritedAttribute(classCollection[classGeneric[parClassName]].getNonPrivateAttribute());
                    }
                }
                else {
                    if (classCollection.find(parClassName) != classCollection.end()) {
                        pair.second.appendInheritedAttribute(classCollection[parClassName].getNonPrivateAttribute());
                    }
                }
            }
        }        
    }
}

// C++ only
// A friend function usually has a parameter of the same type as the class it belongs to
// The assumption here is that friend functions are mostly used for operator overloading, 
//  and cases where friend functions don't have a parameter that use the
//  class name as a type are not checked for  
//      
bool classModelCollection::isFriendFunction(srcml_archive* archive, srcml_unit* unit, methodModel& function, std::string xpathS, int unitNumber){
    bool classNameFound = false;
    std::vector<std::string> parameterTypes = function.getParameterTypeSeparated();
    for (size_t j = 0; j < parameterTypes.size(); j++){ 
        std::string paraType = parameterTypes[j];
        if (classCollection.find(paraType) != classCollection.end()) {
            if (classCollection[paraType].getUnitLanguage() == "C++")   
                classNameFound = true;   
        }       
        else {
            paraType = paraType.substr(0, paraType.find("<"));
            if (classCollection.find(paraType) != classCollection.end()) {
                if (classCollection[paraType].getUnitLanguage() == "C++")   
                    classNameFound = true;    
            }
        }
        if (classNameFound){
            const std::unordered_set<std::string>& friendFunctions = classCollection[paraType].getFriendFunctionName();
            if (friendFunctions.find(function.getName()) != friendFunctions.end()){
                classCollection[paraType].addMethod(archive, unit, function,  xpathS, unitNumber);
            }
                
        }
        classNameFound = false;
    }
    return false;
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
void classModelCollection::outputReport(std::ofstream& out) {
    const int WIDTH = 70;
    const std::string line(WIDTH*2, '-');
    if (out.is_open()) {
        for (auto& pair : classCollection){
            const  std::vector<methodModel>& method = pair.second.getMethod();
            out << std::left << std::setw(WIDTH) << "Class name:" << std::setw(WIDTH) << "Class stereotype:" << std::endl;
            out << std::left << std::setw(WIDTH)  <<  pair.second.getClassName() << std::setw(WIDTH) << pair.second.getClassStereotype() << std::endl << std::endl;  
            out << std::left << std::setw(WIDTH) << "Method name:" << std::setw(WIDTH) << "Method stereotype:" << std::endl; 
            for (const auto& m : method) {
                out << std::left << std::setw(WIDTH) << WStoBlank(m.getName()); 
                out << std::setw(WIDTH) << m.getStereotype() << std::endl; 
            }
            out << line << std::endl;
        }
    }
}

// Used to generate optional views for the distribution of method and class stereotypes
// These views are generated in .csv files
//
void classModelCollection::outputCSV(std::string fname) {
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
    };
    std::unordered_map<std::string, int> methodStereotypes = {
        {"get", 0},
        {"predicate", 0},
        {"property", 0},
        {"accessor", 0},
        {"non-const-get", 0},
        {"non-const-predicate", 0},
        {"non-const-property", 0},
        {"non-const-accessor", 0},
        {"set", 0},
        {"command", 0},
        {"non-void-command", 0},
        {"collaborator", 0},
        {"controller", 0},
        {"factory", 0},
        {"empty", 0},
        {"stateless", 0},
        {"wrapper", 0},
        {"unclassified", 0},
    };

    std::unordered_map<std::string, int> uniqueMethodStereotypes;  
    std::unordered_map<std::string, int> uniqueClassStereotypes;  

    std::ofstream out(fname+"method_class_stereotypes.csv");
  
    // Method and class stereotypes
    //
    out << "Class Name,Class Stereotype,Method Name,Method Stereotype,Method Stereotype Count" << std::endl;
    if (out.is_open()) {
        for (auto& pair : classCollection){         
            for (const std::string& s : pair.second.getStereotypeList()){
                if (classStereotypes.find(s) != classStereotypes.end())
                    classStereotypes[s]++;       
            }
            std::string className =  "\"" + WStoBlank(pair.second.getClassName()) + "\"";
            std::string classStereotype =  pair.second.getClassStereotype();
            const  std::vector<methodModel>& method = pair.second.getMethod();
            if (uniqueClassStereotypes.find(classStereotype) == uniqueClassStereotypes.end())
                uniqueClassStereotypes[classStereotype] = 1;
            else
                uniqueClassStereotypes[classStereotype]++;
            for (const auto& m : method) {
                std::string methodName =  "\"" + WStoBlank(m.getName()) + "\"";
                std::string methodStereotype = m.getStereotype();
                out << className << "," << classStereotype << "," << methodName << ",";
                out << methodStereotype <<  ",";
                out << m.getStereotypeList().size() << std::endl;

                for (const std::string& s : m.getStereotypeList()){
                    if (methodStereotypes.find(s) != methodStereotypes.end())
                        methodStereotypes[s]++;       
                }
                if (uniqueMethodStereotypes.find(methodStereotype) == uniqueMethodStereotypes.end())
                    uniqueMethodStereotypes[methodStereotype] = 1;
                else
                    uniqueMethodStereotypes[methodStereotype]++;

            }
        } 
    }
    out.close();

    // Unique Method View
    std::ofstream outU(fname+"unique_method_view.csv");
    outU << "Unique Method Stereotype,Method Count" <<std::endl;
    int total = 0;
    for (auto& pair : uniqueMethodStereotypes){
        outU << pair.first << ",";
        outU << pair.second << std::endl;
        total += pair.second;
    }
    outU << "Total" << "," << total;
    outU.close();

    // Unique Class View
    std::ofstream outV(fname+"unique_class_view.csv");
    outV << "Unique Class Stereotype,Class Count" <<std::endl;
    total = 0;
    for (auto& pair : uniqueClassStereotypes){
        outV << pair.first << ",";
        outV << pair.second << std::endl;
        total += pair.second;
    }
    outV << "Total" << "," << total;
    outV.close();

    // Method View
    //
    std::ofstream outM(fname+"method_view.csv");
    outM << "Method Stereotype,Method Count" <<std::endl;
    total = 0;
    for (auto& pair : methodStereotypes){
        outM << pair.first << ",";
        outM << pair.second << std::endl;
        total += pair.second;
    }
    outM << "Total" << "," << total;
    outM.close();

    // Class View
    //
    std::ofstream outS(fname+"class_view.csv");
    outS << "Class Stereotype,Class Count" <<std::endl;
    total = 0;
    for (auto& pair : classStereotypes){
        outS << pair.first << ",";
        outS << pair.second << std::endl;
        total += pair.second;
    }
    outS << "Total" << "," << total;
    outS.close();

    // Category view
    //
    std::ofstream outC(fname+"category_view.csv");
    int getters = methodStereotypes["get"];
    int accessors = getters + methodStereotypes["predicate"] +
                    methodStereotypes["property"] +
                    methodStereotypes["accessor"];

    int setters = methodStereotypes["set"];     
    int commands = methodStereotypes["command"] + methodStereotypes["non-void-command"];           
    int mutators = setters + commands;

    int controllers = methodStereotypes["controller"];
    int collaborator =  methodStereotypes["collaborator"]; 
    int collaborators = controllers + collaborator;
    int unclassified = methodStereotypes["unclassified"];

    int factory = methodStereotypes["factory"];

    int degenerates = methodStereotypes["empty"] + methodStereotypes["stateless"] + methodStereotypes["wrapper"]; 

    total = accessors + mutators + factory + collaborators + degenerates + unclassified;
    outC << "Stereotype Category,Method Count" <<std::endl;
    outC << "Accessors" << "," << accessors << std::endl;
    outC << "Mutators" << "," << mutators << std::endl;
    outC << "Creational" << "," << factory << std::endl;
    outC << "Collaborational" << "," << collaborators << std::endl;
    outC << "Degenerate" << "," << degenerates << std::endl;
    outC << "unclassified" << "," << unclassified << std::endl;
    outC << "Total" << "," << total << std::endl;
    outC.close();
}


