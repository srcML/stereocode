// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file query_registry.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "archive/query_registry.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

namespace {
    using textList = std::vector<std::string>;

    const char* const notStatic = "not(src:type/src:specifier='static')";

    // An element test of a type kind and the option that selects the kind (null when the kind is always analyzed)
    //
    struct typeKind {
        const char*             test;
        bool options::*         option;
    };

    // A form in which srcML marks an event without accessors, which is a field of a delegate type
    //
    struct eventForm {
        std::string             element;                // Location step of the element that holds the type and the name of the event
        textList                conditions;             // Conditions that the element is such an event
        std::string             staticTest;             // Test that the event is static
    };

    // The srcML elements, conditions, and operators in which the queries of the languages differ
    //
    struct languageMarkup {
        std::vector<typeKind>   typeKinds;              // Kinds of types to analyze
        std::string             namedType{"src:name"};  // Condition that a type has a name, since anonymous types stay outside the analysis
        textList                typeConditions;         // Further conditions on the types to analyze
        textList                typeElements;           // Elements of types
        textList                parentOwners;           // Elements of types whose parents are read
        std::string             superStep;              // Step from the super list to the parents
        textList                fieldOwners;            // Elements of types whose declarations are fields
        textList                nonFieldSpecifiers;     // Specifiers of declarations that are not fields
        textList                nonFieldOwners;         // Elements of types whose declarations are static, and thus no fields
        std::string             inlineTypeFields;       // Declarators after the definition of a member type, which are fields, if any
        std::string             componentFields;        // Components of a record, which are fields, if any
        std::vector<eventForm>  eventForms;             // Forms of the events without accessors, if any
        textList                methodElements;         // Elements of methods
        textList                methodConditions;       // Conditions on methods
        textList                constructorElements;    // Elements of constructors and destructors
        textList                declarationOwners;      // Elements of types whose method declarations are read
        std::string             freeFunction;           // XPath of free functions
        std::string             globalVariables;        // XPath of the names of the globals, which are the variables at file or namespace scope and the static fields
        std::string             staticImports;          // XPath of the names of static imports, which reach the static fields of a type without its name, if any
        std::string             globalStaticImports;    // XPath of the names of the static imports that hold for every unit, if any
        std::string             typeImports;            // XPath of the names of the imports that bring types into a unit by their names, if any
        std::string             globalTypeImports;      // XPath of the names of such imports that hold for every unit, if any
        std::string             valueTypeNames;         // XPath of the names of the types whose values a parameter copies, if any
        std::string             usingDirectives;        // XPath of the using directives at namespace scope, as using namespace std;, if any
        std::string             typeAliasNames;         // XPath of the names of the aliases of types outside the bodies of code, if any
        std::string             memberAliasNames;       // XPath of the names of the aliases of types that are members of a type, if any
        std::string             annotation;             // Element of the attributes or annotations of methods, if any
        textList                misplacedMembers;       // Location steps of the members that srcML nests in a function after a statement that it misreads
        std::string             misreadDeclarator;      // Test of a declarator that srcML writes for code that declares no variable, if any
        textList                misreadMethodNames;     // Location steps of the names of the methods that srcML marks as other elements, which get no stereotype, if any
        bool                    localFunctions{false};  // A function may hold functions, whose code is code of the function around them
        bool                    functionPointers{false}; // A declaration may name a pointer to a function, as bool (*filter)(int) does
        bool                    operatorFunctions{false}; // An operator may call a function that a type or the code defines, as << of out << value does
        textList                memberOperators;        // Operators of member access
        std::string             methodReference;        // Operator of a method reference, as :: of this::process, if any
        textList                assignmentOperators;    // Operators that modify the name before them
        textList                splitAssignments;       // Operators that srcML marks apart from the = after them, such as ?? of ??=
        std::string             assignmentCondition;    // Condition on the expressions of assignments, if any
        std::string             factoryCall;            // Condition on a call of a function that creates an object as new does, if any
        bool                    properties{false};      // The language has properties, which also count as fields
        bool                    partialTypes{false};    // The language has types defined in several parts
        bool                    outerMethods{false};    // Methods can be defined outside their types
        bool                    ownedFunctions{false};  // Every function belongs to a type, so a function outside the types shows damaged markup
        std::string             topLevelFunction;       // Test of a function among the top-level statements of a program, which belongs to no type, if any
        bool                    anonymousClasses{false}; // new creates an object of a class without a name, whose body follows the arguments of the constructor
        bool                    targetTypedNew{false};  // new may create an object of the type of its target, as new() of Report report = new();
        bool                    memberInitializers{false}; // A block in an expression may name the members of another object, as Left of new Margin { Left = 1 }
        std::string             nameOperator;           // Operator that takes the name of a variable without its value, as nameof, which srcML marks as a call, if any
        std::string             typeTest;               // Operator that tests the type of an object and may declare a variable of that type, as instanceof, if any
        bool                    casePatterns{false};    // A case of a switch may declare a variable of the type that it names, as case Gauge found:
        bool                    tupleDeclarations{false}; // An expression may declare or write several variables at once, as (int low, int high) = pair; and var (low, high) = pair;
        bool                    usingDeclarations{false}; // A using declaration of a body declares its variable without a declaration element, as using var reader = Open();
        bool                    queryClauses{false};    // The clauses of a query expression declare range variables, as item of from item in items
        bool                    cClasses{false};        // The queries also read C units, whose markup turns C++ classes into declarations
        bool                    constMethods{false};    // The language has const methods
        bool                    nullConditional{false}; // The language calls a member of a name that may be null with ?., as in o?.Dispose()
        bool                    interpolation{false};   // The language has interpolated strings, as $"{count} items"
        bool                    referenceArguments{false}; // A call may write a variable passed with ref or out, as in Increment(ref count)
        bool                    nullForgiving{false};   // The operator ! after a value that is not null sets a name apart from its indexes, as in buffer![0]
    };

    // Joins the non-empty parts with a separator
    //
    std::string join(const textList& parts, const char* separator) {
        std::string joined;
        for (const std::string& part : parts) {
            if (part.empty()) continue;
            if (!joined.empty()) joined += separator;
            joined += part;
        }
        return joined;
    }

    // Gets the tests of elements on an axis, such as self::src:class
    //
    textList elementTests(const textList& elements, const char* axis) {
        textList tests;
        for (const std::string& element : elements) tests.push_back(axis + ("src:" + element));
        return tests;
    }

    // Gets the condition that a node is one of the elements, such as "self::src:class or self::src:struct"
    //
    std::string isOneOf(const textList& elements) {
        return join(elementTests(elements, "self::"), " or ");
    }

    // Gets the test that a node is one of the operators, such as "self::src:operator[contains(' . -> ', concat(' ', ., ' '))]", which reads
    // the text of the node once, where a comparison per operator reads the text for each. The lists hold symbols, which no operator
    // of two words, as not in, matches
    //
    std::string isOperator(const textList& operators) {
        if (operators.empty()) return "";
        if (operators.size() == 1) return "self::src:operator='" + operators.front() + "'";
        return "self::src:operator[contains(' " + join(operators, " ") + " ', concat(' ', ., ' '))]";
    }

    // Gets a location step with its non-empty conditions as one predicate. The queries write /descendant:: in place of // before a step with
    // a predicate, which libxml2 reads as a walk over every node and then over the children of each node, at twice the time
    //
    std::string step(const std::string& test, const textList& conditions) {
        const std::string predicate = join(conditions, " and ");
        return predicate.empty() ? test : test + "[" + predicate + "]";
    }

    // Gets the test that the nth element before a node meets a condition. libxml2 ends its walk over the siblings at that element for
    // a step with one positional predicate, while a second predicate on the step makes it walk every sibling of each node
    //
    std::string siblingBefore(const std::string& condition, int n = 1) {
        return "preceding-sibling::*[" + std::to_string(n) + "]/self::*[" + condition + "]";
    }

    // Gets the test that the nth element after a node meets a condition
    //
    std::string siblingAfter(const std::string& condition, int n = 1) {
        return "following-sibling::*[" + std::to_string(n) + "]/self::*[" + condition + "]";
    }

    // Gets the condition that exactly one ancestor is one of the elements
    //
    std::string singleOwner(const textList& elements) {
        return "count(" + join(elementTests(elements, "ancestor::"), " | ") + ") = 1";
    }

    // Gets the condition that no ancestor is one of the elements
    //
    std::string notNested(const textList& elements) {
        return "not(" + join(elementTests(elements, "ancestor::"), " or ") + ")";
    }

    // Gets the operators that modify the name before them, with the compound assignments of a language
    //
    textList assignments(const textList& languageAssignments) {
        textList operators{"=", "+=", "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|="};
        operators.insert(operators.end(), languageAssignments.begin(), languageAssignments.end());
        operators.insert(operators.end(), {"++", "--"});
        return operators;
    }

    // C++: an anonymous union is no type, and its fields belong to the type around it
    //
    languageMarkup cppMarkup() {
        const textList types{"class", "struct", "union"};
        // A typedef names a type without a name of its own by its first name that is no pointer, as Point of typedef struct { } *PointPtr, Point;
        const std::string typedefName = "parent::src:type/parent::src:typedef/src:name[not(*) and "
                                        "not(preceding-sibling::node()[1]/self::text()[contains(., '*') or contains(., '&')])]";

        languageMarkup markup;
        markup.typeKinds           = {{"self::src:class", nullptr}, {"self::src:struct", &options::structs}, {"self::src:union", &options::unions}};
        markup.namedType           = "src:name or " + typedefName;
        // The types in a body of code are local types, and the types in an anonymous type have no qualified name
        const std::string outsideBodies = notNested({"function", "constructor", "destructor", "lambda"});
        const std::string inNamedTypes  = "not(ancestor::*[(self::src:class or self::src:struct or self::src:union) and not(" + markup.namedType + ")])";
        markup.typeConditions      = {outsideBodies, inNamedTypes};
        // An alias declaration or a typedef takes the conditions of the types, while a template alias, as template <typename T> using P = T*;, names no single type
        markup.typeAliasNames      = "/descendant::" + step("src:using", {"src:init/src:type", "not(src:template)", outsideBodies, inNamedTypes}) + "/src:name | " +
                                     "/descendant::" + step("src:typedef", {"src:type", outsideBodies, inNamedTypes}) + "/src:name[not(*)]";
        markup.typeElements        = types;
        // A member alias may stand for a parent of its type, as Base of using Base = Shape;
        markup.memberAliasNames    = "/descendant::" + step("src:using", {"src:init/src:type", "not(src:template)", outsideBodies, singleOwner(types)}) + "/src:name | " +
                                     "/descendant::" + step("src:typedef", {"src:type", outsideBodies, singleOwner(types)}) + "/src:name[not(*)]";
        markup.parentOwners        = {"class", "struct"};
        // The queries of a type read a copy of the type as the only child of the unit, where a union of a typedef has no typedef around it
        markup.fieldOwners         = {"class", "struct", "union[src:name or parent::src:unit or " + typedefName + "]"};
        markup.nonFieldSpecifiers  = {"static"};
        markup.inlineTypeFields    = "/descendant::*[(self::src:class or self::src:struct or self::src:union) and not(ancestor::src:block_content) and " +
                                     singleOwner(markup.fieldOwners) + "]/src:decl[src:name]";
        markup.methodElements      = {"function", "constructor", "destructor"};
        markup.methodConditions    = {notStatic, singleOwner(types), "not(parent::src:friend)"};
        markup.constructorElements = {"constructor", "destructor"};
        markup.declarationOwners   = types;
        // A friend function is a free function, and constructors and destructors outside a type are definitions of the members of a type
        markup.freeFunction        = "/descendant::*[(self::src:function and (" + notNested(types) + " or src:type/src:specifier='static' or parent::src:friend)) or " +
                                     "((self::src:constructor or self::src:destructor) and " + notNested(types) + ")]";
        // The block of an extern "C" declaration, unlike a body, holds declarations at namespace scope
        const std::string inBody           = "ancestor::src:block_content[not(parent::src:block/parent::src:extern)]";
        const std::string atNamespaceScope = "not(ancestor::src:class or ancestor::src:struct or ancestor::src:union or " + inBody + ")";
        // A using directive in a body holds only for the body, where no method is defined
        markup.usingDirectives     = "/descendant::src:using[src:namespace and " + atNamespaceScope + "]";
        // The members of an anonymous union at namespace scope without a declarator are globals, as are the declarators after the body of a type, as x of struct { } x;
        // A declarator with a block is the C markup of a C++ class, as final of class Date final { }
        markup.globalVariables     = "/descendant::src:decl_stmt[" + atNamespaceScope + "]/src:decl[not(src:block)]/src:name | "
                                     "/descendant::*[(self::src:class or self::src:struct or self::src:union or self::src:enum) and " + atNamespaceScope + "]/src:decl/src:name | "
                                     "/descendant::src:decl_stmt[src:decl/src:type/src:specifier='static' and not(" + inBody + ") and "
                                     "(ancestor::src:class or ancestor::src:struct or ancestor::src:union)]/src:decl[not(src:block)]/src:name | "
                                     "/descendant::src:decl_stmt[ancestor::src:union and ancestor::*[self::src:class or self::src:struct or self::src:union][1]"
                                     "[self::src:union and not(" + markup.namedType + ") and not(src:decl) and " + atNamespaceScope + "]]/src:decl[not(src:block)]/src:name";
        markup.outerMethods        = true;
        markup.cClasses            = true;
        markup.functionPointers    = true;
        markup.operatorFunctions   = true;
        markup.memberOperators     = {".", "->"};
        markup.assignmentOperators = assignments({});
        // std::make_unique and std::make_shared, with or without the namespace std
        markup.factoryCall         = "src:name/src:name[last()][.='make_unique' or .='make_shared'] and (count(src:name/src:name) = 1 or src:name/src:name[1]='std')";
        markup.constMethods        = true;
        return markup;
    }

    // C#: static types hold only static members, local functions and property accessors are no methods, the code of a local function is code
    // of the function around it,
    // const fields are static, and the assignments in the block of an object initializer, as in new A { P = 1 }, set the members of the new object
    //
    languageMarkup csharpMarkup() {
        const textList types{"class", "struct", "interface"};

        languageMarkup markup;
        // Interfaces are parents of classes whatever -i says, so they are always analyzed and labeled only with -i
        markup.typeKinds           = {{"self::src:class", nullptr}, {"self::src:struct", &options::structs}, {"self::src:interface", nullptr}};
        // C# has no local types, while srcML 1.2 marks a positional record as a function, whose nested types are analyzed
        markup.typeConditions      = {"not(src:specifier='static')"};
        markup.typeElements        = types;
        markup.parentOwners        = types;
        markup.fieldOwners         = types;
        markup.nonFieldSpecifiers  = {"static", "const"};
        // srcML 1.2 marks expression-bodied members and accessors as expression statements with a lambda and a positional record
        // as a function, and an event with accessors hides the member after it, so these members get no stereotype
        markup.methodElements      = {"function", "constructor", "destructor"};
        markup.methodConditions    = {singleOwner(types), notStatic, "not(ancestor::src:function)", "not(ancestor::src:property)"};
        markup.constructorElements = {"constructor", "destructor"};
        markup.declarationOwners   = types;
        // A function among top-level statements comes before the namespaces and the types of its unit and has no access modifier
        markup.topLevelFunction    = "parent::src:unit and not(preceding-sibling::*[self::src:namespace or self::src:enum or " + isOneOf(types) + "]) and "
                                     "not(src:type/src:specifier[. != 'static' and . != 'async' and . != 'unsafe' and . != 'extern'])";
        markup.freeFunction        = "/descendant::src:function[src:type/src:specifier='static' or ancestor::src:property[1]/src:type/src:specifier='static' or (" +
                                     markup.topLevelFunction + ")]";
        // A declarator with an argument list or a block is a part of an initializer or a method with a default literal that srcML misreads
        markup.misreadDeclarator   = "src:argument_list or src:block";
        markup.globalVariables     = "/descendant::src:decl_stmt[(src:decl/src:type/src:specifier='static' or src:decl/src:type/src:specifier='const') and "
                                     "not(ancestor::src:block_content)]/src:decl[not(" + markup.misreadDeclarator + ")]/src:name | "
                                     "/descendant::src:property[src:type/src:specifier='static' and not(ancestor::src:block_content)]/src:name";
        // srcML 1.2 marks an event without accessors as an expression statement with the word event, and as an event element with its
        // modifiers as text after an attribute or in an interface, while an event with accessors holds a block
        markup.eventForms          = {{"src:expr_stmt/src:expr", {"text()[normalize-space(.)='event']", "not(src:block)", "not(ancestor::src:block_content)"},
                                       "src:specifier='static'"},
                                      {"src:event", {"not(src:block)"}, "text()[contains(concat(' ', normalize-space(.), ' '), ' static ')]"}};
        markup.staticImports       = "/descendant::src:using[starts-with(normalize-space(.), 'using static ')]/src:name";
        // srcML marks the word global of global using static as a macro before the using directive
        const std::string global   = siblingBefore("self::src:macro[src:name='global']");
        markup.globalStaticImports = "/descendant::src:using[starts-with(normalize-space(.), 'using static ') and " + global + "]/src:name";
        // A using directive of a namespace, unlike a using alias directive and a using declaration of a body
        markup.typeImports         = "/descendant::src:using[not(src:init) and not(ancestor::src:block_content) and not(starts-with(normalize-space(.), 'using static '))]/src:name";
        markup.globalTypeImports   = "/descendant::src:using[not(src:init) and not(ancestor::src:block_content) and not(starts-with(normalize-space(.), 'using static ')) and " +
                                     global + "]/src:name";
        markup.valueTypeNames      = "//src:struct/src:name | //src:enum/src:name";
        // A using alias directive of a unit or a namespace, unlike a using declaration of a body, as using var reader = Open();
        markup.typeAliasNames      = "/descendant::src:using[src:init and not(ancestor::src:block_content)]/src:name";
        markup.annotation          = "attribute";
        // srcML 1.2 marks an expression-bodied method as a declarator with an argument list, or as an expression with a lambda after its name
        markup.misreadMethodNames  = {"src:decl_stmt/src:decl[src:argument_list]/src:name",
                                      "src:expr_stmt/src:expr/src:name[" + siblingAfter("self::src:lambda[src:parameter_list[not(@type)]]") + "]"};
        markup.localFunctions      = true;
        // srcML 1.2 nests the members after a switch expression in the function of the expression, where a local function has no access modifier
        markup.misplacedMembers    = elementTests(types, "");
        markup.misplacedMembers.push_back("src:function[src:type/src:specifier[. = 'public' or . = 'private' or . = 'protected' or . = 'internal']]");
        markup.memberOperators     = {".", "->"};
        markup.assignmentOperators = assignments({">>>="});
        markup.splitAssignments    = {"??"};
        markup.assignmentCondition = "not(parent::src:block[parent::src:expr])";
        markup.properties          = true;
        markup.partialTypes        = true;
        markup.ownedFunctions      = true;
        markup.nullConditional     = true;
        markup.typeTest            = "is";
        markup.casePatterns        = true;
        markup.tupleDeclarations   = true;
        markup.usingDeclarations   = true;
        markup.queryClauses        = true;
        markup.interpolation       = true;
        markup.referenceArguments  = true;
        markup.nullForgiving       = true;
        markup.targetTypedNew      = true;
        markup.memberInitializers  = true;
        markup.nameOperator        = "nameof";
        return markup;
    }

    // Java: a class with a super child is an anonymous class, which is skipped
    //
    languageMarkup javaMarkup() {
        const textList types{"class", "interface", "enum"};

        languageMarkup markup;
        // Interfaces are parents of classes, and enums enclose the objects of inner classes and hold methods that calls on fields reach,
        // whatever -i and -e say, so both are always analyzed and labeled only with -i and -e
        markup.typeKinds           = {{"(self::src:class and not(child::src:super[1]))", nullptr}, {"self::src:interface", nullptr}, {"self::src:enum", nullptr}};
        // Local classes lie in bodies of code and initializer blocks, while a declaration holds the body of an enum constant or an anonymous class
        markup.typeConditions      = {notNested({"function", "constructor", "lambda", "block_content", "decl"})};
        markup.typeElements        = types;
        markup.parentOwners        = types;
        markup.superStep           = "*[self::src:extends or self::src:implements]/";
        markup.fieldOwners         = types;
        markup.nonFieldSpecifiers  = {"static"};
        markup.nonFieldOwners      = {"interface"};
        markup.componentFields     = "/src:unit/src:class/src:parameter_list/src:parameter/src:decl[src:name[" + siblingBefore("self::src:type") + "]]";
        markup.methodElements      = {"function", "constructor"};
        markup.methodConditions    = {notStatic, singleOwner(types)};
        markup.constructorElements = {"constructor"};
        markup.declarationOwners   = types;
        markup.freeFunction        = "/descendant::src:function[src:type/src:specifier='static']";
        // The fields of an interface are static, while a declarator with an argument list or a block is a part of an initializer that srcML misreads
        markup.globalVariables     = "/descendant::src:decl_stmt[src:decl/src:type/src:specifier='static' and not(ancestor::src:block_content)]"
                                     "/src:decl[not(src:argument_list or src:block)]/src:name | "
                                     "/descendant::src:decl_stmt[not(ancestor::src:block_content) and ancestor::*[self::src:class or self::src:interface or "
                                     "self::src:enum or self::src:annotation_defn][1][self::src:interface or self::src:annotation_defn]]"
                                     "/src:decl[not(src:argument_list or src:block)]/src:name";
        markup.staticImports       = "/src:unit/src:import[starts-with(normalize-space(.), 'import static ')]/src:name";
        markup.typeImports         = "/src:unit/src:import[not(starts-with(normalize-space(.), 'import static '))]/src:name";
        markup.annotation          = "annotation";
        markup.memberOperators     = {"."};
        markup.methodReference     = "::";
        markup.assignmentOperators = assignments({">>>="});
        markup.ownedFunctions      = true;
        markup.anonymousClasses    = true;
        markup.typeTest            = "instanceof";
        return markup;
    }

    // Gets the condition that a function has one of the decorators, alone or after a qualifier, as @staticmethod and @builtins.staticmethod
    //
    std::string decoratedWith(const textList& decorators) {
        textList tests;
        for (const std::string& decorator : decorators) tests.push_back(". = '" + decorator + "' or src:name[last()] = '" + decorator + "'");
        return "src:attribute/src:expr/src:name[" + join(tests, " or ") + "]";
    }

    // Builds the XPaths of Python, whose markup has no declarations: a class body holds statements, a variable starts at its first assignment,
    // and a decorator marks a static method, a class method, or a property
    // The queries of a class or a function read a copy of its element as the only child of the unit, as those of buildQueries() do
    //
    std::array<std::string, queryCount> pythonQueries() {
        std::array<std::string, queryCount> xpaths;
        auto set = [&xpaths](query name, std::string xpath) { xpaths[static_cast<std::size_t>(name)] = std::move(xpath); };

        // A class in a function is a local class, and a method reaches its class only through an object or the name of the class,
        // so a static method, a class method, and the class methods that the language calls without a decorator are free functions
        const std::string outsideFunctions = "not(ancestor::src:function)";
        const std::string inClassBody      = "count(ancestor::src:class) = 1 and " + outsideFunctions;
        const std::string classLevel       = decoratedWith({"staticmethod", "classmethod"}) + " or src:name = '__init_subclass__' or src:name = '__class_getitem__'";
        set(query::type,         "/descendant::src:class[src:name and " + outsideFunctions + "]");
        set(query::freeFunction, "/descendant::src:function[src:name and " + outsideFunctions + " and (not(ancestor::src:class) or " + classLevel + ")]");
        set(query::method,       "/descendant::src:function[src:name and " + inClassBody + " and not(" + classLevel + ")]");
        set(query::staticMethodName, "/descendant::src:function[src:name and " + inClassBody + " and (" + classLevel + ")]/src:name");
        set(query::nestedTypeName,   "/descendant::src:class[src:name and " + inClassBody + "]/src:name");

        // A name is bound where = or an annotation follows it, alone or in a tuple or a list of targets
        const std::string bound       = siblingAfter("self::src:operator = '=' or self::src:annotation");
        const std::string targetList  = "*[self::src:tuple or self::src:array][" + siblingAfter("self::src:operator = '='") + "]//src:expr/src:name[not(src:name)]";
        // The globals are the names bound at the top level of a file or in the body of a class, and the names of global statements
        const std::string statement   = "/descendant::src:expr_stmt[" + outsideFunctions + "]/src:expr/";
        // A class method declares a variable of its class where it writes a name through its first parameter, as cls.count = 0 does
        const std::string classMethods  = "/descendant::src:function[parent::src:block_content/parent::src:block/parent::src:class and (" +
                                          decoratedWith({"classmethod"}) + " or src:name = '__init_subclass__' or src:name = '__class_getitem__')]";
        const std::string throughClass  = "src:name[count(src:name) = 2 and src:name[1] = ancestor::src:function[1]/src:parameter_list[not(@type = 'generic')]"
                                          "/src:parameter[1]/src:name][" + bound + "]/src:name[2]";
        set(query::globalVariable, statement + "src:name[not(src:name)][" + bound + "] | " + statement + targetList + " | //src:global/src:name | " +
                                   classMethods + "//src:expr/" + throughClass);

        // An import binds its name, the first name of a module with a path, or its alias, as os of import os.path and ET of import xml.etree as ET
        set(query::importedName, "//src:import/src:name[not(src:name)] | //src:import/src:name/src:name[1] | //src:import/src:alias/src:expr/src:name");

        const std::string type = "/src:unit/src:class";
        set(query::structureType, type + "/text()");
        // A keyword in the list of parents, as metaclass=Meta, and a starred entry are no parents
        set(query::parentName,    type + "/src:super_list/src:super/src:expr[not(src:operator)]/src:name");

        // The body of a class declares a data member with an annotation, unless the annotation is ClassVar, and by a string of __slots__,
        // while a name that the body binds without an annotation is a variable of the class
        const std::string classVar   = "src:annotation/src:expr/src:name[. = 'ClassVar' or src:name[1] = 'ClassVar' or src:name[last()] = 'ClassVar' or "
                                       "src:name[1]/src:name[last()] = 'ClassVar']";
        const std::string bodyBound  = "/descendant::src:expr_stmt[" + inClassBody + "]/src:expr";
        set(query::fieldDeclaration, bodyBound + "[src:annotation and not(" + classVar + ")]/src:name[1][not(src:name)] | " +
                                     bodyBound + "[src:name[1] = '__slots__']/descendant::src:literal[@type = 'string']");
        set(query::staticFieldName,  bodyBound + "[not(src:annotation) or " + classVar + "]/src:name[1][not(src:name) and . != '__slots__'][" + bound + "]");

        // Signatures, where the type parameters of def f[T](x) form a list of their own
        const std::string function   = "/src:unit/src:function";
        const std::string parameters = "/src:parameter_list[not(@type = 'generic')]";
        // The first parameter of a method names the object, and that of a class method names the class, where __new__ takes the class
        const std::string first      = parameters + "/src:parameter[1][src:name and not(@type)]/src:name";
        set(query::objectParameter,         function + "[not(" + classLevel + ") and src:name != '__new__']" + first);
        set(query::classParameter,          function + "[" + decoratedWith({"classmethod"}) + " or src:name = '__init_subclass__' or src:name = '__class_getitem__']" + first);
        set(query::methodName,              function + "/src:name");
        set(query::methodParameterList,     function + parameters);
        set(query::constructorOrDestructor, function + "[src:name = '__init__' or src:name = '__new__' or src:name = '__del__']");
        set(query::methodSpecifiers,        function + "/src:specifier");
        set(query::methodAttributes,        function + "/src:attribute");
        set(query::returnType,              function + "/src:annotation");
        // The markers / and * of a parameter list are parameters without a name
        set(query::parameterDeclaration,    function + parameters + "/src:parameter[src:name]");

        // Bodies, with the code of nested functions, as of lambdas, and without the code of local classes
        const std::string body  = function + "/src:block/descendant::";
        const std::string own   = "not(ancestor::src:class)";
        // A return or a yield of a nested function leaves that function only
        const std::string ownReturn = own + " and count(ancestor::src:function) = 1";
        const std::string expr  = body + "src:expr[" + own + "]";
        // The targets of for loops and comprehensions, and of with and except clauses
        const std::string loopTargets  = body + "src:control[" + own + "]/src:expr/descendant-or-self::src:expr/src:name";
        const std::string aliasTargets = body + "src:alias[" + own + "]/src:expr/src:name";
        const textList    assignmentOperators{"=", "+=", "-=", "*=", "/=", "//=", "%=", "**=", ">>=", "<<=", "&=", "^=", "|=", "@=", ":="};

        // A local is a name that the body binds: a target of an assignment, of the operator :=, of a loop, or of a clause, a parameter of a lambda
        // or of a nested function, or the name of a nested function or a local class
        set(query::localDeclaration, expr + "/src:name[not(src:name)][" + siblingAfter(isOperator(assignmentOperators) + " or self::src:annotation") + "] | " +
                                     expr + "/" + targetList + " | " + loopTargets + "[not(src:name)] | " + aliasTargets + "[not(src:name)] | " +
                                     body + "src:lambda[" + own + "]/src:parameter/src:name | " +
                                     body + "src:function[" + own + "]" + parameters + "/src:parameter/src:name | " +
                                     body + "*[(self::src:function or self::src:class) and " + own + "]/src:name");
        // A nested function and a local that a lambda alone binds hold code of the function itself
        set(query::localFunction,    body + "src:function[" + own + "]/src:name | " +
                                     expr + "/src:name[not(src:name)][" + siblingAfter("self::src:operator = '='") + "/" + siblingAfter("self::src:lambda") + "]");
        set(query::globalName,       body + "src:global[" + own + "]/src:name");
        // A yield passes its value to the caller, as a return does
        set(query::returnExpression, body + "*[(self::src:return or self::src:yield) and " + ownReturn + "]/src:expr");

        // super() only reaches the methods of a parent, so it is no call of its own, and type(x)(...) calls the class of x, as x.__class__(...) does
        const std::string afterAccess = siblingBefore("self::src:operator = '.'");
        set(query::functionCall, body + "src:call[" + own + " and src:name[not(src:operator) and . != 'super'] and not(" + afterAccess + ")]");
        // A call after the access operator follows the result of a call, a value, or a variable with an index, which ends the name of a call
        set(query::methodCall,   body + "src:call[" + own + " and (src:name/src:operator and not(" + afterAccess + ") or src:call/src:name = 'type')]");
        set(query::callOnResult, body + "src:call[" + own + " and src:name and " + afterAccess + "]");
        // A body is empty when it holds only comments, a docstring or another string, pass, and ...
        const std::string idle = "self::src:comment or self::src:pass or self::src:expr_stmt[src:expr[not(*[not(self::src:literal[@type = 'string' or @type = 'ellipsis'])])]]";
        set(query::nonCommentStatements, function + "/src:block/src:block_content[*[not(" + idle + ")]]");
        // The names of an annotation name types
        set(query::expressionName,       expr + "[not(ancestor::src:annotation)]/src:name | " + body + "src:delete[" + own + "]/src:name");
        set(query::expressionAssignment, expr + "/src:operator[" + isOperator(assignmentOperators) + "]/preceding-sibling::*[1]/self::src:name | " +
                                         expr + "/src:operator[. = '=']/preceding-sibling::*[1]/self::src:annotation/preceding-sibling::*[1]/self::src:name | " +
                                         expr + "/" + targetList.substr(0, targetList.rfind("[not(src:name)]")) + " | " +
                                         body + "src:delete[" + own + "]/src:name | " + loopTargets + " | " + aliasTargets);
        // srcML marks an f-string as one string literal, whose prefix before the first quote holds f or F
        set(query::interpolatedString,   body + "src:literal[" + own + " and @type = 'string']"
                                         "[contains(translate(substring-before(translate(., \"'\", '\"'), '\"'), 'F', 'f'), 'f')]");
        return xpaths;
    }

    // Builds the XPaths of JavaScript and TypeScript, whose classes declare fields while their methods add data members by writes through this,
    // and whose TypeScript annotations follow the names in the namespace ts
    // The queries of a class or a function read a copy of its element as the only child of the unit, as those of buildQueries() do
    //
    std::array<std::string, queryCount> javascriptQueries() {
        std::array<std::string, queryCount> xpaths;
        auto set = [&xpaths](query name, std::string xpath) { xpaths[static_cast<std::size_t>(name)] = std::move(xpath); };

        // A class in a body of code is a local class, and a class in an expression, as of const Shape = class { }, is a value
        const std::string declared     = "src:name and not(ancestor::src:function or ancestor::src:constructor or ancestor::src:lambda or parent::src:expr)";
        const std::string classes      = "/descendant::src:class[" + declared + "]";
        const std::string staticMember = "src:specifier = 'static' or ts:specifier = 'static'";
        const std::string staticField  = "@type = 'static' or ts:specifier = 'static'";
        const std::string namespaces   = "/descendant::ts:namespace/src:block/src:block_content/";
        set(query::type,         classes + " | /descendant::ts:interface[" + declared + "]");
        // The functions at the top level of a file or of a namespace and the static methods of the classes are free functions, while srcML marks
        // a declaration without a body, as an overload signature or a declare function of TypeScript, as a function without a block
        set(query::freeFunction, "/src:unit/src:function[src:name and src:block] | " + namespaces + "src:function[src:name and src:block] | " +
                                 classes + "/src:block/src:function[src:name and src:block and (" + staticMember + ")]");
        // The globals are the variables of a file or a namespace, also those of a destructuring pattern, and the static fields
        const std::string topLevel = "/src:unit/src:decl_stmt/src:decl";
        set(query::globalVariable, topLevel + "/src:name | " + topLevel + "/src:decl/src:name | " + namespaces + "src:decl_stmt/src:decl/src:name | " +
                                   classes + "/src:block/src:decl_stmt/src:decl[" + staticField + "]/src:name");
        // An import binds its names or their aliases, and a variable that require() initializes holds a module, as fs of const fs = require('fs')
        set(query::importedName, "/src:unit/src:import/src:name[. != '*'] | /src:unit/src:import/src:name_list/src:name | "
                                 "/src:unit/src:import/descendant::src:alias/src:expr/src:name | " +
                                 topLevel + "[src:init/src:expr[count(*) = 1]/src:call/src:name = 'require']/src:name");

        const std::string type = "/src:unit/*[self::src:class or self::ts:interface]";
        set(query::structureType, type + "/text()");
        set(query::parentName,    type + "/src:super_list/*[self::src:extends or self::ts:implements]/src:super/src:expr/src:name");
        // The fields of a class or an interface, a name alone in the body of a class, as #count;, and the parameter properties of a constructor,
        // as y of constructor(public y: number)
        const std::string member = "/src:unit/*/src:block/";
        set(query::fieldDeclaration, member + "src:decl_stmt/src:decl[src:name and not(" + staticField + ")] | " + member + "src:name | " +
                                     member + "src:constructor/src:parameter_list/src:parameter/src:decl[ts:specifier]");
        set(query::staticFieldName,  member + "src:decl_stmt/src:decl[" + staticField + "]/src:name");
        // A field that holds an arrow function is a data member, as a field that holds an object is
        const std::string classBody = "/src:unit/src:class/src:block/";
        set(query::method,           classBody + "src:function[src:block and not(" + staticMember + ")] | " + classBody + "src:constructor[src:block]");
        set(query::staticMethodName, classBody + "src:function[" + staticMember + "]/src:name");
        // The signatures of the methods of an interface, the abstract methods, and the overloads, where a call signature has no name
        const std::string signature = member + "ts:function_decl[src:name and not(" + staticMember + ")]";
        set(query::functionDeclName,       signature + "/src:name");
        set(query::functionDeclParameters, signature + "/src:parameter_list");

        // Signatures, where a constructor has no name element
        const std::string function = "/src:unit/*[self::src:function or self::src:constructor]";
        set(query::methodName,              function + "/src:name");
        set(query::methodParameterList,     function + "/src:parameter_list");
        set(query::constructorOrDestructor, "/src:unit/src:constructor");
        set(query::methodSpecifiers,        function + "/src:specifier | " + function + "/ts:specifier");
        set(query::methodAttributes,        function + "/ts:attribute");
        set(query::returnType,              "/src:unit/src:function/ts:type");
        set(query::returnTypeName,          "/src:unit/src:function/ts:type/src:name");
        set(query::parameterDeclaration,    function + "/src:parameter_list/src:parameter/src:decl");

        // Bodies, with the code of nested functions and lambdas, and without the code of local classes
        const std::string body    = function + "/src:block/descendant::";
        const std::string own     = "not(ancestor::src:class)";
        const std::string ownCode = "[" + own + "]";
        // A return or a yield of a nested function or lambda leaves that function only
        const std::string ownReturn = "[count(ancestor::src:function | ancestor::src:constructor | ancestor::src:lambda) = 1 and " + own + "]";

        // A local is a variable that the body declares, also in a destructuring pattern, a parameter of a catch clause, a lambda, or a nested function,
        // or the name of a nested function, where a key of an object literal, as a of { a: 1 }, declares nothing outside a pattern of a parameter
        set(query::localDeclaration, body + "src:decl[src:name and not(ancestor::ts:type) and (not(parent::src:property) or ancestor::src:parameter) and " + own + "]" + " | " +
                                     body + "src:function[not(parent::src:property) and " + own + "]/src:name");
        set(query::localFunction,    body + "src:function[not(parent::src:property) and " + own + "]/src:name | " +
                                     body + "src:decl[not(parent::src:property) and src:init/src:expr[count(*) = 1]/*[self::src:lambda or self::src:function]]/src:name" + ownCode);
        // A yield passes its value to the caller, as a return does
        set(query::returnExpression,    body + "src:return" + ownReturn + "/src:expr | " + body + "src:yield" + ownReturn + "/src:expr");
        set(query::newReturnExpression, body + "src:return" + ownReturn + "/src:expr[*[not(self::src:operator = '(')][1][self::src:operator = 'new']]");

        // super(...) runs the constructor of the parent and is no call, while super.m() calls a method of a parent through the object, as in Java
        const std::string afterNew = siblingBefore("self::src:operator = 'new'");
        set(query::functionCall,    body + "src:call[not(src:name/src:operator) and src:name != 'super' and not(" + siblingBefore(isOperator({"new", ".", "?."})) + ") and " + own + "]");
        set(query::methodCall,      body + "src:call[src:name/src:operator and not(" + afterNew + ") and " + own + "]");
        set(query::constructorCall, body + "src:call[" + afterNew + " and " + own + "] | " +
                                    body + "src:operator[. = 'new' and " + siblingAfter("self::src:name") + " and " + own + "]");
        set(query::callOnResult,    body + "src:call[not(src:name/src:operator) and " + siblingBefore(isOperator({".", "?."})) + " and " + own + "]");
        set(query::newAssignedVariable, body + "src:decl[src:init/src:expr/*[1][self::src:operator = 'new']]/src:name" + ownCode + " | " +
                                        body + "src:expr_stmt/src:expr[*[self::src:operator = 'new']]/src:name" + ownCode);
        set(query::nonCommentStatements, function + "/src:block/src:block_content[*[not(self::src:comment)]]");

        // The object itself passed to a constructor, as this of new Shape(this), reads no state of the object, and the Boolean literals are names
        // A key without a value in an object literal, as b of { a: 1, b }, reads the variable of its name
        const std::string selfArgument = "parent::src:expr[count(*) = 1]/parent::src:argument/parent::src:argument_list/parent::src:call[" + afterNew + "]";
        set(query::expressionName, body + "src:expr[src:name and " + own + "]/src:name[. != 'true' and . != 'false' and not(. = 'this' and " + selfArgument + ")] | " +
                                   body + "src:object/src:property/src:decl[count(*) = 1 and not(ancestor::src:parameter)]/src:name" + ownCode);
        // The steps start at the operators, as in the other languages, where delete removes a member, as of delete this.cache[key]
        const textList assignmentOperators{"=", "+=", "-=", "*=", "/=", "%=", "**=", "<<=", ">>=", ">>>=", "&=", "^=", "|=", "&&=", "||=", "??=", "++", "--"};
        const std::string inExpression = "[parent::src:expr" + ownCode + "]";
        set(query::expressionAssignment, body + "src:operator[" + isOperator(assignmentOperators) + "]" + inExpression + "/preceding-sibling::*[1]/self::src:name | " +
                                         body + "src:operator[" + isOperator({"++", "--", "delete"}) + "]" + inExpression + "/following-sibling::*[1]/self::src:name");
        // srcML marks a template literal as one string literal, whose holes the model reads from its text
        set(query::interpolatedString, body + "src:literal[@type = 'string' and starts-with(., '`') and contains(., '${') and " + own + "]");
        return xpaths;
    }

    // Builds the XPaths of one language
    // The queries of a type, a property, or a function read a copy of its element as the only child of the unit
    //
    std::array<std::string, queryCount> buildQueries(const languageMarkup& markup, const options& settings) {
        std::array<std::string, queryCount> xpaths;
        auto set = [&xpaths](query name, std::string xpath) { xpaths[static_cast<std::size_t>(name)] = std::move(xpath); };

        // Types
        textList kinds;
        for (const typeKind& kind : markup.typeKinds)
            if (!kind.option || settings.*kind.option) kinds.push_back(kind.test);
        textList typeConditions{"(" + join(kinds, " or ") + ")"};
        typeConditions.insert(typeConditions.end(), markup.typeConditions.begin(), markup.typeConditions.end());
        typeConditions.push_back("(" + markup.namedType + ")");
        set(query::type, "/descendant::" + step("src:*", typeConditions));
        if (markup.ownedFunctions)
            set(query::functionOutsideTypes, "/descendant::" + step("src:function", {notNested(markup.typeElements), markup.topLevelFunction.empty() ? "" :
                                                                          "not(ancestor-or-self::src:function[" + markup.topLevelFunction + "])"}));
        // A static method of a type without a body, as an extern method of C#, a native method of Java, or a C++ method defined outside its type
        set(query::staticMethodDeclaration, "/descendant::" + step("src:function_decl", {"src:type/src:specifier='static'",
                                                                               "(" + join(elementTests(markup.typeElements, "ancestor::"), " or ") + ")"}));
        // The name of an event follows its type and comes before its initializer, if any
        const std::string eventName = "src:name[not(preceding-sibling::src:operator='=')][last()]";
        std::string globalVariables = markup.globalVariables;
        for (const eventForm& form : markup.eventForms) {
            textList conditions = form.conditions;
            conditions.push_back(form.staticTest);
            globalVariables += " | //" + step(form.element, conditions) + "/" + eventName;
        }
        set(query::globalVariable, globalVariables);
        if (!markup.staticImports.empty()) set(query::staticImport, markup.staticImports);
        if (!markup.globalStaticImports.empty()) set(query::globalStaticImport, markup.globalStaticImports);
        if (!markup.typeImports.empty()) set(query::typeImport, markup.typeImports);
        if (!markup.globalTypeImports.empty()) set(query::globalTypeImport, markup.globalTypeImports);
        if (!markup.valueTypeNames.empty()) set(query::valueTypeName, markup.valueTypeNames);
        if (!markup.usingDirectives.empty()) set(query::usingDirective, markup.usingDirectives);
        if (!markup.typeAliasNames.empty()) set(query::typeAliasName, markup.typeAliasNames);
        if (!markup.memberAliasNames.empty()) set(query::memberAliasName, markup.memberAliasNames);
        // The C markup of class X { is a declaration of X with the type class, which turns class X final { into a declaration of final
        if (markup.cClasses) {
            const std::string beforeBody = siblingAfter("self::src:block or self::src:range");
            set(query::cClassName, "/descendant::src:decl[src:type[count(*)=1]/src:name='class']/src:name[" + beforeBody + "] | "
                                   "/descendant::src:decl[src:type[count(*)=2]/src:name[1]='class']/src:name[.='final'][" + beforeBody + "]"
                                   "/preceding-sibling::src:type/src:name[2] | "
                                   "//src:expr/src:name[.='class']/following-sibling::*[1]/self::src:name"
                                   "[" + siblingAfter("self::src:block or (self::src:name='final' and " + siblingAfter("self::src:block") + ")") + "]");
        }

        const std::string type = "/src:unit/src:*[" + isOneOf(markup.typeElements) + "]";
        set(query::structureType, type + "/text()");
        if (markup.partialTypes) set(query::partialType, type + "/src:specifier[.='partial']");
        set(query::parentName,    "/src:unit/src:*[" + isOneOf(markup.parentOwners) + "]/src:super_list/" + markup.superStep + "src:super/src:name");

        // Fields, as the declarations that hold their names and types, in the order of the type
        // Every statement body holds a block_content and no type body does, so the condition skips the locals of every body
        textList fieldConditions;
        for (const std::string& specifier : markup.nonFieldSpecifiers)
            fieldConditions.push_back("not(src:decl/src:type/src:specifier='" + specifier + "')");
        if (!markup.nonFieldOwners.empty()) fieldConditions.push_back(notNested(markup.nonFieldOwners));
        fieldConditions.push_back("not(ancestor::src:block_content)");
        fieldConditions.push_back(singleOwner(markup.fieldOwners));
        const std::string declared = markup.misreadDeclarator.empty() ? "" : "not(" + markup.misreadDeclarator + ")";
        textList fieldDeclarations{"/descendant::" + step("src:decl_stmt", fieldConditions) + "/" + step("src:decl", {"src:name[" + siblingBefore("self::src:type") + "]", declared}),
                                   markup.inlineTypeFields, markup.componentFields};
        if (markup.properties) fieldDeclarations.push_back("/descendant::" + step("src:property", {notStatic, singleOwner(markup.typeElements)}));
        for (const eventForm& form : markup.eventForms) {
            textList conditions = form.conditions;
            conditions.insert(conditions.end(), {"not(" + form.staticTest + ")", singleOwner(markup.fieldOwners)});
            fieldDeclarations.push_back("/descendant::" + step(form.element, conditions));
        }
        // A declaration of a pointer to a function, as bool (*filter)(int);, is a function declaration with a modifier before its name
        if (markup.functionPointers)
            fieldDeclarations.push_back("/descendant::" + step("src:function_decl", {"src:modifier", notStatic, "not(ancestor::src:block_content)", singleOwner(markup.fieldOwners)}));
        set(query::fieldDeclaration, join(fieldDeclarations, " | "));

        // Static fields, which are the declarations that the fields leave out for their specifiers or their owner, and static properties
        textList staticTests;
        for (const std::string& specifier : markup.nonFieldSpecifiers) staticTests.push_back("src:decl/src:type/src:specifier='" + specifier + "'");
        for (const std::string& owner : markup.nonFieldOwners) staticTests.push_back("ancestor::src:" + owner);
        textList staticNames{"/descendant::" + step("src:decl_stmt", {"(" + join(staticTests, " or ") + ")", "not(ancestor::src:block_content)", singleOwner(markup.fieldOwners)}) + "/src:decl/src:name"};
        if (markup.properties) staticNames.push_back("/descendant::" + step("src:property", {"src:type/src:specifier='static'", singleOwner(markup.typeElements)}) + "/src:name");
        for (const eventForm& form : markup.eventForms) {
            textList conditions = form.conditions;
            conditions.insert(conditions.end(), {form.staticTest, singleOwner(markup.fieldOwners)});
            staticNames.push_back("/descendant::" + step(form.element, conditions) + "/" + eventName);
        }
        set(query::staticFieldName, join(staticNames, " | "));

        // Methods, method declarations, and free functions
        textList methodConditions{"(" + isOneOf(markup.methodElements) + ")"};
        methodConditions.insert(methodConditions.end(), markup.methodConditions.begin(), markup.methodConditions.end());
        set(query::method, "/descendant::" + step("*", methodConditions));

        // Static and friend declarations name free functions, not methods of the type
        const std::string declaration = "/descendant::" + step("src:function_decl", {singleOwner(markup.declarationOwners), notStatic, "not(parent::src:friend)", "not(src:modifier)"});
        set(query::functionDeclName,       declaration + "/src:name[" + siblingAfter("self::src:parameter_list") + "]");
        set(query::functionDeclParameters, declaration + "/src:parameter_list[" + siblingBefore("self::src:name") + "]");
        if (markup.outerMethods)
            set(query::staticDeclaration, "/descendant::" + step("src:function_decl", {singleOwner(markup.declarationOwners), "src:type/src:specifier='static'"}));
        set(query::freeFunction,           markup.freeFunction);

        textList misreadMethods;
        for (const std::string& name : markup.misreadMethodNames)
            misreadMethods.push_back("/descendant::" + name + "[not(ancestor::src:block_content) and " + singleOwner(markup.typeElements) + "]");
        if (!misreadMethods.empty()) set(query::misreadMethodName, join(misreadMethods, " | "));

        if (markup.properties) {
            set(query::property,       "/descendant::" + step("src:property", {singleOwner(markup.typeElements), notStatic}));
            set(query::propertyType,   "//src:property/src:type");
            set(query::propertyMethod, "/descendant::src:function[not(ancestor::src:function)]");
        }

        // Signatures
        const std::string method   = "/src:unit/*[" + isOneOf(markup.methodElements) + "]";
        const std::string function = "/src:unit/src:function";
        set(query::methodName,              method + "/src:name");
        set(query::methodParameterList,     method + "/src:parameter_list");
        set(query::constructorOrDestructor, "/src:unit/*[" + isOneOf(markup.constructorElements) + "]");
        set(query::methodSpecifiers,        function + "/src:type/src:specifier");
        if (!markup.annotation.empty()) set(query::methodAttributes, function + "/src:" + markup.annotation);
        if (markup.constMethods)        set(query::constMethod,      function + "/src:specifier[.='const']");
        // A trailing return type, as in auto f() -> int, is the last type of the function, and its annotations name no type
        set(query::returnType,              function + "/src:type[last()]/descendant::text()[not(ancestor::src:parameter_list) and not(ancestor::src:annotation)]");
        set(query::returnTypeName,          function + "/src:type[last()]/src:name");
        // A parameter that points to a function is a function declaration, as void (*callback)() is
        set(query::parameterDeclaration,    function + "/src:parameter_list/src:parameter/*[self::src:decl or self::src:function_decl]");

        // Bodies, of which a C++ function-try-block, as in void f() try { } catch (...) { }, holds the block and the handlers in a try element
        const std::string body = function + "/*[self::src:block or self::src:try]/descendant::";
        // The code of a member that srcML nests in the function is no code of the function. The test walks the ancestors, so it comes after
        // the other tests of a step, or as ownCode at the end of a path whose later steps reach no member
        textList misplaced;
        for (const std::string& member : markup.misplacedMembers) misplaced.push_back("ancestor-or-self::" + member + "[ancestor::src:function]");
        const std::string own     = misplaced.empty() ? "" : "not(" + join(misplaced, " or ") + ")";
        const std::string ownCode = own.empty() ? "" : "[" + own + "]";
        // The arguments of an attribute or an annotation in a body, as of [[deprecated("old")]] or @Min(value = 1), are no code of the function
        const std::string attributeCall     = "parent::src:expr/parent::src:attribute";
        const std::string outsideAttributes = "not(parent::src:attribute or parent::src:argument/parent::src:argument_list/parent::*"
                                              "[self::src:annotation or self::src:call[" + attributeCall + "]])";
        // Locals include catch parameters and the parameters of lambdas and nested functions, as in catch (int& e) or [](int& x) { }
        // A parameter of a lambda may have no type, as in x -> x + 1, and a declarator after the body of a type, as x of struct { } x;
        // has the type that the body defines, while an enumerator is no variable
        set(query::localDeclaration,  body + step("src:decl", {"src:name", "(src:type or parent::src:parameter or parent::src:class or "
                                                                             "parent::src:struct or parent::src:union or parent::src:enum)", own}));
        // A variable that an expression declares has no declaration element. srcML marks the variable of a type pattern, as found of
        // other instanceof Gauge found, as a name after the name of its type, which may follow the word not of a C# pattern, while the
        // words and, or, and with name neither. Only white space stands between the two names, unlike the comma between the arguments
        // of new(low, high)
        textList declaredNames;
        const std::string afterType = "[" + siblingBefore("self::src:name[. != 'not']") + "][. != 'and' and . != 'or' and . != 'with']"
                                      "[preceding-sibling::node()[1]/self::text()[normalize-space(.) = '']]";
        textList beforeType;
        if (!markup.typeTest.empty()) {
            const std::string typeTest = "self::src:operator='" + markup.typeTest + "'";
            beforeType.push_back(typeTest);
            beforeType.push_back("(self::src:name='not' and " + siblingBefore(typeTest) + ")");
        }
        // A variable with its type in parentheses, as low of (int low, int high) = pair; and of other is (int low, int high)
        if (markup.tupleDeclarations) beforeType.push_back("self::src:operator='(' or self::src:operator=','");
        if (markup.usingDeclarations) beforeType.push_back("self::src:operator='await'");
        if (!beforeType.empty())
            declaredNames.push_back(body + "src:expr/src:name" + afterType + "[" + siblingBefore(join(beforeType, " or "), 2) + "]" + ownCode);
        if (markup.casePatterns) declaredNames.push_back(body + "src:case/src:expr/*[2][self::src:name]" + afterType + ownCode);
        if (markup.tupleDeclarations) {
            // The names in the parentheses after var, where srcML marks var (low, high) as a call unless = follows
            declaredNames.push_back(body + step("src:expr", {"parent::src:expr_stmt", "*[1]/self::src:name='var'", "*[2]/self::src:operator='('", own}) +
                                    "/src:operator[. = '='][1]/preceding-sibling::src:name[position() != last()]");
            declaredNames.push_back(body + step("src:call", {"src:name='var'", own}) + "/src:argument_list/src:argument/src:expr[count(*) = 1]/src:name");
        }
        if (markup.usingDeclarations) declaredNames.push_back(body + "src:using/src:name[2]" + ownCode);
        if (markup.queryClauses) {
            declaredNames.push_back(body + step("*", {"(self::src:from or self::src:join)", own}) + "/src:expr/src:name[last()]");
            declaredNames.push_back(body + step("*", {"(self::src:let or self::src:into)", own}) + "/src:expr/src:name[1]");
        }
        set(query::expressionVariable, join(declaredNames, " | "));
        // A return inside a lambda, a nested function, or a local or anonymous type returns from that body, not from the function
        const std::string ownReturn = "count(ancestor::src:function) = 1 and not(ancestor::src:lambda) and " + notNested(markup.typeElements);
        set(query::returnExpression,  body + step("src:return", {ownReturn}) + "/src:expr");

        // A local function and a local that a lambda initializes hold code of the function itself, where srcML marks a local function
        // with an expression body, as int Add(int x) => x + 1;, as a declaration with arguments
        const std::string lambdaLocal = body + "src:decl[src:init/src:expr[count(*) = 1]/*[self::src:lambda or self::src:delegate]]/src:name";
        set(query::localFunction, !markup.localFunctions ? lambdaLocal : lambdaLocal + " | " + body + step("src:function", {own, notNested(markup.typeElements)}) + "/src:name | " +
                                  body + step("src:decl", {"src:argument_list/following-sibling::text()[1][contains(., '=>')]", own}) + "/src:name");

        // An object is created by new or by a call of a factory function, which the tests below find at the element that starts the creation and at the call
        const std::string factoryCall    = markup.factoryCall.empty() ? "" : " or self::src:call[" + markup.factoryCall + "]";
        const std::string creationStart  = "self::src:operator='new'" + factoryCall;
        const std::string creatingCall   = siblingBefore("self::src:operator='new'") + (markup.factoryCall.empty() ? "" : " or " + markup.factoryCall);
        // The object can be created in parentheses, as in return (new A());
        set(query::newReturnExpression, body + step("src:return", {ownReturn}) + "/src:expr[*[not(self::src:operator='(')][1][" + creationStart + "]]");

        // srcML marks each ?. as a ternary without else whose condition holds the chain before ?. and whose then holds the rest of the chain,
        // so a call after the . that opens the then or a condition inside it, as in x?.Z(), x?.y.Z(), x?.F()?.Z(), or F()?.Z(), continues a chain
        const std::string nullConditionalPart = siblingBefore("self::src:operator='.' and not(preceding-sibling::*)") +
                                                "/parent::src:expr/parent::*[self::src:then or self::src:condition]/parent::src:ternary[not(src:else)]";
        const std::string outsideChains       = markup.nullConditional ? "not(" + nullConditionalPart + ")" : "";
        textList notMemberCall{"new"};
        notMemberCall.insert(notMemberCall.end(), markup.memberOperators.begin(), markup.memberOperators.end());
        // srcML marks operator=(other); as a declaration of an operator function without a type, whose parameter types are the arguments of a call
        const std::string operatorStatement = "src:function_decl[@type='operator' and not(src:type)";
        const auto withOperatorStatements = [&](const std::string& calls, const std::string& nameTest) {
            return markup.operatorFunctions ? calls + " | " + body + operatorStatement + " and " + nameTest + "]" : calls;
        };
        set(query::functionCall,    withOperatorStatements(body + step("src:call", {"not(src:name/src:operator)", "not(" + siblingBefore(isOperator(notMemberCall)) + ")",
                                                                                    "not(" + attributeCall + ")", markup.tupleDeclarations ? "not(src:name='var')" : "", own}),
                                                           "not(src:name/src:operator)"));
        set(query::methodCall,      withOperatorStatements(body + step("src:call", {"src:name/src:operator", "not(" + siblingBefore(isOperator({"new"})) + ")", outsideChains,
                                                                                    "not(" + attributeCall + ")", own}),
                                                           "src:name/src:operator"));
        // srcML marks the creation of an anonymous class as a class with its parent and its arguments, as new Runnable() { }
        // It marks the type and the arguments before an initializer as a macro, as in new Report(count) { }, and a creation without arguments
        // after its type with no call, as new Matrix, new Report { }, and new(), while a name with an index or an index after new creates an array
        const std::string afterNew = siblingBefore(isOperator({"new"}));
        textList createdWithoutCall{"self::src:name[not(src:index)]", "self::src:block"};
        if (markup.targetTypedNew) createdWithoutCall.push_back("self::src:operator='('");
        set(query::constructorCall, body + step("src:call", {afterNew, own}) + " | " + body + step("src:macro", {afterNew, own}) + " | " +
                                    body + step("src:operator", {". = 'new'", siblingAfter(join(createdWithoutCall, " or ")), own}) +
                                    (markup.anonymousClasses ? " | " + body + step("src:class", {"src:super", afterNew, own}) : ""));
        // A call on the result of a call or of a cast, such as c() of a.b().c() or Dispose() of ((IDisposable)o).Dispose()
        set(query::callOnResult,    body + step("src:call", {"not(src:name/src:operator)", siblingBefore(isOperator(markup.memberOperators)), outsideChains, own}));
        if (markup.nullConditional) set(query::nullConditionalCall, body + step("src:call", {nullConditionalPart, own}));

        if (markup.operatorFunctions) {
            // An operator with two operands after the object itself, as << of *this << value or of (*this) << value, where a unary * starts
            // the expression or follows an operator, and = writes the whole object
            // The steps start at the object, since a test of the siblings of every operator takes quadratic time in a long expression
            const std::string unary        = "self::src:operator='*' and not(" + siblingBefore("self::src:name or self::src:call or self::src:literal or self::src:operator=')'") + ")";
            const std::string dereferenced = body + "src:expr/src:name[. = 'this'][" + siblingBefore(unary) + "]";
            const textList    twoOperands{"<<", ">>", "==", "!=", "<", ">", "<=", ">=", "<=>", "+", "-", "*", "/", "%", "&", "|", "^", "&&", "||"};
            const std::string operation    = "/following-sibling::*[1]/self::src:operator[" + isOperator(twoOperands) + "]";
            set(query::selfOperator,  dereferenced + operation + " | " +
                                      dereferenced + "[" + siblingBefore("self::src:operator='('", 2) + "]/following-sibling::*[1]/self::src:operator[. = ')']" + operation);
            set(query::shiftOperator, body + "src:expr/src:operator[(. = '<<' or . = '>>') and " + siblingBefore("self::src:name") + "]");
        }

        set(query::newAssignedVariable,  body + "src:decl_stmt/src:decl[src:init/src:expr/*[" + creationStart + "]]/src:name" + ownCode + " | " +
                                         body + "src:expr_stmt/src:expr[*[" + creationStart + "]]/src:name" + ownCode);
        const std::string statements = "src:block/src:block_content[1][*[not(self::src:comment)][1]]";
        set(query::nonCommentStatements, function + "/" + statements + " | " + function + "/src:try/" + statements);
        // The object itself passed to a constructor or a factory call, as this of new A(this) or of new A(*this), reads no state of the object
        // The constructor of a local takes the object the same way, as in Binder binder(*this);
        const std::string selfArgument = "parent::src:expr[count(*) = 1 or (count(*) = 2 and *[1][self::src:operator='*'])]"
                                         "/parent::src:argument/parent::src:argument_list/parent::*[self::src:decl or self::src:call[" + creatingCall + "]]";
        // A method reference names a method, so neither its name nor the object itself before it, as in this::process, is a variable
        textList nameConditions{"not(. = 'this' and " + selfArgument + ")"};
        if (!markup.methodReference.empty()) {
            const std::string reference = "self::src:operator='" + markup.methodReference + "'";
            nameConditions.push_back("not(" + siblingBefore(reference) + ")");
            nameConditions.push_back("not((. = 'this' or . = 'super') and " + siblingAfter(reference) + ")");
        }
        // The name before = or : that opens an expression of an object initializer, a with expression, or a property pattern names a member
        // of the other object, as Left of other with { Left = 1 } and of other is { Left: 0 }
        if (markup.memberInitializers)
            nameConditions.push_back("not(parent::src:expr/parent::src:block/parent::src:expr and not(preceding-sibling::*) and " +
                                     siblingAfter("self::src:operator='=' or self::src:operator=':'") + ")");
        if (!markup.nameOperator.empty())
            nameConditions.push_back("not(parent::src:expr/parent::src:argument/parent::src:argument_list/parent::src:call[src:name='" + markup.nameOperator + "'])");
        set(query::expressionName,       body + step("src:expr", {"src:name", outsideAttributes, own}) + "/" + step("src:name", nameConditions) +
                                         (markup.operatorFunctions ? " | " + body + operatorStatement + "]/src:parameter_list/src:parameter/src:decl/src:type/src:name" : ""));
        // The steps start at the operators, since a test of the siblings of every name takes quadratic time in long expressions
        const std::string inAssignment = "[" + step("parent::src:expr", {markup.assignmentCondition, outsideAttributes, own}) + "]";
        const auto assignment = [&](const std::string& tests) { return body + "src:operator[" + tests + "]" + inAssignment; };
        const std::string target = markup.nullForgiving ? "self::src:name or self::src:index[" + siblingBefore("self::src:index or self::src:operator='!'") + "]"
                                                        : "self::src:name";
        std::string written = assignment(isOperator(markup.assignmentOperators)) + "/preceding-sibling::*[1]/self::*[" + target + "] | " +
                              assignment(isOperator({"++", "--"})) + "/following-sibling::*[1]/self::src:name";
        if (!markup.splitAssignments.empty())
            written += " | " + assignment("(" + isOperator(markup.splitAssignments) + ") and " + siblingAfter("self::src:operator='='")) +
                       "/preceding-sibling::*[1]/self::src:name";
        if (markup.referenceArguments)
            written += " | " + body + step("src:argument", {"(src:modifier='ref' or src:modifier='out')", own}) + "/src:expr[count(*) = 1]/src:name";
        // A statement that opens with a parenthesis writes the names before its first =, as (low, high) = pair; does, where a name
        // beside a name declares a variable with its type
        if (markup.tupleDeclarations)
            written += " | " + body + step("src:expr", {"parent::src:expr_stmt", "*[1]/self::src:operator='('", own}) +
                       "/src:operator[. = '='][1][" + siblingBefore("self::src:operator=')'") + "]"
                       "/preceding-sibling::src:name[not(" + siblingBefore("self::src:name") + ") and not(" + siblingAfter("self::src:name") + ")]";
        set(query::expressionAssignment, written);
        // srcML marks the prefix of an interpolated string, such as $ or @$, as a name before the literal of the text of the string
        if (markup.interpolation)
            set(query::interpolatedString, body + "src:expr/src:name[contains(., '$') and translate(., '$@', '') = '']"
                                           "[following-sibling::node()[1]/self::src:literal[@type='string']]" + ownCode);
        return xpaths;
    }
}

// Gets the language whose queries apply to a unit language
//
queryLanguage toQueryLanguage(const std::string& language) {
    if (language == "C++")    return queryLanguage::cpp;
    if (language == "C#")     return queryLanguage::csharp;
    if (language == "Java")   return queryLanguage::java;
    if (language == "Python") return queryLanguage::python;
    if (language == "JavaScript") return queryLanguage::javascript;
    throw std::invalid_argument("no queries for the language " + language);
}

// Builds the XPaths of every language
//
queryRegistry::queryRegistry(const options& settings) {
    xpaths[static_cast<std::size_t>(queryLanguage::cpp)]    = buildQueries(cppMarkup(), settings);
    xpaths[static_cast<std::size_t>(queryLanguage::csharp)] = buildQueries(csharpMarkup(), settings);
    xpaths[static_cast<std::size_t>(queryLanguage::java)]   = buildQueries(javaMarkup(), settings);
    xpaths[static_cast<std::size_t>(queryLanguage::python)] = pythonQueries();
    xpaths[static_cast<std::size_t>(queryLanguage::javascript)] = javascriptQueries();
}

// Gets the XPath of a query, which is empty when the language has no such query
//
const std::string& queryRegistry::xpath(queryLanguage language, query name) const {
    return xpaths[static_cast<std::size_t>(language)][static_cast<std::size_t>(name)];
}
