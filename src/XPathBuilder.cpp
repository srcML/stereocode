// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file XPathBuilder.cpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
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
    // Unions cannot inherit, but can be declared anonymous (union without a name) 
    // Anonymous unions can only be declared inside a class, struct, or a namespace and cannot have methods 
    //  and their field are accessed directly as part of the enclosing scope
    //  so, field that are defined inside anonymous unions are considered and the anonymous union itself is not considered
    //  also, anonymous unions that are defined in a namespace are not considered because their field are basically globals
    // Classes can be declared without a name (anonymous classes) require you to simultaneously create an instance of it during its definition
    //  so, these are treated as normal classes
    std::string language = "C++";

    std::string xpath = "//src:*[(self::src:class";
    if (STRUCT) xpath += " or self::src:struct";        
    if (UNION) xpath += " or self::src:union[src:name]";  // Only non-anonymous unions are collected     
    xpath += ") and not(ancestor::src:class or ancestor::src:struct or ancestor::src:union)]"; 
    xpathTable[language]["class"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:union]/src:name";
    xpathTable[language]["class_name"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:union]/text()[1]";
    xpathTable[language]["class_type"] = xpath;   

    xpath = "/src:unit/src:*[self::src:class or self::src:struct]/src:super_list/src:super";
    xpathTable[language]["parent_name"] = xpath;  

    // This does not count unions without a name, so their fields will be collected even if nested without a class or a struct
    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union[src:name]) = 1]";
    xpath += "/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["field_name"] = xpath;  

    // Static fields are ignored and treated as globals
    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union[src:name]) = 1]";
    xpath += "/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["field_type"] = xpath;  

    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union[src:name]) = 1";
    xpath += " and (ancestor::src:class and (ancestor::src:public or ancestor::src:protected))"; 
    xpath += " or (ancestor::src:struct and not(ancestor::src:private))]/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["non_private_field_name"] = xpath;  

    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union[src:name]) = 1";
    xpath += " and (ancestor::src:class and (ancestor::src:public or ancestor::src:protected))"; 
    xpath += " or (ancestor::src:struct and not(ancestor::src:private))]/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["non_private_field_type"] = xpath;  

    xpath = "//*[(self::src:function or self::src:constructor or self::src:destructor)";
    xpath += " and not(src:type/src:specifier='static') and count(ancestor::src:class | ancestor::src:struct | ancestor::src:union) = 1]";
    xpathTable[language]["method"] = xpath; 

    xpath = "//*[self::src:function and (not(ancestor::src:class or ancestor::src:struct or ancestor::src:union) or src:type/src:specifier='static')]";
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

    xpath = "//src:expr/src:name";
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
    // C# allows static classes to be declared, and these must contain only static members
    // They are ignored and their methods are collected as free functions
    // Nested classes, struct, or interfaces are ignored
    language = "C#";
    xpath = "//src:*[(self::src:class";
    if (STRUCT) xpath += " or self::src:struct";
    if (INTERFACE) xpath += " or self::src:interface";             
    xpath += ") and not(src:specifier='static') and not(ancestor::src:class or ancestor::src:struct or ancestor::src:interface)]"; 
    xpathTable[language]["class"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:interface]/src:name";
    xpathTable[language]["class_name"] = xpath;

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:interface]/text()[1]";
    xpathTable[language]["class_type"] = xpath;   

    xpath = "/src:unit/src:*[self::src:class or self::src:struct or self::src:interface]/src:super_list/src:super/src:name";
    xpathTable[language]["parent_name"] = xpath;  

    // Auto-properties can be used to declare data members implicitly 
    // And normal properties get or set data members (not always)
    // Therefore, properties will be treated as data members as they can be used and called as normal data members  
    //  where property name = data member name and where property type = data member type 
    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1]";
    xpath += "/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpath += " | //src:property[count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1]/src:name";
    xpathTable[language]["field_name"] = xpath;  

    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1]";
    xpath += "/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpath += " | //src:property[count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1]/src:type";
    xpathTable[language]["field_type"] = xpath;  

    // Also ignores fields with no specifier
    // If struct or interface, include the fields with no specifier
    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1]";
    xpath += "/src:decl[(ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or ((ancestor::src:struct or ancestor::src:interface) and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]"; 
    xpath += "/src:name[preceding-sibling::*[1][self::src:type]]"; 
    xpath += " | //src:property[count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1"; 
    xpath += " and (ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or ((ancestor::src:struct or ancestor::src:interface) and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]/src:name"; 
    xpathTable[language]["non_private_field_name"] = xpath;  

    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1]";
    xpath += "/src:decl[(ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or ((ancestor::src:struct or ancestor::src:interface) and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]"; 
    xpath += "/src:type[following-sibling::*[1][self::src:name]]"; 
    xpath += " | //src:property[count(ancestor::src:class | ancestor::src:struct | ancestor::src:interface) = 1"; 
    xpath += " and (ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or ((ancestor::src:struct or ancestor::src:interface) and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]/src:type"; 
    xpathTable[language]["non_private_field_type"] = xpath;  

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

    xpath = "//*[(self::src:function or self::src:constructor) and src:type/src:specifier='static']";
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

    xpath = "//src:expr[count(ancestor::src:function) = 1]/src:name";
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

    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpath += "/src:decl/src:name[preceding-sibling::*[1][self::src:type]]";
    xpathTable[language]["field_name"] = xpath;  

    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpath += "/src:decl/src:type[following-sibling::*[1][self::src:name]]";
    xpathTable[language]["field_type"] = xpath;  

    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpath += "/src:decl[(ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or (ancestor::src:interface and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]"; 
    xpath += "/src:name[preceding-sibling::*[1][self::src:type]]"; 
    xpathTable[language]["non_private_field_name"] = xpath; 

    xpath = "//src:decl_stmt[not(src:decl/src:type/src:specifier='static') and not(ancestor::src:function) and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpath += "/src:decl[(ancestor::src:class and src:type/src:specifier[not(.='private')])"; 
    xpath += " or (ancestor::src:interface and src:type/src:specifier[not(.='private')] or not(src:type/src:specifier))]"; 
    xpath += "/src:type[following-sibling::*[1][self::src:name]]"; 
    xpathTable[language]["non_private_field_type"] = xpath; 

    xpath = "//*[(self::src:function or self::src:constructor)";
    xpath += " and not(src:type/src:specifier='static') and count(ancestor::src:class | ancestor::src:interface | ancestor::src:enum) = 1]";
    xpathTable[language]["method"] = xpath; 

    xpath = "//*[self::src:function and src:type/src:specifier='static']";
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

    xpath = "//src:call[not(src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["function_call_name"] = xpath;  

    xpath = "//src:call[src:name/src:operator='.' and not(preceding-sibling::*[1][self::src:operator='new'])]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["method_call_name"] = xpath;  

    xpath = "//src:call[preceding-sibling::*[1][self::src:operator='new']]/src:name[following-sibling::*[1][self::src:argument_list]]";
    xpathTable[language]["constructor_call_name"] = xpath;  

    xpath = "//src:call[not(src:name/src:operator='.') and not(preceding-sibling::*[1][self::src:operator='new'])]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["function_call_arglist"] = xpath;  

    xpath = "//src:call[src:name/src:operator='.' and not(preceding-sibling::*[1][self::src:operator='new'])]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["method_call_arglist"] = xpath; 

    xpath = "//src:call[preceding-sibling::*[1][self::src:operator='new']]/src:argument_list[preceding-sibling::*[1][self::src:name]]";
    xpathTable[language]["constructor_call_arglist"] = xpath;

    xpath = "//src:decl_stmt/src:decl[./src:init/src:expr/src:operator[.='new']]/src:name";
    xpath += " | //src:expr_stmt/src:expr[./src:operator[.='new']]/src:name";
    xpathTable[language]["new_operator_assign"] = xpath; 

    xpath = "//src:block_content[1][*[not(self::src:comment)][1]]";
    xpathTable[language]["empty"] = xpath; 

    xpath = "//src:expr/src:name";
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