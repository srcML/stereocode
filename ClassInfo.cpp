//classModel for stereocode
//

#include "ClassInfo.hpp"

//
//  secondUnit is only for C++ in which case firstUnit is hpp and secondUnit is cpp
//
classModel::classModel(srcml_archive* archive, srcml_unit* firstUnit, srcml_unit* secondUnit) : classModel() {
    language = srcml_unit_get_language(firstUnit);
    PRIMITIVES.setLanguage(language);

    //Get class information
    findClassName(archive, firstUnit);
    findParentClassName(archive, firstUnit);
    findAttributeNames(archive, firstUnit);
    findAttributeTypes(archive, firstUnit);

    //Get the methods
    findMethods(archive, firstUnit, true);
    if (secondUnit) findMethods(archive, secondUnit, false);

    //Get basic information on methods
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

        char* unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        if (type == "<type ref=\"prev\"/>"){
            type = prev;
        }else{  
            type = unparsed;
            prev = type;
        }
        free(unparsed);

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
        srcml_archive_free(temp);
        std::string xml(str);
        free(str);

        std::string function = srcml_unit_get_srcml(result_unit);

        bool isConstMethod = checkConst(function);

        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(result_unit, &unparsed, &size);
        std::string header(unparsed);
        free(unparsed);
        
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
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string name(unparsed);
        free(unparsed);

        method[i].setName(name);

        srcml_unit_free(unit);
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_archive_free(archive);
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
        char * unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string parameter_list(unparsed);
        free(unparsed);

        method[i].setParametersXML(parameter_list);

        srcml_unit_free(unit);
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_archive_free(archive);
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

        char * unparsed = nullptr;
        size_t size = 0;
        int error = srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string type(unparsed);
        free(unparsed);

        method[i].setReturnType(type);

        srcml_unit_free(unit);
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_archive_free(archive);
    }
}



//
// Finds all the local variables in each method
//
void classModel::findLocalVariableNames(){
    for (int i = 0; i < method.size(); ++i) {
        method[i].setLocalVariables(method[i].findLocalVariables());
    }
}



// Finds the parameter names in each method
//
void classModel::findParameterNames(){
    for (int i = 0; i < method.size(); ++i) {
        method[i].setParameterNames(method[i].findParameterNames());
    }
}


// Finds the parameter types in each method
//
void classModel::findParameterTypes(){
    for (int i = 0; i < method.size(); ++i) {
        method[i].setParameterTypes(method[i].findParameterTypes());
    }
}



// Determining STEREOTYPES


//Compute class stereotype
// Based on definition from Dragan, Collard, Maletic ICSM 2010
//
void classModel::ComputeClassStereotype() {
    set allMethods;
    set accessors, mutators, getters, setters;
    set collaborators, nonCollaborators, controllers, factory;
    set commands; //command U non-void-command
    set degenerates;

    //Determine sets
    for (int i=0; i < method.size(); ++i) {
        std::string s;
        s = method[i].getStereotype();
        allMethods += i;
        if (s.find("get") != std::string::npos) {   //get or get colaborator
            accessors += i;
            if (s.find("command") == std::string::npos)  //get command is not a get
                getters += i;
        }
        if ((s.find("predicate") != std::string::npos) ||
            (s.find("property") != std::string::npos) ||
            (s.find("void-accessor") != std::string::npos) ) {
            accessors += i;
        }
        if ((s.find("set") != std::string::npos)) {
            mutators += i;
            setters += i;
        }
        if (s.find("command") != std::string::npos) {
            mutators += i;
        }
        if ( (s.find("collaborator") != std::string::npos) ||
             (s.find("controller") != std::string::npos)) {
            collaborators += i;
        } else {
            nonCollaborators += i;
        }
        if (s.find("controller") != std::string::npos) {
            controllers += i;
        }
        if (s.find("factory") != std::string::npos) {
            factory += i;
        }
        if (s.find("command") != std::string::npos) {
            commands += i;
        }
        if ( (s.find("empty") != std::string::npos) ||
             (s.find("stateless") != std::string::npos) ||
             (s.find("wrapper") != std::string::npos) ||
             (s.find("incidental") != std::string::npos) ) {
            degenerates += i;
        }
    }
    if (DEBUG) {  //Print out sets to check
        std::cerr << "Methods: " << allMethods.card() << " " << allMethods << std::endl;
        std::cerr << "Accessors: " << accessors.card() << " " << accessors << std::endl;
        std::cerr << "Getters: " << getters.card() << " " << getters << std::endl;
        std::cerr << "Mutators: " << mutators.card() << " " << mutators << std::endl;
        std::cerr << "Setters: " << setters.card() << " " << setters << std::endl;
        std::cerr << "Collaborators: " << collaborators.card() << " " << collaborators << std::endl;
        std::cerr << "Non Collaborators: " << nonCollaborators.card() << " " << nonCollaborators << std::endl;
        std::cerr << "Controllers: " << controllers.card() << " " << controllers << std::endl;
        std::cerr << "Factory: " << factory.card() << " " << factory << std::endl;
        std::cerr << "Commands: " << commands.card() << " " << commands << std::endl;
        std::cerr << "Degenerates: " << degenerates.card() << " " << degenerates << std::endl;
    }

    classStereotype = "";
    //Entity
    if ( ((accessors - getters) != set()) &&
         ((mutators - setters)  != set()) &&
         (controllers.card() == 0) ) {
        double ratio = double(collaborators.card()) / double(nonCollaborators.card());
        if (ratio > 1.9) {
            if (classStereotype != "") classStereotype += " ";
            classStereotype += "entity";
        }
    }
    //Minimal Entity
    if ((allMethods - (getters + setters + commands)) == set()) {
        double ratio = double(collaborators.card()) / double(nonCollaborators.card());
        if (ratio > 1.9) {
            if (classStereotype != "") classStereotype += " ";
            classStereotype += "minimal-entity";
        }
    }
    //Data Provider
    if ( (accessors.card() > 2 * mutators.card()) &&
         (accessors.card() > 2 * (controllers.card() + factory.card())) ) {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "data-provider";
    }
    //Commander
    if ( (mutators.card() > 2 * accessors.card()) &&
         (mutators.card() > 2 * (controllers.card() + factory.card())) ) {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "command";
    }
    //Boundary
    if ( (collaborators.card() > nonCollaborators.card()) &&
         (factory.card() < 0.5 * allMethods.card()) &&
         (controllers.card() < 0.33 * allMethods.card()) ) {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "boundary";
    }
    //Factory
    if (factory.card() > 0.66 * allMethods.card()) {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "factory";
    }
    //Controller
    if ( (controllers.card() + factory.card() > 0.66 * allMethods.card()) &&
         ((accessors.card() != 0) || (mutators.card() != 0)) ) {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "control";
    }
    //Pure Controller
    if ( (controllers.card() + factory.card() != 0) &&
         (accessors.card() + mutators.card() + collaborators.card() == 0) &&
         (controllers.card() != 0) ) {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "pure-control";
    }
    //Large Class
    {
        int accPlusMut = accessors.card() + mutators.card();
        int facPlusCon = controllers.card() + factory.card();
        int m = allMethods.card();
        if ( ((0.2 * m < accPlusMut) && (accPlusMut < 0.67 * m )) &&
             ((0.2 * m < facPlusCon) && (facPlusCon < 0.67 * m )) &&
             (factory.card() != 0) && (controllers.card() != 0) &&
             (accessors.card() != 0) && (mutators.card() != 0) ) {
            if (m > METHODS_PER_CLASS_THRESHOLD) { //Average methods/class + 1 std (system wide)
                if (classStereotype != "") classStereotype += " ";
                classStereotype += "large-class";
            }
        }
    }
    //Lazy Class
    if ( (getters.card() + setters.card() != 0) &&
         (degenerates.card() / double(allMethods.card()) > 0.33) &&
         ((allMethods.card() - (degenerates.card() + getters.card() + setters.card())) / double(allMethods.card())  <= 0.2) ) {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "lazy-class";
    }
    //Degenerate Class
    if (degenerates.card() / double(allMethods.card()) > 0.33)  {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "degenerate";
    }
    //Data Class
    if (allMethods.card() - getters.card() - setters.card() == 0)  {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "data-class";
    }
    //Small Class
    if (allMethods.card() < 3)  {
        if (classStereotype != "") classStereotype += " ";
        classStereotype += "small-class";
    }
    //Final check if no stereotype was assigned
    if (classStereotype == "") classStereotype = NO_STEREOTYPE;
}



//Compute method stereotypes
void classModel::ComputeMethodStereotype() {
    getter();
    setter();
    commandCollaborator();
    predicate();
    property();
    voidAccessor();
    command();
    factory();
    empty();
    collaborator();
    stateless();
}




// Stereotype get:
// method is const,
// contains at least 1 return statement that returns a data memeber
// return expression must be in the form 'return a;' or 'return *a;'

// Stereotype non-const get:
// method is not const
// contains at least 1 return statement that returns a data memeber
// return expression must be in the form 'return a;' or 'return *a;'
void classModel::getter() {
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
void classModel::setter() {
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
// Stereotype predicate collaborator:
// method is const
// returns boolean
// return expression is not a attribute
// does not use any attribute in the method
// has no pure calls
void classModel::predicate() {
    for (int i = 0; i < method.size(); ++i){
        std::string returnType = separateTypeName(method[i].getReturnType());
        if (returnType == "bool" && !method[i].returnsAttribute() && method[i].isConst()) {
            bool usesAttr = usesAttribute(i);
            std::vector<std::string> real_calls = method[i].findCalls("real");
            bool hasCallOnAttribute = callsAttributesMethod(real_calls, method[i].getLocalVariables(), method[i].getParameterNames());
            usesAttr = usesAttr || hasCallOnAttribute;
            std::vector<std::string> pure_calls = method[i].findCalls("pure");
            int pureCallsCount = countPureCalls(pure_calls);

            if (!usesAttr && pureCallsCount == 0){
                method[i].setStereotype("predicate collaborator");
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

// Stereotype property collaborator
// method is const
// return type is not void or boolean
// does not contain a attribute anywhere in the function
// does not contain any pure calls
// does not contain any calls on attributes
void classModel::property() {
    for (int i = 0; i < method.size(); ++i){
        std::string returnType = separateTypeName(method[i].getReturnType());
        if (returnType != "bool" && returnType != "void" && !method[i].returnsAttribute() && method[i].isConst()){
            bool usesAttr = usesAttribute(i);
            std::vector<std::string> real_calls = method[i].findCalls("real");

            bool hasCallOnAttribute = callsAttributesMethod(real_calls, method[i].getLocalVariables(), method[i].getParameterNames());
            usesAttr = usesAttr || hasCallOnAttribute;

            std::vector<std::string> pure_calls = method[i].findCalls("pure");
            int pureCallsCount = countPureCalls(pure_calls);


            if (!usesAttr && pureCallsCount == 0){
                method[i].setStereotype("property collaborator");
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
void classModel::voidAccessor() {
    for (int i = 0; i < method.size(); ++i){
        if(method[i].isVoidAccessor()){
            bool usesAttr = usesAttribute(i);
            std::vector<std::string> real_calls = method[i].findCalls("real");

            bool hasCallOnAttribute = callsAttributesMethod(real_calls, method[i].getLocalVariables(), method[i].getParameterNames());
            usesAttr = usesAttr || hasCallOnAttribute;

            std::vector<std::string> pure_calls = method[i].findCalls("pure");
            int pureCallsCount = countPureCalls(pure_calls);
            if (!usesAttr && pureCallsCount == 0){
                method[i].setStereotype("void-accessor collaborator");
            }
            else{
                method[i].setStereotype("void-accessor");
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
void classModel::command() {
    for (int i = 0; i < method.size(); ++i){
        std::vector<std::string> real_calls = method[i].findCalls("real");
        std::vector<std::string> pure_calls = method[i].findCalls("pure");

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


//stereotype command collaborator
//method is not const and
//no attributes are written and
//no pure calls, a() a::b() and
//no calls on attributes and
//at least 1 call or parameter or local variable is written
//Calls allowed:  f->g() where f is not a attribute, new f() (which isn't a real call)
//
void classModel::commandCollaborator() {
    for (int i = 0; i < method.size(); ++i){
        std::vector<std::string> all_calls = method[i].findCalls("");
        bool hasCallOnAttribute = callsAttributesMethod(all_calls, method[i].getLocalVariables(), method[i].getParameterNames());
        
        if (!method[i].isConst() && method[i].getAttributesModified() == 0) {
            bool local_var_written = false;
            for (int j = 0; j < method[i].getLocalVariables().size(); ++j){
                if(method[i].variableChanged(method[i].getLocalVariables()[j])){
                    local_var_written = true;
                    break;
                }
            }
            bool param_written = false;
            for (int j = 0; j < method[i].getParameterNames().size(); ++j){
                if(method[i].variableChanged(method[i].getParameterNames()[j])){
                    param_written = true;
                    break;
                }
            }
            std::vector<std::string> pure_calls = method[i].findCalls("pure");
            int pureCallsCount = countPureCalls(pure_calls);

            bool not_command =  pureCallsCount == 0 && !hasCallOnAttribute;
            if (not_command && (all_calls.size() > 0 || local_var_written || param_written)){
                method[i].setStereotype("command collaborator");
            }       
        }
    }
}


// Stereotype Collaborator
// at least one of the following is true:
// makes a call to an attribute that is an external object (only count pointers (* ->) [fix])
// has a parameter that is an external object.
// has a local variable that is an external object.
//
// what about returning an object attribtue !yes!
//
void classModel::collaborator() {
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
        bool param_obj = method[i].containsNonPrimitive(param, className);
        bool local_obj = method[i].containsNonPrimitive(local_var, className);
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

void classModel::factory() {
    for (int i = 0; i < method.size(); ++i){
        if (method[i].isFactory()){
            method[i].setStereotype("factory");
        }
    }
}


// stereotype empty
//
//
void classModel::empty() {
    for (int i = 0; i < method.size(); ++i){
        if (method[i].isEmptyMethod()){
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
// Both are degenerate
//
void classModel::stateless() {
    for (int i = 0; i < method.size(); ++i) {
        bool empty = method[i].isEmptyMethod();
        std::vector<std::string> calls = method[i].findCalls("");
        std::vector<std::string> real_calls = method[i].findCalls("real");
        bool usedAttr = usesAttribute(i);
        bool hasCallOnAttribute = callsAttributesMethod(real_calls, method[i].getLocalVariables(), method[i].getParameterNames());
        usedAttr = usedAttr || hasCallOnAttribute;
        if (!empty && calls.size() < 1 && !usedAttr) {
            if (method[i].getStereotype() == NO_STEREOTYPE)
                method[i].setStereotype("stateless");
            else
                method[i].setStereotype(method[i].getStereotype() + " stateless");
        }
        if (!empty && calls.size() == 1 && !usedAttr) {
            if (method[i].getStereotype() == NO_STEREOTYPE)
                method[i].setStereotype("wrapper");
            else
                method[i].setStereotype(method[i].getStereotype() + " wrapper");
        }
    }
}




//
//for each function, find all return expressions and determine if they contain an attribute
//
void classModel::returnsAttributes() {
    for(int i = 0; i < method.size(); ++i) {
        std::vector<std::string> return_expressions = method[i].findReturnExpressions(true);
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
bool classModel::isAttribute(const std::string& n) const {
    std::string name = trimWhitespace(n);
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
int classModel::findAssignOperatorAttribute(int i, bool check_for_loop) const {
    int changed = 0;

    for (int j = 0; j < ASSIGNMENT_OPERATOR.size(); ++j) {
        srcml_archive*           archive = nullptr;
        srcml_unit*              unit = nullptr;
        srcml_unit*              resultUnit = nullptr;
        srcml_transform_result*  result = nullptr;

        std::string xpath = "//src:function[string(src:name)='";
        xpath += method[i].getName() + "' and string(src:type)='";
        xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
        xpath += method[i].getParametersXML() +"']//src:operator[.='";
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

            char * unparsed = nullptr;
            size_t size = 0;
            srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
            std::string var_name(unparsed);
            free(unparsed);

            var_name = trimWhitespace(var_name); // removes whitespace
            if (isAttribute(var_name)) {
                 ++changed;
            } else if (inherits())
                if (isInheritedAttribute(method[i].getParameterNames(), method[i].getLocalVariables(), var_name)) {
                    //attribute.push_back(variable(var_name, "unknown")); // not needed??
                    ++changed;
                }
        }
        srcml_unit_free(unit);
        srcml_transform_free(result);
        srcml_clear_transforms(archive);
        srcml_archive_close(archive);
        srcml_archive_free(archive);
    }
    return changed;
}


//
//
int classModel::findIncrementedAttribute(int i, bool check_for_loop) const {
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
            xpath += method[i].getParametersXML() + "' and string(src:specifier)='";
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
                char *unparsed = nullptr;
                size_t size = 0;
                srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
                std::string var_name(unparsed);
                free(unparsed);

                var_name = trimWhitespace(var_name);// removes whitespace
                if (isAttribute(var_name)) {
                    ++changed;
                } else if (inherits())
                    if (isInheritedAttribute(method[i].getParameterNames(), method[i].getLocalVariables(), var_name)) {
                        //attribute.push_back(variable(var_name, "unknown"));  // not needed??
                        ++changed;
                    }
            }
            srcml_unit_free(unit);
            srcml_transform_free(result);
            srcml_clear_transforms(archive);
            srcml_archive_close(archive);
            srcml_archive_free(archive);
        }
    }
    return changed;
}






//
//
bool classModel::callsAttributesMethod(const std::vector<std::string>& real_calls,
                                       const std::vector<std::string>& local_var_names,
                                       const std::vector<std::string>& param_names){
    bool result = false;
    for (int i = 0; i < real_calls.size(); ++i) {
        size_t dot = real_calls[i].find(".");
        size_t arrow = real_calls[i].find("->");
        if (dot != std::string::npos) {
            std::string calling_object = real_calls[i].substr(0, dot);
            if (isAttribute(calling_object)) {
                result = true;
            } else if (inherits())
                if (isInheritedAttribute(param_names, local_var_names, calling_object)) {
                    result = true;
                    attribute.push_back(variable(calling_object, "unknown"));  //not needed?
                }
        }
        if (arrow != std::string::npos) {
            std::string calling_object = real_calls[i].substr(0, arrow);
            if (isAttribute(calling_object)) {
                result = true;
            } else if (inherits())
                if (isInheritedAttribute(param_names, local_var_names, calling_object)) {
                    result = true;
                    attribute.push_back(variable(calling_object, "unknown")); //not needed?
                }
        }
    }
    return result;
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
    xpath += method[i].getParametersXML() + "' and string(src:specifier)='";
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
        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string attr_name(unparsed);
        free(unparsed);
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
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return found;
}

//
//
// Returns if a method has a USE of an attribute (including inherited attributes)
//  Does not consider DEF of an attribute
//  Also adds any inherited attributes to attribute
//
bool classModel::usesAttribute(int i)  {
    bool found = false;
    srcml_archive*           archive = nullptr;
    srcml_unit*              unit = nullptr;
    srcml_unit*              resultUnit = nullptr;
    srcml_transform_result*  result = nullptr;

    std::string xpath = "//src:function[string(src:name)='";
    xpath += method[i].getName() + "' and string(src:type)='";
    xpath += method[i].getReturnType() + "' and string(src:parameter_list)='";
    xpath += method[i].getParametersXML() + "' and string(src:specifier)='";
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
        char *unparsed = nullptr;
        size_t size = 0;
        srcml_unit_unparse_memory(resultUnit, &unparsed, &size);
        std::string possible_attr(unparsed);
        free(unparsed);

        if (isAttribute(possible_attr)) {
            found = true;
        } else if (inherits())
            if (isInheritedAttribute(method[i].getParameterNames(), method[i].getLocalVariables(), possible_attr)) {
                attribute.push_back(variable(possible_attr, "unknown"));  // not needed??
                found = true;
            }
    }
    srcml_unit_free(unit);
    srcml_transform_free(result);
    srcml_clear_transforms(archive);
    srcml_archive_close(archive);
    srcml_archive_free(archive);

    return found;
}



//
//  Copy unit and add in stereotype attribute on <class> and <function>
//  Example: <function st:stereotype="get"> ... </function>
//           <class st:stereotype="boundary"> ... ></class>
//
srcml_unit* classModel::outputUnitWithStereotypes(srcml_archive* archive, srcml_unit* unit, bool oneUnit){
    int n = unitOneCount;
    int offset = 0;
    if (!oneUnit) {
        n = unitTwoCount;
        offset = unitOneCount;
    }
    if (oneUnit) { //Add stereotype attribute to <class>
        std::string xpath = "//src:class";
        srcml_append_transform_xpath_attribute(archive, xpath.c_str(), "st",
                                               "http://www.srcML.org/srcML/stereotype",
                                               "stereotype", classStereotype.c_str());
    }

    for (int i = 0; i < n; ++i) { //Add stereotype attribute to each method/function
        std::string stereotype = method[i+offset].getStereotype();
        //if (stereotype == NO_STEREOTYPE && unitOneCount + unitTwoCount == 1) return nullptr;  //Not sure why this is included?

        std::string xpath = "//src:function[string(src:name)='";
        xpath += method[i+offset].getName() + "' and string(src:type)='";
        xpath += method[i+offset].getReturnType() + "' and string(src:parameter_list)='";
        xpath += method[i+offset].getParametersXML() + "' and string(src:specifier)='";
        xpath += method[i+offset].getConst() + "']";
        srcml_append_transform_xpath_attribute(archive, xpath.c_str(), "st",
                                               "http://www.srcML.org/srcML/stereotype",
                                               "stereotype", stereotype.c_str());
    }

    srcml_transform_result* result;
    srcml_unit_apply_transforms(archive, unit, &result);
    unit = srcml_transform_get_unit(result, 0);
    srcml_clear_transforms(archive);
    return unit;
}



// Outputs a report file for a class (tab separated)
//  filepath || class name || method || stereotypes
//
void classModel::outputReport(std::ofstream& out, const std::string& input_file_path){
    if (out.is_open()) {
        for (int i = 0; i < method.size(); ++i){
            out << input_file_path << "\t" << className << "\t" << method[i].getHeader() << "\t" << method[i].getStereotype() << "\n";
        }
    }
}


// Output class information
std::ostream& operator<<(std::ostream& out, const classModel& c) {
    out << std::endl << "--------------------------------------" << std::endl;
    out << "Class: " << c.className << std::endl;
    out << "Class Stereotype: " << c.classStereotype << std::endl;
    out << "# Methods: " << c.method.size() << std::endl;
    for (int i = 0; i < c.method.size(); ++i) {
        out <<  c.method[i] << std::endl;
    }
    out << "# Attributes: " << c.attribute.size() << std::endl;
    for (int i = 0; i < c.attribute.size(); ++i) {
        out << c.attribute[i] << std::endl;
    }
    out << "--------------------------------------" << std::endl;
    return out;
}


//Output for testing
void classModel::printReturnTypes(){
    std::cerr << "RETURN TYPES: \n";
    for (int i = 0; i < method.size(); ++i){
        std::cerr << method[i].getReturnType() << "\n";
    }
    std::cerr << std::endl;
}

void classModel::printStereotypes(){
    std::cerr << "STEREOTYPES: \n";
    for (int i = 0; i < method.size(); ++i){
        std::cerr << method[i].getStereotype() << "\n";
    }
    std::cerr << std::endl;
}

void classModel::printMethodNames(){
    std::cerr << "METHOD Names: \n";
    for (int i = 0; i < method.size(); ++i){
        std::cerr << i << "  " << method[i].getName() << std::endl;
    }
}

void classModel::printMethodHeaders(){
    std::cerr << "METHOD HEADERS: \n";
    for (int i = 0; i < method.size(); ++i){
        std::cerr << i << "  " << method[i].getHeader() << std::endl;
    }
}

void classModel::printAttributes(){
    std::cerr << "ATTRIBUTES: \n";
    std::cerr << attribute.size() << " names\n";
    for (int i = 0; i < attribute.size(); ++i){
        std::cerr << "TYPE: " << attribute[i].getType() << " NAME: " << attribute[i].getName() << std::endl;
    }

}

