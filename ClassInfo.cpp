//classModel for stereocode
//

#include "ClassInfo.hpp"

//
//  secondUnit is only for C++ in which case firstUnit is hpp and secondUnit is cpp
//
classModel::classModel(srcml_archive* archive, srcml_unit* firstUnit, srcml_unit* secondUnit) : classModel() {
    language = srcml_unit_get_language(firstUnit);

    findClassName         (archive, firstUnit);
    findParentClassName   (archive, firstUnit);
    findAttributeNames    (archive, firstUnit);
    findAttributeTypes    (archive, firstUnit);

    findMethodHeaders(archive, firstUnit, true);
    if (secondUnit) findMethodHeaders(archive, secondUnit, false);
    findMethodNames(archive, firstUnit, true);
    if (secondUnit) findMethodNames(archive, secondUnit, false);
    findParameterLists(archive, firstUnit, true);
    if (secondUnit) findParameterLists(archive, secondUnit, false);
    findMethodReturnTypes(archive, firstUnit, true);
    if (secondUnit) findMethodReturnTypes(archive, secondUnit, false);
    returnsAttributes(archive, firstUnit, true);
    if (secondUnit) returnsAttributes(archive, secondUnit, false);
    findLocalVariableNames(archive, firstUnit, true);
    if (secondUnit)findLocalVariableNames(archive, secondUnit, false);
    findParameterNames(archive, firstUnit, true);
    if (secondUnit) findParameterNames(archive, secondUnit, false);
    findParameterTypes(archive, firstUnit, true);
    if (secondUnit) findParameterTypes(archive, secondUnit, false);
    countChangedAttributes(archive, firstUnit, true);
    if (secondUnit) countChangedAttributes(archive, secondUnit, false);
}



// WORKING on CLASS

//
//
void classModel::findClassName(srcml_archive* archive, srcml_unit* unit){
    srcml_append_transform_xpath(archive, "//src:class/src:name");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_result_units = srcml_transform_get_unit_size(result);

    for (int i = 0; i < number_of_result_units; ++i){
        srcml_unit* result_unit = srcml_transform_get_unit(result,i);
        std::string name = srcml_unit_get_srcml(result_unit);

        // chop off begining and ending <name></name>
        size_t end_position = name.find("</name>");
        name = name.substr(6,end_position-6);

        className = name;
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

//
//
void classModel::findParentClassName(srcml_archive* archive, srcml_unit* unit){
    srcml_append_transform_xpath(archive, "//src:class/src:super_list/src:super/src:name");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_result_units = srcml_transform_get_unit_size(result);

    for (int i = 0; i < number_of_result_units; ++i){
        srcml_unit* result_unit = srcml_transform_get_unit(result, i);
        std::string name = srcml_unit_get_srcml(result_unit);

        size_t end_position = name.find("</name>");
        name = name.substr(6,end_position-6);

        parentClass.push_back(name);
        //std::cout << "pushed back the super class name " << parentClass[i] << "\n";
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}


// WORKING on ATTRIBUTES

//
//
void classModel::findAttributeNames(srcml_archive* archive, srcml_unit* unit){
    srcml_append_transform_xpath(archive, "//src:class//src:decl_stmt[not(ancestor::src:function)]/src:decl/src:name");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
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

        attribute.push_back(attributeModel(attribute_name));
     }

    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

//
//
void classModel::findAttributeTypes(srcml_archive* archive, srcml_unit* unit){
    srcml_append_transform_xpath(archive, "//src:class//src:decl_stmt[not(ancestor::src:function)]/src:decl/src:type");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_result_units = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;
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

        attribute[i].setType(attr_type);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}





// WORKING on METHODS



// Checks if method is const
bool checkConst(std::string function_srcml) {
    trimWhitespace(function_srcml);
    size_t end = function_srcml.find("{");
    std::string function_srcml_header = function_srcml.substr(0, end);
    if (function_srcml_header.find("<specifier>const</specifier><block>") != std::string::npos){
        return true;
    } else if (function_srcml_header.find("</parameter_list><specifier>const</specifier>") != std::string::npos){
        return true;
    } else {
        return false;
    }
}


//
//  Finds header and const-ness
//
void classModel::findMethodHeaders(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    srcml_append_transform_xpath(archive, "//src:function");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);

    int number_of_result_units = srcml_transform_get_unit_size(result);
    int start = 0;
    if (oneUnit) {
        unitOneCount = number_of_result_units;       //COMPUTED HERE
    } else {
        unitTwoCount = number_of_result_units;       //COMPUTED HERE
    }
    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result, i);
        std::string function = srcml_unit_get_srcml(result_unit);

        bool isConstMethod = checkConst(function);

        char * unparsed = new char [function.size() + 1];
        size_t size = function.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string header(unparsed);
        delete[] unparsed;
        
        header = header.substr(0, header.find("{"));
        //remove newline characters
        header.erase(std::remove(header.begin(), header.end(), '\n'), header.end());

        method.push_back(methodModel(header, isConstMethod));
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);    
}




//
// Find the name of all functions/methods in the archive
//
void classModel::findMethodNames(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    srcml_append_transform_xpath(archive, "//src:function/src:name");
    srcml_transform_result* result      = nullptr;
    srcml_unit*             result_unit = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);

    int n = srcml_transform_get_unit_size(result);
    int offset = 0;
    if (!oneUnit) offset = unitOneCount;

    for (int i = 0; i < n; ++i){
        result_unit = srcml_transform_get_unit(result, i);
        std::string name_srcml = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [name_srcml.size() + 1];
        size_t size = name_srcml.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string name(unparsed);
        delete[] unparsed;

        method[i+offset].setName(name);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);  
}

//
//
void classModel::findParameterLists(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    srcml_append_transform_xpath(archive, "//src:function/src:parameter_list");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;
    int offset = 0;
    if (!oneUnit) offset = unitOneCount;

    for (int i = 0; i < n; ++i){
        result_unit = srcml_transform_get_unit(result,i);

        std::string name_srcml = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [name_srcml.size() + 1];
        size_t size = name_srcml.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        std::string parameter_list(unparsed);
        delete[] unparsed;
        method[i+offset].setParameters(parameter_list);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);  
}


//
//
// collects return types for each function
void classModel::findMethodReturnTypes(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    srcml_append_transform_xpath(archive, "//src:function/src:type");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;
    int offset = 0;
    if (!oneUnit) offset = unitOneCount;

    for (int i = 0; i < n; ++i){
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

        method[i+offset].setReturnType(ret_type);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);    
}


//
// Finds all the local variables in each method
//
void classModel::findLocalVariableNames(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i) {
        method[i+offset].setLocalVariables(methodLocalVariables(archive, unit, i));
    }
}


// Finds the parameter names in each method
//
void classModel::findParameterNames(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i) {
        method[i+offset].setParameterNames(methodParameterNames(archive, unit, i));
    }
}

// Finds the parameter types in each method
//
void classModel::findParameterTypes(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i) {
        method[i+offset].setParameterTypes(methodParameterTypes(archive, unit, i));
    }
}




// Determining STEREOTYPES



// Stereotype get:
// method is const,
// contains at least 1 return statement that returns a data memeber
// return expression must be in the form 'return a;' or 'return *a;'

// Stereotype non-const get:
// method is not const
// contains at least 1 return statement that returns a data memeber
// return expression must be in the form 'return a;' or 'return *a;'
void classModel::stereotypeGetter(srcml_archive* archive, srcml_unit* firstUnit, srcml_unit* secondUnit){
    for (int i = 0; i < method.size(); ++i){
        if (method[i].returnsAttribute()) {
            if (method[i].isConst())
                method[i].setStereotype("get");
            else
                method[i].setStereotype("non-const-get");
        }
    }
}



// stereotype set:
//  return type void or boolean
//  method not const
//  only 1 data member has been changed
//
void classModel::stereotypeSetter(srcml_archive* archive, srcml_unit* firstUnit, srcml_unit* secondUnit){
    for (int i = 0; i < method.size(); ++i){
        std::string returnType = separateTypeName(method[i].getReturnType());
        bool void_or_bool = (returnType == "void" || returnType == "bool");
        if (method[i].getAttributesModified() == 1 && !method[i].isConst() && void_or_bool){
            method[i].setStereotype("set");
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
void classModel::stereotypePredicate(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i){
        std::string returnType = separateTypeName(method[i+offset].getReturnType());
        if (returnType == "bool" && !method[i+offset].returnsAttribute() && method[i+offset].isConst()) {
            bool data_members = usesAttribute(archive, unit, i+offset);
            std::vector<std::string> real_calls = findCalls(archive, unit, i+offset, "real");
            bool has_call_on_data_member = callsAttributesMethod(real_calls, method[i+offset].getLocalVariables(), method[i+offset].getParameterNames());
            data_members = data_members || has_call_on_data_member;
            std::vector<std::string> pure_calls = findCalls(archive, unit, i+offset, "pure");
            int pure_calls_count = countPureCalls(pure_calls);

            if (!data_members && pure_calls_count == 0){
                method[i+offset].setStereotype("collaborational-predicate");
            }
            else{
                method[i+offset].setStereotype("predicate");
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
void classModel::stereotypeProperty(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i){
        std::string returnType = separateTypeName(method[i+offset].getReturnType());
        if (returnType != "bool" && returnType != "void" && !method[i+offset].returnsAttribute() && method[i+offset].isConst()){
            bool data_members = usesAttribute(archive, unit, i+offset);
            std::vector<std::string> real_calls = findCalls(archive, unit, i+offset, "real");

            bool has_call_on_data_member = callsAttributesMethod(real_calls, method[i+offset].getLocalVariables(), method[i+offset].getParameterNames());
            data_members = data_members || has_call_on_data_member;

            std::vector<std::string> pure_calls = findCalls(archive, unit, i+offset, "pure");
            int pure_calls_count = countPureCalls(pure_calls);


            if (!data_members && pure_calls_count == 0){
                method[i+offset].setStereotype("collaborational-property");
            }
            else{
                method[i+offset].setStereotype("property");
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
void classModel::stereotypeVoidAccessor(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i){
        if(isVoidAccessor(archive, unit, i+offset)){
            bool data_members = usesAttribute(archive, unit, i+offset);
            std::vector<std::string> real_calls = findCalls(archive, unit, i+offset, "real");

            bool has_call_on_data_member = callsAttributesMethod(real_calls, method[i+offset].getLocalVariables(), method[i+offset].getParameterNames());
            data_members = data_members || has_call_on_data_member;

            std::vector<std::string> pure_calls = findCalls(archive, unit, i+offset, "pure");
            int pure_calls_count = countPureCalls(pure_calls);
            if (!data_members && pure_calls_count == 0){
                method[i+offset].setStereotype("collaborational-voidaccessor");
            }
            else{
                method[i+offset].setStereotype("voidaccessor");
            }
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
void classModel::stereotypeCommand(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i){
        std::vector<std::string> real_calls = findCalls(archive, unit, i+offset, "real");
        std::vector<std::string> pure_calls = findCalls(archive, unit, i+offset, "pure");

        std::string returnType = separateTypeName(method[i+offset].getReturnType());
        int pure_calls_count = countPureCalls(pure_calls);
        bool has_call_on_data_member = callsAttributesMethod(real_calls, method[i+offset].getLocalVariables(), method[i+offset].getParameterNames());
        bool case1 = (method[i+offset].getAttributesModified() == 1 && real_calls.size() > 1);
        bool case2 = method[i+offset].getAttributesModified() == 0 && (pure_calls_count > 0 || has_call_on_data_member);
        bool case3 = method[i+offset].getAttributesModified() > 1;
        if ((case1 || case2 || case3) && !method[i+offset].isConst()){
            if (returnType == "void" || returnType == "bool"){
                method[i+offset].setStereotype("command");
            }   
            else{
                method[i+offset].setStereotype("non-void-command");
            }
        }
        // handles case of mutable data members
        else if (method[i+offset].getAttributesModified() > 0 && method[i+offset].isConst()){
            if (method[i+offset].getStereotype() != NO_STEREOTYPE){
                method[i+offset].setStereotype(method[i+offset].getStereotype() + " command");
            }
            else{
                method[i+offset].setStereotype("command");
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
void classModel::stereotypeCollaborationalCommand(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i){
        std::vector<std::string> all_calls = findCalls(archive, unit, i+offset, "");
        bool has_call_on_data_member = callsAttributesMethod(all_calls, method[i+offset].getLocalVariables(), method[i+offset].getParameterNames());
        
        if (!method[i+offset].isConst() && method[i+offset].getAttributesModified() == 0) {
            bool local_var_written = false;
            for (int j = 0; j < method[i+offset].getLocalVariables().size(); ++j){
                if(variableChanged(archive, unit, i+offset, method[i+offset].getLocalVariables()[j])){
                    local_var_written = true;
                    break;
                }
            }
            bool param_written = false;
            for (int j = 0; j < method[i+offset].getParameterNames().size(); ++j){
                if(variableChanged(archive, unit, i+offset, method[i+offset].getParameterNames()[j])){
                    param_written = true;
                    break;
                }
            }
            std::vector<std::string> pure_calls = findCalls(archive, unit, i+offset, "pure");
            int pure_calls_count = countPureCalls(pure_calls);

            bool not_command =  pure_calls_count == 0 && !has_call_on_data_member;
            if (not_command && (all_calls.size() > 0 || local_var_written || param_written)){
                method[i+offset].setStereotype("collaborational-command");
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
void classModel::stereotypeCollaborator(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    std::string param = "/src:parameter_list//src:parameter";
    std::string local_var = "//src:decl_stmt[not(ancestor::src:throw) and not(ancestor::src:catch)]";
    std::vector<std::string> non_primitive_attributes;
    for (int i = 0; i < attribute.size(); ++i){
        bool attr_primitive = isPrimitiveContainer(attribute[i].getType());
        // if the attribute is not a primitive add to list.
        if (!attr_primitive && attribute[i].getType().find("*") != std::string::npos && attribute[i].getType() != className){
            non_primitive_attributes.push_back(attribute[i].getName());
        }
    }

    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i){
        bool param_obj = containsNonPrimitive(archive, unit, i+offset, param);
        bool local_obj = containsNonPrimitive(archive, unit, i+offset, local_var);
        bool attr_obj = usesAttributeObj(archive, unit, i+offset, non_primitive_attributes);
        std::string returnType = separateTypeName(method[i+offset].getReturnType());
        bool matches_class_name = returnType == className;
        bool ret_obj = !isPrimitiveContainer(returnType) && returnType != "void" && !matches_class_name;
        if (local_obj || param_obj || attr_obj || ret_obj){
            if (method[i+offset].getStereotype() == NO_STEREOTYPE && !method[i+offset].isConst()){
                method[i+offset].setStereotype("controller");
            }
            else{
                method[i+offset].setStereotype(method[i+offset].getStereotype() + " collaborator");
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
// for each function i need:
// all non-primitive local variable names that match the return type of that function.
// the variable in the return expression.

void classModel::stereotypeFactory(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i){
        bool method_is_factory = isFactory(archive, unit, i+offset);
        if (method_is_factory){
            method[i+offset].setStereotype("factory");
        }
    }
}


// stereotype empty
//
//
void classModel::stereotypeEmpty(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i){
        if (isEmptyMethod(archive, unit, i+offset)){
            method[i+offset].setStereotype("empty");
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
void classModel::stereotypeStateless(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n = unitOneCount;
    if (!oneUnit) {
        offset = unitOneCount;
        n = unitTwoCount;
    }

    for (int i = 0; i < n; ++i){
        bool empty = isEmptyMethod(archive, unit, i+offset);
        std::vector<std::string> calls = findCalls(archive, unit, i+offset, "");
        std::vector<std::string> real_calls = findCalls(archive, unit, i+offset, "real");
        bool usedAttr = usesAttribute(archive, unit, i+offset);
        bool has_call_on_data_member = callsAttributesMethod(real_calls, method[i+offset].getLocalVariables(), method[i+offset].getParameterNames());
        usedAttr = usedAttr || has_call_on_data_member;
        if (!empty && calls.size() < 1 && !usedAttr){
            method[i+offset].setStereotype(method[i+offset].getStereotype() + " stateless");
            if (method[i+offset].getStereotype() == "nothing-yet stateless"){
                method[i+offset].setStereotype("stateless");
            }
        }
        if (!empty && calls.size() == 1 && !usedAttr){
            method[i+offset].setStereotype(method[i+offset].getStereotype() + " wrapper");
            if (method[i+offset].getStereotype() == "nothing-yet wrapper"){
                method[i+offset].setStereotype("wrapper");
            }
        }
    }
}


//
// Checks if a primitive type
//
bool classModel::isPrimitiveContainer(std::string return_type){
    return_type = separateTypeName(return_type); // trim whitespace, specifiers and modifiers

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
        return(primitives.isPrimitive(key) && primitives.isPrimitive(value));
    }
    // else check if primitive(NOT container).
    return primitives.isPrimitive(return_type);
    
}



//
//for each function, find all return expressions and determine if they contain an attribute
//
void classModel::returnsAttributes(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int offset = 0;
    int n;
    if (oneUnit) {
        n = unitOneCount;
    } else {
        n = unitTwoCount;
        offset = unitOneCount;
    }

    for(int i = 0; i < n; ++i) {
        std::vector<std::string> return_expressions = findReturnExpressions(archive, unit, i+offset, true);
        bool returns_attribute = false;
        for (int j = 0; j < return_expressions.size(); ++j){
            std::string return_expr = return_expressions[j];
            if (return_expr.find("*") == 0) return_expr.erase(0, 1);  // handles the case of '*a'
            if (isAttribute(return_expr)){
                returns_attribute = true;
                break;
            } else if (isInheritedMember(method[i+offset].getParameterNames(), method[i+offset].getLocalVariables(), return_expr)){
                returns_attribute = true;
                attribute.push_back(attributeModel(return_expr, method[i+offset].getReturnType()));
                break;
            }
        }
        method[i+offset].setReturnsAttribute(returns_attribute);
    }
}



//
//
bool classModel::isAttribute(std::string& name) const{
    trimWhitespace(name);
    size_t left_sq_bracket = name.find("[");    // remove [] if the name is an array
    if (left_sq_bracket != std::string::npos){
        name = name.substr(0, left_sq_bracket);
    }
    for (int i = 0; i < attribute.size(); ++i) {
        if (name == attribute[i].getName()) return true;
    }
    return false;
}

//
//
bool classModel::isInheritedMember(const std::vector<std::string>& parameter_names,
                                   const std::vector<std::string>& local_var_names,
                                   const std::string& expr){

    bool is_inherited = true;
    // checks for literal return expression
    if (expr == "#"){
        is_inherited = false;
    }
    if (expr == "true" || expr == "false" || expr == "TRUE" || expr == "FALSE"){
        is_inherited = false;
    }

    // expr is not inherited if it is a parameter name
    for (int k = 0; k < parameter_names.size(); ++k){
        if(expr == parameter_names[k]){
            is_inherited = false;
        }
    }
    // expr is not inherited if it is a local variable
    for (int k = 0; k < local_var_names.size(); ++k){
        if (expr == local_var_names[k]){
            is_inherited = false;
        }
    }
    // expr is not inherited if it contains an operator
    for(int k = 0; k < expr.size(); ++k){
        if (expr[k] == '+' || expr[k] == '-' || expr[k] == '*' || expr[k] == '/'
            || expr[k] == '%' || expr[k] == '(' || expr[k] == '!' || expr[k] == '&'
            || expr[k] == '|' || expr[k] == '=' || expr[k] == '>' || expr[k] == '<'
            || expr[k] == '.' || expr[k] == '?' || expr[k] == ':' || expr[k] == '"'){
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
    bool has_parent_class = parentClass.size() > 0;
    return is_inherited && has_parent_class;
}



//
//
bool classModel::isVoidAccessor(srcml_archive* archive, srcml_unit* unit, int funcIndex){
    std::vector<std::string> parameter_types = method[funcIndex].getParameterTypes();
    std::vector<std::string> parameter_names = method[funcIndex].getParameterNames();
    std::string returnType = separateTypeName(method[funcIndex].getReturnType());

    for (int j = 0; j < parameter_types.size(); ++j){
        bool reference = parameter_types[j].find("&") != std::string::npos;
        bool constant = parameter_types[j].find("const") != std::string::npos;
        bool primitive = isPrimitiveContainer(parameter_types[j]);

        // if the parameter type contains an &, is not const and is primitive 
        // and the return type of the method is void.
        if (reference && !constant && primitive && returnType == "void" && method[funcIndex].isConst()){
            bool param_changed = variableChanged(archive, unit, funcIndex, parameter_names[j]);
                if (param_changed || method[funcIndex].getStereotype() == NO_STEREOTYPE){
                return true;
            }
        }
    }
    if (returnType == "void" && method[funcIndex].isConst() &&
        method[funcIndex].getStereotype() == NO_STEREOTYPE){
        return true;
    }
    return false;
}

//
//
// returns a vector of strings containing the parameters type and specifiers of function #i
//
std::vector<std::string> classModel::methodParameterTypes(srcml_archive* archive, srcml_unit* unit, int i){
    std::vector<std::string> parameter_types;
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']/src:parameter_list//src:parameter/src:decl/src:type";

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_result_units = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [type.size() + 1];
        size_t size = type.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string param_type(unparsed);
        delete[] unparsed;
        parameter_types.push_back(param_type);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result); 
    return parameter_types;
}

//
//
// returns a vector of string containing the parameters name of function #i
//
std::vector<std::string> classModel::methodParameterNames(srcml_archive* archive, srcml_unit* unit, int i){
    std::vector<std::string> parameter_names;
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']/src:parameter_list//src:parameter/src:decl/src:name";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_result_units = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_result_units; ++i){
        result_unit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [type.size() + 1];
        size_t size = type.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string param_name(unparsed);
         delete[] unparsed;
        parameter_names.push_back(param_name);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
    return parameter_names;
}


//
//
bool classModel::variableChanged(srcml_archive* archive, srcml_unit* unit, int i, const std::string& var_name){
    bool var_changed = false;
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:name[.='" + var_name + "' and not(ancestor::src:throw) ";
    xpath += "and not(ancestor::src:catch)]/following-sibling::*[1]";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);
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
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
    return var_changed;
}


//
// need to count different data members and not the same one twice.
// check with multiple changes with same opperator(same var), different operators(same var), same operator(dif var)
// test ++data_member += 10;
//
void classModel::countChangedAttributes(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int n = unitOneCount ;
    int offset = 0;
    if (!oneUnit) {
        n = unitTwoCount;
        offset = unitOneCount;
    }
    for (int i = 0; i < n; ++i){
        int data_members_changed = 0;
        data_members_changed += findIncrementedDataMembers(archive, unit, i+offset, false);     //No loop
        data_members_changed += findIncrementedDataMembers(archive, unit, i+offset, true);      // loop
        data_members_changed += findAssignOperatorDataMembers(archive, unit, i+offset, false);  // No loop
        data_members_changed += findAssignOperatorDataMembers(archive, unit, i+offset, true);   // loop
        method[i+offset].setAttributesModified(data_members_changed);
    }
}


//
//
int classModel::findAssignOperatorDataMembers(srcml_archive* archive, srcml_unit* unit, int i, bool check_for_loop){
    const int number_of_operators = 12;
    std::string assignment_operators[number_of_operators] = {"=", "+=", "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|=", "<<"};
    std::vector<std::string> local_var_names = method[i].getLocalVariables();
    std::vector<std::string> param_names = method[i].getParameterNames();
    int data_members_changed = 0;

    for (int j = 0; j < number_of_operators; ++j){
        std::string assign_operator_xpath = "//src:function[string(src:name)='";
        assign_operator_xpath += method[i].getName() + "' and string(src:type)='";
        assign_operator_xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
        assign_operator_xpath += method[i].getParameters() +"']//src:operator[.='";
        assign_operator_xpath += assignment_operators[j] + "' and not(ancestor::src:control)";
        assign_operator_xpath += " and not(ancestor::src:throw) and not(ancestor::src:catch)";
        if (check_for_loop){
            assign_operator_xpath += " and ancestor::src:for";
        }
        assign_operator_xpath += "]/preceding-sibling::src:name";
        srcml_append_transform_xpath(archive, assign_operator_xpath.c_str());
        srcml_transform_result* result = nullptr;
        srcml_unit_apply_transforms(archive, unit, &result);
        int number_of_result_units = srcml_transform_get_unit_size(result);
        srcml_unit* result_unit = nullptr;

        for (int k = 0; k < number_of_result_units; ++k){
            result_unit = srcml_transform_get_unit(result, k);
            std::string name = srcml_unit_get_srcml(result_unit);
            char * unparsed = new char [name.size() + 1];
            size_t size = name.size() + 1;
            srcml_unit_unparse_memory(result_unit, &unparsed, &size);
            std::string var_name(unparsed);
            delete[] unparsed;
            trimWhitespace(var_name); // removes whitespace
            bool attr = isAttribute(var_name);
            bool inherit = isInheritedMember(param_names, method[i].getLocalVariables(), var_name);
            if (attr) {
                 ++data_members_changed;
            } else if (inherit){
                attribute.push_back(attributeModel(var_name, "unknown"));
                 ++data_members_changed;
            }
        }
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
    }
    return data_members_changed;
}

//
//
int classModel::findIncrementedDataMembers(srcml_archive* archive, srcml_unit* unit, int i, bool check_for_loop){
    const int number_of_operators = 2;
    std::string increment_operators[number_of_operators] = {"++", "--"};
    std::string name_location[2] = {"following-sibling", "preceding-sibling"};
    std::vector<std::string> param_names = method[i].getParameterNames();
    int data_members_changed = 0;

    for (int j = 0; j < number_of_operators; ++j){   //for each operator (++ and --)
        // check following and preceeding
        for (int k = 0; k < 2; ++k){
            std::string xpath = "//src:function[string(src:name)='";
            xpath += method[i].getName() + "' and string(src:type)='";
            xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
            xpath += method[i].getParameters() + "' and string(src:specifier)='";
            xpath += method[i].getConst() + "']//src:operator[.='";
            xpath += increment_operators[j] + "' and not(ancestor::src:control)";
            xpath += " and not(ancestor::src:throw) and not(ancestor::src:catch)";
            if (check_for_loop){
                xpath += " and ancestor::src:for";
            }
            xpath += "]/" + name_location[k] + "::src:name[1]";
            srcml_append_transform_xpath(archive, xpath.c_str());
            srcml_transform_result* result = nullptr;
            int success = srcml_unit_apply_transforms(archive, unit, &result);
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
                trimWhitespace(var_name);// removes whitespace
                bool attr = isAttribute(var_name);
                bool inherit = isInheritedMember(param_names, method[i].getLocalVariables(), var_name);
                if (attr) {
                    ++data_members_changed;
                } else if (inherit) {
                    attribute.push_back(attributeModel(var_name, "unknown"));
                    ++data_members_changed;
                }
            }
            srcml_transform_free(result);
            srcml_clear_transforms(archive);
        }
    }
    return data_members_changed;
}


// returns a list of call names that are not below throw or following new: when pure_call is false
// does not include calls following the . or -> operators: when pure_call is true
//
std::vector<std::string> classModel::findCalls(srcml_archive* archive, srcml_unit* unit, int i, const std::string& call_type){
    std::vector<std::string> calls;
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:call[not(ancestor::src:throw) and not(ancestor::src:catch)";

    if (call_type == "pure"){
        xpath += " and not(preceding-sibling::*[1][self::src:operator='new'])";
        xpath += " and not(preceding-sibling::*[1][self::src:operator='.'])";
        xpath += " and not(preceding-sibling::*[1][self::src:operator='->'])";
    }
    else if(call_type == "real"){
        xpath += "and not(preceding-sibling::*[1][self::src:operator='new'])";
    }
    xpath += "]/src:name";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_results = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_results; ++i){
        result_unit = srcml_transform_get_unit(result, i);
        std::string call = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [call.size() + 1];
        size_t size = call.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        call = unparsed;
        delete[] unparsed;
        trimWhitespace(call);
        if (call != "assert" && !isPrimitiveContainer(call)) {
            calls.push_back(call);
        }
    }
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    return calls;
}


// dont count calls if
// there is a . or -> somewhere in the name
// or call is static and class name is the same
//
int classModel::countPureCalls(const std::vector<std::string>& all_calls) const {
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
            --pure_calls;
        }
    }
    return pure_calls;
}


//
//
bool classModel::callsAttributesMethod(const std::vector<std::string>& real_calls,
                                       const std::vector<std::string>& local_var_names,
                                       const std::vector<std::string>& param_names){
    bool ret = false;
    for (int i = 0; i < real_calls.size(); ++i){
        size_t dot = real_calls[i].find(".");
        size_t arrow = real_calls[i].find("->");
        if (dot != std::string::npos){
            std::string calling_object = real_calls[i].substr(0, dot);
            bool attr = isAttribute(calling_object);
            bool inherit = isInheritedMember(param_names, local_var_names, calling_object);
            if(attr){
                ret = true;     
            }
            else if (inherit){
                ret = true;
                attribute.push_back(attributeModel(calling_object, "unknown"));
            }
        }
        if (arrow != std::string::npos){
            std::string calling_object = real_calls[i].substr(0, arrow);
            bool attr = isAttribute(calling_object);
            bool inherit = isInheritedMember(param_names, local_var_names, calling_object);
            if (attr){
                ret = true;
            }
            else if (inherit){
                ret = true;
                attribute.push_back(attributeModel(calling_object, "unknown"));
             }
        }
    }
    return ret;
}


//
//
// checks for non primitive parameters and local variables
//
bool classModel::containsNonPrimitive(srcml_archive* archive, srcml_unit* unit, int i, const std::string& x){
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']" + x + "/src:decl/src:type/src:name";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_results = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;

    for (int i = 0; i < number_of_results; ++i){
        result_unit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [type.size() + 1];
        size_t size = type.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string param_type(unparsed);
        delete[] unparsed;
        // handles case of (void) for param list
        if (number_of_results == 1 && x == "/src:parameter_list//src:parameter" && param_type == "void"){
            return false;
        }
        if (!isPrimitiveContainer(param_type) && param_type != className){
             srcml_transform_free(result);
            srcml_clear_transforms(archive);
            return true;
        }
        
    }
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    return false;
}



//
//
bool classModel::usesAttributeObj(srcml_archive* archive, srcml_unit* unit, int i, const std::vector<std::string>& obj_names){
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:name[not(ancestor::src:throw) and not(ancestor::src:catch)]";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result;
    srcml_unit_apply_transforms(archive, unit, &result);
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
             if (attr_name == obj_names[j]){
                srcml_clear_transforms(archive);
                srcml_transform_free(result);
                return true;
            }
        }
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
    return false;
}

//
// USES primitives
//
// Returns if a method has a USE of an attribute (including inherited attributes)
//  Does not consider DEF of an attribute
//  Also adds any inherited attributes to attribute
//
bool classModel::usesAttribute(srcml_archive* archive, srcml_unit* unit, int i){
    std::vector<std::string> param_names = method[i].getParameterNames();
    std::vector<std::string> local_var_names = method[i].getLocalVariables();
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:expr[not(ancestor::src:throw) and";
    xpath += "not(ancestor::src:argument_list[@type='generic']) and not(ancestor::src:catch)]/src:name";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result;
    srcml_unit_apply_transforms(archive, unit, &result);
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

        //Removing this passes the 5 orginal tests.  Remove?
        if (primitives.isPrimitive(possible_attr)) continue;  //Should never be a primitive type???

        bool attr = isAttribute(possible_attr);
        bool inherit = isInheritedMember(param_names, local_var_names, possible_attr);
        if (attr) {
             found_data_mem = true;
        } else if (inherit){
            attribute.push_back(attributeModel(possible_attr, "unknown"));
            found_data_mem = true;
        }
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
    return found_data_mem;
}


//
//
// returns true if method, specified by number and unit, is a factory
//
bool classModel::isFactory(srcml_archive* archive, srcml_unit* unit, int funcIndex){
    std::vector<std::string> return_expressions = findReturnExpressions(archive, unit, funcIndex, false);
    std::vector<std::string> param_names        = method[funcIndex].getParameterNames();
    bool                     returns_ptr        = method[funcIndex].getReturnType().find("*") != std::string::npos;
    bool                     returns_obj        = !isPrimitiveContainer(method[funcIndex].getReturnType());
    bool returns_local = false;
    bool returns_new = false;
    bool returns_param = false; 
    for (int i = 0; i < return_expressions.size(); ++i){
        for (int j = 0; j < method[funcIndex].getLocalVariables().size(); ++j){
            if(return_expressions[i] == method[funcIndex].getLocalVariables()[j]){
                 returns_local = true;
            }
        }
        for (int k = 0; k < param_names.size(); ++k){
            if (return_expressions[i] == param_names[k]){
                returns_param = true;
            }
        }
        if(return_expressions[i].find("new") != std::string::npos){
            returns_new = true;
        }
    }
    bool new_call = findConstructorCall(archive, unit, funcIndex);
    bool return_ex = (returns_local || returns_new || returns_param || method[funcIndex].returnsAttribute());
    bool is_factory = returns_obj && returns_ptr && new_call && return_ex;
    return is_factory;
}


//
//
std::vector<std::string> classModel::findReturnExpressions(srcml_archive* archive, srcml_unit* unit, int i, bool getter){
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:return/src:expr";
    if (getter){
        xpath += "[(count(*)=1 and src:name) or (count(*)=2 and *[1][self::src:operator='*'] and *[2][self::src:name])]";
    }

    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result;
    int error = srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_results = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;
    std::vector<std::string> return_expressions;

    for (int j = 0; j < number_of_results; ++j){
        result_unit = srcml_transform_get_unit(result, j);
        std::string return_ex = srcml_unit_get_srcml(result_unit);
        if (return_ex.find("<expr><literal type=\"number\"") == 0){
            return_ex = "#";
        }else{
            char * unparsed = new char [return_ex.size() + 1];
            size_t size = return_ex.size() + 1;
            srcml_unit_unparse_memory(result_unit, &unparsed, &size);
            return_ex = unparsed;
            delete[] unparsed;
        }
        return_expressions.push_back(return_ex);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);   
    return return_expressions;
}


//
// Finds all the local variables within a given method[i]
//
std::vector<std::string> classModel::methodLocalVariables(srcml_archive* archive, srcml_unit* unit, int i){
    std::vector<std::string> local_var_names;
    
    std::string xpath_function = "//src:function[string(src:name)='";
    xpath_function += method[i].getName() + "' and string(src:type)='";
    xpath_function += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath_function += method[i].getParameters() + "' and string(src:specifier)='";
    xpath_function += method[i].getConst() + "']";
    std::string decl_stmt = "//src:decl_stmt[not(ancestor::src:throw) and not(ancestor::src:catch)]";
    std::string control = "//src:control/src:init";
    std::string decl_name = "/src:decl/src:name";
    std::string xpath = xpath_function + decl_stmt + decl_name + " | ";
    xpath += xpath_function + control + decl_name;
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_results = srcml_transform_get_unit_size(result);
    srcml_unit* result_unit = nullptr;

    for (int j = 0; j < number_of_results; ++j){
        result_unit = srcml_transform_get_unit(result, j);
        std::string var_name = srcml_unit_get_srcml(result_unit);
        char * unparsed = new char [var_name.size() + 1];
        size_t size = var_name.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        var_name = unparsed;
        delete[] unparsed;
        trimWhitespace(var_name);
        size_t arr = var_name.find("[");
        if (arr != std::string::npos){
            var_name.erase(arr, arr-var_name.size());
        }
        local_var_names.push_back(var_name);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
    return local_var_names;
}

//
//
// this function finds a constuctor call that is after a new operator that matches the class name
//
bool classModel::findConstructorCall(srcml_archive* archive, srcml_unit* unit, int i){
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:call[not(ancestor::src:throw) and not(ancestor::src:catch) and";
    xpath += " preceding-sibling::*[1][self::src:operator='new']]/src:name";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_results = srcml_transform_get_unit_size(result);
    bool found_constructor_call = false;
    if (number_of_results > 0){
        found_constructor_call = true;
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
    return found_constructor_call;
}


//
//
bool classModel::isEmptyMethod(srcml_archive* archive, srcml_unit* unit, int i){
    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "'][not(src:block/src:block_content/*[not(self::src:comment)][1])]";
    srcml_append_transform_xpath(archive, xpath.c_str());
    srcml_transform_result* result;
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_results = srcml_transform_get_unit_size(result);
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
    return number_of_results == 1;
}


//
//  TODO: output the default stereotype if NO_STEREOTYPE - currently gives no attribute
//  Copy and add in stereotype attribute on <function>
//
//  Example <function stereotype="get">
//
srcml_unit* classModel::writeStereotypeAttribute(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int n = unitOneCount;
    int offset = 0;
    if (!oneUnit) {
        n = unitTwoCount;
        offset = unitOneCount;
    }

    for (int i = 0; i < n; ++i){
        std::string stereotype = method[i+offset].getStereotype();
        if (stereotype == NO_STEREOTYPE && unitOneCount + unitTwoCount == 1) return nullptr;
        if (stereotype != NO_STEREOTYPE){
            std::string xpath = "//src:function[string(src:name)='";
            xpath += method[i+offset].getName() + "' and string(src:type)='";
            xpath += method[i+offset].getReturnType() + "' and string(src:parameter_list)='";
            xpath += method[i+offset].getParameters() + "' and string(src:specifier)='";
            xpath += method[i+offset].getConst() + "']";
            srcml_append_transform_xpath_attribute(archive, xpath.c_str(), "st", "http://www.srcML.org/srcML/stereotype", "stereotype", stereotype.c_str());
        }
    }
    srcml_transform_result* result;
    srcml_unit_apply_transforms(archive, unit, &result);
    unit = srcml_transform_get_unit(result, 0);
    srcml_clear_transforms(archive);
    return unit;
}

//
//
void classModel::printReturnTypes(){
    std::cout << "RETURN TYPES: \n";
    for (int i = 0; i < method.size(); ++i){
        std::cout << method[i].getReturnType() << "\n";
    }
    std::cerr << std::endl;
}

//
//
void classModel::printStereotypes(){
    std::cout << "STEREOTYPES: \n";
    for (int i = 0; i < method.size(); ++i){
        std::cout << method[i].getStereotype() << "\n";
    }
    std::cerr << std::endl;
}

//
//
void classModel::printMethodNames(){
    std::cout << "METHOD Names: \n";
    for (int i = 0; i < method.size(); ++i){
        std::cerr << i << "  " << method[i].getName() << std::endl;
    }
}


//
//
void classModel::printMethodHeaders(){
    std::cout << "METHOD HEADERS: \n";
    for (int i = 0; i < method.size(); ++i){
        std::cerr << i << "  " << method[i].getHeader() << std::endl;
    }
}

//
//
void classModel::printAttributes(){
    std::cout << "ATTRIBUTES: \n";
    std::cout << attribute.size() << " names\n";

    for (int i = 0; i < attribute.size(); ++i){
        std::cerr << "TYPE: " << attribute[i].getType() << " NAME: " << attribute[i].getName() << std::endl;
    }

}

//
//
void classModel::printReportToFile(std::ofstream& output_file, const std::string& input_file_path){
    int func_count = unitOneCount + unitTwoCount;
    if (output_file.is_open()){
        for (int i = 0; i < func_count; ++i){
            output_file << input_file_path << "|" << method[i].getHeader() << "||" << method[i].getStereotype() << "\n";
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

