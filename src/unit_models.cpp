// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file unit_models.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "unit_models.hpp"
#include "helper_functions.hpp"
#include "language_tables.hpp"
#include "node_text.hpp"
#include "qualified_name.hpp"
#include "query_engine.hpp"
#include "srcml_document.hpp"
#include "srcml_elements.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace {
    // Adds the parts of the name of a namespace or a package in front of a scope
    //
    void prependName(const xmlNode* owner, const std::string& unitLanguage, std::vector<std::string>& scope) {
        const xmlNode* nameElement = firstChildElement(owner, "name");
        if (!nameElement) return;

        std::string name = sourceOf(nameElement);
        HELPERS::removeWhitespace(name);
        const std::vector<std::string> parts = splitQualifiedName(name, unitLanguage);
        scope.insert(scope.begin(), parts.begin(), parts.end());
    }

    // Gets the namespaces or the package around an element of a unit, outermost first, where an anonymous namespace adds nothing
    // A Java package and a C# file-scoped namespace precede the types of the unit instead of holding them
    //
    std::vector<std::string> scopeOf(const xmlNode* element, const std::string& unitLanguage) {
        std::vector<std::string> scope;
        for (const xmlNode* node = element->parent; node && node->type == XML_ELEMENT_NODE; node = node->parent)
            if (isSrcElement(node, "namespace")) prependName(node, unitLanguage, scope);

        const xmlNode* unit = xmlDocGetRootElement(element->doc);
        for (const xmlNode* child = unit->children; child; child = child->next) {
            if (isSrcElement(child, "package") || (isSrcElement(child, "namespace") && !firstChildElement(child, "block"))) {
                prependName(child, unitLanguage, scope);
                break;
            }
        }
        return scope;
    }

    // Gets the C++ using directives of a unit at namespace scope, each with the namespaces around it and its place in the unit
    //
    std::vector<usingDirective> usingDirectivesOf(const srcmlDocument& unit, const std::string& unitLanguage) {
        std::vector<usingDirective> directives;
        if (unitLanguage != "C++") return directives;

        const nodeSet result = unit.select(query::usingDirective);
        for (int i = 0; i < result.size(); ++i) {
            usingDirective directive;
            prependName(firstChildElement(result.node(i), "namespace"), unitLanguage, directive.name);
            directive.scope    = scopeOf(result.node(i), unitLanguage);
            directive.position = unit.wrap(result.node(i)).path();
            directives.push_back(std::move(directive));
        }
        return directives;
    }

    // Gets the names of the types around a nested type, outermost first, each with the separator of its language after it, such as Outer:: or Outer.
    //
    std::string enclosingTypeNames(const xmlNode* type, const std::string& unitLanguage) {
        const std::string separator = unitLanguage == "C++" ? "::" : ".";
        std::string names;
        for (const xmlNode* node = type->parent; node && node->type == XML_ELEMENT_NODE; node = node->parent) {
            const xmlNode* nameElement = isTypeElement(node) ? typeNameElement(node) : nullptr;
            if (!nameElement) continue;

            std::string name = sourceOf(nameElement);
            HELPERS::removeWhitespace(name);
            names.insert(0, name + separator);
        }
        return names;
    }

    // Gets the last name of a name element without its qualifiers and generic arguments, such as Inner of Outer<T>::Inner
    //
    std::string lastNameOf(const xmlNode* nameElement, const std::string& unitLanguage) {
        std::string name = sourceOf(nameElement);
        HELPERS::removeWhitespace(name);
        return withoutGenericArguments(splitQualifiedName(name, unitLanguage).back());
    }

    // Gets the names of the named types around an element, whose static fields the code of the element reaches without their names
    //
    std::unordered_set<std::string> typesAround(const xmlNode* element, const std::string& unitLanguage) {
        std::unordered_set<std::string> names;
        for (const xmlNode* node = element->parent; node && node->type == XML_ELEMENT_NODE; node = node->parent) {
            const xmlNode* nameElement = isTypeElement(node) ? typeNameElement(node) : nullptr;
            if (nameElement) names.insert(lastNameOf(nameElement, unitLanguage));
        }
        return names;
    }

    // Gets the types of the static imports of a query in a C# or Java unit, such as Settings of using static Shop.Settings; or import static shop.Settings.level;
    // A Java import names a member of the type or *, while a C# import names the type
    //
    std::unordered_set<std::string> staticImportTypes(const srcmlDocument& unit, const std::string& unitLanguage, query imports) {
        std::unordered_set<std::string> types;
        if (unitLanguage == "C++") return types;

        const nodeSet result = unit.select(imports);
        for (int i = 0; i < result.size(); ++i) {
            std::string name = result.src(i);
            HELPERS::removeWhitespace(name);
            const std::vector<std::string> parts = splitQualifiedName(name, unitLanguage);
            if (unitLanguage == "C#")   types.insert(withoutGenericArguments(parts.back()));
            else if (parts.size() > 1)  types.insert(withoutGenericArguments(parts[parts.size() - 2]));
        }
        return types;
    }

    // The type or the namespace that holds a global, where the global namespace has an empty name
    //
    struct globalHolder {
        std::string name;
        bool        isNamespace{true};
    };

    // Gets the innermost named type or namespace around the declaration of a global, or else the global namespace
    // A variable declared with the type that it defines, as x of struct { } x; belongs to the scope around the type
    //
    globalHolder holderOf(const xmlNode* nameElement, const std::string& unitLanguage) {
        const xmlNode* node = nameElement->parent->parent;
        if (isTypeElement(node)) node = node->parent;
        for (; node && node->type == XML_ELEMENT_NODE; node = node->parent) {
            const bool     isType     = isTypeElement(node);
            const xmlNode* holderName = isType ? typeNameElement(node) : isSrcElement(node, "namespace") ? firstChildElement(node, "name") : nullptr;
            if (holderName) return {lastNameOf(holderName, unitLanguage), !isType};
        }
        return {};
    }

    // Tells whether the name of a declaration can name a variable: a name, or in C++ a name that the names of its holders qualify,
    // since srcML can mark parts of an expression as a declaration, as string.Format of a C# initializer
    //
    bool isVariableName(const std::string& name, const std::string& unitLanguage) {
        if (name.empty()) return false;
        const std::string bare = withoutGenericArguments(name);
        for (std::size_t i = 0; i < bare.size(); ++i) {
            if (unitLanguage == "C++" && bare.compare(i, 2, "::") == 0) ++i;
            else if (!std::isalnum(static_cast<unsigned char>(bare[i])) && bare[i] != '_' && bare[i] != '$' && bare[i] != '@') return false;
        }
        return true;
    }

    // Tells whether a file is a C or C++ source file, unlike a header, of which each unit that includes it holds a copy
    //
    bool isSourceFile(const std::string& fileName) {
        const std::size_t dot = fileName.rfind('.');
        if (dot == std::string::npos) return false;

        std::string extension = fileName.substr(dot + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return extension == "c" || extension == "cc" || extension == "cpp" || extension == "cxx" || extension == "c++" || extension == "cp";
    }

    // Tells whether a C# using directive is global, which holds in every file of the compilation, where srcML marks the word global as a macro before it
    //
    bool isGlobalUsing(const xmlNode* directive) {
        const xmlNode* before = xmlPreviousElementSibling(const_cast<xmlNode*>(directive));
        return isSrcElement(before, "macro") && sourceOf(before) == "global";
    }

    // Tells whether a C or C++ global has internal linkage, as a variable of an unnamed namespace or a static variable at namespace scope has
    //
    bool hasInternalLinkage(const xmlNode* nameElement, bool namespaceScope) {
        for (const xmlNode* node = nameElement->parent; node && node->type == XML_ELEMENT_NODE; node = node->parent)
            if (isSrcElement(node, "namespace") && !firstChildElement(node, "name")) return true;
        if (!namespaceScope) return false;

        // static qualifies the first declarator of a statement or the type that the declarators follow, as in static int a, b; or static union { };
        const xmlNode* owner = nameElement->parent->parent;
        if (isSrcElement(owner, "decl_stmt")) {
            if (declaresStatic(firstChildElement(firstChildElement(owner, "decl"), "type"))) return true;
            while (owner && !isTypeElement(owner)) owner = owner->parent;
        }
        return owner && declaresStatic(owner);
    }

    // Tells whether the specifiers of a C++ type before its first * or &, as in const Widget* or std::string const, make its object constant
    //
    bool namesConstObject(const xmlNode* type) {
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(type)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
            const std::string text = sourceOf(child);
            if (isSrcElement(child, "modifier") && (text == "*" || text == "&" || text == "&&")) return false;
            if (isSrcElement(child, "specifier") && (text == "const" || text == "constexpr")) return true;
        }
        return false;
    }

    // Tells whether a C# or Java type names a primitive type or a string, as int, string, or java.lang.String do
    //
    bool namesPrimitiveType(const xmlNode* type, const languageTables& tables, const std::string& unitLanguage) {
        const xmlNode* typeName = firstChildElement(type, "name");
        if (!typeName) return false;

        std::string name = sourceOf(typeName);
        HELPERS::removeWhitespace(name);
        HELPERS::removeNamespace(name, unitLanguage, true);
        return tables.primitiveTypes.isPrimitive(name, unitLanguage);
    }

    // Tells whether a Java declaration lies directly in an interface or an annotation, whose fields are final
    //
    bool inInterface(const xmlNode* declaration) {
        for (const xmlNode* node = declaration->parent; node && node->type == XML_ELEMENT_NODE; node = node->parent) {
            if (isSrcElement(node, "interface") || isSrcElement(node, "annotation_defn")) return true;
            if (isTypeElement(node)) return false;
        }
        return false;
    }

    // Tells whether no call changes a global: a C++ global whose object is const, a C# constant, or a C# readonly or Java final global
    // of a primitive type or a string, where the fields of a Java interface are final
    //
    bool isImmutable(const xmlNode* nameElement, const languageTables& tables, const std::string& unitLanguage) {
        const xmlNode* declaration = nameElement->parent;
        if (!isSrcElement(declaration, "decl")) return false;

        // A later declarator stands for the type of the first, as b of const int a, b; does
        const xmlNode* type = firstChildElement(declaration, "type");
        if (type && refersToPreviousType(type)) type = firstChildElement(firstChildElement(declaration->parent, "decl"), "type");
        if (!type) return false;
        if (unitLanguage == "C++") return namesConstObject(type);

        bool constant = false;
        bool readOnly = unitLanguage == "Java" && inInterface(declaration);
        for (const xmlNode* specifier = firstChildElement(type, "specifier"); specifier; specifier = nextNamedSibling(specifier, "specifier")) {
            const std::string text = sourceOf(specifier);
            constant = constant || (unitLanguage == "C#" && text == "const");
            readOnly = readOnly || text == (unitLanguage == "C#" ? "readonly" : "final");
        }
        return constant || (readOnly && namesPrimitiveType(type, tables, unitLanguage));
    }

    // Finds the globals of a unit, each with its holder, which a C++ definition outside its scope, as int Registry::count = 0;, names in its qualifier
    // A primitive type names no variable, though srcML can mark one as a declared name in a declaration that it misreads
    //
    void findGlobals(const srcmlDocument& unit, const languageTables& tables, const std::string& unitLanguage, int unitNumber, unitModels& models) {
        const bool sourceFile = unitLanguage == "C++" && isSourceFile(unit.fileName());
        const nodeSet result = unit.select(query::globalVariable);
        for (int i = 0; i < result.size(); ++i) {
            std::string name = result.src(i);
            HELPERS::removeWhitespace(name);
            HELPERS::removeBracketSuffix(name); // Chop off [] for arrays, as in int a[4] of C++ or static int a[] of Java
            if (!isVariableName(name, unitLanguage)) continue;

            const std::vector<std::string> parts = splitQualifiedName(name, unitLanguage);
            const std::string variable = withoutGenericArguments(parts.back());
            if (variable.empty() || tables.primitiveTypes.isPrimitive(variable, unitLanguage)) continue;

            const globalHolder holder = parts.size() > 1 ? globalHolder{withoutGenericArguments(parts[parts.size() - 2]), false} : holderOf(result.node(i), unitLanguage);
            const bool fileLocal = sourceFile && hasInternalLinkage(result.node(i), holder.isNamespace);
            models.globals.add({unitLanguage, variable, holder.name, unitNumber, holder.isNamespace, fileLocal, isImmutable(result.node(i), tables, unitLanguage)});
        }
    }

    // Finds the aliases of a unit outside the bodies of code that name a type of one name, the alias declarations and typedefs of C++ and the using alias
    // directives of C#, each in the scope of the namespaces and the types around it, which also holds the type that it names
    //
    void findAliases(const srcmlDocument& unit, const std::string& unitLanguage, int unitNumber, unitModels& models) {
        if (unitLanguage == "Java") return;

        const nodeSet result = unit.select(query::typeAliasName);
        for (int i = 0; i < result.size(); ++i) {
            const xmlNode* name        = result.node(i);
            const xmlNode* declaration = name->parent;
            const xmlNode* type        = isSrcElement(declaration, "typedef") ? firstChildElement(declaration, "type")
                                                                              : xmlFirstElementChild(firstChildElement(declaration, "init"));
            const std::string target = namedTypeOf(type);
            if (target.empty()) continue;

            // A pointer declarator, as *OwnerPtr of typedef std::unique_ptr<Engine> Owner, *OwnerPtr;, points to the smart pointer itself,
            // which -> then reaches in place of the object of the smart pointer
            const xmlNode* declarator = xmlPreviousElementSibling(const_cast<xmlNode*>(name));
            const std::string pointee = isSrcElement(declarator, "modifier") && sourceOf(declarator) == "*" ? target : pointeeTypeOf(type);

            std::vector<std::string> scope = scopeOf(declaration, unitLanguage);
            const std::size_t namespaces = scope.size();
            for (const std::string& part : splitQualifiedName(enclosingTypeNames(declaration, unitLanguage), unitLanguage))
                if (!part.empty()) scope.push_back(withoutGenericArguments(part));

            indexedAlias alias;
            alias.place.language   = unitLanguage;
            alias.place.path       = scope;
            alias.place.path.push_back(sourceOf(name));
            alias.place.unitNumber = unitNumber;
            alias.place.fileName   = unit.fileName();
            alias.place.position   = unit.wrap(name->parent).path();
            alias.target           = referenceOf(splitQualifiedName(target, unitLanguage), unitLanguage, scope, unitNumber, unit.fileName());
            alias.target.position  = alias.place.position;
            if (!pointee.empty()) {
                alias.pointee          = referenceOf(splitQualifiedName(pointee, unitLanguage), unitLanguage, scope, unitNumber, unit.fileName());
                alias.pointee.position = alias.place.position;
            }
            alias.enclosingTypes   = scope.size() - namespaces;
            // The units that include a header see its aliases, while the global namespace is shared by the programs of an archive
            alias.otherUnits       = unitLanguage == "C++" ? !scope.empty() && !isSourceFile(unit.fileName()) : isGlobalUsing(declaration);
            models.aliases.push_back(std::move(alias));
        }
    }

    // Gets the element of the type whose object encloses an object of a Java member class without static, or null
    // The member classes of an interface are static
    //
    const xmlNode* enclosingInstanceType(const xmlNode* type) {
        if (!isSrcElement(type, "class") || declaresStatic(type)) return nullptr;
        for (const xmlNode* node = type->parent; node && node->type == XML_ELEMENT_NODE; node = node->parent) {
            if (isSrcElement(node, "class") || isSrcElement(node, "enum")) return node;
            if (isSrcElement(node, "interface") || isSrcElement(node, "annotation_defn")) return nullptr;
        }
        return nullptr;
    }

    // Finds the C++ classes of a unit that srcML parsed as C, which hides each class in a declaration or an expression
    //
    void findCClasses(const srcmlDocument& unit, unitModels& models) {
        const nodeSet result = unit.select(query::cClassName);
        if (result.size() == 0) return;

        std::string names;
        for (int i = 0; i < result.size(); ++i) {
            models.cClassNames.push_back(result.src(i));
            names += (i == 0 ? "" : ", ") + models.cClassNames.back();
        }
        models.warning = unit.fileName() + ": srcML parsed this unit as C, which hides the classes " + names +
                         ", so their methods are unclassified; convert C++ headers with srcml --register-ext h=C++";
    }

    // Counts the functions of a C# or Java unit outside every type, which srcML leaves there only when it misparses the unit
    //
    void findFunctionsOutsideTypes(const srcmlDocument& unit, unitModels& models) {
        const int functions = unit.select(query::functionOutsideTypes).size();
        if (functions > 0)
            models.warning = unit.fileName() + ": " + std::to_string(functions) +
                             (functions == 1 ? " function lies outside every type in the srcML markup and is not analyzed"
                                             : " functions lie outside every type in the srcML markup and are not analyzed");
    }

    // Gets the name of the type that the qualifier of a C++ definition names, such as Box for Box<T>::reset, or an empty string
    // The model of a method defined outside its type then compares the types of its variables with its own type, as a method in its type does
    //
    std::string qualifierTypeName(const srcmlDocument& function, const std::string& unitLanguage) {
        if (unitLanguage != "C++") return "";
        const nodeSet result = function.select(query::methodName);
        if (result.size() == 0) return "";

        std::string name = result.src(0);
        HELPERS::removeWhitespace(name);
        const std::vector<std::string> parts = splitQualifiedName(name, unitLanguage);
        return parts.size() < 2 ? "" : withoutGenericArguments(parts[parts.size() - 2]);
    }

    // Finds types in a unit
    //
    // In C++, type names are usually in the form of:
    //      myType
    // or
    //      myType<type1, type2, ...> for a specialized templated type from myType
    //
    // In C# and Java:
    //      myType
    // or
    //      myType<T, G, ... > for a generic type
    //
    // Unlike C++, C# and Java can allow multiple types with the same name but
    //  different number of generic parameters to exist
    // For example, foo<T> and foo<T, T1> are valid
    //
    // A type nested in another type is a type of its own, whose name the names of the types around it qualify, such as Outer::Inner or Outer.Inner
    // Local types, which a body of code holds, and anonymous types are skipped
    //
    // C++:
    //  Unions can be declared anonymous (union without a name)
    //  Anonymous unions can only be declared inside a type, struct, or a namespace and cannot have methods
    //   and their data members are accessed directly as part of the enclosing scope
    //   so, data members that are defined inside anonymous unions are considered and the anonymous union itself is not considered
    //   also, anonymous unions that are defined in a namespace are not considered because their data members are basically globals
    //  Classes and structs without a name (anonymous types), which declare an instance at their definition, are skipped
    // C#:
    //  Allows static types to be declared, and these must contain only static data members
    //   They are ignored and their methods are collected as free functions
    // Java:
    //  enums in Java can contain methods and data members
    //  The static methods of a nested class, static or not, are collected as free functions
    //  Anonymous classes (classes without names and are nested as instances) are ignored
    //  A class nested without static is an inner class, whose objects reach the data members of an enclosing object, which the types
    //   of the unit hold before it unless the run leaves out its kind of type, while member records, enums, and interfaces are static
    //
    void findTypes(const srcmlDocument& unit, const languageTables& tables, const std::string& unitLanguage, int unitNumber,
                   const std::unordered_set<std::string>& staticImports, unitModels& models) {
        const nodeSet result = unit.select(query::type);
        std::unordered_map<const xmlNode*, std::size_t> positions;

        for (int i = 0; i < result.size(); i++) {
            const srcmlDocument typeUnit = unit.wrap(result.node(i));

            // The type query selects only types with a name
            typeModel type(tables, unitLanguage, enclosingTypeNames(result.node(i), unitLanguage) + sourceOf(typeNameElement(result.node(i))));
            type.findData(typeUnit, unitNumber, scopeOf(result.node(i), unitLanguage));
            type.addStaticHolders(typesAround(result.node(i), unitLanguage));
            type.addStaticHolders(staticImports);

            std::optional<std::size_t> enclosing;
            if (unitLanguage == "Java" && type.getStructure() == "class") {
                const auto found = positions.find(enclosingInstanceType(result.node(i)));
                if (found != positions.end()) enclosing = found->second;
            }
            positions.emplace(result.node(i), models.types.size());
            models.enclosingTypes.push_back(enclosing);
            models.types.push_back(std::move(type));
        }
    }

    // C++ only
    //
    // Finds free functions as well as methods defined externally
    //
    // Cases for functions and methods (defined externally)
    //   Cases:
    //      Method (Foo) could belong to a specialized templated type:
    //          template<typename T> type myType{}; // Generic templated type.
    //          template<optional> type myType<int>{}; // Specialized templated type from the generic one.
    //          void myType<int>::Foo(){} // Belongs to the specialized templated type.
    //      Method could belong to the generic templated type
    //          template<typename T> type myType{}; // Generic templated type.
    //          template<typename T> void myType<T>::Foo(){} // Generic templated method belongs to generic templated type.
    //          template<optional>   void myType<int>::Foo(){} // Specialized templated method belongs to a generic templated type.
    //           If a specialized templated type is defined for <int> then the previous method won't be allowed. It must be replaced with:
    //           void myType<int>::Foo(){}
    //      Method could be contained in a namespace
    //          namespace::myType<int>::Foo(){}
    //      Method could belong to a normal type
    //          myType::Foo(){}
    //      Function could be a free function (including normal free functions, friend functions, static methods, methods defined for external types)
    //          Foo(){}, namespace::Foo(){}, static Foo(){}, externalType::Foo(){},
    //
    void findFreeFunctions(const srcmlDocument& unit, const languageTables& tables, const std::string& unitLanguage, int unitNumber,
                           const std::unordered_set<std::string>& staticImports, unitModels& models) {
        const nodeSet result = unit.select(query::freeFunction);
        const std::vector<usingDirective> directives = usingDirectivesOf(unit, unitLanguage);

        for (int i = 0; i < result.size(); i++) {
            const std::string functionSrcml = result.srcml(i);
            const srcmlDocument functionUnit = unit.wrap(result.node(i));

            const std::string qualifierType = qualifierTypeName(functionUnit, unitLanguage);
            functionModel function(tables, functionUnit, elementLocation{unitNumber, functionUnit.path()}, unitLanguage, qualifierType, "", "",
                                   HELPERS::extractFirstLineNumber(functionSrcml), false);

            // The definition of a C++ static method outside its type, as Registry::count(), reaches the static fields of the type
            std::unordered_set<std::string> holders = typesAround(result.node(i), unitLanguage);
            if (!qualifierType.empty()) holders.insert(qualifierType);
            holders.insert(staticImports.begin(), staticImports.end());
            function.addStaticHolders(holders);
            if (!function.getName().empty()) {
                models.freeFunctions.push_back(std::move(function));
                models.freeFunctionScopes.push_back(scopeOf(result.node(i), unitLanguage));
                models.freeFunctionDirectives.push_back(qualifierType.empty() ? std::vector<usingDirective>{} : directives);
            }
        }
    }
}

// Parses the srcML of a unit and finds its types, free functions, and globals, and a warning for markup that hides code from the analysis
// The models read only the unit and the tables, so units can be analyzed on separate threads, each with a query engine of its own
//
unitModels findUnitModels(queryEngine& engine, const languageTables& tables, std::string_view srcml, const std::string& unitLanguage, bool cUnit, int unitNumber) {
    const srcmlDocument unit(engine, toQueryLanguage(unitLanguage), srcml);

    unitModels models;
    const std::unordered_set<std::string> staticImports = staticImportTypes(unit, unitLanguage, query::staticImport);
    if (unitLanguage == "C#") {
        models.globalStaticImports = staticImportTypes(unit, unitLanguage, query::globalStaticImport);
        const nodeSet valueTypes = unit.select(query::valueTypeName);
        for (int i = 0; i < valueTypes.size(); ++i) models.valueTypes.insert(valueTypes.src(i));
    }
    findTypes(unit, tables, unitLanguage, unitNumber, staticImports, models);
    findFreeFunctions(unit, tables, unitLanguage, unitNumber, staticImports, models);
    findGlobals(unit, tables, unitLanguage, unitNumber, models);
    findAliases(unit, unitLanguage, unitNumber, models);
    if (cUnit) findCClasses(unit, models);
    else if (unitLanguage != "C++") findFunctionsOutsideTypes(unit, models);
    return models;
}
