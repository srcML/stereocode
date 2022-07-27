//
//  method.cpp
//  
//
//  Created by jmaletic on July 20 2022.
//

#include "method.hpp"

methodModel::methodModel() : name(), parametersXML(),
                             parameterNames(), parameterTypes(),
                             srcML(), headerXML(),
                             returnType(), constMethod(false),
                             retAttribute(false), attributesModified(0),
                             localVariables(), stereotype(NO_STEREOTYPE) {

};


methodModel::methodModel(const std::string& xml, const std::string& s, bool f) : methodModel() {
    srcML = xml;
    headerXML = s;
    constMethod = f;
};


//
//
// Finds a constuctor call that is after a new operator that matches the class name
//
bool methodModel::findConstructorCall() const{
    bool found = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += getName() + "' and string(src:type)='";
    xpath += getReturnType() + "' and string(src:parameter_list)='";
    xpath += getParametersXML() + "' and string(src:specifier)='";
    xpath += getConst() + "']//src:call[not(ancestor::src:throw) and not(ancestor::src:catch) and";
    xpath += " preceding-sibling::*[1][self::src:operator='new']]/src:name";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, getsrcML().c_str(), getsrcML().size());
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
std::vector<std::string> methodModel::findReturnExpressions(bool getter) const {
    std::vector<std::string> returnExpressions;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += getName() + "' and string(src:type)='";
    xpath += getReturnType() + "' and string(src:parameter_list)='";
    xpath += getParametersXML() + "' and string(src:specifier)='";
    xpath += getConst() + "']//src:return/src:expr";
    if (getter){
        xpath += "[(count(*)=1 and src:name) or (count(*)=2 and *[1][self::src:operator='*'] and *[2][self::src:name])]";
    }
    srcml_archive*          archive = nullptr;
    srcml_unit*             unit = nullptr;
    srcml_unit*             resultUnit = nullptr;
    srcml_transform_result* result = nullptr;

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, getsrcML().c_str(), getsrcML().size());
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
// returns true if method is a factory
//
bool methodModel::isFactory() const {
    std::vector<std::string> return_expressions = findReturnExpressions(false);
    std::vector<std::string> param_names        = getParameterNames();
    bool                     returns_ptr        = getReturnType().find("*") != std::string::npos;
    bool                     returns_obj        = !isPrimitiveContainer(getReturnType());
    bool returns_local = false;
    bool returns_new   = false;
    bool returns_param = false;

    for (int i = 0; i < return_expressions.size(); ++i){
        for (int j = 0; j < getLocalVariables().size(); ++j) {
            if(return_expressions[i] == getLocalVariables()[j])
                 returns_local = true;
        }
        for (int k = 0; k < param_names.size(); ++k) {
            if (return_expressions[i] == param_names[k])
                returns_param = true;
        }
        if(return_expressions[i].find("new") != std::string::npos)
            returns_new = true;
    }
    bool new_call   = findConstructorCall();

    bool return_ex  = (returns_local || returns_new || returns_param || returnsAttribute());
    bool is_factory = returns_obj && returns_ptr && new_call && return_ex;
    return is_factory;
}

//
//
bool methodModel::isEmptyMethod() const {
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += getName() + "' and string(src:type)='";
    xpath += getReturnType() + "' and string(src:parameter_list)='";
    xpath += getParametersXML() + "' and string(src:specifier)='";
    xpath += getConst() + "'][not(src:block/src:block_content/*[not(self::src:comment)][1])]";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, getsrcML().c_str(), getsrcML().size());
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
//
// checks for non primitive parameters and local variables
//
bool methodModel::containsNonPrimitive(const std::string& x, const std::string& className) const {
    bool contains = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += getName() + "' and string(src:type)='";
    xpath += getReturnType() + "' and string(src:parameter_list)='";
    xpath += getParametersXML() + "' and string(src:specifier)='";
    xpath += getConst() + "']" + x + "/src:decl/src:type/src:name";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, getsrcML().c_str(), getsrcML().size());
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
//
bool methodModel::variableChanged(const std::string& var_name) const {
    bool changed = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += getName() + "' and string(src:type)='";
    xpath += getReturnType() + "' and string(src:parameter_list)='";
    xpath += getParametersXML() + "' and string(src:specifier)='";
    xpath += getConst() + "']//src:name[.='" + var_name + "' and not(ancestor::src:throw) ";
    xpath += "and not(ancestor::src:catch)]/following-sibling::*[1]";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, getsrcML().c_str(), getsrcML().size());
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
//
bool methodModel::isVoidAccessor() const {
    std::vector<std::string> types = getParameterTypes();
    std::vector<std::string> names = getParameterNames();
    std::string returnType = separateTypeName(getReturnType());

    for (int j = 0; j < types.size(); ++j){
        bool reference = types[j].find("&") != std::string::npos;
        bool constant = types[j].find("const") != std::string::npos;
        bool primitive = isPrimitiveContainer(types[j]);

        // if the parameter type contains an &, is not const and is primitive
        // and the return type of the method is void.
        if (reference && !constant && primitive && returnType == "void" && isConst()){
            bool param_changed = variableChanged(names[j]);
                if (param_changed || getStereotype() == NO_STEREOTYPE){
                return true;
            }
        }
    }
    if (returnType == "void" && isConst() &&
        getStereotype() == NO_STEREOTYPE){
        return true;
    }
    return false;
}


//
// Finds all the local variables within a given method[i]
//
std::vector<std::string> methodModel::findLocalVariables() const {
    std::vector<std::string> locals;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath_function = "//src:function[string(src:name)='";
    xpath_function += getName() + "' and string(src:type)='";
    xpath_function += getReturnType() + "' and string(src:parameter_list)='";
    xpath_function += getParametersXML() + "' and string(src:specifier)='";
    xpath_function += getConst() + "']";
    std::string decl_stmt = "//src:decl_stmt[not(ancestor::src:throw) and not(ancestor::src:catch)]";
    std::string control = "//src:control/src:init";
    std::string decl_name = "/src:decl/src:name";
    std::string xpath = xpath_function + decl_stmt + decl_name + " | ";
    xpath += xpath_function + control + decl_name;

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, getsrcML().c_str(), getsrcML().size());
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
        var_name = trimWhitespace(var_name);
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


//
//
// returns a vector of string containing the parameters name of the function
//
std::vector<std::string> methodModel::findParameterNames() const {
    std::vector<std::string> names;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += getName() + "' and string(src:type)='";
    xpath += getReturnType() + "' and string(src:parameter_list)='";
    xpath += getParametersXML() + "' and string(src:specifier)='";
    xpath += getConst() + "']/src:parameter_list//src:parameter/src:decl/src:name";


    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, getsrcML().c_str(), getsrcML().size());
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



//
//
// returns a vector of strings containing the parameters type and specifiers of function #i
//
std::vector<std::string> methodModel::findParameterTypes() const {
    std::vector<std::string> types;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += getName() + "' and string(src:type)='";
    xpath += getReturnType() + "' and string(src:parameter_list)='";
    xpath += getParametersXML() + "' and string(src:specifier)='";
    xpath += getConst() + "']/src:parameter_list//src:parameter/src:decl/src:type";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, getsrcML().c_str(), getsrcML().size());
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




// returns a list of call names that are not below throw or following new: when pure_call is false
// does not include calls following the . or -> operators: when pure_call is true
//
std::vector<std::string> methodModel::findCalls(const std::string& call_type) const {
    std::vector<std::string> calls;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += getName() + "' and string(src:type)='";
    xpath += getReturnType() + "' and string(src:parameter_list)='";
    xpath += getParametersXML() + "' and string(src:specifier)='";
    xpath += getConst() + "']//src:call[not(ancestor::src:throw) and not(ancestor::src:catch)";
    if (call_type == "pure") {
        xpath += " and not(preceding-sibling::*[1][self::src:operator='new'])";
        xpath += " and not(preceding-sibling::*[1][self::src:operator='.'])";
        xpath += " and not(preceding-sibling::*[1][self::src:operator='->'])";
    } else if (call_type == "real") {
        xpath += "and not(preceding-sibling::*[1][self::src:operator='new'])";
    }
    xpath += "]/src:name";

    archive = srcml_archive_create();
    srcml_archive_read_open_memory(archive, getsrcML().c_str(), getsrcML().size());
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
        call = trimWhitespace(call);
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


// Output a method for checking and testing
std::ostream& operator<<(std::ostream& out, const methodModel& m) {
    out << "Method: " << trimWhitespace(m.name) << " " << m.getConst();
    out << " returns: " << LRtoSpace(m.getReturnType()) << std::endl;
    out << "   Stereotype: " << m.stereotype << std::endl;
    out << "   Parameters: ";
    for (int i=0; i< m.parameterNames.size(); ++i) {
        out << "(" << m.parameterTypes[i] << " : " << m.parameterNames[i] << ") ";
    }
    out << std::endl;
    out << "   Local Variables: ";
    for (int i=0; i< m.localVariables.size(); ++i) {
        out << m.localVariables[i] << "  ";
    }
    out << std::endl;
    return out;
}

