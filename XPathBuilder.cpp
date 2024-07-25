// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file XPathBuilder.cpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "XPathBuilder.hpp"

extern bool   STRUCT;        
extern bool   INTERFACE;   
extern bool   UNION;
extern bool   ENUM;

void XPathBuilder::generateXpath() {
    //// C++ ////
    // Unions can't inherit, but can be declared anonymous (union without a name)
    // Anonymous unions can't have methods, and their data members can be accessed directly without a dot or arrow
    // Therefore, anonymous unions declared outside classes or structs are ignored
    //  and data members that are defined in anonymous unions nested in classes or structs are considered
    // Classes can be decalared without a name, but you need an instance to use their data members
    //  Therefore, these are treated as normal classes
    std::string language = "C++";

    std::string xpath = "//src:*[(self::src:class";
    if (STRUCT) xpath += " or self::src:struct";        
    if (UNION) xpath += " or self::src:union[src:name]";       
    xpath += ") and not(ancestor::src:class or ancestor::src:struct or ancestor::src:union)]"; 
    xpathTable[language]["class"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:union]/src:name";
    xpathTable[language]["class_name"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:union]/text()[1]";
    xpathTable[language]["class_type"] = xpath;   

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:union]/src:super_list/src:super";
    xpathTable[language]["parent_name"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union[src:name]) = 1]";
    xpath += "/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["attribute_name"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union[src:name]) = 1]";
    xpath += "/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["attribute_type"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union[src:name]) = 1";
    xpath += " and (ancestor::src:class and (ancestor::src:public or ancestor::src:protected))"; 
    xpath += " or (ancestor::src:struct and not(ancestor::src:private))]/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["non_private_attribute_name"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union[src:name]) = 1";
    xpath += " and (ancestor::src:class and (ancestor::src:public or ancestor::src:protected))"; 
    xpath += " or (ancestor::src:struct and not(ancestor::src:private))]/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["non_private_attribute_type"] = xpath;  

    xpath = "//*[(self::src:function or self::src:constructor or self::src:destructor)";
    xpath += " and not(src:type/src:specifier='static') and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union) = 1";
    // This is needed to make sure that methods defined in a class where the class 
    //  is inside a free function are stereotyped correctly (C++ case)
    //  and not treated as a nested function
    xpath += " and not(ancestor::src:function[not(descendant::src:class | descendant::src:struct | descendant::src:union)])]";
    xpathTable[language]["method"] = xpath; 

    xpath = "//*[(self::src:function or self::src:constructor or self::src:destructor)";
    xpath += " and (not(ancestor::src:class or ancestor::src:struct or ancestor::src:union) or src:type/src:specifier='static')]";
    xpathTable[language]["free_function"] = xpath; 

    xpath = "/src:unit/src:function/src:name";
    xpathTable[language]["method_name"] = xpath; 

    xpath = "/src:unit/*[self::src:constructor or self::src:destructor]/src:name";
    xpathTable[language]["constructor_destructor_name"] = xpath; 

    xpath = "/src:unit/*[self::src:constructor or self::src:destructor]";
    xpathTable[language]["constructor_or_destructor"] = xpath;

    xpath = "/src:unit/*[self::src:constructor or self::src:destructor]/src:parameter_list";
    xpathTable[language]["constructor_destructor_parameter_list"] = xpath; 

    xpath = "/src:unit/src:function/src:parameter_list";
    xpathTable[language]["method_parameter_list"] = xpath; 

    // This skips the generics parameters in Java return types
    xpath = "/src:unit/src:function/src:type//text()[not(ancestor::src:parameter_list)]";
    xpathTable[language]["method_return_type"] = xpath; 

    xpath = "//src:decl_stmt/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpath += " | //src:control/src:init/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["local_variable_name"] = xpath; 

    xpath = "//src:decl_stmt/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpath += " | //src:control/src:init/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["local_variable_type"] = xpath; 

    xpath = "/src:unit/src:function/src:parameter_list/src:parameter/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["parameter_name"] = xpath; 

    xpath = "/src:unit/src:function/src:parameter_list/src:parameter/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["parameter_type"] = xpath; 

    xpath = "//src:return/src:expr";
    xpathTable[language]["return_expression"] = xpath; 

    // Constructor calls are a type of function calls 
    xpath = "//src:call[not(src:name/src:operator='->') and not(src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["function_call_name"] = xpath;  

    xpath = "//src:call[(src:name/src:operator='->' or src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["method_call_name"] = xpath;  

    xpath = "//src:call[preceding-sibling::*[1][self::src:operator='new']]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["constructor_call_name"] = xpath;  

    xpath = "//src:call[not(src:name/src:operator='->') and not(src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["function_call_arglist"] = xpath;  

    xpath = "//src:call[(src:name/src:operator='->' or src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["method_call_arglist"] = xpath; 

    xpath = "//src:call[preceding-sibling::*[1][self::src:operator='new']]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["constructor_call_arglist"] = xpath; 

    xpath = "//src:decl_stmt/src:decl[./src:init/src:expr/src:operator[.='new']]/src:name";
    xpath += " | //src:expr_stmt[count(ancestor::src:function) = 1]/src:expr[./src:operator[.='new']]/src:name";
    xpathTable[language]["new_operator_assign"] = xpath; 

    xpath = "/src:unit/src:function/src:specifier[.='const']";
    xpathTable[language]["const"] = xpath; 

    xpath = "//src:block_content[1][*[not(self::src:comment)][1]]";
    xpathTable[language]["empty"] = xpath; 

    xpath = "//src:expr[not(ancestor::src:call)]/src:name";
    xpathTable[language]["expression_name"] = xpath;    

    xpath = "//src:expr/src:name[";
    xpath += "following-sibling::*[1][self::src:operator='=' or self::src:operator='+='";
    xpath += " or self::src:operator='-=' or self::src:operator='*=' or self::src:operator='/='";
    xpath += " or self::src:operator='%=' or self::src:operator='>>=' or self::src:operator='<<='";
    xpath += " or self::src:operator='&=' or self::src:operator='^=' or self::src:operator='|='";
    xpath += " or self::src:operator='\\?\\?=' or self::src:operator='>>>=' or self::src:operator='++'"; 
    xpath += " or self::src:operator='--'] or preceding-sibling::*[1][self::src:operator='++' or self::src:operator='--']]";
    xpathTable[language]["expression_assignment"] = xpath;    

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// C# ////
    // C# allows static classes to be declared, and these must contain only static data members and static fields
    // They are ignored and their methods are collected as free functions
    language = "C#";
    xpath = "//src:*[(self::src:class";
    if (STRUCT) xpath += " or self::src:struct";
    if (INTERFACE) xpath += " or self::src:interface";             
    xpath += ") and not(src:specifier='static') and not(ancestor::src:class or ancestor::src:struct or ancestor::src:interface)]"; // Ignores nested classes, struct, or interfaces
    xpathTable[language]["class"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:interface]/src:name";
    xpathTable[language]["class_name"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:interface]/text()[1]";
    xpathTable[language]["class_type"] = xpath;   

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:interface]/src:super_list/src:super/src:name";
    xpathTable[language]["parent_name"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1]";
    xpath += "/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    // Auto-properties can be used to declare fields implicitly where property name = field name 
    // We can detect an auto property with count(descendant::src:function) = 0
    xpath += " | //src:property[count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1 and count(descendant::src:function) = 0]/src:name";
    xpathTable[language]["attribute_name"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1]";
    xpath += "/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpath += " | //src:property[count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1 and count(descendant::src:function) = 0]/src:type";
    xpathTable[language]["attribute_type"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1]";
    // Also ignores attributes with no specifier
    // If struct or interface, include the attributes with no specifier
    xpath += "/src:decl[(ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or ((ancestor::src:struct or ancestor::src:interface) and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]"; 
    xpath += "/src:name[preceding-sibling::*[1][self::src:type]]"; 
    xpath += " | //src:property[count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1"; 
    xpath += " and count(descendant::src:function) = 0 and (ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or ((ancestor::src:struct or ancestor::src:interface) and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]/src:name"; 
    xpathTable[language]["non_private_attribute_name"] = xpath;  

    xpath += "/src:decl[(ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or ((ancestor::src:struct or ancestor::src:interface) and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]"; 
    xpath += "/src:type[following-sibling::*[1][self::src:name]]"; 
    xpath += " | //src:property[count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1"; 
    xpath += " and count(descendant::src:function) = 0 and (ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or ((ancestor::src:struct or ancestor::src:interface) and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]/src:type"; 
    xpathTable[language]["non_private_attribute_type"] = xpath;  

    // Nested local functions within methods in C# are ignored 
    xpath = "//*[(self::src:function or self::src:constructor or self::src:destructor)";
    xpath += " and count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1";
    xpath += " and not(src:type/src:specifier='static') and not(ancestor::src:function) and not(ancestor::src:property)]";
    xpathTable[language]["method"] = xpath; 

    // C# properties can't be nested in methods or in other properties, so no need to check
    xpath = "//src:property[count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1 and not(src:type/src:specifier='static')]";
    xpathTable[language]["property"] = xpath; 

    xpath = "//src:property/src:type";
    xpathTable[language]["property_type"] = xpath; 

    xpath = "//src:function[not(ancestor::src:function)]";
    xpathTable[language]["property_method"] = xpath; 

    xpath = "//*[(self::src:function or self::src:constructor or self::src:destructor) and src:type/src:specifier='static']";
    xpathTable[language]["free_function"] = xpath; 

    xpath = "/src:unit/src:function/src:name";
    xpathTable[language]["method_name"] = xpath; 

    xpath = "/src:unit/*[self::src:constructor or self::src:destructor]/src:name";
    xpathTable[language]["constructor_destructor_name"] = xpath; 

    xpath = "/src:unit/*[self::src:constructor or self::src:destructor]";
    xpathTable[language]["constructor_or_destructor"] = xpath;

    xpath = "/src:unit/*[self::src:constructor or self::src:destructor]/src:parameter_list";
    xpathTable[language]["constructor_destructor_parameter_list"] = xpath; 

    xpath = "/src:unit/src:function/src:parameter_list";
    xpathTable[language]["method_parameter_list"] = xpath; 

    xpath = "/src:unit/src:function/src:type//text()[not(ancestor::src:parameter_list)]";
    xpathTable[language]["method_return_type"] = xpath; 

    xpath = "//src:decl_stmt[count(ancestor::src:function) = 1]/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpath += " | //src:control/src:init/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["local_variable_name"] = xpath; 

    xpath = "//src:decl_stmt[count(ancestor::src:function) = 1]/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpath += " | //src:control/src:init/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["local_variable_type"] = xpath; 

    xpath = "/src:unit/src:function/src:parameter_list/src:parameter/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["parameter_name"] = xpath; 

    xpath = "/src:unit/src:function/src:parameter_list/src:parameter/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["parameter_type"] = xpath; 

    xpath = "//src:return[count(ancestor::src:function) = 1]/src:expr";

    xpathTable[language]["return_expression"] = xpath; 

    xpath = "//src:call[count(ancestor::src:function) = 1 and not(src:name/src:operator='->') and not(src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["function_call_name"] = xpath;  

    xpath = "//src:call[count(ancestor::src:function) = 1 and (src:name/src:operator='->' or src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["method_call_name"] = xpath;  

    xpath = "//src:call[count(ancestor::src:function) = 1 and preceding-sibling::*[1][self::src:operator='new']]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["constructor_call_name"] = xpath;  

    xpath = "//src:call[count(ancestor::src:function) = 1 and not(src:name/src:operator='->') and not(src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["function_call_arglist"] = xpath;  

    xpath = "//src:call[count(ancestor::src:function) = 1 and (src:name/src:operator='->' or src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["method_call_arglist"] = xpath; 

    xpath = "//src:call[count(ancestor::src:function) = 1 and preceding-sibling::*[1][self::src:operator='new']]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["constructor_call_arglist"] = xpath;

    xpath = "//src:decl_stmt[count(ancestor::src:function) = 1]/src:decl[./src:init/src:expr/src:operator[.='new']]/src:name";
    xpath += " | //src:expr_stmt[count(ancestor::src:function) = 1]/src:expr[./src:operator[.='new']]/src:name";
    xpathTable[language]["new_operator_assign"] = xpath;  

    xpath = "//src:block_content[1][*[not(self::src:comment)][1]]";
    xpathTable[language]["empty"] = xpath; 

    xpath = "//src:expr[count(ancestor::src:function) = 1 and not(ancestor::src:call)]/src:name";
    xpathTable[language]["expression_name"] = xpath;    

    xpath = "//src:expr[count(ancestor::src:function) = 1]/src:name[";
    xpath += "following-sibling::*[1][self::src:operator='=' or self::src:operator='+='";
    xpath += " or self::src:operator='-=' or self::src:operator='*=' or self::src:operator='/='";
    xpath += " or self::src:operator='%=' or self::src:operator='>>=' or self::src:operator='<<='";
    xpath += " or self::src:operator='&=' or self::src:operator='^=' or self::src:operator='|='";
    xpath += " or self::src:operator='\\?\\?=' or self::src:operator='>>>=' or self::src:operator='++'"; 
    xpath += " or self::src:operator='--'] or preceding-sibling::*[1][self::src:operator='++' or self::src:operator='--']]";
    xpathTable[language]["expression_assignment"] = xpath;  
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// Java ////
    // enums in Java can contain methods and fields
    // enums They are implicitly static
    // Java allows static classes, but these can only be nested within other classes, interfaces, or enums
    // Static classes in java can contain non-static fields or methods
    // They are ignored (since they are nested) and their methods (only if static) are collected as free functions
    language = "Java";
    xpath = "//src:*[((self::src:class and not(child::src:super[1]))"; // Ignores anonymous classes (classes without names and are nested as instances)
    if (INTERFACE) xpath += " or self::src:interface";       
    if (ENUM) xpath += " or self::src:enum";        
    xpath += ") and not(ancestor::src:class or ancestor::src:interface or ancestor::src:enum)]"; 

    xpathTable[language]["class"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:interface or self::src:enum]/src:name";
    xpathTable[language]["class_name"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:interface or self::src:enum]/text()[1]";
    xpathTable[language]["class_type"] = xpath;   

    xpath = "/src:unit/src:*[self::src:class or self::src:interface or self::src:enum]/src:super_list/*[self::src:extends or self::src:implements]/src:super/src:name";
    xpathTable[language]["parent_name"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpath += "/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["attribute_name"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpath += "/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["attribute_type"] = xpath;  

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpath += "/src:decl[(ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or (ancestor::src:interface and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]"; 
    xpath += "/src:name[preceding-sibling::*[1][self::src:type]]"; 
    xpathTable[language]["non_private_attribute_name"] = xpath; 

    xpath = "//src:decl_stmt[not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpath += "/src:decl[(ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or (ancestor::src:interface and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]"; 
    xpath += "/src:type[following-sibling::*[1][self::src:name]]"; 
    xpathTable[language]["non_private_attribute_type"] = xpath; 

    xpath = "//*[(self::src:function or self::src:constructor)";
    xpath += " and not(src:type/src:specifier='static') and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpathTable[language]["method"] = xpath; 

    xpath = "//*[(self::src:function or self::src:constructor) and src:type/src:specifier='static']";
    xpathTable[language]["free_function"] = xpath; 

    xpath = "/src:unit/src:function/src:name";
    xpathTable[language]["method_name"] = xpath; 

    xpath = "/src:unit/*[self::src:constructor]/src:name";
    xpathTable[language]["constructor_destructor_name"] = xpath; 

    xpath = "/src:unit/*[self::src:constructor]";
    xpathTable[language]["constructor_or_destructor"] = xpath;

    xpath = "/src:unit/*[self::src:constructor]/src:parameter_list";
    xpathTable[language]["constructor_destructor_parameter_list"] = xpath; 

    xpath = "/src:unit/src:function/src:parameter_list";
    xpathTable[language]["method_parameter_list"] = xpath; 

    xpath = "/src:unit/src:function/src:type//text()[not(ancestor::src:parameter_list)]";
    xpathTable[language]["method_return_type"] = xpath; 

    xpath = "//src:decl_stmt/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpath += " | //src:control/src:init/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["local_variable_name"] = xpath; 
    
    xpath = "//src:decl_stmt/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpath += " | //src:control/src:init/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["local_variable_type"] = xpath; 

    xpath = "/src:unit/src:function/src:parameter_list/src:parameter/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["parameter_name"] = xpath; 

    xpath = "/src:unit/src:function/src:parameter_list/src:parameter/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["parameter_type"] = xpath; 

    xpath = "//src:return/src:expr";
    xpathTable[language]["return_expression"] = xpath; 

    xpath = "//src:call[not(src:name/src:operator='->') and not(src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["function_call_name"] = xpath;  

    xpath = "//src:call[(src:name/src:operator='->' or src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["method_call_name"] = xpath;  

    xpath = "//src:call[preceding-sibling::*[1][self::src:operator='new']]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["constructor_call_name"] = xpath;  

    xpath = "//src:call[not(src:name/src:operator='->') and not(src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["function_call_arglist"] = xpath;  

    xpath = "//src:call[(src:name/src:operator='->' or src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["method_call_arglist"] = xpath; 

    xpath = "//src:call[preceding-sibling::*[1][self::src:operator='new']]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["constructor_call_arglist"] = xpath;

    xpath = "//src:decl_stmt/src:decl[./src:init/src:expr/src:operator[.='new']]/src:name";
    xpath += " | //src:expr_stmt/src:expr[./src:operator[.='new']]/src:name";
    xpathTable[language]["new_operator_assign"] = xpath; 

    xpath = "//src:block_content[1][*[not(self::src:comment)][1]]";
    xpathTable[language]["empty"] = xpath; 

    xpath = "//src:expr[not(ancestor::src:call)]/src:name";
    xpathTable[language]["expression_name"] = xpath;    

    xpath = "//src:expr/src:name[";
    xpath += "following-sibling::*[1][self::src:operator='=' or self::src:operator='+='";
    xpath += " or self::src:operator='-=' or self::src:operator='*=' or self::src:operator='/='";
    xpath += " or self::src:operator='%=' or self::src:operator='>>=' or self::src:operator='<<='";
    xpath += " or self::src:operator='&=' or self::src:operator='^=' or self::src:operator='|='";
    xpath += " or self::src:operator='\\?\\?=' or self::src:operator='>>>=' or self::src:operator='++'"; 
    xpath += " or self::src:operator='--'] or preceding-sibling::*[1][self::src:operator='++' or self::src:operator='--']]";
    xpathTable[language]["expression_assignment"] = xpath;  

}

const std::string& XPathBuilder::getXpath(const std::string& language, const std::string& xpathName) {
    return xpathTable[language][xpathName];
}