//ClassInfo for stereocode
//

#include "ClassInfo.hpp"

//
//
ClassInfo::ClassInfo(srcml_archive* input_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    findClassName(input_archive, hpp_unit);
    findParentClassName(input_archive, hpp_unit);

    findAttributeNames(input_archive, hpp_unit);
    findAttributeTypes(input_archive, hpp_unit);

    findMethodNames(input_archive, hpp_unit);
    findMethodNames(input_archive, cpp_unit);

    findParameterLists(input_archive, hpp_unit);
    findParameterLists(input_archive, cpp_unit);

    findMethodHeaders(input_archive, hpp_unit, true);
    findMethodHeaders(input_archive, cpp_unit, false);
    
    findMethodReturnTypes(input_archive, hpp_unit);
    findMethodReturnTypes(input_archive, cpp_unit);
    readPrimitives();
}

//
//
void ClassInfo::findClassName(srcml_archive* input_archive, srcml_unit* unit){

    srcml_append_transform_xpath(input_archive, "//src:class/src:name");

    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(input_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);

    for (int i = 0; i < number_of_result_units; ++i){
        srcml_unit* result_unit = srcml_transform_get_unit(result,i);
        std::string name = srcml_unit_get_srcml(result_unit);

        // chop off begining and ending <name></name>
        size_t end_position = name.find("</name>");
        name = name.substr(6,end_position-6);

        class_name = name;
    }
    srcml_clear_transforms(input_archive);
    srcml_transform_free(result);
}

//
//
void ClassInfo::findParentClassName(srcml_archive* input_archive, srcml_unit* unit){
    srcml_append_transform_xpath(input_archive, "//src:class/src:super_list/src:super/src:name");
    
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(input_archive, unit, &result);
    
    int number_of_result_units = srcml_transform_get_unit_size(result);
    for (int i = 0; i < number_of_result_units; ++i){
        srcml_unit* result_unit = srcml_transform_get_unit(result, i);
        std::string name = srcml_unit_get_srcml(result_unit);

        size_t end_position = name.find("</name>");
        name = name.substr(6,end_position-6);

        parent_class_names.push_back(name);
        //std::cout << "pushed back the super class name " << parent_class_names[i] << "\n";
    }
    srcml_clear_transforms(input_archive);
    srcml_transform_free(result);
}

//
//
void ClassInfo::findAttributeNames(srcml_archive* input_archive, srcml_unit* unit){
    // append xpath to archive, gives variable names
    srcml_append_transform_xpath(input_archive, "//src:class//src:decl_stmt[not(ancestor::src:function)]/src:decl/src:name");

    
    // apply xpath transformation to the archive
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(input_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);

    srcml_unit* result_unit = nullptr;
    // proessing results to collect variable names.
    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result,i);
        std::string attribute_name = srcml_unit_get_srcml(result_unit);

        // chop off everything beyond variable name
        size_t end_position = attribute_name.find("</name>");
        attribute_name = attribute_name.substr(0,end_position);

        // chop off begining <name>(regular variables) or <name><name>(arrays)
        while (attribute_name.substr(0,6) == "<name>")
            attribute_name.erase(0,6);

        attribute_names.push_back(attribute_name);
    }

    srcml_clear_transforms(input_archive);
    srcml_transform_free(result);
}

//
//
void ClassInfo::findAttributeTypes(srcml_archive* method_archive, srcml_unit* unit){
    srcml_append_transform_xpath(method_archive, "//src:class//src:decl_stmt[not(ancestor::src:function)]/src:decl/src:type");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;
    //std::cout << "NUMBER OF RESULTS FOR ATTR TYPES " << number_of_result_units << "\n";
    std::string prev = "";
    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result, i);
        std::string attr_type = srcml_unit_get_srcml(result_unit);
        
        char* unparsed = new char[attr_type.size() + 1];
        std::strcpy(unparsed, attr_type.c_str());
        size_t size = attr_type.size();

        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        if (attr_type == "<type ref=\"prev\"/>"){
            attr_type = prev;
        }else{  
            attr_type = unparsed;
            prev = attr_type;
        }
        delete[] unparsed;

        //std::cout << "adding attribute TYPE: "<< attr_type << std::endl;
        //std::cout << "_______\n";

        attribute_types.push_back(attr_type); 
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);
}

//
//
void ClassInfo::findMethodHeaders(srcml_archive* method_archive, srcml_unit* unit, bool inline_list){

    srcml_append_transform_xpath(method_archive, "//src:function");

    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);

    if (inline_list){
        inline_function_count = number_of_result_units;
    }
    else{
        outofline_function_count = number_of_result_units;
    }
    srcml_unit* result_unit = nullptr;
    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result,i);

        std::string function_srcml = srcml_unit_get_srcml(result_unit);
        addConstSpecifier(function_srcml);

        char * unparsed = new char [function_srcml.size() + 1];
        size_t size = function_srcml.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        std::string function_header(unparsed);
        delete[] unparsed;
        
        function_header = function_header.substr(0, function_header.find("{"));
        //remove newline characters
        function_header.erase(std::remove(function_header.begin(), function_header.end(), '\n'), function_header.end());

        headers.push_back(function_header);
        stereotypes.push_back("nothing-yet");
        //specifiers.push_back(false);
    }

    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);    
}

//
// Adds const to specifiers list
void ClassInfo::addConstSpecifier(std::string function_srcml){
    trimWhitespace(function_srcml);
    size_t end = function_srcml.find("{");
    std::string function_srcml_header = function_srcml.substr(0, end);

    if (function_srcml_header.find("<specifier>const</specifier><block>") != std::string::npos){
        specifiers.push_back("const");
    } else if (function_srcml_header.find("</parameter_list><specifier>const</specifier>") != std::string::npos){
        specifiers.push_back("const");
    } else {
        specifiers.push_back("");
    }
}

//
//
void ClassInfo::findMethodNames(srcml_archive* method_archive, srcml_unit* unit){
    srcml_append_transform_xpath(method_archive, "//src:function/src:name");

    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;
    
    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result,i);

        std::string name_srcml = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [name_srcml.size() + 1];
        size_t size = name_srcml.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        std::string function_name(unparsed);
        delete[] unparsed;
        method_names.push_back(function_name);
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);  
}

//
//
void ClassInfo::findParameterLists(srcml_archive* method_archive, srcml_unit* unit){
    srcml_append_transform_xpath(method_archive, "//src:function/src:parameter_list");

    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;
    
    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result,i);

        std::string name_srcml = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [name_srcml.size() + 1];
        size_t size = name_srcml.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        std::string parameter_list(unparsed);
        delete[] unparsed;
        parameter_lists.push_back(parameter_list);
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);  
}


//
//
// collects return types for each function
void ClassInfo::findMethodReturnTypes(srcml_archive* method_archive, srcml_unit* unit){
    srcml_append_transform_xpath(method_archive, "//src:function/src:type");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);

    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result,i);

        std::string return_type = srcml_unit_get_srcml(result_unit);
        //std::cout << "return type srcml = " << return_type << std::endl;

        char * unparsed = new char[return_type.size() + 1];
        std::strcpy (unparsed, return_type.c_str());
        size_t size = return_type.size() +1;

        //std::cout << unparsed << std::endl;
        int error = srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string ret_type(unparsed);

        
        delete[] unparsed;

        return_types.push_back(ret_type);
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);    
}

//
//
void ClassInfo::readPrimitives(){
    std::ifstream primitives_file;
    primitives_file.open("PrimitiveTypes.txt");

    if (primitives_file.is_open()){
        std::string type;
        std::getline(primitives_file, type);
        std::getline(primitives_file, type);
        std::getline(primitives_file, type);

        while(std::getline(primitives_file, type)){
            primitive_types.push_back(type);
        }
    }
}

// Stereotype get:
// method is const,
// contains at least 1 return statement that returns a data memeber
// return expression must be in the form 'return a;' or 'return *a;'

// Stereotype non-const get:
// method is not const
// contains at least 1 return statement that returns a data memeber
// return expression must be in the form 'return a;' or 'return *a;'
void ClassInfo::stereotypeGetters(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){

    returnsDataMembers(method_archive, hpp_unit, inline_function_count, true);
    returnsDataMembers(method_archive, cpp_unit, outofline_function_count, false);
    
    //std::cout << "inside getters" << std::endl;

    int total = inline_function_count + outofline_function_count;
    for (int i = 0; i < total; ++i){
        if (returns_data_members[i]){
            if (specifiers[i] == "const"){
                stereotypes[i] = "get";
            }
            else{
                stereotypes[i] = "non-const-get";
            }
        }
    }

}


// Stereotype predicate:
// method is const
// returns boolean
// return expression is not a data member
//
// Stereotype collaborational-predicate:
// method is const
// returns boolean
// return expression is not a data member
// does not use any data member in the method
// has no pure calls 
void ClassInfo::stereotypePredicates(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    //std::cout << "inside predicates\n";
    for (int i = 0; i < inline_function_count; ++i){
        std::string returnType = separateTypeName(return_types[i]);
        if (returnType == "bool" && !returns_data_members[i] && specifiers[i] == "const"){
            bool data_members = usesAttribute(method_archive, hpp_unit, i);
            std::vector<std::string> local_var_names = findLocalNames(method_archive, hpp_unit, i);
            std::vector<std::string> param_names = findParameterNames(method_archive, hpp_unit, i);
            std::vector<std::string> real_calls = findCalls(method_archive, hpp_unit, i, "real");
            bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);
            data_members = data_members || has_call_on_data_member;

            std::vector<std::string> pure_calls = findCalls(method_archive, hpp_unit, i, "pure");
            int pure_calls_count = countPureCalls(pure_calls);

            if (!data_members && pure_calls_count == 0){
                stereotypes[i] = "collaborational-predicate";
            }
            else{
                stereotypes[i] = "predicate";
            }

        }
    }
    int total = inline_function_count + outofline_function_count;
    for (int i = inline_function_count; i < total; ++i){
        std::string returnType = separateTypeName(return_types[i]);
        if (returnType == "bool" && !returns_data_members[i] && specifiers[i] == "const"){
            bool data_members = usesAttribute(method_archive, cpp_unit, i);

            std::vector<std::string> local_var_names = findLocalNames(method_archive, cpp_unit, i);
            std::vector<std::string> param_names = findParameterNames(method_archive, cpp_unit, i);
            std::vector<std::string> real_calls = findCalls(method_archive, cpp_unit, i, "real");
            
            bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);
            data_members = data_members || has_call_on_data_member;

            std::vector<std::string> pure_calls = findCalls(method_archive, cpp_unit, i, "pure");
            int pure_calls_count = countPureCalls(pure_calls);

            if (!data_members && pure_calls_count == 0){
                stereotypes[i] = "collaborational-predicate";
            }
            else{
                stereotypes[i] = "predicate";
            }
        }
    }

}


// Stereotype property:
// method is const
// return type is not boolean or void
// does not return a data member

// Stereotype collaborational-property
// method is const
// return type is not void or boolean
// does not contain a data member anywhere in the function
// does not contain any pure calls
// does not contain any calls on data members
void ClassInfo::stereotypeProperties(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    //std::cout << "STEREOTYPING PROPERTIES!\n";
    for (int i = 0; i < inline_function_count; ++i){
        std::string returnType = separateTypeName(return_types[i]);
        if (returnType != "bool" && returnType != "void" && !returns_data_members[i] && specifiers[i] == "const"){
            bool data_members = usesAttribute(method_archive, hpp_unit, i);
            std::vector<std::string> local_var_names = findLocalNames(method_archive, hpp_unit, i);
            std::vector<std::string> param_names = findParameterNames(method_archive, hpp_unit, i);
            std::vector<std::string> real_calls = findCalls(method_archive, hpp_unit, i, "real");

            bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);
            data_members = data_members || has_call_on_data_member;

            std::vector<std::string> pure_calls = findCalls(method_archive, hpp_unit, i, "pure");
            int pure_calls_count = countPureCalls(pure_calls);


            if (!data_members && pure_calls_count == 0){
                stereotypes[i] = "collaborational-property";
            }
            else{
                stereotypes[i] = "property"; 
            }
        }
    }

    int total = outofline_function_count + inline_function_count;
    for (int i = inline_function_count; i < total; ++i){
        std::string returnType = separateTypeName(return_types[i]);
        if (returnType != "bool" && returnType != "void" && !returns_data_members[i] && specifiers[i] == "const"){
            bool data_members = usesAttribute(method_archive, cpp_unit, i);

            std::vector<std::string> local_var_names = findLocalNames(method_archive, cpp_unit, i);
            std::vector<std::string> param_names = findParameterNames(method_archive, cpp_unit, i);
            std::vector<std::string> real_calls = findCalls(method_archive, cpp_unit, i, "real");

            bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);
            data_members = data_members || has_call_on_data_member;

            std::vector<std::string> pure_calls = findCalls(method_archive, cpp_unit, i, "pure");
            int pure_calls_count = countPureCalls(pure_calls);

            if (!data_members && pure_calls_count == 0){
                stereotypes[i] = "collaborational-property";
            }
            else{
                stereotypes[i] = "property"; 
            }
        }
    }
}


// Stereotype Void accessor:
// method returns void
// contains at least 1 parameter that is: 
//     passed by non-const reference
//     is a primitive type
//     and is assigned a value(one = in the expression).
void ClassInfo::stereotypeVoidAccessor(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    //std::cout << "stereotyping void accessors\n";
    // for each out of line function
    int total = inline_function_count + outofline_function_count;
    for (int i = inline_function_count; i < total; ++i){
        if(isVoidAccessor(method_archive, cpp_unit, i)){
            //std::cout << "method name of a void accessor " << method_names[i] << "\n";
            bool data_members = usesAttribute(method_archive, cpp_unit, i);

            std::vector<std::string> local_var_names = findLocalNames(method_archive, cpp_unit, i);
            std::vector<std::string> param_names = findParameterNames(method_archive, cpp_unit, i);
            std::vector<std::string> real_calls = findCalls(method_archive, cpp_unit, i, "real");

            bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);
            data_members = data_members || has_call_on_data_member;


            std::vector<std::string> pure_calls = findCalls(method_archive, cpp_unit, i, "pure");
            int pure_calls_count = countPureCalls(pure_calls);
            
            //std::cout << "\tcalls\n";
            //for (int i = 0; i < pure_calls.size(); ++i){
            //  std::cout << "\t\t" << pure_calls[i] << "\n";
            //}
            //std::cout << "\t pure calls count:" << pure_calls_count << " data members?" << data_members << "\n"; 
            if (!data_members && pure_calls_count == 0){
                stereotypes[i] = "collaborational-voidaccessor";
            }
            else{
                stereotypes[i] = "voidaccessor";
            }
        }
    }
    for (int i = 0; i < inline_function_count; ++i){
        if(isVoidAccessor(method_archive, hpp_unit, i)){
            bool data_members = usesAttribute(method_archive, hpp_unit, i);

            std::vector<std::string> local_var_names = findLocalNames(method_archive, hpp_unit, i);
            std::vector<std::string> param_names = findParameterNames(method_archive, hpp_unit, i);
            std::vector<std::string> real_calls = findCalls(method_archive, hpp_unit, i, "real");

            bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);
            data_members = data_members || has_call_on_data_member;

            std::vector<std::string> pure_calls = findCalls(method_archive, hpp_unit, i, "pure");
            int pure_calls_count = countPureCalls(pure_calls);

            if (!data_members && pure_calls_count == 0){
                stereotypes[i] = "collaborational-voidaccessor";
            }
            else{
                stereotypes[i] = "voidaccessor";
            }
            
        }
    }
}


// stereotype set:
//  return type void or boolean
//  method not const
//  only 1 data member has been changed
//
void ClassInfo::stereotypeSetters(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    //std::cout << "STEREOTYPING SETTERS!!!!\n";
    countChangedDataMembers(method_archive, hpp_unit, true);
    countChangedDataMembers(method_archive, cpp_unit, false);

    int total = inline_function_count + outofline_function_count;
    for (int i = 0; i < total; ++i){
        //std::cout << "testing method number " << i << std::endl;
        std::string returnType = separateTypeName(return_types[i]);
        bool void_or_bool = (returnType == "void" || returnType == "bool");
        if (changes_to_data_members[i] == 1 && specifiers[i] == "" && void_or_bool){
            stereotypes[i] = "set";
        }
    }
}


// stereotype Command:
//
//    method is not const
//
//    return type contains void or bool
//
//    exactly one data member is written to and the number of calls
//    in expression statements or returns is at least 2
//
//    no data members are written to and there is a call not
//    in a throw statement that is a simple real call (not a constructor call)
//    or a complex call for a data member
//
//REAL CALL
//~ does not following a new operator
// ~ is not inside a <throw> statement.
// ~ is not on the list of ignorable calls
//    assert  assertEquals clone equals finalize getClass hashCode notify notifyAll
//    toString wait $more_ignorable_calls
// ~ is not on the list of native types (string, vector, map, list)
//
//PURE CALL
// ~ does not contain . or ->
//
// need to handle the case where 0 data members are written and there is a call on a data member.
//
void ClassInfo::stereotypeCommand(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    //std::cout << "STEREOTYPING COMMAND\n";
    int total = inline_function_count + outofline_function_count;
    for (int i = inline_function_count; i < total; ++i){
        //std::cout << "method name " << method_names[i] << "\n";
        std::vector<std::string> real_calls = findCalls(method_archive, cpp_unit, i, "real");
        std::vector<std::string> pure_calls = findCalls(method_archive, cpp_unit, i, "pure");

        //std::cout << "------------" << real_calls.size() <<" real calls--------------------\n";
        
        std::string returnType = separateTypeName(return_types[i]);
        int pure_calls_count = countPureCalls(pure_calls);
        //std::cout << "------------" << pure_calls_count <<" pure calls--------------------\n";

        std::vector<std::string> local_var_names = findLocalNames(method_archive, cpp_unit, i);
        std::vector<std::string> param_names = findParameterNames(method_archive, cpp_unit, i);
        bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);

        bool case1 = (changes_to_data_members[i] == 1 && real_calls.size() > 1);
        bool case2 = changes_to_data_members[i] == 0 && (pure_calls_count > 0 || has_call_on_data_member);
        bool case3 = changes_to_data_members[i] > 1;
        //std::cout << "case1:" << case1 << " case2:" << case2 << std::endl;
        //std::cout << changes_to_data_members[i] << " " << real_calls.size() << " " << pure_calls_count << "\n"; 
        if ((case1 || case2 || case3) && specifiers[i] == ""){
            if (returnType == "void" || returnType == "bool"){
                stereotypes[i] = "command";
            }   
            else{
                stereotypes[i] = "non-void-command";
            }
        }
        // handles case of mutable data members
        else if (changes_to_data_members[i] > 0 && specifiers[i] == "const"){
            if (stereotypes[i] != "nothing-yet"){
                stereotypes[i] += " command";
            }
            else{
                stereotypes[i] = "command";
            }
        }
    }

    for (int i = 0; i < inline_function_count; ++i){
        std::vector<std::string> real_calls = findCalls(method_archive, hpp_unit, i, "real");
        std::vector<std::string> pure_calls = findCalls(method_archive, hpp_unit, i, "pure");

        std::string returnType = separateTypeName(return_types[i]);
        int pure_calls_count = countPureCalls(pure_calls);

        std::vector<std::string> local_var_names = findLocalNames(method_archive, hpp_unit, i);
        std::vector<std::string> param_names = findParameterNames(method_archive, hpp_unit, i);
        bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);

        bool case1 = (changes_to_data_members[i] == 1 && real_calls.size() > 1);
        bool case2 = (changes_to_data_members[i] == 0 && (pure_calls_count > 0 || has_call_on_data_member));
        bool case3 = changes_to_data_members[i] > 1;
        if ((case1 || case2 || case3) && specifiers[i] == ""){
            if (returnType == "void" || returnType == "bool"){
                stereotypes[i] = "command";
            }
            else{
                stereotypes[i] = "non-void-command";
            }
        }
        // handles case of mutable data members
        else if (changes_to_data_members[i] > 0 && specifiers[i] == "const"){
            if (stereotypes[i] != "nothing-yet"){
                stereotypes[i] += " command";
            }
            else{
                stereotypes[i] = "command";
            }
        }
    }
}


//stereotype collaborational-command
//method is not const and
//no data members are written and
//no pure calls, a() a::b() and
//no calls on data members and
//at least 1 call or parameter or local variable is written
//Calls allowed:  f->g() where f is not a data member, new f() (which isn't a real call)
//
void ClassInfo::stereotypeCollaborationalCommand(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    //std::cout << "STEREOTYPING COLLABORATIONAL COMMAND\n";
    for (int i = 0; i < inline_function_count; ++i){
        std::vector<std::string> local_var_names = findLocalNames(method_archive, hpp_unit, i);
        std::vector<std::string> param_names = findParameterNames(method_archive, hpp_unit, i);
        std::vector<std::string> all_calls = findCalls(method_archive, hpp_unit, i, "");
        bool has_call_on_data_member = callsAttributesMethod(all_calls, local_var_names, param_names);
        
        if (specifiers[i] == "" && changes_to_data_members[i] == 0){

            bool local_var_written = false;
            for (int j = 0; j < local_var_names.size(); ++j){
                if(variableChanged(method_archive, hpp_unit, i, local_var_names[j])){
                    local_var_written = true;
                    break;
                }
            }
            bool param_written = false;
            for (int j = 0; j < param_names.size(); ++j){
                if(variableChanged(method_archive, hpp_unit, i, param_names[j])){
                    param_written = true;
                    break;
                }
            }
            std::vector<std::string> pure_calls = findCalls(method_archive, hpp_unit, i, "pure");
            int pure_calls_count = countPureCalls(pure_calls);

            bool not_command =  pure_calls_count == 0 && !has_call_on_data_member;
            if (not_command && (all_calls.size() > 0 || local_var_written || param_written)){
                stereotypes[i] = "collaborational-command";
            }       
        }
    }
    int total = inline_function_count + outofline_function_count;
    for (int i = inline_function_count; i < total; ++i){
        std::vector<std::string> local_var_names = findLocalNames(method_archive, cpp_unit, i);
        std::vector<std::string> param_names = findParameterNames(method_archive, cpp_unit, i);

        std::vector<std::string> all_calls = findCalls(method_archive, cpp_unit, i, "");
        bool has_call_on_data_member = callsAttributesMethod(all_calls, local_var_names, param_names);
        if (specifiers[i] == "" && changes_to_data_members[i] == 0){

            bool local_var_written = false;
            for (int j = 0; j < local_var_names.size(); ++j){
                if (variableChanged(method_archive, cpp_unit, i, local_var_names[j])){
                    local_var_written = true;
                    break;
                }
            }
            bool param_written = false;
            for (int j = 0; j < param_names.size(); ++j){
                if (variableChanged(method_archive, cpp_unit, i, param_names[j])){
                    param_written = true;
                    break;
                }
            }
            std::vector<std::string> pure_calls = findCalls(method_archive, cpp_unit, i, "pure");
            int pure_calls_count = countPureCalls(pure_calls);
            
            bool not_command = pure_calls_count == 0 && !has_call_on_data_member;
            if (not_command && (all_calls.size() > 0 || local_var_written || param_written)){
                stereotypes[i] = "collaborational-command";
            }
        }
    }
}


// Stereotype Collaborators
// at least one of the following is true:
// makes a call to an attribute that is an object (only count pointers (* ->) [fix])
// has a parameter that is an object.
// has a local variable that is an object.
//
// what about returning an object attribtue !yes!
//
void ClassInfo::stereotypeCollaborators(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    std::string param = "/src:parameter_list//src:parameter";
    std::string local_var = "//src:decl_stmt[not(ancestor::src:throw) and not(ancestor::src:catch)]";

    //std::cout << "STEREOTYPING COLLABORATORS " << std::endl;

    // create a list of attribute names that are non-primitive types.
    std::vector<std::string> non_primitive_attributes;
    for (int i = 0; i < attribute_types.size(); ++i){
        //std::cout << "attribute type: " << attribute_types[i] << std::endl;
        bool attr_primitive = isPrimitiveContainer(attribute_types[i]);

        // if the attribute is not a primitive add to list.
        if (!attr_primitive && attribute_types[i].find("*") != std::string::npos && attribute_types[i] != class_name){
            //std::cout << "Non prim attritube pointer found name is: " << attribute_names[i] << std::endl;
            non_primitive_attributes.push_back(attribute_names[i]);
        }
    }

    for (int i = 0; i < inline_function_count; ++i){
        bool param_obj = containsNonPrimitive(method_archive, hpp_unit, i, param);
        bool local_obj = containsNonPrimitive(method_archive, hpp_unit, i, local_var);
        bool attr_obj = usesAttributeObj(method_archive, hpp_unit, i, non_primitive_attributes);
        std::string returnType = separateTypeName(return_types[i]);
        bool matches_class_name = returnType == class_name;
        bool ret_obj = !isPrimitiveContainer(returnType) && returnType != "void" && !matches_class_name;

        //std::cout << "inline function number " << i << "\n";
        //std::cout << "\tlocal obj " << local_obj << " param obj " << param_obj << " attr obj " << attr_obj << " ret obj " << ret_obj << std::endl;

        if (local_obj || param_obj || attr_obj || ret_obj){
            if (stereotypes[i] == "nothing-yet" && specifiers[i] == ""){
                stereotypes[i] = "controller";
            }
            else{
                stereotypes[i] += " collaborator";
            }
        }

    }
    int total = inline_function_count + outofline_function_count;
    for (int i = inline_function_count; i < total; ++i){    
        //std::cout << "\n\noutofline function number " << i << "\n";       

        bool param_obj = containsNonPrimitive(method_archive, cpp_unit, i, param);
        bool local_obj = containsNonPrimitive(method_archive, cpp_unit, i, local_var);
        bool attr_obj = usesAttributeObj(method_archive, cpp_unit, i, non_primitive_attributes);

        std::string returnType = separateTypeName(return_types[i]);
        bool matches_class_name = returnType == class_name;
        bool ret_obj = !isPrimitiveContainer(returnType) && returnType != "void" && !matches_class_name;

        //std::cout << "\t\tlocal obj " << local_obj << " param obj " << param_obj << " attr obj " << attr_obj << " ret obj " << ret_obj;
        //std::cout << " ret type " << return_types[i] << " class name " << class_name << "\n";
        if (local_obj || param_obj || attr_obj || ret_obj){
            if (stereotypes[i] == "nothing-yet" && specifiers[i] == ""){
                stereotypes[i] = "controller";
            }
            else{
                stereotypes[i] += " collaborator";
            }
        }
    }
}



// stereotype factory
//  the method returns a non-primitive type.
//  that was created in the body of the function.
//  return type includes pointer to object
//  a return statement includes a new operator or a local variable
//
void ClassInfo::stereotypeFactories(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    // for each function i need:
    // all non-primitive local variable names that match the return type of that function.
    // the variable in the return expression.
    for (int i = 0; i < inline_function_count; ++i){
        bool method_is_factory = isFactory(method_archive, hpp_unit, i);
        if (method_is_factory){
            //std::cout << "method is factory\n";
            stereotypes[i] = "factory";
        }
    }   
    int total = inline_function_count + outofline_function_count;
    for (int i = inline_function_count; i < total; ++i){
        bool method_is_factory = isFactory(method_archive, cpp_unit, i);
        if (method_is_factory){
            //std::cout << "method is factory\n";           
            stereotypes[i] = "factory";
        }
    }   
}
void ClassInfo::stereotypeEmpty(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    //std::cout << "STEREOTYPING EMPTY METHODS! _______\n";
    for (int i = 1; i <= inline_function_count; ++i){
        int index = i-1;
        //std::cout << "method index " << index << "\n";
        if (isEmptyMethod(method_archive, hpp_unit, index)){
            stereotypes[index] = "empty";
        }
    }
    for (int i = 1; i <= outofline_function_count; ++i){
        int index = inline_function_count + i - 1;
        //std::cout << "method index " << index << "\n";
        if (isEmptyMethod(method_archive, cpp_unit, index)){
            stereotypes[index] = "empty";
        }
    }
}


// stereotype stateless
// is not an empty method
// has no real calls including new calls
// does not use any data members
//
// stereotype wrapper
// is not empty
// has exactly 1 real call including new calls
// does not use any data memebers
//
void ClassInfo::stereotypeStateless(srcml_archive* method_archive, srcml_unit* hpp_unit, srcml_unit* cpp_unit){
    for (int i = 0; i < inline_function_count; ++i){
        bool empty = isEmptyMethod(method_archive, hpp_unit, i);
        std::vector<std::string> calls = findCalls(method_archive, hpp_unit, i, "");
        std::vector<std::string> real_calls = findCalls(method_archive, hpp_unit, i, "real");
        std::vector<std::string> local_var_names = findLocalNames(method_archive, hpp_unit, i);
        std::vector<std::string> param_names = findParameterNames(method_archive, hpp_unit, i);
        bool usedAttr = usesAttribute(method_archive, hpp_unit, i);
        bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);
        usedAttr = usedAttr || has_call_on_data_member;
        if (!empty && calls.size() < 1 && !usedAttr){
            stereotypes[i] += " stateless";
            if (stereotypes[i] == "nothing-yet stateless"){
                stereotypes[i] = "stateless";
            }
        }
        if (!empty && calls.size() == 1 && !usedAttr){
            stereotypes[i] += " wrapper";
            if (stereotypes[i] == "nothing-yet wrapper"){
                stereotypes[i] = "wrapper";
            }
        }
    }
    int total = inline_function_count + outofline_function_count;
    for (int i = inline_function_count; i < total; ++i){
        //std::cout << "STATELESS out line method HEADER \n________" << headers[i] << std::endl;
        bool empty = isEmptyMethod(method_archive, cpp_unit, i);
        std::vector<std::string> calls = findCalls(method_archive, cpp_unit, i, "");
        std::vector<std::string> real_calls = findCalls(method_archive, cpp_unit, i, "real");
        std::vector<std::string> local_var_names = findLocalNames(method_archive, cpp_unit, i);
        std::vector<std::string> param_names = findParameterNames(method_archive, cpp_unit, i);
        bool usedAttr = usesAttribute(method_archive, cpp_unit, i);

        bool has_call_on_data_member = callsAttributesMethod(real_calls, local_var_names, param_names);
        usedAttr = usedAttr || has_call_on_data_member;

        //std::cout << "calling uses attr for statless method " << headers[i] << "\n";
        //std::cout << "usedAttr" << usedAttr << "\n";

        if (!empty && calls.size() < 1 && !usedAttr){
            stereotypes[i] += " stateless";
            if (stereotypes[i] == "nothing-yet stateless"){
                stereotypes[i] = "stateless";
            }
        }
        if (!empty && calls.size() == 1 && !usedAttr){
            stereotypes[i] += " wrapper";
            if (stereotypes[i] == "nothing-yet wrapper"){
                stereotypes[i] = "wrapper";
            }
        }
    }
}

//
//
bool ClassInfo::isPrimitiveContainer(std::string return_type){
    // trim whitespace, specifiers and modifiers
    return_type = separateTypeName(return_type);

    // if the type is a vector or list, check if the element type is primitive

    if(return_type.find("vector") != std::string::npos || return_type.find("list") != std::string::npos){
        size_t start = return_type.find("<") + 1;
        size_t end = return_type.find(">");

        return_type = return_type.substr(start, end - start);

        // in the case of vector<vector<x>>
        if (return_type.find("vector") != std::string::npos || return_type.find("list") != std::string::npos){
            size_t start = return_type.find("<") + 1;
            return_type = return_type.substr(start);
        }
    }

    // if the type is a map check if the key and value are both primivite 
    // assumes never get map<map<x,y>,z>
    if (return_type.find("map") != std::string::npos){
        size_t start = return_type.find("<") + 1;
        size_t split = return_type.find(",");
        size_t end = return_type.find(">");

        std::string key = return_type.substr(start, split-start);
        std::string value = return_type.substr(split + 1, end - split - 1);
        return(isPrimitive(key) && isPrimitive(value));
    }
    // else check if primitive(NOT container).
    //std::cout << "testing the type against the file: " << return_type << "\n";    
    return isPrimitive(return_type);
    
}


//
//
bool ClassInfo::isPrimitive(const std::string& type){
// check the primitives list for the type
    for (int j = 0; j < primitive_types.size(); ++j){
        if (type == primitive_types[j]){
            return true;
        }
    }
    return false;
}

//
//
void ClassInfo::returnsDataMembers(srcml_archive* method_archive, srcml_unit* unit, const int& number_of_functions, bool inline_list){
    //std::cout << "finding returns data members\n";
    // for each function, find all return expressions and determine if they contain an attribute
    for(int i = 0; i < number_of_functions; ++i){   

        int index = inline_list ? i : i + inline_function_count;

        std::vector<std::string> return_expressions = findReturnExpressions(method_archive, unit, index, true);
        std::vector<std::string> parameter_names = findParameterNames(method_archive, unit, index);
        std::vector<std::string> local_var_names = findLocalNames(method_archive, unit, index);
    
        bool returns_attribute = false;
        for (int j = 0; j < return_expressions.size(); ++j){
            std::string return_expr = return_expressions[j];
            
            // handles the case of '*a'
            size_t pointer = return_expr.find("*");
            if (pointer == 0){
                return_expr.erase(pointer, 1);  
            }
            if (isAttribute(return_expr)){
                returns_attribute = true;
                break;
            }else if (isInheritedMember(parameter_names, local_var_names, return_expr)){
                returns_attribute = true;
                attribute_names.push_back(return_expr);
                attribute_types.push_back(return_types[index]);
                break;
            }
        }
        returns_data_members.push_back(returns_attribute);
        //std::cout << "Method number " << index << "returns a data member: " << returns_attribute << std::endl; 
        //std::cout << "_________________________________________\n\n";
    }
}

//
//
bool ClassInfo::isAttribute(std::string& name) const{
    bool found = false; 
    trimWhitespace(name);
    // remove [] if the name is an array
    size_t left_sq_bracket = name.find("[");
    if (left_sq_bracket != std::string::npos){
        name = name.substr(0, left_sq_bracket);
    }
    for (int i = 0; i < attribute_names.size(); ++i){
        if (name == attribute_names[i]){
            found = true;
        }
    }
    return found;
}

//
//
bool ClassInfo::isInheritedMember(const std::vector<std::string>& parameter_names, const std::vector<std::string>& local_var_names, const std::string& expr){

    bool is_inherited = true;
    //std::cout << "testing the expression is inherited member '" << expr << "'\n";
    
    // checks for literal return expression
    if (expr == "#"){
        //std::cout << "\t expression is a number not inherited data member\n";
        is_inherited = false;
    }
    if (expr == "true" || expr == "false" || expr == "TRUE" || expr == "FALSE"){
        //std::cout << "\t expression is a boolean not inherited data member\n";
        is_inherited = false;
    }

    // expr is not inherited if it is a parameter name
    for (int k = 0; k < parameter_names.size(); ++k){
        if(expr == parameter_names[k]){
            //std::cout << "\t expression is a parameter not inherited data member\n";
            is_inherited = false;
        }
    }
    // expr is not inherited if it is a local variable
    for (int k = 0; k < local_var_names.size(); ++k){
        //std::cout << "checking local name " << local_var_names[k] << "\n";
        if (expr == local_var_names[k]){
            //std::cout << "\t expression is a local variable not inherited data member\n";
            is_inherited = false;
        }
    }
    // expr is not inherited if it contains an operator
    for(int k = 0; k < expr.size(); ++k){
        if (expr[k] == '+' || expr[k] == '-' || expr[k] == '*' || expr[k] == '/'
            || expr[k] == '%' || expr[k] == '(' || expr[k] == '!' || expr[k] == '&'
            || expr[k] == '|' || expr[k] == '=' || expr[k] == '>' || expr[k] == '<'
            || expr[k] == '.' || expr[k] == '?' || expr[k] == ':' || expr[k] == '"'){
            //std::cout << "\t expression contains an operator not inherited data member\n";
            is_inherited = false;
        }
    }
    // expr is a keyword that is not a data member
    if (expr == "this" || expr == "cout" || expr == "endl"){
        is_inherited = false;
    }
    // expr is all uppercase letters 
    // assumed to be global variable
    //if (upper_case(expr)){
    //  is_inherited = false;
    //}
    bool has_parent_class = parent_class_names.size() > 0;
    return is_inherited && has_parent_class;
}



//
//
bool ClassInfo::isVoidAccessor(srcml_archive* method_archive, srcml_unit* unit, const int& func_index){
    std::vector<std::string> parameter_types = findParameterTypes(method_archive, unit, func_index);
    std::vector<std::string> parameter_names = findParameterNames(method_archive, unit, func_index);
    std::string returnType = separateTypeName(return_types[func_index]);

    for (int j = 0; j < parameter_types.size(); ++j){
        bool reference = parameter_types[j].find("&") != std::string::npos;
        bool constant = parameter_types[j].find("const") != std::string::npos; 

        bool primitive = isPrimitiveContainer(parameter_types[j]);

        // if the parameter type contains an &, is not const and is primitive 
        // and the return type of the method is void.
        if (reference && !constant && primitive && returnType == "void" && specifiers[func_index] == "const"){
            bool param_changed = variableChanged(method_archive, unit, func_index, parameter_names[j]);
    
                if (param_changed || stereotypes[func_index] == "nothing-yet"){
                return true;
            }
        }
        
    }
    if (returnType == "void" && specifiers[func_index] == "const" && 
        stereotypes[func_index] == "nothing-yet"){
        return true;
    }
    return false;
}

//
//
// returns a vector of strings containing the parameters type and specifiers of function #i
std::vector<std::string> ClassInfo::findParameterTypes(srcml_archive* method_archive, srcml_unit* unit, const int& i){
    std::vector<std::string> parameter_types;
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "']/src:parameter_list//src:parameter/src:decl/src:type";

    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result = nullptr;

    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);

    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result, i);

        std::string type = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [type.size() + 1];
        size_t size = type.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        std::string param_type(unparsed);
        //std::cout << param_type << std::endl;
        delete[] unparsed;
        parameter_types.push_back(param_type);
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result); 
    return parameter_types;
}

//
//
// returns a vector of string containing the parameters name of function #i
std::vector<std::string> ClassInfo::findParameterNames(srcml_archive* method_archive, srcml_unit* unit, const int& i){
    std::vector<std::string> parameter_names;
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "']/src:parameter_list//src:parameter/src:decl/src:name";
    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result, i);

        std::string type = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [type.size() + 1];
        size_t size = type.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        std::string param_name(unparsed);
        //std::cout << param_name << std::endl;
        delete[] unparsed;
        parameter_names.push_back(param_name);
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);
    return parameter_names;
}


//
//
bool ClassInfo::variableChanged(srcml_archive* method_archive, srcml_unit* unit, const int& i, const std::string& var_name){
    bool var_changed = false;
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "']//src:name[.='" + var_name + "' and not(ancestor::src:throw) ";
    xpath += "and not(ancestor::src:catch)]/following-sibling::*[1]";
    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result, i);
        std::string next_element = srcml_unit_get_srcml(result_unit);

        int equal_sign_count = 0;
        for (int j = 0; j < next_element.size(); ++j){
            if (next_element[j] == '='){
                equal_sign_count++;
            }
        }

        if (equal_sign_count == 1){
            var_changed = true;
        }
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);
    return var_changed;

}


// need to count different data members and not the same one twice.
// check with multiple changes with same opperator(same var), different operators(same var), same operator(dif var)
// test ++data_member += 10;
void ClassInfo::countChangedDataMembers(srcml_archive* method_archive, srcml_unit* unit, bool inline_list){
    int number_of_functions = inline_list ? inline_function_count : outofline_function_count;
    for (int i = 0; i < number_of_functions; ++i){

        int index = inline_list ? i : i + inline_function_count;
        
        //std::cout << "COUNTING CHANGED DATA MEMBERS FOR METHOD index " << index << "\n";
        

        //std::cout << "finding data members changed for function number: " << i << std::endl;
        int data_members_changed = 0;
        data_members_changed += findIncrementedDataMembers(method_archive, unit, index, false);
        data_members_changed += findIncrementedDataMembers(method_archive, unit, index, true);
        data_members_changed += findAssignOperatorDataMembers(method_archive, unit, index, false);
        data_members_changed += findAssignOperatorDataMembers(method_archive, unit, index, true);
        //std::cout << "\tdata members changed is: " << data_members_changed << std::endl;
        changes_to_data_members.push_back(data_members_changed);

    }
}


//
//
int ClassInfo::findAssignOperatorDataMembers(srcml_archive* method_archive, srcml_unit* unit, const int& i, bool check_for_loop){
    const int number_of_operators = 12;
    std::string assignment_operators[number_of_operators] = {"=", "+=", "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|=", "<<"};

    //std::cout << "FINDING ASSIGN OPP DATA MEMBERS!!!\n";
    //std::cout << "for the method " << method_names[i] << "\n";
    std::vector<std::string> local_var_names = findLocalNames(method_archive, unit, i);
    std::vector<std::string> param_names = findParameterNames(method_archive, unit, i);

    int data_members_changed = 0;
    for (int j = 0; j < number_of_operators; ++j){
        //std::cout << "\t FOR THE OPERATOR " << assignment_operators[j] << "\n";
        std::string assign_operator_xpath = "//src:function[string(src:name)='";
        assign_operator_xpath += method_names[i] + "' and string(src:type)='";
        assign_operator_xpath += return_types[i] + "' and string(src:parameter_list)='";
        assign_operator_xpath += parameter_lists[i] +"']//src:operator[.='";
        assign_operator_xpath += assignment_operators[j] + "' and not(ancestor::src:control)";
        assign_operator_xpath += " and not(ancestor::src:throw) and not(ancestor::src:catch)";

        if (check_for_loop){
            assign_operator_xpath += " and ancestor::src:for";
        }

        assign_operator_xpath += "]/preceding-sibling::src:name";

        //std::cout << "\t XPATH IS:" << assign_operator_xpath << "\n";
        srcml_append_transform_xpath(method_archive, assign_operator_xpath.c_str());

        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(method_archive, unit, &result);

        int number_of_result_units = srcml_transform_get_unit_size(result);
        srcml_unit* result_unit = nullptr;

        //std::cout << "\t_________ NUMBER OF RESULTS!! _______" << number_of_result_units << "_______\n";
        for (int k = 0; k < number_of_result_units; ++k){
            result_unit = srcml_transform_get_unit(result, k);

            std::string name = srcml_unit_get_srcml(result_unit);
            char * unparsed = new char [name.size() + 1];
            size_t size = name.size() + 1;
            srcml_unit_unparse_memory(result_unit, &unparsed, &size);

            std::string var_name(unparsed);
            //std::cout <<"the variable name to the left of an assignment operator is: "<< var_name << std::endl;
            delete[] unparsed;

            // removes whitespace
            trimWhitespace(var_name);

            bool attr = isAttribute(var_name);
            bool inherit = isInheritedMember(param_names, local_var_names, var_name);

            if (attr){
                //std::cout << "variable " << var_name << " changed!" << std::endl; 
                ++data_members_changed;
            }else if (inherit){
                //std::cout << "\tvariable " << var_name << " is probably inherited\n";
                attribute_names.push_back(var_name);
                attribute_types.push_back("unknown");
                ++data_members_changed;
            }
        }
        srcml_transform_free(result);
        srcml_clear_transforms(method_archive);
    }
    return data_members_changed;
}

//
//
int ClassInfo::findIncrementedDataMembers(srcml_archive* method_archive, srcml_unit* unit, const int& i, bool check_for_loop){
    const int number_of_operators = 2;
    std::string increment_operators[number_of_operators] = {"++", "--"};

    std::string name_location[2] = {"following-sibling", "preceding-sibling"};
    std::vector<std::string> local_var_names = findLocalNames(method_archive, unit, i);
    std::vector<std::string> param_names = findParameterNames(method_archive, unit, i);

    int data_members_changed = 0;
    //for each operator (++ and --)
    for (int j = 0; j < number_of_operators; ++j){
        // check following and preceeding
        for (int k = 0; k < 2; ++k){
            std::string xpath = "//src:function[string(src:name)='";
            xpath += method_names[i] + "' and string(src:type)='";
            xpath += return_types[i] + "' and string(src:parameter_list)='";
            xpath += parameter_lists[i] + "' and string(src:specifier)='";
            xpath += specifiers[i] + "']//src:operator[.='";
            xpath += increment_operators[j] + "' and not(ancestor::src:control)";
            xpath += " and not(ancestor::src:throw) and not(ancestor::src:catch)";

            if (check_for_loop){
                xpath += " and ancestor::src:for";
            }

            xpath += "]/" + name_location[k] + "::src:name[1]";

            srcml_append_transform_xpath(method_archive, xpath.c_str());

            srcml_transform_result* result = nullptr;
            int success = srcml_unit_apply_transforms(method_archive, unit, &result);

            int number_of_result_units = srcml_transform_get_unit_size(result);
            srcml_unit* result_unit = nullptr;

            if (number_of_result_units != 0){
                result_unit = srcml_transform_get_unit(result, 0);

                std::string name = srcml_unit_get_srcml(result_unit);
                char * unparsed = new char [name.size() + 1];
                size_t size = name.size() + 1;
                srcml_unit_unparse_memory(result_unit, &unparsed, &size);

                std::string var_name(unparsed);
                delete[] unparsed;

                // removes whitespace
                trimWhitespace(var_name);
            
                //std::cout <<"the variable name to the " << k << " of an " << j << " operator is: "<< var_name << " func_num " << i << std::endl;

                bool attr = isAttribute(var_name);
                bool inherit = isInheritedMember(param_names, local_var_names, var_name);

                if (attr){
                    //std::cout << "variable " << var_name << " changed!" << std::endl;
                    ++data_members_changed;
                }else if (inherit){
                    attribute_names.push_back(var_name);
                    attribute_types.push_back("unknown");
                    ++data_members_changed; 
                }
            }
            srcml_transform_free(result);
            srcml_clear_transforms(method_archive);
        }
    }
    return data_members_changed;
}


// returns a list of call names that are not below throw or following new: when pure_call is false
// does not include calls following the . or -> operators: when pure_call is true
//
std::vector<std::string> ClassInfo::findCalls(srcml_archive* method_archive, srcml_unit* unit, const int& i, const std::string& call_type){
    std::vector<std::string> calls;
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "']//src:call[not(ancestor::src:throw) and not(ancestor::src:catch)";
    if (call_type == "pure"){
        xpath += " and not(preceding-sibling::*[1][self::src:operator='new'])";
        xpath += " and not(preceding-sibling::*[1][self::src:operator='.'])";
        xpath += " and not(preceding-sibling::*[1][self::src:operator='->'])";
    }
    else if(call_type == "real"){
        xpath += "and not(preceding-sibling::*[1][self::src:operator='new'])";
    }

    xpath += "]/src:name";
    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_results = srcml_transform_get_unit_size(result);
    //std::cout << "number of calls are:" << number_of_results << std::endl;

    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_results; ++i){
        //std::cout << "iteration number " << i << std::endl;
        result_unit = srcml_transform_get_unit(result, i);

        std::string call = srcml_unit_get_srcml(result_unit);
        
        char * unparsed = new char [call.size() + 1];
        size_t size = call.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        call = unparsed;
        delete[] unparsed;
        //std::cout << "\t\t\tcall name:'" << call << "'\n"; 

        trimWhitespace(call);
        if(call != "assert" && !isPrimitiveContainer(call)){
            //std::cout << call << std::endl;
            calls.push_back(call);
        }
    }
    srcml_transform_free(result);
    srcml_clear_transforms(method_archive);
    return calls;
}


// dont count calls if
// there is a . or -> somewhere in the name
// or call is static and class name is the same
int ClassInfo::countPureCalls(const std::vector<std::string>& all_calls) const{
    int pure_calls = all_calls.size();
    for (int i = 0; i < all_calls.size(); ++i){
        size_t colon = all_calls[i].find(":");
        size_t dot = all_calls[i].find(".");
        size_t arrow = all_calls[i].find("->");

        if (dot != std::string::npos || arrow != std::string::npos){
            --pure_calls;
        }
        else if(colon != std::string::npos){
            std::string name = all_calls[i].substr(0, colon);
            //std::cout << "Static call found class name:" << name << std::endl;
            --pure_calls;
            
        }
    }
    return pure_calls;
}

//
//
bool ClassInfo::callsAttributesMethod(const std::vector<std::string>& real_calls, const std::vector<std::string>& local_var_names, const std::vector<std::string>& param_names){
    bool ret = false;
    for (int i = 0; i < real_calls.size(); ++i){
        size_t dot = real_calls[i].find(".");
        size_t arrow = real_calls[i].find("->");
        if (dot != std::string::npos){
            std::string calling_object = real_calls[i].substr(0, dot);
            //std::cout << "checking " << calling_object << "\n";
            bool attr = isAttribute(calling_object);
            bool inherit = isInheritedMember(param_names, local_var_names, calling_object);
            if(attr){
                //std::cout << "found a call on an attribute with the name." << calling_object << "\n"; 
                ret = true;     
            }
            else if (inherit){
                ret = true;
                //std::cout << "pushed back " << calling_object << "\n";
                attribute_names.push_back(calling_object);
                attribute_types.push_back("unknown");
            }
        }
        if (arrow != std::string::npos){
            std::string calling_object = real_calls[i].substr(0, arrow);
            bool attr = isAttribute(calling_object);
            bool inherit = isInheritedMember(param_names, local_var_names, calling_object);
            if (attr){
                //std::cout << "found a call on an attribute with the name-> " << calling_object << "\n";
                ret = true;
            }
            else if (inherit){
                ret = true;
                // std::cout << "pushed back " << calling_object << "\n";
                attribute_names.push_back(calling_object);
                attribute_types.push_back("unknown");
            }
        }
    }
    return ret;
}

//
//
// checks for non primitive parameters and local variables
bool ClassInfo::containsNonPrimitive(srcml_archive* method_archive, srcml_unit* unit, const int & i, const std::string& x){
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "']" + x + "/src:decl/src:type/src:name";
    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_results = srcml_transform_get_unit_size(result);

    srcml_unit* result_unit = nullptr;
    //std::cout << "\txpath: '" << xpath << "'\n";
    //std::cout << "\tnumber of results: '" << number_of_results << "'\n";
    for (int i = 0; i < number_of_results; ++i){
        result_unit = srcml_transform_get_unit(result, i);

        std::string type = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [type.size() + 1];
        size_t size = type.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        //std::cout << "unparsed:'" << unparsed << "'" << std::endl;
        std::string param_type(unparsed);
        delete[] unparsed;

        // handles case of (void) for param list
        if (number_of_results == 1 && x == "/src:parameter_list//src:parameter" && param_type == "void"){
            return false;
        }
        if (!isPrimitiveContainer(param_type) && param_type != class_name){
            //std::cout << "\tnon primitive found in " << x <<" type is '" << param_type << "' class name is: " << class_name << std::endl;
            srcml_transform_free(result);
            srcml_clear_transforms(method_archive);
            return true;
        }
        
    }
    srcml_transform_free(result);
    srcml_clear_transforms(method_archive);
    return false;

}

//
//
bool ClassInfo::usesAttributeObj(srcml_archive* method_archive, srcml_unit* unit, const int& i, const std::vector<std::string>& obj_names){
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "']//src:name[not(ancestor::src:throw) and not(ancestor::src:catch)]";
    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_results = srcml_transform_get_unit_size(result);

    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_results; ++i){
        result_unit = srcml_transform_get_unit(result, i);

        std::string attribute_name = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [attribute_name.size() + 1];
        size_t size = attribute_name.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string attr_name(unparsed);
        delete[] unparsed;
        for (int j = 0; j < obj_names.size(); ++j){
            //std::cout << "\tchecking " << attr_name << " against " << obj_names[j] << "\n";
            if (attr_name == obj_names[j]){
                srcml_clear_transforms(method_archive);
                srcml_transform_free(result);
                return true;
            }
        }
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);
    return false;
}

//
//
bool ClassInfo::usesAttribute(srcml_archive* method_archive, srcml_unit* unit, const int& i){
    //std::cout << "called usesAttribute!!!!\n";
    //std::cout << "method name \n\t" << headers[i] << "________________\n\n";
    
    std::vector<std::string> param_names = findParameterNames(method_archive, unit, i);
    std::vector<std::string> local_var_names = findLocalNames(method_archive, unit, i);

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "']//src:expr[not(ancestor::src:throw) and";
    xpath += "not(ancestor::src:argument_list[@type='generic']) and not(ancestor::src:catch)]/src:name";
    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_results = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;

    bool found_data_mem = false;
    for (int i = 0; i < number_of_results; ++i){
        result_unit = srcml_transform_get_unit(result, i);

        std::string name = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [name.size() + 1];
        size_t size = name.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string possible_attr(unparsed);
        delete[] unparsed;

        if (isPrimitive(possible_attr)){
            continue;
        }
        bool attr = isAttribute(possible_attr);
        bool inherit = isInheritedMember(param_names, local_var_names, possible_attr);
        //std::cout << "testing if the name '" << possible_attr << "' is a data member\n";
        if (attr){
            //std::cout << "is an attribute\n";
            found_data_mem = true;
        }else if (inherit){
            //std::cout << "'" << possible_attr << "' has been found inherited\n";
            attribute_names.push_back(possible_attr);
            attribute_types.push_back("unknown");
            found_data_mem = true;
        }
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);
    return found_data_mem;
}

//
//
// returns true if method, specified by number and unit, is a factory
bool ClassInfo::isFactory(srcml_archive* method_archive, srcml_unit* unit, const int& func_index){
    std::vector<std::string> return_expressions = findReturnExpressions(method_archive, unit, func_index, false);
    std::vector<std::string> local_var_names = findLocalNames(method_archive, unit, func_index);
    std::vector<std::string> param_names = findParameterNames(method_archive, unit, func_index);
    //std::cout << "return type " << return_types[func_index] << std::endl;
    bool returns_ptr = return_types[func_index].find("*") != std::string::npos;
    bool returns_obj = !isPrimitiveContainer(return_types[func_index]);


    bool returns_local = false;
    bool returns_new = false;
    bool returns_param = false; 
    for (int i = 0; i < return_expressions.size(); ++i){
        for (int j = 0; j < local_var_names.size(); ++j){
            //std::cout << "\tret: " << return_expressions[i] << " loc: " << local_var_names[j] << std::endl;
            if(return_expressions[i] == local_var_names[j]){
                //std::cout << "local var is returned\n";
                returns_local = true;
            }
        }
        for (int k = 0; k < param_names.size(); ++k){
            if (return_expressions[i] == param_names[k]){
                //std::cout << "param is returned\n";
                returns_param = true;
            }
        }
        if(return_expressions[i].find("new") != std::string::npos){
            returns_new = true;
        }
    }
    bool new_call = findConstructorCall(method_archive, unit, func_index);
    //std::cout << "returns obj " << returns_obj << " returns ptr " << returns_ptr << " returns local " 
    //<< returns_local << " returns new " << returns_new << " returns param "<< returns_param  << " new call " << new_call << std::endl;
    
    bool return_ex = (returns_local || returns_new || returns_param || returns_data_members[func_index]);
    bool is_factory = returns_obj && returns_ptr && new_call && return_ex;
    return is_factory;
}


//
//
std::vector<std::string> ClassInfo::findReturnExpressions(srcml_archive* method_archive, srcml_unit* unit, const int &i, bool getter){
    
    //std::cout << "\tfinding return expression with the header '" << return_types[i] + method_names[i];
    //std::cout << parameter_lists[i] + specifiers[i] << "'\n";

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "']//src:return/src:expr";
    if (getter){
        xpath += "[(count(*)=1 and src:name) or (count(*)=2 and *[1][self::src:operator='*'] and *[2][self::src:name])]";
    }

    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result;
    int error = srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_results = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;

    std::vector<std::string> return_expressions;
    for (int j = 0; j < number_of_results; ++j){
        result_unit = srcml_transform_get_unit(result, j);

        std::string return_ex = srcml_unit_get_srcml(result_unit);
        if (return_ex.find("<expr><literal type=\"number\"") == 0){
            //std::cout << "found literal expression return" << return_ex << std::endl;
            return_ex = "#";
        }else{
            char * unparsed = new char [return_ex.size() + 1];
            size_t size = return_ex.size() + 1;
            srcml_unit_unparse_memory(result_unit, &unparsed, &size);

            //std::cout << "\t\treturn expression: " << unparsed << std::endl;
            return_ex = unparsed;
            delete[] unparsed;
        }
        //std::cout << "pushing back the return ex: " << return_ex << std::endl;
        return_expressions.push_back(return_ex);
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);   
    return return_expressions;
}

//
//
std::vector<std::string> ClassInfo::findLocalNames(srcml_archive* method_archive, srcml_unit* unit, const int &i){
    std::vector<std::string> local_var_names;
    
    std::string xpath_function = "//src:function[string(src:name)='";
    xpath_function += method_names[i] + "' and string(src:type)='";
    xpath_function += return_types[i] + "' and string(src:parameter_list)='";
    xpath_function += parameter_lists[i] + "' and string(src:specifier)='";
    xpath_function += specifiers[i] + "']";

    std::string decl_stmt = "//src:decl_stmt[not(ancestor::src:throw) and not(ancestor::src:catch)]";
    std::string control = "//src:control/src:init";
    std::string decl_name = "/src:decl/src:name";
    
    std::string xpath = xpath_function + decl_stmt + decl_name + " | ";
    xpath += xpath_function + control + decl_name;

    //std::cout << "looking for local variable names ... xpath is '" << xpath << "'" << std::endl;
    //std::cout << "looking for local variables in the method " << headers[i] <<"\n__________\n"; 
    srcml_append_transform_xpath(method_archive, xpath.c_str());
    
    srcml_transform_result* result;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_results = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;
    //std::cout << "number of results from xpath to find local names " << number_of_results << "\n";
    for (int j = 0; j < number_of_results; ++j){
        result_unit = srcml_transform_get_unit(result, j);
        
        std::string var_name = srcml_unit_get_srcml(result_unit);

        char * unparsed = new char [var_name.size() + 1];
        size_t size = var_name.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        //std::cout << "\t" <<  unparsed << std::endl;
        var_name = unparsed;
        delete[] unparsed;

        trimWhitespace(var_name);
        size_t arr = var_name.find("[");
        if (arr != std::string::npos){
            var_name.erase(arr, arr-var_name.size());
        }
        local_var_names.push_back(var_name);

    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);
    return local_var_names;
}

//
//
// this function finds a constuctor call that is after a new operator that matches the class name
bool ClassInfo::findConstructorCall(srcml_archive* method_archive, srcml_unit* unit, const int& i){
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "']//src:call[not(ancestor::src:throw) and not(ancestor::src:catch) and";
    xpath += " preceding-sibling::*[1][self::src:operator='new']]/src:name";

    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_results = srcml_transform_get_unit_size(result);
    bool found_constructor_call = false;
    if (number_of_results > 0){
        found_constructor_call = true;
        //std::cout << "found a call that has 'new' operator before it\n";
    }
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);
    return found_constructor_call;
}


//
//
bool ClassInfo::isEmptyMethod(srcml_archive* method_archive, srcml_unit* unit, const int& i){
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method_names[i] + "' and string(src:type)='";
    xpath += return_types[i] + "' and string(src:parameter_list)='";
    xpath += parameter_lists[i] + "' and string(src:specifier)='";
    xpath += specifiers[i] + "'][not(src:block/src:block_content/*[not(self::src:comment)][1])]";

    //std::cout << "\t xpath:\n" << "\t" << xpath << "\n";
    srcml_append_transform_xpath(method_archive, xpath.c_str());

    srcml_transform_result* result;
    srcml_unit_apply_transforms(method_archive, unit, &result);

    int number_of_results = srcml_transform_get_unit_size(result);
    //std::cout << "number of results: " << number_of_results << "\n";
    srcml_clear_transforms(method_archive);
    srcml_transform_free(result);
    return number_of_results == 1;
}

//
//
srcml_unit* ClassInfo::writeStereotypeAttribute(srcml_archive* method_archive, srcml_unit* unit, bool inline_list){
    //std::cout << "inside function of death " << std::endl;

    int func_count = inline_list ? inline_function_count : outofline_function_count;
    //std::cout << "TOTAL function count is: " << inline_function_count + outofline_function_count << std::endl;
    for (int i = 1; i <= func_count; ++i){

        int index = inline_list ? i-1 : inline_function_count + i - 1;

        //std::cout << "function index is " << function_index <<std::endl;

        std::string stereotype = stereotypes[index];
        //std::cout << "stereotype is " << stereotype << std::endl;
        if (stereotype == "nothing-yet" && inline_function_count + outofline_function_count == 1){
            return nullptr;
        }
        if (stereotype != "nothing-yet"){
            std::string xpath = "//src:function[string(src:name)='";
            xpath += method_names[index] + "' and string(src:type)='";
            xpath += return_types[index] + "' and string(src:parameter_list)='";
            xpath += parameter_lists[index] + "' and string(src:specifier)='";
            xpath += specifiers[index] + "']";
            //std::cout << "adding " << xpath << " to list of transformations" << std::endl;
            srcml_append_transform_xpath_attribute(method_archive, xpath.c_str(), "src", "/srcML/src", "stereotype", stereotype.c_str());
        }
    }
    srcml_transform_result* result;
    srcml_unit_apply_transforms(method_archive, unit, &result);
    unit = srcml_transform_get_unit(result, 0);
    srcml_clear_transforms(method_archive);
    return unit;

}

//
//
void ClassInfo::printReturnTypes(){
    std::cout << "RETURN TYPES: \n";
    for (int i = 0; i < return_types.size(); ++i){
        std::cout << return_types[i] << "\n";
    }
    std::cout << std::endl;
}

//
//
void ClassInfo::printStereotypes(){
    std::cout << "STEREOTYPES: \n";
    for (int i = 0; i < stereotypes.size(); ++i){
        std::cout << stereotypes[i] << "\n";
    }
    std::cout << std::endl;
}

//
//
void ClassInfo::printMethodHeaders(){
    std::cout << "METHOD HEADERS: \n";
    for (int i = 0; i < headers.size(); ++i){
        std::cout << "method header: " << headers[i] << std::endl; 
    }
}

//
//
void ClassInfo::printAttributes(){
    std::cout << "ATTRIBUTES: \n";
    std::cout << attribute_types.size() << "types " << attribute_names.size() << "names\n";
    for (int i = 0; i < attribute_names.size(); ++i){
        std::cout << "TYPE: " << attribute_types[i] << " NAME: " << attribute_names[i] << std::endl;
    }
}

//
//
void ClassInfo::printReportToFile(std::ofstream& output_file, const std::string& input_file_path){
    int func_count = inline_function_count + outofline_function_count;
    if (output_file.is_open()){
        for (int i = 0; i < func_count; ++i){
            output_file << input_file_path << "|" << headers[i] << "||" << stereotypes[i] << "\n";
        }
    }
}


//Free Functions

//
// TODO: Should return a string with no side effect. But this is efficent
//
void trimWhitespace(std::string& str) {
    str.erase(std::remove_if(str.begin(),
                             str.end(),
                             [](char c) { return (c == ' ' || c == '\t' || c == '\n'); }),
              str.end());
}

//
//
std::string separateTypeName(const std::string& type){
    std::string name = type;
    trimWhitespace(name);
    size_t stat = name.find("static");
    if (stat != std::string::npos){
        name.erase(stat, 6);
    }
    size_t mut = name.find("mutable");
    if (mut != std::string::npos){
        name.erase(mut, 7);
    }
    size_t in = name.find("inline");
    if(in != std::string::npos){
        name.erase(in, 6);
    }
    size_t virt = name.find("virtual");
    if (virt != std::string::npos){
        name.erase(virt, 7);
    }

    size_t star = name.find("*");
    if (star != std::string::npos){
        name.erase(star, 1);
    }
    size_t amp = name.find("&");
    if (amp != std::string::npos){
        name.erase(amp, 1);
    }

    size_t con = name.find("const");
    if (con != std::string::npos){
        name.erase(con, 5);
    }
    return name;
}

