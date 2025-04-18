// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotypes.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "stereotypes.hpp"

extern std::unordered_map
       <int, std::unordered_map
       <std::string, std::string>>   XPATH_LIST;   
extern int                           METHODS_PER_CLASS_THRESHOLD;

// Compute method stereotypes
//
void stereotypes::computeMethodStereotypes(std::unordered_map<std::string, classModel>& classCollection) {
    for (auto& pair : classCollection) {
        // Common operations
        const std::vector<std::string>& className = pair.second.getName();
        const std::string& classUnitLanguage      = pair.second.getUnitLanguage();
        int constructorDestructorCount            = 0;
        std::vector<methodModel>& methods         = pair.second.getMethods();

        for (auto& m : methods) {
            // Common operations
            const std::string& returnTypeParsed             = m.getReturnTypeParsed();
            int   dataMembersModifiedCount                  = m.getDataMembersModifiedCount();
            int   callsOnDataMembersCount                   = m.getMethodCalls().size();
            int   callsOnClassMethodsCount                  = m.getFunctionCalls().size();
            int   newConstructorCallsCount                  = m.getNewConstructorCalls().size();
            int   callsOnFreeFunctionsCount                 = m.getExternalFunctionCallsCount();
            int   callsToOtherClassMethods                  = m.getExternalMethodCallsCount();
            int   NumOfNonCommentStatements                 = m.getNonCommentStatementsCount();
            bool  isDataMemberUsed                          = m.isDataMemberUsed();
            bool  isMethodConst                             = m.isMethodConst();
            bool  isVoidPointer                             = false;
            bool  isVariableCreatedAndReturnedWithNew       = m.isVariableCreatedAndReturnedWithNew();
            bool  isNonPrimitiveReturnType                  = m.isNonPrimitiveReturnType();
            bool  isNewReturned                             = m.isNewReturned();
            bool  isParameterRefModified                    = m.isParameterRefModified();
            bool  hasSimpleReturn                           = m.hasSimpleReturn();   
            bool  hasComplexReturn                          = m.hasComplexReturn(); 
            bool  isNonPrimitiveLocalOrParameterModified    = m.isNonPrimitiveLocalOrParameterModified();
            bool  isNonPrimitiveDataMemberExternal          = m.isNonPrimitiveDataMemberExternal();
            bool  isNonPrimitiveLocalExternal               = m.isNonPrimitiveLocalExternal();
            bool  isNonPrimitiveParamaterExternal           = m.isNonPrimitiveParamaterExternal();
            bool  isNonPrimitiveReturnTypeExternal          = m.isNonPrimitiveReturnTypeExternal();

            // Covers the case of void with * or more
            if (classUnitLanguage != "Java") if (m.getReturnType().find("void*") != std::string::npos) isVoidPointer = true;
            
            // constructor copy-constructor destructor
            //
            if (m.isConstructorOrDestructor()) {  
                ++constructorDestructorCount;

                const std::string& parameterList = m.getParameterList();
                const std::string& srcML         = m.getSrcML();
    
                if      (srcML.find("<destructor>") != std::string::npos      ) m.setStereotype ("destructor"      ); 
                else if (parameterList.find(className[3]) != std::string::npos) m.setStereotype ("copy-constructor");
                else                                                            m.setStereotype ("constructor"     );
            }
            // empty
            //
            // 1] Method has no statements except for comments
            //
            else if (NumOfNonCommentStatements == 0) m.setStereotype("empty");
            else {
                // get
                //
                // 1] Return type is not void
                // 2] Contains at least one simple return expression that 
                //     returns a data member (e.g., return dm;) or the value to a data member (e.g., return *dm; or return **dm; ... etc)
                //    The data member 'dm' can be of any data type (e.g., primitive, non-primitive, pointer, reference, etc)
                //
                // Returning "this" by itself is not a getter (e.g., return this;) 
                //  as it points to the current object rather than a data member
                //
                if (hasSimpleReturn) m.setStereotype("get"); 
                    
                
                // predicate
                //
                // 1] Return type is Boolean
                // 2] Contains at least one complex return expression
                // 3] Uses a data member in an expression or has at least 
                //     one function call (except constructor calls) to other methods in class
                //
                // Returning "this" by itself is not a predicate (e.g., return this;) 
                //  as it points to the current object rather than a bool value found using data members
                // 
                bool  returnType = false;

                if      (classUnitLanguage == "C++")  returnType = (returnTypeParsed == "bool");
                else if (classUnitLanguage == "C#")   returnType = (returnTypeParsed == "bool") || (returnTypeParsed == "Boolean");
                else if (classUnitLanguage == "Java") returnType = (returnTypeParsed == "boolean");

                if (returnType && hasComplexReturn && (isDataMemberUsed || (callsOnClassMethodsCount > 0))) 
                    m.setStereotype("predicate"); 
            
                
                // property
                //
                // 1] Return type is not void or Boolean
                // 2] Contains at least one complex return statement (e.g., return a+5;)
                // 3] Uses a data member in an expression or has at least 
                //     one function call (except constructor calls) to other methods in class
                //
                // Returning "this" by itself is not a property (e.g., return this;) 
                //  as it points to the current object rather than a non-bool value found using data members
                //  
                bool returnNotVoidOrBool = false;

                if      (classUnitLanguage == "C++")  returnNotVoidOrBool = (returnTypeParsed != "bool" && returnTypeParsed != "void" && 
                                                                            returnTypeParsed != "") || isVoidPointer;
                else if (classUnitLanguage == "C#")   returnNotVoidOrBool = (returnTypeParsed != "bool" && returnTypeParsed != "Boolean" && 
                                                                            returnTypeParsed != "void" && returnTypeParsed != "Void" && 
                                                                            returnTypeParsed != "") || isVoidPointer;
                else if (classUnitLanguage == "Java") returnNotVoidOrBool = (returnTypeParsed != "boolean" && returnTypeParsed != "void" && 
                                                                            returnTypeParsed != "Void" && returnTypeParsed != "");

                if (returnNotVoidOrBool && hasComplexReturn && (isDataMemberUsed || (callsOnClassMethodsCount > 0))) 
                    m.setStereotype("property");
                
            
                // void-accessor
                //
                // 1] Return type is void 
                // 2] Contains at least one parameter that is passed by non-const reference and is assigned a value
                // 3] Uses a data member in an expression or has at least 
                //     one function call (except constructor calls) to other methods in class 
                //
                // The "this" keyword by itself is considered (e.g., p = this or p = *this) 
                //  as an accessor to the state of the object where 'p' is passed by reference
                //
                if (isParameterRefModified && (returnTypeParsed == "void") && !isVoidPointer && (isDataMemberUsed || (callsOnClassMethodsCount > 0))) 
                    m.setStereotype("void-accessor");       
                

                // set
                //
                // 1] Only one data member is changed or there is a single call on a data member
                // 2] No calls to methods in class
                //
                // The "this" keyword by itself is considered (e.g., this["index"] = value; for indexers in C#)
                //       
                if (callsOnClassMethodsCount == 0 && 
                   ((dataMembersModifiedCount == 1 && callsOnDataMembersCount == 0) || 
                    (dataMembersModifiedCount == 0 && callsOnDataMembersCount == 1)))
                    m.setStereotype("set"); 
                

                // command
                //
                // Method has a void return type
                // Method is not const or const but has mutable data members (C++ only)
                // Cases:
                //   Case 1: More than one data member is modifed
                //           
                //   Case 2: one data member is modifed and
                //            there is at least one call on a data member or
                //            at least one function call to other methods (except constructor calls) in class
                //   Case 3: zero data members are modifed and
                //            there is at least two calls on data member or
                //            at least one function call to other methods (except constructor calls) in class  
                //
                // The "this" keyword by itself is considered (e.g., this["index"] = value; for indexers in C#)
                //
                // non-void-command    
                //   Method return type is not void
                //             
                bool case1       = dataMembersModifiedCount > 1;
                bool case2       = (dataMembersModifiedCount == 1) && ((callsOnClassMethodsCount + callsOnDataMembersCount) > 0);
                bool case3       = (dataMembersModifiedCount == 0) && ((callsOnDataMembersCount > 1) || (callsOnClassMethodsCount > 0));
                
                bool isMutable = isMethodConst && case1;
                bool isNonVoidReturn = returnTypeParsed != "void" && returnTypeParsed != "Void" && !isVoidPointer;

                if (case1 || case2 || case3) {
                    if (!isMethodConst || isMutable){ // Handles case of mutable data members in C++
                        if (isNonVoidReturn) m.setStereotype("non-void-command");  
                        else m.setStereotype("command");
                    }
                } 
                    

                // factory
                //
                // 1] Factories must include a non-primitive type in their return type
                //      and their return expression must be a local variable, parameter, or data member, that 
                //      call a constructor call or has a return expression with a constructor call (e.g., new)
                //
                //
                if (isNonPrimitiveReturnType && (isNewReturned || isVariableCreatedAndReturnedWithNew))
                    m.setStereotype("factory"); 
                         
                
                // wrapper
                //
                // 1] No data members are modified
                // 2] No calls to methods in class
                // 3] No calls on data members
                // 4] Has at least one free function call 
                // Constructor calls using the 'new' operator are not considered 
                //
                // controller
                //
                // 1] No data members are modified
                // 2] No calls to methods in class
                // 3] No calls on data members
                // 3] Has at least one call to other class methods or mutates a parameter or a local that is non-primitive
                //
                // collaborator
                //
                // 1] It must use at least 1 non-primitive type (not of this class)
                // 2] Type could be a parameter, local variable, return type, or an data member
                //
                //
                if ((dataMembersModifiedCount == 0) && (callsOnClassMethodsCount == 0) && (callsOnDataMembersCount == 0) 
                    && (callsToOtherClassMethods == 0) && (callsOnFreeFunctionsCount > 0)) 
                    m.setStereotype("wrapper");

                else if ((dataMembersModifiedCount == 0) && (callsOnClassMethodsCount == 0) && (callsOnDataMembersCount == 0) &&
                    ((callsToOtherClassMethods > 0) || isNonPrimitiveLocalOrParameterModified))
                    m.setStereotype("controller");   

                else if (isNonPrimitiveDataMemberExternal || isNonPrimitiveLocalExternal || 
                    isNonPrimitiveParamaterExternal || (isNonPrimitiveReturnTypeExternal || isVoidPointer))
                    m.setStereotype("collaborator"); 


                // incidental 
                //
                // 1] Method contains at least one non-comment statement (i.e., method is not empty)
                // 2] No data members are used or modified (including no use of keyword "this" by itself)
                // 3] No calls of any kind
                // 
                bool noCalls = callsOnClassMethodsCount == 0 && callsOnDataMembersCount == 0 && 
                               newConstructorCallsCount == 0 && callsToOtherClassMethods == 0 && callsOnFreeFunctionsCount == 0;

                if (!isDataMemberUsed & noCalls) 
                    m.setStereotype("incidental");          
            
                    
                // stateless
                //
                // 1]	Method contains at least one non-comment statement (i.e., method is not empty)
                // 2]	No data members are used or modified (including no use of keyword "this" by itself)
                // 3]	No calls to methods in class 
                // 4]   No calls on data members
                // 5]   Has at least one call to other class methods (including constructor calls) or to a free function 
                //
                if (!isDataMemberUsed && callsOnClassMethodsCount == 0 && callsOnDataMembersCount == 0 &&
                   ((callsOnFreeFunctionsCount > 0) || (callsToOtherClassMethods > 0) || (newConstructorCallsCount > 0)))
                   m.setStereotype("stateless");             
                
            }
            
            // unclassified
            //
            // No stereotype found
            //
            if (m.getStereotypeList().size() == 0)  m.setStereotype("unclassified");

            // Used to for re-documenting the system with the stereotype information
            XPATH_LIST[m.getUnitNumber()].insert({m.getXpath(), m.getStereotype()});    
        }
        pair.second.setConstructorDestructorCount(constructorDestructorCount);
    }
}


// Compute class stereotype
// Constructors and destructors are not considered in the computation of class stereotypes
// Other structures (e.g., struct, interface, enum, and unions) are labeled with class stereotypes
// 
void stereotypes::computeClassStereotypes(std::unordered_map<std::string, classModel>& classCollection) {
    for (auto& pair : classCollection) {
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

        const std::vector<methodModel>& methods           = pair.second.getMethods();
        int                             nonCollaborators  = 0;
        for (const auto& m : methods) {      
            if (!m.isConstructorOrDestructor()) {
                for (const std::string& s : m.getStereotypeList()) methodStereotypes[s]++;
            
                std::string methodStereotype = m.getStereotype();
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


        // Large Class
        //
        {
            int accPlusMut = accessors + mutators;
            int facPlusCon = controllers + factory;
            if (((0.2 * allMethods < accPlusMut) && (accPlusMut < 0.67 * allMethods )) &&
                ((0.2 * allMethods < facPlusCon) && (facPlusCon < 0.67 * allMethods )) &&
                (factory != 0) && (controllers != 0) && (accessors != 0) && (mutators != 0) ) {
                    if (allMethods > METHODS_PER_CLASS_THRESHOLD) { 
                        pair.second.setStereotype("large-class");
                }
            }
        }


        // Lazy Class
        //
        if ((getters + setters != 0) && (((degenerates / double(allMethods)) > 0.33)) &&
        (((allMethods - (degenerates + getters + setters)) / double(allMethods))  <= 0.2))
            pair.second.setStereotype("lazy-class");
        

        // Degenerate Class
        //
        if ((degenerates / double(allMethods)) > 0.5)  
            pair.second.setStereotype("degenerate");
        

        // Data Class
        //
        if ((allMethods - (getters + setters) == 0) && ((getters + setters) != 0))
            pair.second.setStereotype("data-class");
        

        // Small Class
        //
        if ((0 < allMethods) && (allMethods < 3))
            pair.second.setStereotype("small-class");


        // Empty Class (Considered degenerate)
        //
        if (allMethods == 0)
            pair.second.setStereotype("empty");
   

        if (pair.second.getStereotype().size() == 0) 
            pair.second.setStereotype("unclassified");

        const std::unordered_map<int, std::vector<std::string>>& xpath = pair.second.getXpath();
        for (const auto& pairXpath : xpath) 
            for (const auto& classXpath : pairXpath.second) XPATH_LIST[pairXpath.first].insert({classXpath, pair.second.getStereotype()});
    }  
}

void stereotypes::computeFreeFunctionsStereotypes(std::vector<methodModel>& freeFunctions) {
    for (methodModel& f : freeFunctions) {
        // Common operations
        const std::string& methodName                               = f.getName();
        const std::string& returnTypeParsed                         = f.getReturnTypeParsed(); 
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
            // Does not change parameters passed by reference. Has at least one call to other class methods or to a free function  
            // Constructor calls using the 'new' operator are not considered
            //
            if (!isParameterRefModified && (nonNewConstructorCallsCount > 0)) f.setStereotype("wrapper");


            if (f.getStereotype() == "") f.setStereotype("unclassified");
        }
        XPATH_LIST[f.getUnitNumber()].insert({f.getXpath(), f.getStereotype()});
    }
}