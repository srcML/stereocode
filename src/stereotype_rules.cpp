// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_rules.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "stereotype_rules.hpp"
#include "helper_functions.hpp"

std::unordered_map<int, std::unordered_map
                  <std::string, std::string>> XPATH_LIST; // Map key = unit number. Each map value is a pair of xpath and stereotype  
extern int                                    METHODS_PER_TYPE_THRESHOLD;

// Compute method stereotypes
//
void stereotypeRules::computeMethodStereotypes(std::unordered_map<std::string, typeModel>& typeCollection) {
    for (auto& pair : typeCollection) {
        // Common operations
        const std::string&          unitLanguage               = pair.second.getUnitLanguage();
        int                         constructorDestructorCount = 0;
        std::vector<functionModel>& methods                    = pair.second.getMethods();

        for (auto& m : methods) {
            // Common operations
            std::string        returnTypeParsed                       = m.getReturnTypeParsed();
            int                fieldsModifiedCount                    = m.getFieldsModifiedCount();
            int                callsOnFieldsCount                     = m.getMethodCalls().size();
            int                callsOnTypeMethodsCount                = m.getFunctionCalls().size();
            int                newConstructorCallsCount               = m.getNewConstructorCalls().size();
            int                callsOnFreeFunctionsCount              = m.getExternalFunctionCallsCount();
            int                callsToOtherTypeMethods                = m.getExternalMethodCallsCount();
            int                numOfNonCommentStatements              = m.getNonCommentStatementsCount();
            bool               isFieldUsed                            = m.isFieldUsed();
            bool               isMethodConst                          = m.isMethodConst();
            bool               isVoidPointer                          = false;
            bool               isVariableCreatedAndReturnedWithNew    = m.isVariableCreatedAndReturnedWithNew();
            bool               isNonPrimitiveReturnType               = m.isNonPrimitiveReturnType();
            bool               isNewReturned                          = m.isNewReturned();
            bool               isParameterRefModified                 = m.isParameterRefModified();
            bool               hasSimpleReturn                        = m.hasSimpleReturn();   
            bool               hasComplexReturn                       = m.hasComplexReturn(); 
            bool               isNonPrimitiveLocalOrParameterModified = m.isNonPrimitiveLocalOrParameterModified();
            bool               isNonPrimitiveFieldExternal            = m.isNonPrimitiveFieldExternal();
            bool               isNonPrimitiveLocalExternal            = m.isNonPrimitiveLocalExternal();
            bool               isNonPrimitiveParamaterExternal        = m.isNonPrimitiveParamaterExternal();
            bool               isNonPrimitiveReturnTypeExternal       = m.isNonPrimitiveReturnTypeExternal();

            // Covers the case of void with * or more
            std::string returnTypeVoid = m.getReturnType().getType();
            HELPERS::removeWhitespace(returnTypeVoid);
            if (unitLanguage != "Java") if (returnTypeVoid.find("void*") != std::string::npos) isVoidPointer = true;
            
            
            // constructor copy-constructor destructor
            //
            if (!m.getConstructorOrDestructor().empty()) {  
                ++constructorDestructorCount;

                if      (m.getConstructorOrDestructor() == "destructor"      ) m.setStereotype ("destructor"      ); 
                else if (m.getConstructorOrDestructor() == "copy-constructor") m.setStereotype ("copy-constructor");
                else                                                           m.setStereotype ("constructor"     );
            }

            // empty
            //
            // 1] Method has no statements except for comments
            //
            else if (numOfNonCommentStatements == 0) m.setStereotype("empty");
            else {
                // get
                //
                // 1] Return type is not void
                // 2] Contains at least one simple return expression that 
                //     returns a field (e.g., return a;) or the value to a field (e.g., return *a; or return **a; ... etc)
                //     The field ( a ) can be of any data type (e.g., primitive, non-primitive, pointer, reference, etc)
                //
                // Returning ( this ) by itself is not a getter (e.g., return this;) 
                //  as it points to the current object rather than a field
                //
                if (hasSimpleReturn) m.setStereotype("get"); 
                    
                
                // predicate
                //
                // 1] Return type is Boolean
                // 2] Contains at least one complex return expression (e.g., return a+5;)
                // 3] Uses a field in an expression or has at least 
                //     one function call (except constructor calls) to other methods in type
                //
                // Returning "this" by itself is not a predicate (e.g., return this;) 
                //  as it points to the current object rather than a bool value found using fields
                // 
                bool  returnType = false;

                if      (unitLanguage == "C++")  returnType = (returnTypeParsed == "bool");
                else if (unitLanguage == "C#")   returnType = (returnTypeParsed == "bool") || (returnTypeParsed == "Boolean");
                else if (unitLanguage == "Java") returnType = (returnTypeParsed == "boolean");

                if (returnType && hasComplexReturn && (isFieldUsed || (callsOnTypeMethodsCount > 0))) 
                    m.setStereotype("predicate"); 

                // property
                //
                // 1] Return type is not void or Boolean
                // 2] Contains at least one complex return statement (e.g., return a+5;)
                // 3] Uses a field in an expression or has at least 
                //     one function call (except constructor calls) to other methods in type
                //
                // Returning "this" by itself is not a property (e.g., return this;) 
                //  as it points to the current object rather than a non-bool value found using fields
                //  
                bool returnNotVoidOrBool = false;

                if      (unitLanguage == "C++")  returnNotVoidOrBool = (returnTypeParsed != "bool" && returnTypeParsed != "void" && 
                                                                        returnTypeParsed != "") || isVoidPointer;
                else if (unitLanguage == "C#")   returnNotVoidOrBool = (returnTypeParsed != "bool" && returnTypeParsed != "Boolean" && 
                                                                        returnTypeParsed != "void" && returnTypeParsed != "Void" && 
                                                                        returnTypeParsed != "") || isVoidPointer;
                else if (unitLanguage == "Java") returnNotVoidOrBool = (returnTypeParsed != "boolean" && returnTypeParsed != "void" && 
                                                                        returnTypeParsed != "Void" && returnTypeParsed != "");

                if (returnNotVoidOrBool && hasComplexReturn && (isFieldUsed || (callsOnTypeMethodsCount > 0))) 
                    m.setStereotype("property");
                
            
                // void-accessor
                //
                // 1] Return type is void 
                // 2] Contains at least one parameter that is passed by non-const reference and is assigned a value
                // 3] Uses a field in an expression or has at least 
                //     one function call (except constructor calls) to other methods in type 
                //
                // The "this" keyword by itself is considered (e.g., p = this or p = *this) 
                //  as an accessor to the state of the object where 'p' is passed by reference
                //
                if (isParameterRefModified && (returnTypeParsed == "void") && !isVoidPointer && (isFieldUsed || (callsOnTypeMethodsCount > 0))) 
                    m.setStereotype("void-accessor");       
                

                // set
                //
                // 1] Only one field is changed or there is a single call on a field
                // 2] No calls to methods in type
                //
                // The "this" keyword by itself is considered (e.g., this["index"] = value; for indexers in C# or this./->dataMember = value)
                //       
                if (callsOnTypeMethodsCount == 0 && 
                   ((fieldsModifiedCount == 1 && callsOnFieldsCount == 0) || 
                    (fieldsModifiedCount == 0 && callsOnFieldsCount == 1)))
                    m.setStereotype("set"); 
                

                // command
                //
                // Method has a void return type
                // Method is not const or const but has mutable fields (C++ only)
                // Cases:
                //   Case 1: More than one field is modifed
                //           
                //   Case 2: one field is modifed and
                //            there is at least one call on a field or
                //            at least one function call to other methods (except constructor calls) in type
                //   Case 3: zero fields are modifed and
                //            there is at least two calls on one or more fields or
                //            at least one function call to other methods (except constructor calls) in type  
                //
                // The "this" keyword by itself is considered (e.g., this["index"] = value; for indexers in C# or this./->dataMember = value)
                //
                // non-void-command    
                //   Method return type is not void
                //             
                bool case1       = fieldsModifiedCount > 1;
                bool case2       = (fieldsModifiedCount == 1) && ((callsOnTypeMethodsCount + callsOnFieldsCount) > 0);
                bool case3       = (fieldsModifiedCount == 0) && ((callsOnFieldsCount > 1) || (callsOnTypeMethodsCount > 0));
                
                bool isMutable = isMethodConst && case1;
                bool isNonVoidReturn = returnTypeParsed != "void" && returnTypeParsed != "Void" && !isVoidPointer;

                if (case1 || case2 || case3) {
                    if (!isMethodConst || isMutable){ // Handles case of mutable fields in C++
                        if (isNonVoidReturn) m.setStereotype("non-void-command");  
                        else m.setStereotype("command");
                    }
                } 
                    

                // factory
                //
                // 1] Factories must include a non-primitive type in their return type
                //      and their return expression must be a local variable, parameter, or field, that 
                //      call a constructor call or has a return expression with a constructor call (e.g., new)
                //
                //
                if (isNonPrimitiveReturnType && (isNewReturned || isVariableCreatedAndReturnedWithNew))
                    m.setStereotype("factory"); 
                         
                // wrapper
                //
                // 1] No fields are modified
                // 2] No calls to methods in type
                // 3] No calls on fields
                // 4] Has at least one free function call 
                // Constructor calls using the 'new' operator are not considered 
                //
                // controller
                //
                // 1] No fields are modified
                // 2] No calls to methods in type
                // 3] No calls on fields
                // 3] Has at least one call to other type methods or mutates a parameter or a local that is non-primitive
                //
                // collaborator
                //
                // 1] It must use at least 1 non-primitive type (not of this type)
                // 2] Type could be a parameter, local variable, return type, or an field
                //
                //
                if ((fieldsModifiedCount == 0) && (callsOnTypeMethodsCount == 0) && (callsOnFieldsCount == 0) 
                    && (callsToOtherTypeMethods == 0) && (callsOnFreeFunctionsCount > 0)) 
                    m.setStereotype("wrapper");

                else if ((fieldsModifiedCount == 0) && (callsOnTypeMethodsCount == 0) && (callsOnFieldsCount == 0) &&
                    ((callsToOtherTypeMethods > 0) || isNonPrimitiveLocalOrParameterModified))
                    m.setStereotype("controller");   

                else if (isNonPrimitiveFieldExternal || isNonPrimitiveLocalExternal || 
                    isNonPrimitiveParamaterExternal || (isNonPrimitiveReturnTypeExternal || isVoidPointer))
                    m.setStereotype("collaborator"); 


                // incidental 
                //
                // 1] Method contains at least one non-comment statement (i.e., method is not empty)
                // 2] No fields are used or modified (including no use of keyword "this" by itself)
                // 3] No calls of any kind
                // 
                bool noCalls = callsOnTypeMethodsCount == 0 && callsOnFieldsCount == 0 && 
                               newConstructorCallsCount == 0 && callsToOtherTypeMethods == 0 && callsOnFreeFunctionsCount == 0;

                if (!isFieldUsed & noCalls) 
                    m.setStereotype("incidental");          
            
                    
                // stateless
                //
                // 1]	Method contains at least one non-comment statement (i.e., method is not empty)
                // 2]	No fields are used or modified (including no use of keyword "this" by itself)
                // 3]	No calls to methods in type 
                // 4]   No calls on fields
                // 5]   Has at least one call to other type methods (including constructor calls) or to a free function 
                //
                if (!isFieldUsed && callsOnTypeMethodsCount == 0 && callsOnFieldsCount == 0 &&
                   ((callsOnFreeFunctionsCount > 0) || (callsToOtherTypeMethods > 0) || (newConstructorCallsCount > 0)))
                   m.setStereotype("stateless");             
                
            }
            
            // unclassified
            //
            // No stereotype found
            //
            if (m.getStereotypes().size() == 0)  m.setStereotype("unclassified");

            // Used to for re-documenting the system with the stereotype information
            XPATH_LIST[m.getUnitNumber()].insert({m.getXpath(), m.getStereotypesString()});    
        }
        pair.second.setConstructorDestructorCount(constructorDestructorCount);
    }
}

// Compute type stereotype
// Constructors and destructors are not considered in the computation of type stereotypes
// 
void stereotypeRules::computeTypeStereotypes(std::unordered_map<std::string, typeModel>& typeCollection) {
    for (auto& pair : typeCollection) {
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
            {"factory", 0},
            {"incidental", 0},
            {"stateless", 0},  
            {"empty", 0},
            {"unclassified", 0},
        };

        const std::vector<functionModel>& methods           = pair.second.getMethods();
        int                               nonCollaborators  = 0;
        for (const auto& m : methods) {      
            if (m.getConstructorOrDestructor().empty()) {
                for (const std::string& s : m.getStereotypes()) methodStereotypes[s]++;
            
                std::string methodStereotype = m.getStereotypesString();
                if (methodStereotype.find("collaborator") == std::string::npos &&
                    methodStereotype.find("controller") == std::string::npos && 
                    methodStereotype.find("wrapper") == std::string::npos)
                    nonCollaborators++;
            }
        }

        int getters       = methodStereotypes["get"];
        int accessors     = getters + methodStereotypes["predicate"] +
                            methodStereotypes["property"] +
                            methodStereotypes["void-accessor"]; 
        int setters       = methodStereotypes["set"];     
        int commands      = methodStereotypes["command"] + methodStereotypes["non-void-command"];           
        int mutators      = setters + commands;
        int controllers   = methodStereotypes["controller"];
        int collaborator  = methodStereotypes["collaborator"] + methodStereotypes["wrapper"]; 
        int collaborators = controllers + collaborator;
        int factory       = methodStereotypes["factory"];
        int degenerates   = methodStereotypes["incidental"] + methodStereotypes["stateless"] + methodStereotypes["empty"];
        int allMethods    = methods.size() - pair.second.getConstructorDestructorCount();

        // Entity
        //
        if (((accessors - getters) != 0) && ((mutators - setters)  != 0) ) {
            double ratio = double(collaborators) / double(nonCollaborators);
            if (ratio >= 2 && controllers == 0) 
                pair.second.setStereotype("entity");   
        }


        // Minimal Entity
        //
        if (((allMethods - (getters + setters + commands)) == 0) && (getters != 0) && (setters != 0) & (commands != 0)) {
            double ratio = double(collaborators) / double(nonCollaborators);
            if (ratio >= 2) 
                pair.second.setStereotype("minimal-entity");   
        }


        // Data Provider
        //
        if ((accessors > 2 * mutators) && (accessors > 2 * (controllers + factory)) )
            pair.second.setStereotype("data-provider");


        // Commander
        //
        if ((mutators > 2 * accessors) && (mutators > 2 * (controllers + factory)))
            pair.second.setStereotype("commander");


        // Boundary
        //
        if ((collaborators > nonCollaborators) && (factory < 0.5 * allMethods) && (controllers < 0.33 * allMethods))
            pair.second.setStereotype("boundary");


        // Factory
        //
        if (factory > 0.67 * allMethods)
            pair.second.setStereotype("factory");
        

        // Controller
        //
        if ((controllers + factory > 0.67 * allMethods) && ((accessors != 0) || (mutators != 0)))
            pair.second.setStereotype("controller");


        // Pure Controller
        //
        if ((controllers + factory != 0) && ((accessors + mutators + collaborator) == 0) && (controllers != 0)) 
            pair.second.setStereotype("pure-controller");


        // Large
        //
        std::string dynamicStereotype = "large-" + pair.second.getStructure();
        int accPlusMut = accessors + mutators;
        int facPlusCon = controllers + factory;
        if (((0.2 * allMethods < accPlusMut) && (accPlusMut < 0.67 * allMethods )) &&
            ((0.2 * allMethods < facPlusCon) && (facPlusCon < 0.67 * allMethods )) &&
            (factory != 0) && (controllers != 0) && (accessors != 0) && (mutators != 0) ) {
                if (allMethods > METHODS_PER_TYPE_THRESHOLD) { 
                    pair.second.setStereotype(dynamicStereotype);
            }
        }
        

        // Lazy
        //
        dynamicStereotype = "lazy-" + pair.second.getStructure();
        if ((getters + setters != 0) && (((degenerates / double(allMethods)) > 0.33)) &&
        (((allMethods - (degenerates + getters + setters)) / double(allMethods))  <= 0.2))
            pair.second.setStereotype(dynamicStereotype);
        

        // Degenerate
        //
        if ((degenerates / double(allMethods)) > 0.5)  
            pair.second.setStereotype("degenerate");
        

        // Data
        //
        dynamicStereotype = "data-" + pair.second.getStructure();
        if ((allMethods - (getters + setters) == 0) && ((getters + setters) != 0))
            pair.second.setStereotype(dynamicStereotype);
        

        // Small
        //
        dynamicStereotype = "small-" + pair.second.getStructure();
        if ((0 < allMethods) && (allMethods < 3))
            pair.second.setStereotype(dynamicStereotype);


        // Empty (Considered degenerate)
        //
        if (allMethods == 0)
            pair.second.setStereotype("empty");
   

        if (pair.second.getStereotypes().size() == 0) 
            pair.second.setStereotype("unclassified");

        const std::unordered_map<int, std::vector<std::string>>& xpath = pair.second.getXpath();
        for (const auto& pairXpath : xpath) 
            for (const auto& typeXpath : pairXpath.second) XPATH_LIST[pairXpath.first].insert({typeXpath, pair.second.getStereotypesString()});
    }  
}

void stereotypeRules::computeFreeFunctionStereotypes(std::vector<functionModel>& freeFunctions) {
    for (functionModel& f : freeFunctions) {
        // Common operations
        const std::string& methodName                               = f.getName()[1];
        std::string        returnTypeParsed                         = f.getReturnTypeParsed();
        const std::string& unitLanguage                             = f.getUnitLanguage();
        int                nonNewConstructorCallsCount              = f.getMethodCalls().size() + f.getFunctionCalls().size();
        int                nonCommentStatementsCount                = f.getNonCommentStatementsCount();
        bool               isVariableCreatedAndReturnedWithNew      = f.isVariableCreatedAndReturnedWithNew();
        bool               hasParameterComplexReturn                = f.hasParameterComplexReturn();
        bool               isParamaterUsed                          = f.isParameterUsed();
        bool               isParameterRefModified                   = f.isParameterRefModified();
        bool               isNonPrimitiveReturnType                 = f.isNonPrimitiveReturnType();
        bool               isNewReturned                            = f.isNewReturned();
        bool               isGlobalOrStaticVariableModified         = f.isGlobalOrStaticVariableModified();


        // main
        //
        // A main function
        //
        if (methodName == "main" || methodName == "Main") f.setStereotype("main");


        // empty
        //
        // Has no statements
        //
        else if (nonCommentStatementsCount == 0) f.setStereotype("empty");
                
        
        else {
            // predicate
            //
            // Returns a bool derived from the parameters
            //
            bool                              boolReturnType = false;
            if      (unitLanguage == "C++")   boolReturnType = (returnTypeParsed == "bool");
            else if (unitLanguage == "C#")    boolReturnType = (returnTypeParsed == "bool") ||  (returnTypeParsed == "Boolean");
            else if (unitLanguage == "Java")  boolReturnType = (returnTypeParsed == "boolean");

            if (boolReturnType && hasParameterComplexReturn && isParamaterUsed) f.setStereotype("predicate"); 

            // property
            //
            // Returns a non-bool derived from the parameters
            //
            // The returnTypeParsed != "" is used to handle the case of a function that has no return type
            //
            bool nonBoolOrVoidReturnType = false;
            if      (unitLanguage == "C++")  nonBoolOrVoidReturnType = (returnTypeParsed != "bool" && returnTypeParsed != "void" && returnTypeParsed != "");
            else if (unitLanguage == "C#")   nonBoolOrVoidReturnType = (returnTypeParsed != "bool" && returnTypeParsed != "Boolean" &&
                                                                        returnTypeParsed != "void" && returnTypeParsed != "Void" && returnTypeParsed != "");
            else if (unitLanguage == "Java") nonBoolOrVoidReturnType = (returnTypeParsed != "boolean" && returnTypeParsed != "void" && 
                                                                        returnTypeParsed != "Void" && returnTypeParsed != "");

            if (nonBoolOrVoidReturnType && hasParameterComplexReturn && isParamaterUsed) f.setStereotype("property"); 
            

            // global-command
            //
            // Modifies a global or a static variable
            //
            if (isGlobalOrStaticVariableModified) f.setStereotype("global-command");
            

            // command
            //
            // Modifies a parameter passed by reference
            //
            if (isParameterRefModified) f.setStereotype("command");


            // factory
            //
            // Creates and returns a 'new' locally created object
            // Constructor calls that are not using the 'new' operator are not considered 
            //
            if (isNonPrimitiveReturnType && (isNewReturned || isVariableCreatedAndReturnedWithNew)) f.setStereotype("factory");

            
            // literal
            //
            // Does not read or change parameters
            //
            if (!isParamaterUsed) f.setStereotype("literal");


            // wrapper
            //
            // Does not change parameters passed by reference. Has at least one call to other type methods or to a free function  
            // Constructor calls using the 'new' operator are not considered
            //
            if (!isParameterRefModified && (nonNewConstructorCallsCount > 0)) f.setStereotype("wrapper");


            if (f.getStereotypes().size() == 0) f.setStereotype("unclassified");
        }
        XPATH_LIST[f.getUnitNumber()].insert({f.getXpath(), f.getStereotypesString()});
    }
}