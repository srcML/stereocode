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
    findInheritedAttribute();

    if (PRIMITIVES.getLanguage() == "C++")
        findFreeFunction(archive, units); 

    for(auto& pair : classCollection){
        pair.second.ComputeMethodStereotype();
        pair.second.ComputeClassStereotype();
    }
}

// Finds all classes in an archive. Performs analysis on these classes.
// For C# and Java, interfaces are not considered.
//      C# interfaces don't have any fields (attributes), but could have method definitions. 
//      Java interfaces could have fields and methods definitions.
// Structs are not being considered in C++ or C#. Java doesn't have structs.
// Nested classes are ignored.
//
// Cases for multiple classes with the same name. They could be:
//      In Different namespaces.
//      Differentiated by preprocessor directives.
//      In different files.
// Usually, these classes behave in the same way.
// Thus, only one is stereotyped (first one collected) and is applied to all. 
//
void classModelCollection::findClassInfo(srcml_archive* archive, std::vector<srcml_unit*> units){
    int classOrder = 1;
    for (size_t j = 0; j < units.size(); j++){
        std::string lang = srcml_unit_get_language(units[j]);    
        if (lang== "C++" || lang == "C#" || lang == "Java"){
            PRIMITIVES.setLanguage(lang);    
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
                srcml_unit* unit = srcml_archive_read_unit(temp); // This unit is a class only, not a whole unit.
                
                std::string fileName = srcml_unit_get_filename(resultUnit);
                std::string xpath = "//src:unit[@filename='" + fileName + "']//src:class[" + std::to_string(classOrder) + "]";
                
                classModel c(temp, unit, xpath, i);

                // In C++, class names are usually in the form of:
                //      myClass  
                // or  
                //      myClass<type1, type2, ...> for specialized templated class from myClass.
                // In C++, a specialized templated class can exist if there is a generic one.
                //
                // In C# and Java, there is no concept of a specialized templated class, but one could define:
                //      myClass
                // or
                //      myClass<T, G, ... >
                // Where these two are different classes. First is normal class and second is a generic. 

                std::string className = trimWhitespace(c.getClassName()); // Remove all whitespace for easier matching with method names.
                auto result = classCollection.insert({className, c}); // Only inserts key-value pair if they don't exist (class doesn't exists yet).
                if (result.second) // Class doesn't exist and was inserted
                    result.first->second.findClassInfo(temp, unit, classOrder, j); // Do class analysis.                             
                free(unparsed);
                srcml_unit_free(unit);
                srcml_archive_close(temp);
                srcml_archive_free(temp);       
                classOrder++;     
            }   
            srcml_transform_free(result);
            srcml_clear_transforms(archive);  
            classOrder = 1;  
        }   
    }
}

// Finds inherited attributes for classes that inherit
// In C++, you can inherit from a specialized templated class or
//  you can specialize the inheritance itself from the generic class, or
//  you can inherit from generic class itself.
// For example:
//  myClass --> childClass : myClass<T> or childClass : myClass<int>
//  specializedClass<int> --> childClass : myClass<int>
//
// In Java and C#, you can inherit from the generic class or specialize the inheritance.
// For example:
//  myClass<T> --> childClass : myClass<T> or childClass : myClass<int>
//
// So, we need to match with the specialized templated classes first.
//
void classModelCollection::findInheritedAttribute(){
     for(auto& pair : classCollection){
        const std::vector<std::string>& parentClassName = pair.second.getParentClassName();
        for (size_t i = 0; i < parentClassName.size(); i++){
            if (classCollection.find(trimWhitespace(parentClassName[i])) != classCollection.end())
                pair.second.appendInheritedAttribute(classCollection[parentClassName[i]].getNonPrivateAttribute());
            else{
                std::string parClassName = trimWhitespace(parentClassName[i]);
                if (classCollection.find(parClassName) != classCollection.end()){
                    pair.second.appendInheritedAttribute(classCollection[parentClassName[i]].getNonPrivateAttribute());
                }
            }
        }       
     }
}


// C++ only
// Finds free functions as well as methods defined externally.
// Free functions could also have :: in their names. 
//  They could belong to a namespace or to an external class (class not defined in the archive),
//   so classes need to be found first 
//
// Cases for functions and methods 
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
    int functionOrder = 1;
    for (size_t j = 0; j < units.size(); j++){
        std::string lang = srcml_unit_get_language(units[j]);        
        srcml_append_transform_xpath(archive, "//src:function[not(ancestor::src:class)]");
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, units[j], &result);
        int n = srcml_transform_get_unit_size(result);  

        srcml_unit* resultUnit = nullptr;
        for (int i = 0; i < n; ++i) {
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

            std::string fileName = srcml_unit_get_filename(resultUnit);
            std::string xpath = "//src:unit[@filename='" + fileName + "']//src:function[not(ancestor::src:class)][" + std::to_string(functionOrder) + "]";

            methodModel function(unitArchive, unit, xpath, j); // Create function and perform basic analysis on it

            // Method names are in the form of:
            //      MyClass<int>::Foo, namespaceA::MyClass<int>::Foo, MyClass::Foo, 
            // Free and friend functions are in the form:
            //      Foo, operator<<
            std::string functionName = trimWhitespace(removeNamespace(function.getMethodName())); // Removes namespaces if any.

            // Get the class name (if any). Else, it is a free function or a friend function name.
            size_t isClassName = functionName.find("::");
            if (isClassName != std::string::npos){ // Class found, it is a method.          
                std::string className = functionName.substr(0, isClassName); 
                if (classCollection.find(className) != classCollection.end()) {
                    // Add method and perform the rest of the analysis             
                    classCollection[className].addMethod(unitArchive, unit, function, trimWhitespace(function.getMethodHeader())); 
                }          
                else { // Case specialized template method belongs to the generic template class
                    className = className.substr(0, className.find("<"));
                    if (classCollection.find(className) != classCollection.end()){
                        classCollection[className].addMethod(unitArchive, unit, function, trimWhitespace(function.getMethodHeader()));           
                    }
                    else{
                        freeFunction.push_back(function.getMethodHeader());
                    }
                    
                }
            }
            else{
                if (!isFriendFunction(unitArchive, unit, function, functionName)){ // Method could be a friend function
                    freeFunction.push_back(function.getMethodHeader());
                }
            }
            free(unparsed); 
            srcml_unit_free(unit);
            srcml_archive_close(unitArchive);
            srcml_archive_free(unitArchive);
            functionOrder++;
        }
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
        functionOrder = 1;
        
    }
}

// C++ only
// A friend function must always be in the same namespace as the class,
//      unless it is declared with the global namespace in the class such as "friend dataType ::operator<<();""
//      So it is usually in the form of "friend dataType operator<<();""
//      
bool classModelCollection::isFriendFunction(srcml_archive* archive, srcml_unit* unit, methodModel& method, std::string possibleFriendFunction){
    for(auto& pair : classCollection){
        const std::vector<std::string>& friendFunctionName = pair.second.getFriendFunctionName();
        for (size_t i = 0; i < friendFunctionName.size(); i++){
            std::string name = trimWhitespace(friendFunctionName[i]);
            if (name == possibleFriendFunction){ // Match with whole key
                // Parameters of friend function will usually have the class name
                // Can be used to determine which class does the friend function belong to
                std::vector<std::string> parameterTypes = method.getParameterTypes();
                for (size_t j = 0; j < parameterTypes.size(); j++){ 
                    if (parameterTypes[j].find(pair.first) != std::string::npos){
                        pair.second.addMethod(archive, unit, method, trimWhitespace(method.getMethodHeader()));
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

//  Copy unit and add in stereotype attribute on <class> and <function>
//  Example: <function st:stereotype="get"> ... </function>
//           <class st:stereotype="boundary"> ... ></class>
//
void classModelCollection::outputWithStereotypes(srcml_archive* archive, srcml_archive* outputArchive, std::vector<srcml_unit*> units) {   
    for (size_t i = 0; i < units.size(); i++){
        for (auto& pair : classCollection){  // Add stereotype attribute to each class
            std::string xpath = pair.second.getXpath(i);
            if (xpath != ""){
                srcml_append_transform_xpath_attribute(archive, xpath.c_str(), "st",
                                                    "http://www.srcML.org/srcML/stereotype",
                                                    "stereotype", pair.second.getClassStereotype().c_str());
            }
            std::unordered_map<std::string, methodModel>& method = pair.second.getMethod();
            for (auto& pairM : method) { // Add stereotype attribute to each method/function
                xpath =  pairM.second.getXpath(i);
                if (xpath != ""){
                    
                    std::string stereotype =  pairM.second.getStereotype();
                    srcml_append_transform_xpath_attribute(archive, xpath.c_str(), "st",
                                                    "http://www.srcML.org/srcML/stereotype",
                                                    "stereotype", stereotype.c_str());       
                }
            } 
        }
        srcml_transform_result* result = nullptr; 
        srcml_unit_apply_transforms(archive, units[i], &result);
        int n = srcml_transform_get_unit_size(result);  
        for (int i = 0; i < n; i++){
            srcml_unit* tempUnit = srcml_transform_get_unit(result, i);
            srcml_archive_write_unit(outputArchive, tempUnit);
        }
        srcml_transform_free(result);                     
    }
    srcml_clear_transforms(archive);
}

// Outputs a report file for a class (tab separated)
// class name || method || stereotypes
//
void classModelCollection::outputReport(std::ofstream& out) {
    const int WIDTH = 70;
    const std::string line(WIDTH*2, '-');
    if (out.is_open()) {
        for (auto& pair : classCollection){
            const  std::unordered_map<std::string, methodModel>& method = pair.second.getMethod();
            out << std::left << std::setw(WIDTH) << "Class name:" << std::setw(WIDTH) << "Class stereotype:" << std::endl;
            out << std::left << std::setw(WIDTH)  <<  pair.second.getClassName() << std::setw(WIDTH) << pair.second.getClassStereotype() << std::endl << std::endl;  
            out << std::left << std::setw(WIDTH) << "Method name:" << std::setw(WIDTH) << "Method stereotype:" << std::endl; 
            for (const auto& pairM : method) {
                out << std::left << std::setw(WIDTH) << WStoBlank(pairM.second.getMethodName()); 
                out << std::setw(WIDTH) << pairM.second.getStereotype() << std::endl; 
            }
            out << line << std::endl;
        }
    }
}


void classModelCollection::outputCSV() {
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

    std::ofstream out("method_class_stereotypes.csv");
  
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
            const  std::unordered_map<std::string, methodModel>& method = pair.second.getMethod();
            for (const auto& pairM : method) {
                std::string methodName =  "\"" + WStoBlank(pairM.second.getMethodName()) + "\"";
                std::string methodStereotype = pairM.second.getStereotype();
                out << className << "," << classStereotype << "," << methodName << ",";
                out << methodStereotype << ",";
                out << pairM.second.getStereotypeList().size() << std::endl;

                for (const std::string& s : pairM.second.getStereotypeList()){
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

    // Stereotype View
    std::ofstream outU("stereotype_view.csv");
    outU << "Unique Method Stereotype,Method Count,%" <<std::endl;
    int total = 0;
    for (auto& pair : uniqueMethodStereotypes){
        outU << pair.first << ",";
        outU << pair.second << std::endl;
        total += pair.second;
    }
    outU << "Total" << "," << total;
    outU.close();

    // Method View
    //
    std::ofstream outM("method_view.csv");
    outM << "Method Stereotype,Method Count,%" <<std::endl;
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
    std::ofstream outS("class_view.csv");
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
    std::ofstream outC("category_view.csv");
    outM << "Class Stereotype,Class Count,%" <<std::endl;
    int getters = methodStereotypes["get"] + methodStereotypes["non-const-get"];
    int accessors = getters + methodStereotypes["predicate"] + methodStereotypes["non-const-predicate"] +
                    methodStereotypes["property"] + methodStereotypes["non-const-property"] +
                    methodStereotypes["accessor"] + methodStereotypes["non-const-accessor"]; 

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
    outC << "Stereotype Category,Method Count,%" <<std::endl;
    outC << "Accessors" << "," << accessors << std::endl;
    outC << "Mutators" << "," << mutators << std::endl;
    outC << "Creational" << "," << factory << std::endl;
    outC << "Collaborational" << "," << collaborators << std::endl;
    outC << "Degenerate" << "," << degenerates << std::endl;
    outC << "unclassified" << "," << unclassified << std::endl;
    outC << "Total" << "," << total << std::endl;
    outC.close();
}


