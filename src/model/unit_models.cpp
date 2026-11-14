// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file unit_models.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/unit_models.hpp"
#include "helper_functions.hpp"
#include "rules/language_tables.hpp"
#include "archive/node_text.hpp"
#include "model/javascript_elements.hpp"
#include "model/python_elements.hpp"
#include "model/qualified_name.hpp"
#include "archive/query_engine.hpp"
#include "archive/source_language.hpp"
#include "archive/srcml_document.hpp"
#include "archive/srcml_elements.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

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

    // Gets the using directives of a unit at namespace scope, which only C++ has, each with the namespaces around it and its place in the unit
    //
    std::vector<usingDirective> usingDirectivesOf(const srcmlDocument& unit, const std::string& unitLanguage) {
        std::vector<usingDirective> directives;
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

    // Gets the type that a C# extension method extends, the type of its first parameter with this, without qualifiers and with its generic
    // arguments reduced to their commas, as Basket of IsEmpty(this Basket basket) and EventHandler<> of (this EventHandler<T> handler), or nothing
    //
    std::string extendedTypeOf(const xmlNode* function, const std::string& unitLanguage) {
        const xmlNode* parameter = firstChildElement(firstChildElement(function, "parameter_list"), "parameter");
        const xmlNode* type      = firstChildElement(firstChildElement(parameter, "decl"), "type");
        const xmlNode* specifier = firstChildElement(type, "specifier");
        const xmlNode* name      = firstChildElement(type, "name");
        if (!specifier || sourceOf(specifier) != "this" || !name) return "";

        for (const xmlNode* next = name; next; next = nextNamedSibling(next, "name")) name = next;
        std::string written = sourceOf(name);
        HELPERS::removeWhitespace(written);
        return genericSkeleton(splitQualifiedName(written, unitLanguage).back());
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

    // Gets the name of the innermost named type around an element, or an empty string
    //
    std::string innermostTypeName(const xmlNode* element, const std::string& unitLanguage) {
        for (const xmlNode* node = element->parent; node && node->type == XML_ELEMENT_NODE; node = node->parent) {
            const xmlNode* nameElement = isTypeElement(node) ? typeNameElement(node) : nullptr;
            if (nameElement) return lastNameOf(nameElement, unitLanguage);
        }
        return "";
    }

    // Gets the types of the static imports of a query, which only C# and Java units have, such as Settings of using static Shop.Settings; or import static shop.Settings.level;
    // A Java import names a member of the type or *, while a C# import names the type
    //
    std::unordered_set<std::string> staticImportTypes(const srcmlDocument& unit, const std::string& unitLanguage, query imports) {
        std::unordered_set<std::string> types;
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

    // Adds the imports of a query that bring types into a unit by their names, which only C# and Java units have
    // A C# using directive names a namespace, and a Java import names one type or, with *, every type of a package or a type
    //
    void findTypeImports(const srcmlDocument& unit, const std::string& unitLanguage, int unitNumber, query imports, bool everyUnit, unitModels& models) {
        const nodeSet result = unit.select(imports);
        for (int i = 0; i < result.size(); ++i) {
            std::string name = result.src(i);
            HELPERS::removeWhitespace(name);

            typeImport import;
            import.path       = splitQualifiedName(name, unitLanguage);
            import.everyType  = unitLanguage == "C#" || import.path.back() == "*";
            import.unitNumber = unitNumber;
            import.everyUnit  = everyUnit;
            if (import.path.back() == "*") import.path.pop_back();
            models.typeImports.push_back(std::move(import));
        }
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
            else if (unitLanguage == "JavaScript" && i == 0 && bare[i] == '#') continue;
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

    // Tells whether a JavaScript declaration is const and holds a primitive value, by its annotation or by a literal as its initializer
    //
    bool isConstantPrimitive(const xmlNode* declaration, const languageTables& tables) {
        xmlChar* keyword = xmlGetProp(declaration, BAD_CAST "type");
        const bool constant = keyword && xmlStrEqual(keyword, BAD_CAST "const");
        xmlFree(keyword);
        if (!constant) return false;

        if (const xmlNode* annotation = typeAnnotationOf(declaration)) return tables.primitiveTypes.isPrimitive(javascriptTypeOf(annotation), "JavaScript");
        const xmlNode* initializer = firstChildElement(declaration, "init");
        const xmlNode* value       = initializer ? firstChildElement(initializer, "expr") : nullptr;
        const xmlNode* first       = value ? xmlFirstElementChild(const_cast<xmlNode*>(value)) : nullptr;
        return first && !xmlNextElementSibling(const_cast<xmlNode*>(first)) &&
               (isSrcElement(first, "literal") || (isSrcElement(first, "name") && (sourceOf(first) == "true" || sourceOf(first) == "false")));
    }

    // Tells whether no call changes a global: a C++ global whose object is const, a C# constant, a C# readonly or Java final global
    // of a primitive type or a string, where the fields of a Java interface are final, or a JavaScript const global of a primitive value
    //
    bool isImmutable(const xmlNode* nameElement, const languageTables& tables, const std::string& unitLanguage) {
        const xmlNode* declaration = nameElement->parent;
        if (!isSrcElement(declaration, "decl")) return false;
        if (unitLanguage == "JavaScript") return isConstantPrimitive(declaration, tables);

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
    // A name that the file of a module imports names a module or a member of one, also where the file binds the name again,
    // as ssl = None does once import ssl fails
    //
    void findGlobals(const srcmlDocument& unit, const languageTables& tables, const std::string& unitLanguage, int unitNumber, unitModels& models) {
        const bool sourceFile = unitLanguage == "C++" && isSourceFile(unit.fileName());
        const bool module     = languageOf(unitLanguage).modules;
        const std::string moduleName = module ? moduleNameOf(unit.fileName()) : "";

        std::unordered_set<std::string> importedNames;
        const nodeSet imports = unit.select(query::importedName);
        for (int i = 0; i < imports.size(); ++i) importedNames.insert(imports.src(i));

        const nodeSet result = unit.select(query::globalVariable);
        for (int i = 0; i < result.size(); ++i) {
            std::string name = result.src(i);
            HELPERS::removeWhitespace(name);
            HELPERS::removeBracketSuffix(name); // Chop off [] for arrays, as in int a[4] of C++ or static int a[] of Java
            if (!isVariableName(name, unitLanguage)) continue;

            const std::vector<std::string> parts = splitQualifiedName(name, unitLanguage);
            const std::string variable = withoutGenericArguments(parts.back());
            if (variable.empty() || tables.primitiveTypes.isPrimitive(variable, unitLanguage)) continue;

            globalHolder holder = parts.size() > 1 ? globalHolder{withoutGenericArguments(parts[parts.size() - 2]), false} : holderOf(result.node(i), unitLanguage);

            // The name of a module qualifies its globals for the code of other files, and a global statement in a function names a global of its module
            if (module && (holder.name.empty() || isSrcElement(result.node(i)->parent, "global"))) holder = {moduleName, true};
            if (holder.isNamespace && importedNames.count(variable) > 0) continue;
            const bool fileLocal = sourceFile && hasInternalLinkage(result.node(i), holder.isNamespace);
            models.globals.add({unitLanguage, variable, holder.name, unitNumber, holder.isNamespace, fileLocal, isImmutable(result.node(i), tables, unitLanguage)});
        }
    }

    // Tells whether a type holds the definition of a type, as struct { } of typedef struct { } Point; does
    //
    bool definesType(const xmlNode* type) {
        for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(type)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child)))
            if (isTypeElement(child)) return true;
        return false;
    }

    // Gets the modifiers of a declarator of a typedef after the first without whitespace, as * of *CountPtr and *const of *const CountCPtr
    // in typedef int Count, *CountPtr, *const CountCPtr;, or an empty text for the first declarator, whose modifiers belong to its type
    //
    std::string declaratorModifiers(const xmlNode* name) {
        std::string modifiers;
        for (const xmlNode* node = name->prev; node && (node->type != XML_ELEMENT_NODE || isSrcElement(node, "modifier")); node = node->prev) {
            std::string text = sourceOf(node);
            const std::size_t comma = text.rfind(',');
            if (comma != std::string::npos) text.erase(0, comma + 1);
            HELPERS::removeWhitespace(text);
            modifiers.insert(0, text);
            if (comma != std::string::npos) break;
        }
        return modifiers;
    }

    // Finds the aliases of types of a unit outside the bodies of code, the alias declarations and typedefs of C++ and the using alias directives of C#,
    // each in the scope of the namespaces and the types around it, which also holds the type that it names
    // A typedef of a type that it defines, as typedef struct { } Point;, stands for no type that a signature writes
    //
    void findAliases(const srcmlDocument& unit, const std::string& unitLanguage, int unitNumber, unitModels& models) {
        const nodeSet result = unit.select(query::typeAliasName);
        for (int i = 0; i < result.size(); ++i) {
            const xmlNode* name        = result.node(i);
            const xmlNode* declaration = name->parent;
            const xmlNode* type        = isSrcElement(declaration, "typedef") ? firstChildElement(declaration, "type")
                                                                              : xmlFirstElementChild(firstChildElement(declaration, "init"));
            if (!type || definesType(type)) continue;
            const std::string target = namedTypeOf(type);

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
            alias.target           = referenceOf(target.empty() ? std::vector<std::string>{} : splitQualifiedName(target, unitLanguage), unitLanguage, scope,
                                                 unitNumber, unit.fileName());
            alias.target.position  = alias.place.position;
            alias.spelledType      = writtenTypeOf(type);
            if (isSrcElement(declaration, "typedef")) addText(alias.spelledType, declaratorModifiers(name));
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

    // Tells whether an element declares or defines a C++ type or an alias, whose name element names the type
    //
    bool declaresTypeName(const xmlNode* element) {
        for (std::string_view kind : {"class", "struct", "union", "enum", "class_decl", "struct_decl", "union_decl", "enum_decl", "typedef"})
            if (isSrcElement(element, kind)) return true;
        return isSrcElement(element, "using") && firstChildElement(element, "init");
    }

    // Adds the name of a type without its scopes and template arguments, as Nodes of typedef Node Nodes[4];
    //
    void addDeclaredTypeName(const xmlNode* nameElement, unitModels& models) {
        if (!nameElement) return;
        std::string name = sourceOf(nameElement);
        HELPERS::removeWhitespace(name);
        HELPERS::removeBracketSuffix(name);
        if (!name.empty()) models.declaredTypeNames.insert(withoutGenericArguments(splitQualifiedName(name, "C++").back()));
    }

    // Finds the names of the C++ types and aliases that a unit declares or defines, also of a typedef of a function
    // One walk over the elements costs a fraction of an XPath query that tests every element of the unit
    //
    void findDeclaredTypeNames(const srcmlDocument& unit, unitModels& models) {
        std::vector<const xmlNode*> pending{unit.unitElement()};
        while (!pending.empty()) {
            const xmlNode* parent = pending.back();
            pending.pop_back();
            for (const xmlNode* child = xmlFirstElementChild(const_cast<xmlNode*>(parent)); child; child = xmlNextElementSibling(const_cast<xmlNode*>(child))) {
                // A typedef names a type in each of its declarators, as Count and CountPtr of typedef int Count, *CountPtr;
                if (declaresTypeName(child))
                    for (const xmlNode* part = xmlFirstElementChild(const_cast<xmlNode*>(child)); part; part = xmlNextElementSibling(const_cast<xmlNode*>(part)))
                        if (isSrcElement(part, "name")) addDeclaredTypeName(part, models);
                if (const xmlNode* function = isSrcElement(child, "typedef") ? firstChildElement(child, "function_decl") : nullptr)
                    addDeclaredTypeName(firstChildElement(function, "name"), models);
                pending.push_back(child);
            }
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

    // Counts the functions outside every type of a unit of C# or Java, where every function belongs to a type, which srcML leaves there only when it misparses the unit
    // Each type of the unit may hold such a function, so a call without an object may name it
    // A static function among them gets its stereotype as a free function, as every static method does
    //
    void findFunctionsOutsideTypes(const srcmlDocument& unit, unitModels& models) {
        const nodeSet outside = unit.select(query::functionOutsideTypes);
        std::unordered_set<std::string> names;
        int functions = 0;
        for (int i = 0; i < outside.size(); ++i) {
            names.insert(withoutGenericArguments(sourceOf(firstChildElement(outside.node(i), "name"))));
            const xmlNode* type = firstChildElement(outside.node(i), "type");
            if (!type || !declaresStatic(type)) ++functions;
        }
        for (typeModel& type : models.types) type.addMisreadMethods(names);

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
    //   of the unit hold before it whatever the options say, while member records, enums, and interfaces are static
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
            type.addStaticImports(staticImports);

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

            // A function in the body of a class of a dynamic language, as a static method of Python, compares the types of its variables with the class
            const bool dynamic = languageOf(unitLanguage).dynamic;
            const std::string qualifierType = dynamic ? innermostTypeName(result.node(i), unitLanguage) : qualifierTypeName(functionUnit, unitLanguage);
            functionModel function(tables, functionUnit, elementLocation{unitNumber, functionUnit.path()}, unitLanguage, qualifierType, "", "",
                                   HELPERS::extractFirstLineNumber(functionSrcml), false);

            // The definition of a C++ static method outside its type, as Registry::count(), reaches the static fields of the type,
            // while the code of a dynamic language reaches a variable of a class only through the name of the class
            std::unordered_set<std::string> holders;
            if (!dynamic) {
                holders = typesAround(result.node(i), unitLanguage);
                if (!qualifierType.empty()) holders.insert(qualifierType);
                holders.insert(staticImports.begin(), staticImports.end());
            }
            function.addStaticHolders(holders);
            if (!function.getName().empty()) {
                // A function in a type is a static method of the type, which a call without an object names in the type
                const std::string holder = dynamic ? "" : innermostTypeName(result.node(i), unitLanguage);
                if (!holder.empty())
                    models.staticMethods.push_back({unitLanguage + ' ' + holder, function.getName()[3], function.getArguments(), extendedTypeOf(result.node(i), unitLanguage)});
                models.freeFunctions.push_back(std::move(function));
                models.freeFunctionScopes.push_back(scopeOf(result.node(i), unitLanguage));
                models.freeFunctionDirectives.push_back(qualifierType.empty() ? std::vector<usingDirective>{} : directives);
            }
        }
    }
}

namespace {
    // Finds the static methods that the types of a unit declare without a body, as an extern method of C#, a native method of Java,
    // and a C++ method defined outside its type
    //
    void findStaticMethodDeclarations(const srcmlDocument& unit, const std::string& unitLanguage, unitModels& models) {
        const nodeSet declared = unit.select(query::staticMethodDeclaration);
        for (int i = 0; i < declared.size(); ++i) {
            const std::string name = withoutGenericArguments(sourceOf(firstChildElement(declared.node(i), "name")));
            models.staticMethods.push_back({unitLanguage + ' ' + innermostTypeName(declared.node(i), unitLanguage), name,
                                            argumentRangeOf(firstChildElement(declared.node(i), "parameter_list")), ""});
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
        // The lookup reads the type of a parameter without its generic arguments, as Slot of Slot<string>
        for (int i = 0; i < valueTypes.size(); ++i) models.valueTypes.insert(withoutGenericArguments(valueTypes.src(i)));
    }
    findTypes(unit, tables, unitLanguage, unitNumber, staticImports, models);
    findFreeFunctions(unit, tables, unitLanguage, unitNumber, staticImports, models);
    findStaticMethodDeclarations(unit, unitLanguage, models);
    findGlobals(unit, tables, unitLanguage, unitNumber, models);
    findAliases(unit, unitLanguage, unitNumber, models);
    findTypeImports(unit, unitLanguage, unitNumber, query::typeImport, false, models);
    findTypeImports(unit, unitLanguage, unitNumber, query::globalTypeImport, true, models);
    if (unitLanguage == "C++") findDeclaredTypeNames(unit, models);
    if (cUnit) findCClasses(unit, models);
    else findFunctionsOutsideTypes(unit, models);
    return models;
}
