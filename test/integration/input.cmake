# SPDX-License-Identifier: GPL-3.0-only
# @file input.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Integration checks of the input archives, with missing attributes, damaged input, unusual markup, and labeled input
# test/CMakeLists.txt registers each function check_<name> as the test integration/input/<name>

include(${CMAKE_CURRENT_LIST_DIR}/run_and_check.cmake)

# srcML read from standard input or given with --text has no filename attribute, and such a unit is labeled like any other
function(check_no_filename)
    string(REPLACE " filename=\"sample.cpp\"" "" INPUT "${SAMPLE_INPUT}")
    string(REPLACE " filename=\"sample.cpp\"" "" EXPECTED_OUTPUT "${SAMPLE_OUTPUT}")
    if("${INPUT}" STREQUAL "${SAMPLE_INPUT}")
        message(FATAL_ERROR "The sample archive has no filename attribute to remove")
    endif()
    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${EXPECTED_OUTPUT}")
endfunction()

# A unit without a language attribute is written without stereotypes, while the other units of the archive are labeled
# libsrcml writes language="" for the unit that had no language attribute
#
# The archive holds a.cpp and b.cpp, and the unit of b.cpp has no language attribute:
#   class A {                          class B {
#       int x;                             int y;
#   public:                            public:
#       int getX() const { return x; }     void setY(int v) { y = v; }
#   };                                 };
function(check_no_language)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit revision="1.0.0" language="C++" filename="a.cpp" hash="ef75ab7ce29800ea9e18a77a6aa6520e5fbae4e1"><class>class <name>A</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit revision="1.0.0" filename="b.cpp" hash="9dd93ce2ad9242f07f77c5c808a2c04ed7f288bf"><class>class <name>B</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>y</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>void</name></type> <name>setY</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><name>y</name> <operator>=</operator> <name>v</name></expr>;</expr_stmt> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

</unit>
]=])

    set(EXPECTED_OUTPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="a.cpp" hash="ef75ab7ce29800ea9e18a77a6aa6520e5fbae4e1"><class st:stereotype="data-provider data-class small-class">class <name>A</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</private><public>public:
    <function st:stereotype="get"><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="" filename="b.cpp" hash="9dd93ce2ad9242f07f77c5c808a2c04ed7f288bf"><class>class <name>B</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>y</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>void</name></type> <name>setY</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><name>y</name> <operator>=</operator> <name>v</name></expr>;</expr_stmt> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${EXPECTED_OUTPUT}")
endfunction()

# Input that is not a srcML archive stops the run with an error and writes no output
# The cases are a Git LFS pointer (a clone without git lfs pull), an empty file, XML that is not srcML, a folder, and a missing file
# The command line rejects the folder and the missing file with the exit code of an invalid option value
function(check_invalid_input)
    file(WRITE "${WORK_DIR}/lfs.xml" "version https://git-lfs.github.com/spec/v1\noid sha256:4d7a214614ab2935c943f9e0ff69d22eadbb8f32b1258daaa5e2ca24d17e2393\nsize 12345\n")
    file(WRITE "${WORK_DIR}/empty.xml" "")
    file(MAKE_DIRECTORY "${WORK_DIR}/folder.xml")
    file(WRITE "${WORK_DIR}/plain.xml" "<?xml version=\"1.0\"?>\n<root><a/></root>\n")

    foreach(CASE lfs empty plain folder missing)
        run_stereocode(${CASE}.xml -f)
        if(CASE STREQUAL "folder" OR CASE STREQUAL "missing")
            expect_exit(105)
        else()
            expect_error()
        endif()
        expect_missing("${WORK_DIR}/${CASE}.stereotypes.xml")
        expect_missing("${WORK_DIR}/${CASE}.stereotypes.xml.tmp")
    endforeach()
endfunction()

# An archive that ends inside a unit, or after a unit but before its end tag, stops the run with an error and writes no output
# libsrcml reads such an archive as if it ended after its last complete unit, and it prints its own message first
#
# The archive holds a.cpp and b.cpp:
#   int f() { return 1; }              int g() { return 2; }
function(check_damaged_input)
    set(ARCHIVE_START [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit revision="1.0.0" language="C++" filename="a.cpp"><function><type><name>int</name></type> <name>f</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">1</literal></expr>;</return> </block_content>}</block></function>
</unit>
]=])

    set(SECOND_UNIT_START [=[

<unit revision="1.0.0" language="C++" filename="b.cpp"><function><type><name>int</name></type> <name>g</name>]=])

    file(WRITE "${WORK_DIR}/inside_unit.xml" "${ARCHIVE_START}${SECOND_UNIT_START}")
    file(WRITE "${WORK_DIR}/after_unit.xml" "${ARCHIVE_START}")

    foreach(CASE inside_unit after_unit)
        run_stereocode(${CASE}.xml -f -r)
        expect_exit(1)
        if(NOT "${RUN_ERROR}" MATCHES "(^|\n)Error: ${CASE}.xml is not well-formed XML at line [0-9]+: ")
            message(FATAL_ERROR "The error does not name the damaged archive: ${RUN_ERROR}")
        endif()
        expect_missing("${WORK_DIR}/${CASE}.stereotypes.xml")
        expect_missing("${WORK_DIR}/${CASE}.stereotypes.xml.tmp")
        expect_missing("${WORK_DIR}/${CASE}.stereotypes.csv")
    endforeach()
endfunction()

# An archive without a unit in C, C++, C#, Java, or Python gives a warning and an output without stereotypes
# libsrcml reads an archive without units as one unit without a language and writes revision="" and language=""
function(check_archive_without_code)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0"/>
]=])

    set(EXPECTED_OUTPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:st="http://www.srcML.org/srcML/stereotype" revision="" language=""/>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f -r)
    expect_exit(0)
    if(NOT "${RUN_ERROR}" MATCHES "^Warning: input.xml holds no unit in C, C\\+\\+, C#, Java, or Python\n$")
        message(FATAL_ERROR "Expected a warning about the missing code, got '${RUN_ERROR}'")
    endif()
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${EXPECTED_OUTPUT}")
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "type_name,type_stereotype,function_name,function_stereotype\n")
    expect_file_content("${WORK_DIR}/input.free_functions.stereotypes.csv" "type_name,type_stereotype,function_name,function_stereotype\n")
endfunction()

# Markup that hides code from the analysis gets one warning per unit: a C++ header that srcML parsed as C,
# whose methods are unclassified, and a C# unit with a function outside every type
#
# The files:
#   counter.h   class Counter { int m_count; public: void setCount(int c); };
#   counter.cpp void Counter::setCount(int c) { m_count = c; }
#   Damaged.cs  class Stock : Shelf { int Twice() { return 2 * Size(); } } and the functions Size and static Depth outside every type,
#               as srcML leaves them after a misparse, where the static function gets its stereotype as a free function
#   Program.cs  System.Console.WriteLine(Twice(2)); int Twice(int x) { return x * 2; }, whose function among top-level statements
#               is a free function and no damaged markup
function(check_damaged_markup)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit revision="1.0.0" language="C" filename="counter.h"><decl_stmt><decl><type><name>class</name></type> <name>Counter</name> <block>{<block_content>
  <decl_stmt><decl><type><name>int</name></type> <name>m_count</name></decl>;</decl_stmt>
<label><name>public</name>:</label>
  <function_decl><type><name>void</name></type> <name>setCount</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>c</name></decl></parameter>)</parameter_list>;</function_decl>
</block_content>}</block></decl></decl_stmt><empty_stmt>;</empty_stmt>
</unit>

<unit revision="1.0.0" language="C++" filename="counter.cpp"><function><type><name>void</name></type> <name><name>Counter</name><operator>::</operator><name>setCount</name></name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>c</name></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><name>m_count</name> <operator>=</operator> <name>c</name></expr>;</expr_stmt> </block_content>}</block></function>
</unit>

<unit revision="1.0.0" language="C#" filename="Damaged.cs"><class>class <name>Stock</name> <super_list>: <super><name>Shelf</name></super></super_list> <block>{ <function><type><name>int</name></type> <name>Twice</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">2</literal> <operator>*</operator> <call><name>Size</name><argument_list>()</argument_list></call></expr>;</return> </block_content>}</block></function> }</block></class>
<function><type><name>int</name></type> <name>Size</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">0</literal></expr>;</return> </block_content>}</block></function>
<function><type><specifier>static</specifier> <name>int</name></type> <name>Depth</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">1</literal></expr>;</return> </block_content>}</block></function>
</unit>

<unit revision="1.0.0" language="C#" filename="Program.cs"><expr_stmt><expr><call><name><name>System</name><operator>.</operator><name>Console</name><operator>.</operator><name>WriteLine</name></name><argument_list>(<argument><expr><call><name>Twice</name><argument_list>(<argument><expr><literal type="number">2</literal></expr></argument>)</argument_list></call></expr></argument>)</argument_list></call></expr>;</expr_stmt>
<function><type><name>int</name></type> <name>Twice</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>x</name></decl></parameter>)</parameter_list> <block>{<block_content> <return>return <expr><name>x</name> <operator>*</operator> <literal type="number">2</literal></expr>;</return> </block_content>}</block></function>
</unit>

</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f)
    expect_exit(0)

    string(REGEX MATCHALL "Warning: " warnings "${RUN_ERROR}")
    list(LENGTH warnings warningCount)
    if(NOT warningCount EQUAL 2)
        message(FATAL_ERROR "Expected one warning for counter.h and one for Damaged.cs, got:\n${RUN_ERROR}")
    endif()
    if(NOT RUN_ERROR MATCHES "Warning: counter\\.h: [^\n]*Counter[^\n]*--register-ext h=C\\+\\+")
        message(FATAL_ERROR "Expected a warning that names the class Counter and the option --register-ext h=C++, got:\n${RUN_ERROR}")
    endif()
    if(NOT RUN_ERROR MATCHES "Warning: Damaged\\.cs: 1 function lies outside every type in the srcML markup and is not analyzed")
        message(FATAL_ERROR "Expected a warning for the function outside every type of Damaged.cs, got:\n${RUN_ERROR}")
    endif()

    file(READ "${WORK_DIR}/input.stereotypes.xml" output)
    if(NOT output MATCHES "<function st:stereotype=\"unclassified\"><type><name>void</name></type> <name><name>Counter</name>")
        message(FATAL_ERROR "Expected Counter::setCount to be unclassified:\n${output}")
    endif()
    # The class around the call may hold the function outside every type, so the call names no method of the parent Shelf outside the archive
    if(NOT output MATCHES "<function st:stereotype=\"[a-z -]+\"><type><specifier>static</specifier> <name>int</name></type> <name>Depth</name>")
        message(FATAL_ERROR "Expected the static function Depth outside every type to get a stereotype:\n${output}")
    endif()
    if(NOT output MATCHES "<function st:stereotype=\"wrapper stateless\"><type><name>int</name></type> <name>Twice</name>")
        message(FATAL_ERROR "Expected the call of Size in Twice to stay a call of a function:\n${output}")
    endif()
    if(NOT output MATCHES "<function st:stereotype=\"property\"><type><name>int</name></type> <name>Twice</name><parameter_list>\\(<parameter>")
        message(FATAL_ERROR "Expected the function Twice among the top-level statements of Program.cs to get a stereotype:\n${output}")
    endif()
endfunction()

# srcML writes a control character as an escape element, which the analysis reads inside a returned expression
#
# The C++ file, with a form feed in the string literal:
#   class A {
#       int x;
#   public:
#       int getX() const { return x; }
#   };
#
#   const char* page() { return "a\fb"; }
function(check_escape_in_expression)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="escape.cpp"><class>class <name>A</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>

<function><type><specifier>const</specifier> <name>char</name><modifier>*</modifier></type> <name>page</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><literal type="string">"a<escape char="0x0c"/>b"</literal></expr>;</return> </block_content>}</block></function>
</unit>
]=])

    set(EXPECTED_OUTPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="escape.cpp"><class st:stereotype="data-provider data-class small-class">class <name>A</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</private><public>public:
    <function st:stereotype="get"><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>

<function st:stereotype="literal"><type><specifier>const</specifier> <name>char</name><modifier>*</modifier></type> <name>page</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><literal type="string">"a<escape char="0x0c"/>b"</literal></expr>;</return> </block_content>}</block></function>
</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${EXPECTED_OUTPUT}")
endfunction()

# Each local takes the type of its own declaration, also after a declaration that srcML misparses with a second name,
# as it does for a lambda in an initializer of catch.hpp
#
# The C++ file, where the declaration of w holds the names w and parent:
#   class Gauge {
#       int level;
#   public:
#       void sample() {
#           Widget w = source parent;
#           int n = 0;
#           n = level;
#       }
#   };
#
# The local n is no field, so sample changes no field and reads level
function(check_declaration_pairs)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="gauge.cpp"><class>class <name>Gauge</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>level</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>void</name></type> <name>sample</name><parameter_list>()</parameter_list> <block>{<block_content>
        <decl_stmt><decl><type><name>Widget</name></type> <name>w</name> <init>= <expr><name>source</name></expr></init> <name>parent</name></decl>;</decl_stmt>
        <decl_stmt><decl><type><name>int</name></type> <name>n</name> <init>= <expr><literal type="number">0</literal></expr></init></decl>;</decl_stmt>
        <expr_stmt><expr><name>n</name> <operator>=</operator> <name>level</name></expr>;</expr_stmt>
    </block_content>}</block></function>
</public>}</block>;</class>
</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -d -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" [=[
type_name,type_stereotype,function_name,function_stereotype
"Gauge","boundary small-class","sample","collaborator"
]=])
endfunction()

# The analysis of a method that returns a sum of 40,000 terms ends within the timeout of 30 seconds
#
# The C++ file:
#   class A {
#       int v0, v1;
#   public:
#       int big() { return v0+v1+v0+v1+...; }
#   };
function(check_long_expression)
    string(REPEAT "<operator>+</operator><name>v0</name><operator>+</operator><name>v1</name>" 19999 TERMS)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="sum.cpp"><class>class <name>A</name> <block>{<private type="default"> <decl_stmt><decl><type><name>int</name></type> <name>v0</name></decl>, <decl><type ref="prev"/><name>v1</name></decl>;</decl_stmt>
</private><public>public: <function><type><name>int</name></type> <name>big</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>v0</name><operator>+</operator><name>v1</name>@TERMS@</expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>
]=])
    string(CONFIGURE "${INPUT}" INPUT @ONLY)

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -d -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" [=[
type_name,type_stereotype,function_name,function_stereotype
"A","data-provider small-class","big","property"
]=])
endfunction()

# Fails unless two output archives are the same
function(expect_same_archive fileName expectedFileName)
    file(READ "${WORK_DIR}/${expectedFileName}" expected)
    expect_file_content("${WORK_DIR}/${fileName}" "${expected}")
endfunction()

# The st:stereotype attributes of an input that stereocode labeled before give way to the stereotypes of the run,
# with and without -m. A second run on an output thus writes the same archive, and an element that the run leaves
# unlabeled loses its stereotypes. The comments of -m are source code of the input, which a run without -m keeps
#
# The units of the archive besides sample.cpp:
#   class Holder { int* p; public: ~Holder() { delete p; } int get() { return *p; } };
#   interface Shape { int sides(); }
#   class Square implements Shape {
#       int side;
#       public int sides() { return 4; }
#       int side() { return side; }
#   }
function(check_labeled_input)
    string(REGEX REPLACE "^<\\?xml[^\n]*\n" "" SAMPLE_UNIT "${SAMPLE_INPUT}")
    string(REPLACE "<unit xmlns=\"http://www.srcML.org/srcML/src\" revision=\"1.0.0\"" "<unit revision=\"1.0.0\"" SAMPLE_UNIT "${SAMPLE_UNIT}")
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

@SAMPLE_UNIT@
<unit revision="1.0.0" language="C++" filename="holder.cpp"><class>class <name>Holder</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name><modifier>*</modifier></type> <name>p</name></decl>;</decl_stmt>
</private><public>public:
    <destructor><name>~Holder</name><parameter_list>()</parameter_list> <block>{<block_content> <expr_stmt><expr><operator>delete</operator> <name>p</name></expr>;</expr_stmt> </block_content>}</block></destructor>
    <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><operator>*</operator><name>p</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit revision="1.0.0" language="Java" filename="Shape.java"><interface>interface <name>Shape</name> <block>{ <function_decl><type><name>int</name></type> <name>sides</name><parameter_list>()</parameter_list>;</function_decl> }</block></interface>
<class>class <name>Square</name> <super_list><implements>implements <super><name>Shape</name></super></implements></super_list> <block>{
    <decl_stmt><decl><type><name>int</name></type> <name>side</name></decl>;</decl_stmt>
    <function><type><specifier>public</specifier> <name>int</name></type> <name>sides</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">4</literal></expr>;</return> </block_content>}</block></function>
    <function><type><name>int</name></type> <name>side</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>side</name></expr>;</return> </block_content>}</block></function>
}</block></class>
</unit>

</unit>
]=])
    string(CONFIGURE "${INPUT}" INPUT @ONLY)

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    foreach(comments "" "-m")
        run_stereocode(input.xml -f -i ${comments} -o once${comments}.xml)
        expect_exit(0)
        file(READ "${WORK_DIR}/once${comments}.xml" once)
        if(NOT once MATCHES "<interface st:stereotype=\"[^\"]*\">" OR NOT once MATCHES "<destructor st:stereotype=\"destructor\">")
            message(FATAL_ERROR "The interface or the destructor of the first run lacks its stereotype:\n${once}")
        endif()

        run_stereocode(once${comments}.xml -f -i ${comments} -o twice${comments}.xml)
        expect_exit(0)
        expect_same_archive(twice${comments}.xml once${comments}.xml)

        # Without -i, the interface loses the stereotype of the first run
        run_stereocode(once${comments}.xml -f ${comments} -o fewer${comments}.xml)
        expect_exit(0)
        file(READ "${WORK_DIR}/fewer${comments}.xml" fewer)
        if(fewer MATCHES "<interface st:stereotype=")
            message(FATAL_ERROR "The interface keeps the stereotype of the first run:\n${fewer}")
        endif()
    endforeach()

    run_stereocode(input.xml -f -o direct.xml)
    expect_exit(0)
    expect_same_archive(fewer.xml direct.xml)

    # Without -m, the comments of an earlier run stay as comments of the source code
    run_stereocode(once-m.xml -f -i -o uncommented.xml)
    expect_exit(0)
    expect_same_archive(uncommented.xml once-m.xml)
endfunction()

run_check()
