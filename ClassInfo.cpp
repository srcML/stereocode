//classModel for stereocode
//

#include "ClassInfo.hpp"

//
//  secondUnit is only for C++ in which case firstUnit is hpp and secondUnit is cpp
//
classModel::classModel(srcml_archive* archive, srcml_unit* firstUnit, srcml_unit* secondUnit) : classModel() {
    language = srcml_unit_get_language(firstUnit);

    findClassName(archive, firstUnit);
    findParentClassName(archive, firstUnit);
    findAttributeNames(archive, firstUnit);
    findAttributeTypes(archive, firstUnit);

    findMethods(archive, firstUnit, true);
    if (secondUnit) findMethods(archive, secondUnit, false);

    findMethodNames();
    findParameterLists();
    findMethodReturnTypes();
    returnsAttributes();
    findLocalVariableNames();
    findParameterNames();
    findParameterTypes();
    countChangedAttributes();
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
        std::string name = srcml_unit_get_srcml(result_unit);

        // chop off everything beyond variable name
        size_t end_position = name.find("</name>");
        name = name.substr(0,end_position);

        // chop off begining <name>(regular variables) or <name><name>(arrays)
        while (name.substr(0,6) == "<name>")
            name.erase(0,6);

        attribute.push_back(variable(name));
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
        std::string type = srcml_unit_get_srcml(result_unit);
        
        char* unparsed = new char[type.size() + 1];
        std::strcpy(unparsed, type.c_str());
        size_t size = type.size();

        srcml_unit_unparse_memory(result_unit, &unparsed, &size);

        if (type == "<type ref=\"prev\"/>"){
            type = prev;
        }else{  
            type = unparsed;
            prev = type;
        }
        delete[] unparsed;

        attribute[i].setType(type);
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}





// WORKING on METHODS






//
//  Calls ctor for methodModel and finds:
//      srcML for the method
//      header
//      const-ness
//
void classModel::findMethods(srcml_archive* archive, srcml_unit* unit, bool oneUnit) {
    srcml_append_transform_xpath(archive, "//src:function");
    srcml_transform_result* result = nullptr;
    srcml_unit_apply_transforms(archive, unit, &result);

    int n = srcml_transform_get_unit_size(result);

    if (oneUnit) {
        unitOneCount = n;       //COMPUTED HERE
    } else {
        unitTwoCount = n;       //COMPUTED HERE
    }


    srcml_unit* result_unit = nullptr;
    srcml_archive* temp = nullptr;

    for (int i = 0; i < n; ++i){
        result_unit = srcml_transform_get_unit(result, i);

        temp = srcml_archive_create();
        char* str = nullptr;
        size_t s = 0;
        srcml_archive_write_open_memory(temp, &str, &s);
        srcml_archive_write_unit(temp, result_unit);
        srcml_archive_close(temp);
        std::string xml(str);
        free(str);

        std::string function = srcml_unit_get_srcml(result_unit);

        bool isConstMethod = checkConst(function);
        char * unparsed = new char [function.size() + 1];
        size_t size = function.size() + 1;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string header(unparsed);
        delete[] unparsed;
        
        header = header.substr(0, header.find("{"));
        header.erase(std::remove(header.begin(), header.end(), '\n'), header.end());  //remove newline characters

        method.push_back(methodModel(xml, header, isConstMethod));
    }
    srcml_clear_transforms(archive);
    srcml_transform_free(result);
}

//
// Gets the name of all methods
//
void classModel::findMethodNames() {
    for (int i = 0; i < method.size(); ++i) {
        srcml_archive*          archive = nullptr;
        srcml_unit*             unit = nullptr;
        srcml_unit*             resultUnit = nullptr;
        srcml_transform_result* result = nullptr;

        archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
        srcml_append_transform_xpath(archive, "//src:function/src:name");
        unit = srcml_archive_read_unit(archive);
        srcml_unit_apply_transforms(archive, unit, &result);
        resultUnit = srcml_transform_get_unit(result, 0);

        std::string name_srcml = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char [name_srcml.size() + 1];
        size_t size = name_srcml.size() + 1;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string name(unparsed);
        delete[] unparsed;

        method[i].setName(name);

        srcml_unit_free(unit);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_transform_free(result);
    }
}


//
// Gets the parameter list of all methods
//
void classModel::findParameterLists() {
    for (int i = 0; i < method.size(); ++i) {
        srcml_archive*          archive = nullptr;
        srcml_unit*             unit = nullptr;
        srcml_unit*             resultUnit = nullptr;
        srcml_transform_result* result = nullptr;

        archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
        srcml_append_transform_xpath(archive, "//src:function/src:parameter_list");
        unit = srcml_archive_read_unit(archive);
        srcml_unit_apply_transforms(archive, unit, &result);
        resultUnit = srcml_transform_get_unit(result, 0);

        std::string name_srcml = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char [name_srcml.size() + 1];
        size_t size = name_srcml.size() + 1;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);

        std::string parameter_list(unparsed);
        delete[] unparsed;

        method[i].setParameters(parameter_list);

        srcml_unit_free(unit);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_transform_free(result);
    }
}

//
//
// collects return types for each function
//
void classModel::findMethodReturnTypes(){
    for (int i = 0; i < method.size(); ++i) {
        srcml_archive*          archive = nullptr;
        srcml_unit*             unit = nullptr;
        srcml_unit*             resultUnit = nullptr;
        srcml_transform_result* result = nullptr;

        archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
        srcml_append_transform_xpath(archive, "//src:function/src:type");
        unit = srcml_archive_read_unit(archive);
        srcml_unit_apply_transforms(archive, unit, &result);
        resultUnit = srcml_transform_get_unit(result, 0);

        std::string return_type = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char[return_type.size() + 1];
        std::strcpy (unparsed, return_type.c_str());
        size_t size = return_type.size() +1;

        int error = srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string type(unparsed);
        delete[] unparsed;

        method[i].setReturnType(type);

        srcml_unit_free(unit);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_transform_free(result);
    }
}



//
// Finds all the local variables in each method
//
void classModel::findLocalVariableNames(){
    for (int i = 0; i < method.size(); ++i) {
        method[i].setLocalVariables(methodLocalVariables(i));
    }
}

//
// Finds all the local variables within a given method[i]
//
std::vector<std::string> classModel::methodLocalVariables(int i){
    std::vector<std::string> locals;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

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

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int j = 0; j < n; ++j){
        resultUnit = srcml_transform_get_unit(result, j);
        std::string var_name = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char [var_name.size() + 1];
        size_t size = var_name.size() + 1;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        var_name = unparsed;
        delete[] unparsed;
        trimWhitespace(var_name);
        size_t arr = var_name.find("[");
        if (arr != std::string::npos){
            var_name.erase(arr, arr-var_name.size());
        }
        locals.push_back(var_name);
    }
    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return locals;
}


// Finds the parameter names in each method
//
void classModel::findParameterNames(){
    for (int i = 0; i < method.size(); ++i) {
        method[i].setParameterNames(methodParameterNames(i));
    }
}

//
//
// returns a vector of string containing the parameters name of function(i)
//
std::vector<std::string> classModel::methodParameterNames(int i){
    std::vector<std::string> names;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']/src:parameter_list//src:parameter/src:decl/src:name";


    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);

    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i){
        resultUnit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char [type.size() + 1];
        size_t size = type.size() + 1;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string param_name(unparsed);
        delete[] unparsed;

        names.push_back(param_name);
    }
    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return names;
}


// Finds the parameter types in each method
//
void classModel::findParameterTypes(){
    for (int i = 0; i < method.size(); ++i) {
        method[i].setParameterTypes(methodParameterTypes(i));
    }
}

//
//
// returns a vector of strings containing the parameters type and specifiers of function #i
//
std::vector<std::string> classModel::methodParameterTypes(int i){
    std::vector<std::string> types;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']/src:parameter_list//src:parameter/src:decl/src:type";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);

    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i){
        resultUnit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char [type.size() + 1];
        size_t size = type.size() + 1;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string param_type(unparsed);
        delete[] unparsed;
        types.push_back(param_type);
    }
    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return types;
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
void classModel::stereotypeGetter() {
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
//  only 1 attribute has been changed
//
void classModel::stereotypeSetter() {
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
// return expression is not a attribute
//
// Stereotype collaborational-predicate:
// method is const
// returns boolean
// return expression is not a attribute
// does not use any attribute in the method
// has no pure calls
void classModel::stereotypePredicate() {
    for (int i = 0; i < method.size(); ++i){
        std::string returnType = separateTypeName(method[i].getReturnType());
        if (returnType == "bool" && !method[i].returnsAttribute() && method[i].isConst()) {
            bool usesAttr = usesAttribute(i);
            std::vector<std::string> real_calls = findCalls(i, "real");
            bool hasCallOnAttribute = callsAttributesMethod(real_calls, method[i].getLocalVariables(), method[i].getParameterNames());
            usesAttr = usesAttr || hasCallOnAttribute;
            std::vector<std::string> pure_calls = findCalls(i, "pure");
            int pureCallsCount = countPureCalls(pure_calls);

            if (!usesAttr && pureCallsCount == 0){
                method[i].setStereotype("collaborational-predicate");
            }
            else{
                method[i].setStereotype("predicate");
            }
        }
    }
}


// Stereotype property:
// method is const
// return type is not boolean or void
// does not return a attribute

// Stereotype collaborational-property
// method is const
// return type is not void or boolean
// does not contain a attribute anywhere in the function
// does not contain any pure calls
// does not contain any calls on attributes
void classModel::stereotypeProperty() {
    for (int i = 0; i < method.size(); ++i){
        std::string returnType = separateTypeName(method[i].getReturnType());
        if (returnType != "bool" && returnType != "void" && !method[i].returnsAttribute() && method[i].isConst()){
            bool usesAttr = usesAttribute(i);
            std::vector<std::string> real_calls = findCalls(i, "real");

            bool hasCallOnAttribute = callsAttributesMethod(real_calls, method[i].getLocalVariables(), method[i].getParameterNames());
            usesAttr = usesAttr || hasCallOnAttribute;

            std::vector<std::string> pure_calls = findCalls(i, "pure");
            int pureCallsCount = countPureCalls(pure_calls);


            if (!usesAttr && pureCallsCount == 0){
                method[i].setStereotype("collaborational-property");
            }
            else{
                method[i].setStereotype("property");
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
void classModel::stereotypeVoidAccessor() {
    for (int i = 0; i < method.size(); ++i){
        if(isVoidAccessor(i)){
            bool usesAttr = usesAttribute(i);
            std::vector<std::string> real_calls = findCalls(i, "real");

            bool hasCallOnAttribute = callsAttributesMethod(real_calls, method[i].getLocalVariables(), method[i].getParameterNames());
            usesAttr = usesAttr || hasCallOnAttribute;

            std::vector<std::string> pure_calls = findCalls(i, "pure");
            int pureCallsCount = countPureCalls(pure_calls);
            if (!usesAttr && pureCallsCount == 0){
                method[i].setStereotype("collaborational-voidaccessor");
            }
            else{
                method[i].setStereotype("voidaccessor");
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
//    exactly one attribute is written to and the number of calls
//    in expression statements or returns is at least 2
//
//    no attributes are written to and there is a call not
//    in a throw statement that is a simple real call (not a constructor call)
//    or a complex call for a attribute
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
// need to handle the case where 0 attributes are written and there is a call on a attribute.
//
void classModel::stereotypeCommand() {
    for (int i = 0; i < method.size(); ++i){
        std::vector<std::string> real_calls = findCalls(i, "real");
        std::vector<std::string> pure_calls = findCalls(i, "pure");

        std::string returnType = separateTypeName(method[i].getReturnType());
        int pureCallsCount = countPureCalls(pure_calls);
        bool hasCallOnAttribute = callsAttributesMethod(real_calls, method[i].getLocalVariables(), method[i].getParameterNames());
        bool case1 = (method[i].getAttributesModified() == 1 && real_calls.size() > 1);
        bool case2 = method[i].getAttributesModified() == 0 && (pureCallsCount > 0 || hasCallOnAttribute);
        bool case3 = method[i].getAttributesModified() > 1;
        if ((case1 || case2 || case3) && !method[i].isConst()){
            if (returnType == "void" || returnType == "bool") {
                method[i].setStereotype("command");
            }   
            else {
                method[i].setStereotype("non-void-command");
            }
        } else if (method[i].getAttributesModified() > 0 && method[i].isConst()) { // handles case of mutable attributes
            if (method[i].getStereotype() != NO_STEREOTYPE){
                method[i].setStereotype(method[i].getStereotype() + " command");
            } else {
                method[i].setStereotype("command");
            }
        }
    }
}


//stereotype collaborational-command
//method is not const and
//no attributes are written and
//no pure calls, a() a::b() and
//no calls on attributes and
//at least 1 call or parameter or local variable is written
//Calls allowed:  f->g() where f is not a attribute, new f() (which isn't a real call)
//
void classModel::stereotypeCollaborationalCommand() {
    for (int i = 0; i < method.size(); ++i){
        std::vector<std::string> all_calls = findCalls(i, "");
        bool hasCallOnAttribute = callsAttributesMethod(all_calls, method[i].getLocalVariables(), method[i].getParameterNames());
        
        if (!method[i].isConst() && method[i].getAttributesModified() == 0) {
            bool local_var_written = false;
            for (int j = 0; j < method[i].getLocalVariables().size(); ++j){
                if(variableChanged(i, method[i].getLocalVariables()[j])){
                    local_var_written = true;
                    break;
                }
            }
            bool param_written = false;
            for (int j = 0; j < method[i].getParameterNames().size(); ++j){
                if(variableChanged(i, method[i].getParameterNames()[j])){
                    param_written = true;
                    break;
                }
            }
            std::vector<std::string> pure_calls = findCalls(i, "pure");
            int pureCallsCount = countPureCalls(pure_calls);

            bool not_command =  pureCallsCount == 0 && !hasCallOnAttribute;
            if (not_command && (all_calls.size() > 0 || local_var_written || param_written)){
                method[i].setStereotype("collaborational-command");
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
void classModel::stereotypeCollaborator() {
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

    for (int i = 0; i < method.size(); ++i){
        bool param_obj = containsNonPrimitive(i, param);
        bool local_obj = containsNonPrimitive(i, local_var);
        bool attr_obj = usesAttributeObj(i, non_primitive_attributes);
        std::string returnType = separateTypeName(method[i].getReturnType());
        bool matches_class_name = returnType == className;
        bool ret_obj = !isPrimitiveContainer(returnType) && returnType != "void" && !matches_class_name;
        if (local_obj || param_obj || attr_obj || ret_obj){
            if (method[i].getStereotype() == NO_STEREOTYPE && !method[i].isConst()){
                method[i].setStereotype("controller");
            }
            else{
                method[i].setStereotype(method[i].getStereotype() + " collaborator");
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

void classModel::stereotypeFactory() {
    for (int i = 0; i < method.size(); ++i){
        if (isFactory(i)){
            method[i].setStereotype("factory");
        }
    }
}


// stereotype empty
//
//
void classModel::stereotypeEmpty() {
    for (int i = 0; i < method.size(); ++i){
        if (isEmptyMethod(i)){
            method[i].setStereotype("empty");
        }
    }
}


// stereotype stateless
// is not an empty method
// has no real calls including new calls
// does not use any attributes
//
// stereotype wrapper
// is not empty
// has exactly 1 real call including new calls
// does not use any data memebers
//
void classModel::stereotypeStateless() {
    for (int i = 0; i < method.size(); ++i) {
        bool empty = isEmptyMethod(i);
        std::vector<std::string> calls = findCalls(i, "");
        std::vector<std::string> real_calls = findCalls(i, "real");
        bool usedAttr = usesAttribute(i);
        bool hasCallOnAttribute = callsAttributesMethod(real_calls, method[i].getLocalVariables(), method[i].getParameterNames());
        usedAttr = usedAttr || hasCallOnAttribute;
        if (!empty && calls.size() < 1 && !usedAttr){
            method[i].setStereotype(method[i].getStereotype() + " stateless");
            if (method[i].getStereotype() == "nothing-yet stateless"){
                method[i].setStereotype("stateless");
            }
        }
        if (!empty && calls.size() == 1 && !usedAttr){
            method[i].setStereotype(method[i].getStereotype() + " wrapper");
            if (method[i].getStereotype() == "nothing-yet wrapper"){
                method[i].setStereotype("wrapper");
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
        return(PRIMITIVES.isPrimitive(key) && PRIMITIVES.isPrimitive(value));
    }
    // else check if primitive(NOT container).
    return PRIMITIVES.isPrimitive(return_type);
    
}



//
//for each function, find all return expressions and determine if they contain an attribute
//
void classModel::returnsAttributes(){
    for(int i = 0; i < method.size(); ++i) {
        std::vector<std::string> return_expressions = findReturnExpressions(i, true);
        bool returns_attribute = false;
        for (int j = 0; j < return_expressions.size(); ++j){
            std::string return_expr = return_expressions[j];
            if (return_expr.find("*") == 0) return_expr.erase(0, 1);  // handles the case of '*a'
            if (isAttribute(return_expr)){
                returns_attribute = true;
                break;
            }
            if (inherits())
                if (isInheritedAttribute(method[i].getParameterNames(), method[i].getLocalVariables(), return_expr)){
                    returns_attribute = true;
                    attribute.push_back(variable(return_expr, method[i].getReturnType()));
                    break;
                }
        }
        method[i].setReturnsAttribute(returns_attribute);
    }
}


//
//
std::vector<std::string> classModel::findReturnExpressions(int i, bool getter){
    std::vector<std::string> returnExpressions;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:return/src:expr";
    if (getter){
        xpath += "[(count(*)=1 and src:name) or (count(*)=2 and *[1][self::src:operator='*'] and *[2][self::src:name])]";
    }
    srcml_archive*          archive = nullptr;
    srcml_unit*             unit = nullptr;
    srcml_unit*             resultUnit = nullptr;
    srcml_transform_result* result = nullptr;

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int number_of_results = srcml_transform_get_unit_size(result);

    for (int j = 0; j < number_of_results; ++j) {
        resultUnit = srcml_transform_get_unit(result, j);
        std::string expr = srcml_unit_get_srcml(resultUnit);
        if (expr.find("<expr><literal type=\"number\"") == 0) {
            expr = "#";
        } else {
            char * unparsed = new char [expr.size() + 1];
            size_t size = expr.size() + 1;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
            expr = unparsed;
            delete[] unparsed;
        }
        returnExpressions.push_back(expr);
    }
    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return returnExpressions;
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
bool classModel::isVoidAccessor(int funcIndex){
    std::vector<std::string> types = method[funcIndex].getParameterTypes();
    std::vector<std::string> names = method[funcIndex].getParameterNames();
    std::string returnType = separateTypeName(method[funcIndex].getReturnType());

    for (int j = 0; j < types.size(); ++j){
        bool reference = types[j].find("&") != std::string::npos;
        bool constant = types[j].find("const") != std::string::npos;
        bool primitive = isPrimitiveContainer(types[j]);

        // if the parameter type contains an &, is not const and is primitive 
        // and the return type of the method is void.
        if (reference && !constant && primitive && returnType == "void" && method[funcIndex].isConst()){
            bool param_changed = variableChanged(funcIndex, names[j]);
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
//
bool classModel::variableChanged(int i, const std::string& var_name){
    bool changed = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:name[.='" + var_name + "' and not(ancestor::src:throw) ";
    xpath += "and not(ancestor::src:catch)]/following-sibling::*[1]";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i){
        resultUnit = srcml_transform_get_unit(result, i);
        std::string next_element = srcml_unit_get_srcml(resultUnit);
        int equal_sign_count = 0;
        for (int j = 0; j < next_element.size(); ++j){
            if (next_element[j] == '='){
                equal_sign_count++;
            }
        }
        if (equal_sign_count == 1){
            changed = true;
        }
    }
    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return changed;
}




//
// need to count different attributes and not the same one twice.
// check with multiple changes with same opperator(same var), different operators(same var), same operator(dif var)
// test ++attribute += 10;
//
void classModel::countChangedAttributes(){
    for (int i = 0; i < method.size(); ++i){
        int changes = 0;
        changes += findIncrementedAttribute(i, false);     //No loop
        changes += findIncrementedAttribute(i, true);      //loop
        changes += findAssignOperatorAttribute(i, false);  //No loop
        changes += findAssignOperatorAttribute(i, true);   //loop

        method[i].setAttributesModified(changes);
    }
}


//
//
int classModel::findAssignOperatorAttribute(int i, bool check_for_loop) {
    int changed = 0;

    for (int j = 0; j < ASSIGNMENT_OPERATOR.size(); ++j) {
        srcml_archive*           archive = nullptr;
        srcml_unit*              unit = nullptr;
        srcml_unit*              resultUnit = nullptr;
        srcml_transform_result*  result = nullptr;

        std::string xpath = "//src:function[string(src:name)='";
        xpath += method[i].getName() + "' and string(src:type)='";
        xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
        xpath += method[i].getParameters() +"']//src:operator[.='";
        xpath += ASSIGNMENT_OPERATOR[j] + "' and not(ancestor::src:control)";
        xpath += " and not(ancestor::src:throw) and not(ancestor::src:catch)";
        if (check_for_loop) xpath += " and ancestor::src:for";
        xpath += "]/preceding-sibling::src:name";

        archive = srcml_archive_create();
        srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
        srcml_append_transform_xpath(archive, xpath.c_str());
        unit = srcml_archive_read_unit(archive);
        srcml_unit_apply_transforms(archive, unit, &result);
        int n = srcml_transform_get_unit_size(result);

        for (int k = 0; k < n; ++k){
            resultUnit = srcml_transform_get_unit(result, k);
            std::string name = srcml_unit_get_srcml(resultUnit);
            char * unparsed = new char [name.size() + 1];
            size_t size = name.size() + 1;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
            std::string var_name(unparsed);
            delete[] unparsed;
            trimWhitespace(var_name); // removes whitespace
            if (isAttribute(var_name)) {
                 ++changed;
            } else if (inherits())
                if (isInheritedAttribute(method[i].getParameterNames(), method[i].getLocalVariables(), var_name)) {
                    attribute.push_back(variable(var_name, "unknown"));
                    ++changed;
                }
        }
        srcml_unit_free(unit);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_transform_free(result);
    }
    return changed;
}


//
//
int classModel::findIncrementedAttribute(int i, bool check_for_loop){
    const std::vector<std::string> INC_OPS = {"++", "--"};
    const std::vector<std::string> LOCATION = {"following-sibling", "preceding-sibling"};
    int changed = 0;

    for (int j = 0; j < INC_OPS.size(); ++j){          //for each operator (++ and --)
        for (int k = 0; k < LOCATION.size(); ++k){     //check following and preceeding
            srcml_archive*           archive = nullptr;
            srcml_unit*              unit = nullptr;
            srcml_unit*              resultUnit = nullptr;
            srcml_transform_result*  result = nullptr;

            std::string xpath = "//src:function[string(src:name)='";
            xpath += method[i].getName() + "' and string(src:type)='";
            xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
            xpath += method[i].getParameters() + "' and string(src:specifier)='";
            xpath += method[i].getConst() + "']//src:operator[.='";
            xpath += INC_OPS[j] + "' and not(ancestor::src:control)";
            xpath += " and not(ancestor::src:throw) and not(ancestor::src:catch)";
            if (check_for_loop) xpath += " and ancestor::src:for";
            xpath += "]/" + LOCATION[k] + "::src:name[1]";

            archive = srcml_archive_create();
            srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
            srcml_append_transform_xpath(archive, xpath.c_str());
            unit = srcml_archive_read_unit(archive);
            srcml_unit_apply_transforms(archive, unit, &result);
            int n = srcml_transform_get_unit_size(result);

            if (n != 0){
                resultUnit = srcml_transform_get_unit(result, 0);
                std::string name = srcml_unit_get_srcml(resultUnit);
                char * unparsed = new char [name.size() + 1];
                size_t size = name.size() + 1;
                srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
                std::string var_name(unparsed);
                delete[] unparsed;
                trimWhitespace(var_name);// removes whitespace
                if (isAttribute(var_name)) {
                    ++changed;
                } else if (inherits())
                    if (isInheritedAttribute(method[i].getParameterNames(), method[i].getLocalVariables(), var_name)) {
                        attribute.push_back(variable(var_name, "unknown"));
                        ++changed;
                    }
            }
            srcml_unit_free(unit);
            srcml_clear_transforms(archive);
            srcml_archive_close(archive);
            srcml_transform_free(result);
        }
    }
    return changed;
}





// returns a list of call names that are not below throw or following new: when pure_call is false
// does not include calls following the . or -> operators: when pure_call is true
//
std::vector<std::string> classModel::findCalls(int i, const std::string& call_type){
    std::vector<std::string> calls;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:call[not(ancestor::src:throw) and not(ancestor::src:catch)";
    if (call_type == "pure") {
        xpath += " and not(preceding-sibling::*[1][self::src:operator='new'])";
        xpath += " and not(preceding-sibling::*[1][self::src:operator='.'])";
        xpath += " and not(preceding-sibling::*[1][self::src:operator='->'])";
    } else if (call_type == "real") {
        xpath += "and not(preceding-sibling::*[1][self::src:operator='new'])";
    }
    xpath += "]/src:name";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i){
        resultUnit = srcml_transform_get_unit(result, i);
        std::string call = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char [call.size() + 1];
        size_t size = call.size() + 1;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        call = unparsed;
        delete[] unparsed;
        trimWhitespace(call);
        if (call != "assert" && !isPrimitiveContainer(call)) {
            calls.push_back(call);
        }
    }
    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return calls;
}



//
//
bool classModel::callsAttributesMethod(const std::vector<std::string>& real_calls,
                                       const std::vector<std::string>& local_var_names,
                                       const std::vector<std::string>& param_names){
    bool result = false;
    for (int i = 0; i < real_calls.size(); ++i){
        size_t dot = real_calls[i].find(".");
        size_t arrow = real_calls[i].find("->");
        if (dot != std::string::npos){
            std::string calling_object = real_calls[i].substr(0, dot);
            if(isAttribute(calling_object)){
                result = true;
            }
            else if (inherits())
                if (isInheritedAttribute(param_names, local_var_names, calling_object)) {
                    result = true;
                    attribute.push_back(variable(calling_object, "unknown"));
                }
        }
        if (arrow != std::string::npos) {
            std::string calling_object = real_calls[i].substr(0, arrow);
            if (isAttribute(calling_object)) {
                result = true;
            }
            else if (inherits())
                if (isInheritedAttribute(param_names, local_var_names, calling_object)) {
                    result = true;
                    attribute.push_back(variable(calling_object, "unknown"));
                }
        }
    }
    return result;
}


//
//
// checks for non primitive parameters and local variables
//
bool classModel::containsNonPrimitive(int i, const std::string& x) {
    bool contains = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']" + x + "/src:decl/src:type/src:name";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i){
        resultUnit = srcml_transform_get_unit(result, i);
        std::string type = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char [type.size() + 1];
        size_t size = type.size() + 1;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string param_type(unparsed);
        delete[] unparsed;
        // handles case of (void) for param list
        if (n == 1 && x == "/src:parameter_list//src:parameter" && param_type == "void")
            break;
        if (!isPrimitiveContainer(param_type) && param_type != className){
            contains = true;
            break;
        }
        
    }
    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);
    return contains;
}



//
//
bool classModel::usesAttributeObj(int i, const std::vector<std::string>& obj_names){
    bool found = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:name[not(ancestor::src:throw) and not(ancestor::src:catch)]";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int i = 0; i < n; ++i){
        resultUnit = srcml_transform_get_unit(result, i);
        std::string attribute_name = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char [attribute_name.size() + 1];
        size_t size = attribute_name.size() + 1;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string attr_name(unparsed);
        delete[] unparsed;
        for (int j = 0; j < obj_names.size(); ++j) {
            if (attr_name == obj_names[j]){
                srcml_clear_transforms(archive);
                srcml_transform_free(result);
                found = true;
                break;
            }
        }
    }
    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return found;
}

//
//
// Returns if a method has a USE of an attribute (including inherited attributes)
//  Does not consider DEF of an attribute
//  Also adds any inherited attributes to attribute
//
bool classModel::usesAttribute(int i){
    bool found = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:expr[not(ancestor::src:throw) and";
    xpath += "not(ancestor::src:argument_list[@type='generic']) and not(ancestor::src:catch)]/src:name";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    for (int j = 0; j < n; ++j){
        resultUnit = srcml_transform_get_unit(result, j);
        std::string name = srcml_unit_get_srcml(resultUnit);
        char * unparsed = new char [name.size() + 1];
        size_t size = name.size() + 1;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string possible_attr(unparsed);
        delete[] unparsed;

        if (PRIMITIVES.isPrimitive(possible_attr)) continue;  //Should never be a primitive type??? Remove??

        if (isAttribute(possible_attr)) {
            found = true;
        } else if (inherits())
            if (isInheritedAttribute(method[i].getParameterNames(), method[i].getLocalVariables(), possible_attr)) {
                attribute.push_back(variable(possible_attr, "unknown"));
                found = true;
            }
    }
    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return found;
}


//
//
// returns true if method, specified by number and unit, is a factory
//
bool classModel::isFactory(int funcIndex){
    std::vector<std::string> return_expressions = findReturnExpressions(funcIndex, false);
    std::vector<std::string> param_names        = method[funcIndex].getParameterNames();
    bool                     returns_ptr        = method[funcIndex].getReturnType().find("*") != std::string::npos;
    bool                     returns_obj        = !isPrimitiveContainer(method[funcIndex].getReturnType());
    bool returns_local = false;
    bool returns_new   = false;
    bool returns_param = false;

    for (int i = 0; i < return_expressions.size(); ++i){
        for (int j = 0; j < method[funcIndex].getLocalVariables().size(); ++j) {
            if(return_expressions[i] == method[funcIndex].getLocalVariables()[j])
                 returns_local = true;
        }
        for (int k = 0; k < param_names.size(); ++k) {
            if (return_expressions[i] == param_names[k])
                returns_param = true;
        }
        if(return_expressions[i].find("new") != std::string::npos)
            returns_new = true;
    }
    bool new_call   = findConstructorCall(funcIndex);

    bool return_ex  = (returns_local || returns_new || returns_param || method[funcIndex].returnsAttribute());
    bool is_factory = returns_obj && returns_ptr && new_call && return_ex;
    return is_factory;
}




//
//
// this function finds a constuctor call that is after a new operator that matches the class name
//
bool classModel::findConstructorCall(int i){
    bool found = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "']//src:call[not(ancestor::src:throw) and not(ancestor::src:catch) and";
    xpath += " preceding-sibling::*[1][self::src:operator='new']]/src:name";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    if (n > 0)
        found = true;

    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return found;
}


//
//
bool classModel::isEmptyMethod(int i) {
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParameters() + "' and string(src:specifier)='";
    xpath += method[i].getConst() + "'][not(src:block/src:block_content/*[not(self::src:comment)][1])]";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, method[i].getsrcML().c_str(), method[i].getsrcML().size());
    srcml_append_transform_xpath(archive, xpath.c_str());
    unit = srcml_archive_read_unit(archive);
    srcml_unit_apply_transforms(archive, unit, &result);
    int n = srcml_transform_get_unit_size(result);

    srcml_unit_free(unit);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_transform_free(result);

    return n == 1;
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
            srcml_append_transform_xpath_attribute(archive, xpath.c_str(), "st",
                                                   "http://www.srcML.org/srcML/stereotype",
                                                   "stereotype", stereotype.c_str());
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






