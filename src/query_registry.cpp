// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file query_registry.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "query_registry.hpp"

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
        textList                methodElements;         // Elements of methods
        textList                methodConditions;       // Conditions on methods
        textList                constructorElements;    // Elements of constructors and destructors
        textList                declarationOwners;      // Elements of types whose method declarations are read
        std::string             freeFunction;           // XPath of free functions
        std::string             globalVariables;        // XPath of the names of the globals, which are the variables at file or namespace scope and the static fields
        std::string             staticImports;          // XPath of the names of static imports, which reach the static fields of a type without its name, if any
        std::string             globalStaticImports;    // XPath of the names of the static imports that hold for every unit, if any
        std::string             valueTypeNames;         // XPath of the names of the types whose values a parameter copies, if any
        std::string             usingDirectives;        // XPath of the using directives at namespace scope, as using namespace std;, if any
        std::string             typeAliasNames;         // XPath of the names of the aliases of types outside the bodies of code, if any
        std::string             annotation;             // Element of the attributes or annotations of methods, if any
        std::string             nestedGuard;            // Condition that skips the code of nested functions, if any
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
        bool                    cClasses{false};        // The queries also read C units, whose markup turns C++ classes into declarations
        bool                    constMethods{false};    // The language has const methods
        bool                    nullConditional{false}; // The language calls a member of a name that may be null with ?., as in o?.Dispose()
        bool                    interpolation{false};   // The language has interpolated strings, as $"{count} items"
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

    // Joins operator tests, such as "self::src:operator='new' or self::src:operator='.'"
    //
    std::string operatorTests(const textList& operators) {
        textList tests;
        for (const std::string& op : operators) tests.push_back("self::src:operator='" + op + "'");
        return join(tests, " or ");
    }

    // Gets a location step with its non-empty conditions as one predicate
    //
    std::string step(const std::string& test, const textList& conditions) {
        const std::string predicate = join(conditions, " and ");
        return predicate.empty() ? test : test + "[" + predicate + "]";
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
                                        "not(preceding-sibling::node()[1][self::text()][contains(., '*') or contains(., '&')])]";

        languageMarkup markup;
        markup.typeKinds           = {{"self::src:class", nullptr}, {"self::src:struct", &options::structs}, {"self::src:union", &options::unions}};
        markup.namedType           = "src:name or " + typedefName;
        // The types in a body of code are local types, and the types in an anonymous type have no qualified name
        const std::string outsideBodies = notNested({"function", "constructor", "destructor", "lambda"});
        const std::string inNamedTypes  = "not(ancestor::*[(self::src:class or self::src:struct or self::src:union) and not(" + markup.namedType + ")])";
        markup.typeConditions      = {outsideBodies, inNamedTypes};
        // An alias declaration or a typedef takes the conditions of the types, while a template alias, as template <typename T> using P = T*;, names no single type
        markup.typeAliasNames      = "//" + step("src:using", {"src:init/src:type", "not(src:template)", outsideBodies, inNamedTypes}) + "/src:name | " +
                                     "//" + step("src:typedef", {"src:type", outsideBodies, inNamedTypes}) + "/src:name[not(*)]";
        markup.typeElements        = types;
        markup.parentOwners        = {"class", "struct"};
        // The queries of a type read a copy of the type as the only child of the unit, where a union of a typedef has no typedef around it
        markup.fieldOwners         = {"class", "struct", "union[src:name or parent::src:unit or " + typedefName + "]"};
        markup.nonFieldSpecifiers  = {"static"};
        markup.inlineTypeFields    = "//*[(self::src:class or self::src:struct or self::src:union) and not(ancestor::src:block_content) and " +
                                     singleOwner(markup.fieldOwners) + "]/src:decl[src:name]";
        markup.methodElements      = {"function", "constructor", "destructor"};
        markup.methodConditions    = {notStatic, singleOwner(types), "not(parent::src:friend)"};
        markup.constructorElements = {"constructor", "destructor"};
        markup.declarationOwners   = types;
        // A friend function is a free function, and constructors and destructors outside a type are definitions of the members of a type
        markup.freeFunction        = "//*[(self::src:function and (" + notNested(types) + " or src:type/src:specifier='static' or parent::src:friend)) or " +
                                     "((self::src:constructor or self::src:destructor) and " + notNested(types) + ")]";
        // The block of an extern "C" declaration, unlike a body, holds declarations at namespace scope
        const std::string inBody           = "ancestor::src:block_content[not(parent::src:block/parent::src:extern)]";
        const std::string atNamespaceScope = "not(ancestor::src:class or ancestor::src:struct or ancestor::src:union or " + inBody + ")";
        // A using directive in a body holds only for the body, where no method is defined
        markup.usingDirectives     = "//src:using[src:namespace and " + atNamespaceScope + "]";
        // The members of an anonymous union at namespace scope without a declarator are globals, as are the declarators after the body of a type, as x of struct { } x;
        // A declarator with a block is the C markup of a C++ class, as final of class Date final { }
        markup.globalVariables     = "//src:decl_stmt[" + atNamespaceScope + "]/src:decl[not(src:block)]/src:name | "
                                     "//*[(self::src:class or self::src:struct or self::src:union or self::src:enum) and " + atNamespaceScope + "]/src:decl/src:name | "
                                     "//src:decl_stmt[src:decl/src:type/src:specifier='static' and not(" + inBody + ") and "
                                     "(ancestor::src:class or ancestor::src:struct or ancestor::src:union)]/src:decl[not(src:block)]/src:name | "
                                     "//src:decl_stmt[ancestor::*[self::src:class or self::src:struct or self::src:union][1]"
                                     "[self::src:union and not(" + markup.namedType + ") and not(src:decl) and " + atNamespaceScope + "]]/src:decl[not(src:block)]/src:name";
        markup.outerMethods        = true;
        markup.cClasses            = true;
        markup.memberOperators     = {".", "->"};
        markup.assignmentOperators = assignments({});
        // std::make_unique and std::make_shared, with or without the namespace std
        markup.factoryCall         = "src:name/src:name[last()][.='make_unique' or .='make_shared'] and (count(src:name/src:name) = 1 or src:name/src:name[1]='std')";
        markup.constMethods        = true;
        return markup;
    }

    // C#: static types hold only static members, local functions and property accessors are no methods,
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
        markup.freeFunction        = "//src:function[(src:type/src:specifier='static' or ancestor::src:property[1]/src:type/src:specifier='static')]";
        // A declarator with an argument list or a block is a part of an initializer that srcML misreads
        markup.globalVariables     = "//src:decl_stmt[(src:decl/src:type/src:specifier='static' or src:decl/src:type/src:specifier='const') and "
                                     "not(ancestor::src:block_content)]/src:decl[not(src:argument_list or src:block)]/src:name | "
                                     "//src:property[src:type/src:specifier='static' and not(ancestor::src:block_content)]/src:name";
        markup.staticImports       = "//src:using[starts-with(normalize-space(.), 'using static ')]/src:name";
        // srcML marks the word global of global using static as a macro before the using directive
        markup.globalStaticImports = "//src:using[starts-with(normalize-space(.), 'using static ') and preceding-sibling::*[1][self::src:macro[src:name='global']]]/src:name";
        markup.valueTypeNames      = "//src:struct/src:name | //src:enum/src:name";
        // A using alias directive of a unit or a namespace, unlike a using declaration of a body, as using var reader = Open();
        markup.typeAliasNames      = "//src:using[src:init and not(ancestor::src:block_content)]/src:name";
        markup.annotation          = "attribute";
        markup.nestedGuard         = "count(ancestor::src:function) = 1";
        markup.memberOperators     = {".", "->"};
        markup.assignmentOperators = assignments({">>>="});
        markup.splitAssignments    = {"??"};
        markup.assignmentCondition = "not(parent::src:block[parent::src:expr])";
        markup.properties          = true;
        markup.partialTypes        = true;
        markup.ownedFunctions      = true;
        markup.nullConditional     = true;
        markup.interpolation       = true;
        return markup;
    }

    // Java: a class with a super child is an anonymous class, which is skipped
    //
    languageMarkup javaMarkup() {
        const textList types{"class", "interface", "enum"};

        languageMarkup markup;
        // Interfaces are parents of classes whatever -i says, so they are always analyzed and labeled only with -i
        markup.typeKinds           = {{"(self::src:class and not(child::src:super[1]))", nullptr}, {"self::src:interface", nullptr}, {"self::src:enum", &options::enums}};
        // Local classes lie in bodies of code and initializer blocks, while a declaration holds the body of an enum constant or an anonymous class
        markup.typeConditions      = {notNested({"function", "constructor", "lambda", "block_content", "decl"})};
        markup.typeElements        = types;
        markup.parentOwners        = types;
        markup.superStep           = "*[self::src:extends or self::src:implements]/";
        markup.fieldOwners         = types;
        markup.nonFieldSpecifiers  = {"static"};
        markup.nonFieldOwners      = {"interface"};
        markup.componentFields     = "/src:unit/src:class/src:parameter_list/src:parameter/src:decl[src:name[preceding-sibling::*[1][self::src:type]]]";
        markup.methodElements      = {"function", "constructor"};
        markup.methodConditions    = {notStatic, singleOwner(types)};
        markup.constructorElements = {"constructor"};
        // Known defect: Java has no struct, and the abstract methods that an enum declares are missed
        markup.declarationOwners   = {"class", "struct", "interface"};
        markup.freeFunction        = "//src:function[src:type/src:specifier='static']";
        // The fields of an interface are static, while a declarator with an argument list or a block is a part of an initializer that srcML misreads
        markup.globalVariables     = "//src:decl_stmt[src:decl/src:type/src:specifier='static' and not(ancestor::src:block_content)]"
                                     "/src:decl[not(src:argument_list or src:block)]/src:name | "
                                     "//src:decl_stmt[not(ancestor::src:block_content) and ancestor::*[self::src:class or self::src:interface or "
                                     "self::src:enum or self::src:annotation_defn][1][self::src:interface or self::src:annotation_defn]]"
                                     "/src:decl[not(src:argument_list or src:block)]/src:name";
        markup.staticImports       = "/src:unit/src:import[starts-with(normalize-space(.), 'import static ')]/src:name";
        markup.annotation          = "annotation";
        markup.memberOperators     = {"."};
        markup.methodReference     = "::";
        markup.assignmentOperators = assignments({">>>="});
        markup.ownedFunctions      = true;
        return markup;
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
        set(query::type, "//" + step("src:*", typeConditions));
        if (markup.ownedFunctions) set(query::functionOutsideTypes, "//" + step("src:function", {notNested(markup.typeElements)}));
        set(query::globalVariable, markup.globalVariables);
        if (!markup.staticImports.empty()) set(query::staticImport, markup.staticImports);
        if (!markup.globalStaticImports.empty()) set(query::globalStaticImport, markup.globalStaticImports);
        if (!markup.valueTypeNames.empty()) set(query::valueTypeName, markup.valueTypeNames);
        if (!markup.usingDirectives.empty()) set(query::usingDirective, markup.usingDirectives);
        if (!markup.typeAliasNames.empty()) set(query::typeAliasName, markup.typeAliasNames);
        // The C markup of class X { is a declaration of X with the type class, which turns class X final { into a declaration of final
        if (markup.cClasses)
            set(query::cClassName, "//src:decl[src:type[count(*)=1]/src:name='class']/src:name[following-sibling::*[1][self::src:block or self::src:range]] | "
                                   "//src:decl[src:type[count(*)=2]/src:name[1]='class']/src:name[.='final'][following-sibling::*[1][self::src:block or self::src:range]]"
                                   "/preceding-sibling::src:type/src:name[2] | "
                                   "//src:expr/src:name[.='class']/following-sibling::*[1][self::src:name]"
                                   "[following-sibling::*[1][self::src:block or (self::src:name='final' and following-sibling::*[1][self::src:block])]]");

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
        textList fieldDeclarations{"//" + step("src:decl_stmt", fieldConditions) + "/src:decl[src:name[preceding-sibling::*[1][self::src:type]]]",
                                   markup.inlineTypeFields, markup.componentFields};
        if (markup.properties) fieldDeclarations.push_back("//" + step("src:property", {notStatic, singleOwner(markup.typeElements)}));
        set(query::fieldDeclaration, join(fieldDeclarations, " | "));

        // Static fields, which are the declarations that the fields leave out for their specifiers or their owner, and static properties
        textList staticTests;
        for (const std::string& specifier : markup.nonFieldSpecifiers) staticTests.push_back("src:decl/src:type/src:specifier='" + specifier + "'");
        for (const std::string& owner : markup.nonFieldOwners) staticTests.push_back("ancestor::src:" + owner);
        textList staticNames{"//" + step("src:decl_stmt", {"(" + join(staticTests, " or ") + ")", "not(ancestor::src:block_content)", singleOwner(markup.fieldOwners)}) + "/src:decl/src:name"};
        if (markup.properties) staticNames.push_back("//" + step("src:property", {"src:type/src:specifier='static'", singleOwner(markup.typeElements)}) + "/src:name");
        set(query::staticFieldName, join(staticNames, " | "));

        // Methods, method declarations, and free functions
        textList methodConditions{"(" + isOneOf(markup.methodElements) + ")"};
        methodConditions.insert(methodConditions.end(), markup.methodConditions.begin(), markup.methodConditions.end());
        set(query::method, "//" + step("*", methodConditions));

        // Static and friend declarations name free functions, not methods of the type
        const std::string declaration = "//" + step("src:function_decl", {singleOwner(markup.declarationOwners), notStatic, "not(parent::src:friend)"});
        set(query::functionDeclName,       declaration + "/src:name[following-sibling::*[1][self::src:parameter_list]]");
        set(query::functionDeclParameters, declaration + "/src:parameter_list[preceding-sibling::*[1][self::src:name]]");
        if (markup.outerMethods)
            set(query::staticDeclaration, "//" + step("src:function_decl", {singleOwner(markup.declarationOwners), "src:type/src:specifier='static'"}));
        set(query::freeFunction,           markup.freeFunction);

        if (markup.properties) {
            set(query::property,       "//" + step("src:property", {singleOwner(markup.typeElements), notStatic}));
            set(query::propertyType,   "//src:property/src:type");
            set(query::propertyMethod, "//src:function[not(ancestor::src:function)]");
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
        // A trailing return type, as in auto f() -> int, is the last type of the function
        set(query::returnType,              function + "/src:type[last()]//text()[not(ancestor::src:parameter_list)]");
        set(query::parameterDeclaration,    function + "/src:parameter_list/src:parameter/src:decl");

        // Bodies, of which a C++ function-try-block, as in void f() try { } catch (...) { }, holds the block and the handlers in a try element
        const std::string body = function + "/*[self::src:block or self::src:try]//";
        const std::string& guard = markup.nestedGuard;
        // Locals include catch parameters and the parameters of lambdas and nested functions, as in catch (int& e) or [](int& x) { }
        // A parameter of a lambda may have no type, as in x -> x + 1, and a declarator after the body of a type, as x of struct { } x;
        // has the type that the body defines, while an enumerator is no variable
        set(query::localDeclaration,  body + step("src:decl", {guard, "src:name", "(src:type or parent::src:parameter or parent::src:class or "
                                                                                  "parent::src:struct or parent::src:union or parent::src:enum)"}));
        // A return inside a lambda, a nested function, or a local or anonymous type returns from that body, not from the function
        const std::string ownReturn = "count(ancestor::src:function) = 1 and not(ancestor::src:lambda) and " + notNested(markup.typeElements);
        set(query::returnExpression,  body + step("src:return", {ownReturn}) + "/src:expr");

        // An object is created by new or by a call of a factory function, which the tests below find at the element that starts the creation and at the call
        const std::string factoryCall    = markup.factoryCall.empty() ? "" : " or self::src:call[" + markup.factoryCall + "]";
        const std::string creationStart  = "self::src:operator='new'" + factoryCall;
        const std::string creatingCall   = "preceding-sibling::*[1][self::src:operator='new']" + (markup.factoryCall.empty() ? "" : " or " + markup.factoryCall);
        // The object can be created in parentheses, as in return (new A());
        set(query::newReturnExpression, body + step("src:return", {ownReturn}) + "/src:expr[*[not(self::src:operator='(')][1][" + creationStart + "]]");

        textList notMemberCall{"new"};
        notMemberCall.insert(notMemberCall.end(), markup.memberOperators.begin(), markup.memberOperators.end());
        set(query::functionCall,    body + step("src:call", {guard, "not(src:name/src:operator)", "not(preceding-sibling::*[1][" + operatorTests(notMemberCall) + "])"}));
        set(query::methodCall,      body + step("src:call", {guard, "src:name/src:operator", "not(preceding-sibling::*[1][" + operatorTests({"new"}) + "])"}));
        set(query::constructorCall, body + step("src:call", {guard, "preceding-sibling::*[1][" + operatorTests({"new"}) + "]"}));
        // A call on the result of a call or of a cast, such as c() of a.b().c() or Dispose() of ((IDisposable)o).Dispose()
        // srcML marks o?.Dispose() as a ternary whose condition holds o and whose then holds .Dispose(), a call on the name o
        const std::string nullConditionalName = "preceding-sibling::*[1][self::src:operator='.' and not(preceding-sibling::*)]"
                                                "/parent::src:expr/parent::src:then/parent::src:ternary[not(src:else)]/src:condition/src:expr[count(*) = 1]/src:name";
        set(query::callOnResult,    body + step("src:call", {guard, "not(src:name/src:operator)", "preceding-sibling::*[1][" + operatorTests(markup.memberOperators) + "]",
                                                             markup.nullConditional ? "not(" + nullConditionalName + ")" : ""}));
        if (markup.nullConditional)
            set(query::nullConditionalCall, body + step("src:call", {guard, "not(src:name/src:operator)", nullConditionalName}));

        set(query::newAssignedVariable,  body + step("src:decl_stmt", {guard}) + "/src:decl[src:init/src:expr/*[" + creationStart + "]]/src:name | " +
                                         body + step("src:expr_stmt", {guard}) + "/src:expr[*[" + creationStart + "]]/src:name");
        const std::string statements = "src:block/src:block_content[1][*[not(self::src:comment)][1]]";
        set(query::nonCommentStatements, function + "/" + statements + " | " + function + "/src:try/" + statements);
        // The object itself passed to a constructor or a factory call, as this of new A(this) or of new A(*this), reads no state of the object
        const std::string selfArgument = "parent::src:expr[count(*) = 1 or (count(*) = 2 and *[1][self::src:operator='*'])]"
                                         "/parent::src:argument/parent::src:argument_list/parent::src:call[" + creatingCall + "]";
        // A method reference names a method, so neither its name nor the object itself before it, as in this::process, is a variable
        textList nameConditions{"not(. = 'this' and " + selfArgument + ")"};
        if (!markup.methodReference.empty()) {
            const std::string reference = "self::src:operator='" + markup.methodReference + "'";
            nameConditions.push_back("not(preceding-sibling::*[1][" + reference + "])");
            nameConditions.push_back("not((. = 'this' or . = 'super') and following-sibling::*[1][" + reference + "])");
        }
        set(query::expressionName,       body + step("src:expr", {guard}) + "/" + step("src:name", nameConditions));
        // The steps start at the operators, since a test of the siblings of every name takes quadratic time in long expressions
        const std::string assignment = body + step("src:expr", {guard, markup.assignmentCondition}) + "/src:operator[";
        std::string written = assignment + operatorTests(markup.assignmentOperators) + "]/preceding-sibling::*[1][self::src:name] | " +
                              assignment + operatorTests({"++", "--"}) + "]/following-sibling::*[1][self::src:name]";
        if (!markup.splitAssignments.empty())
            written += " | " + assignment + "(" + operatorTests(markup.splitAssignments) + ") and following-sibling::*[1][self::src:operator='=']]/preceding-sibling::*[1][self::src:name]";
        set(query::expressionAssignment, written);
        // srcML marks the prefix of an interpolated string, such as $ or @$, as a name before the literal of the text of the string
        if (markup.interpolation)
            set(query::interpolatedString, body + step("src:expr", {guard}) + "/src:name[contains(., '$') and translate(., '$@', '') = '']"
                                           "[following-sibling::node()[1][self::src:literal[@type='string']]]");
        return xpaths;
    }
}

// Gets the language whose queries apply to a unit language
//
queryLanguage toQueryLanguage(const std::string& language) {
    if (language == "C++")  return queryLanguage::cpp;
    if (language == "C#")   return queryLanguage::csharp;
    if (language == "Java") return queryLanguage::java;
    throw std::invalid_argument("no queries for the language " + language);
}

// Builds the XPaths of every language
//
queryRegistry::queryRegistry(const options& settings) {
    xpaths[static_cast<std::size_t>(queryLanguage::cpp)]    = buildQueries(cppMarkup(), settings);
    xpaths[static_cast<std::size_t>(queryLanguage::csharp)] = buildQueries(csharpMarkup(), settings);
    xpaths[static_cast<std::size_t>(queryLanguage::java)]   = buildQueries(javaMarkup(), settings);
}

// Gets the XPath of a query, which is empty when the language has no such query
//
const std::string& queryRegistry::xpath(queryLanguage language, query name) const {
    return xpaths[static_cast<std::size_t>(language)][static_cast<std::size_t>(name)];
}
