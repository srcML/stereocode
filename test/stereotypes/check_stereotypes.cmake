# SPDX-License-Identifier: GPL-3.0-only
# @file check_stereotypes.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Self-checks of check_stereotypes, the helper program of the stereotype cases and the system tests
# test/CMakeLists.txt registers each function check_<name> as the test stereotypes/check_stereotypes/<name>

include(${CMAKE_CURRENT_LIST_DIR}/../integration/run_and_check.cmake)

# Runs the program CHECK_STEREOTYPES in the work folder and fails unless it exits with the code and prints the report
# The comparison ignores the CR LF line ends that the program writes on Windows
function(expect_run exitCode report)
    execute_process(
        COMMAND "${CHECK_STEREOTYPES}" ${ARGN}
        WORKING_DIRECTORY "${WORK_DIR}"
        RESULT_VARIABLE actualExitCode
        OUTPUT_VARIABLE actualReport
    )
    string(REPLACE "\r\n" "\n" actualReport "${actualReport}")
    if(NOT actualExitCode STREQUAL exitCode OR NOT actualReport STREQUAL report)
        list(JOIN ARGN " " arguments)
        message(FATAL_ERROR "check_stereotypes ${arguments} exited with '${actualExitCode}' and printed:\n${actualReport}\nExpected ${exitCode} and:\n${report}")
    endif()
endfunction()

# The mode comments reports each kind of problem in a hand-written output archive
function(check_comments)
    set(ARCHIVE [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0">

<unit revision="1.0.0" language="C++" filename="sample.cpp"><comment type="line">// @stereotype get</comment>
<function st:stereotype="get"><type><name>int</name></type> <name>a</name><parameter_list>()</parameter_list> <block>{<block_content/>}</block></function>
<comment type="line">// @stereotype set</comment>
<function st:stereotype="get"><type><name>int</name></type> <name>b</name><parameter_list>()</parameter_list> <block>{<block_content/>}</block></function>
<comment type="line">// @stereotype get @defect rules-B1</comment>
<function st:stereotype="get"><type><name>int</name></type> <name>c</name><parameter_list>()</parameter_list> <block>{<block_content/>}</block></function>
<comment type="line">// @stereotype set @defect rules-B2</comment>
<function st:stereotype="get"><type><name>int</name></type> <name>d</name><parameter_list>()</parameter_list> <block>{<block_content/>}</block></function>
<comment type="line">// @stereotype set @spec rules-B3</comment>
<function st:stereotype="get"><type><name>int</name></type> <name>e</name><parameter_list>()</parameter_list> <block>{<block_content/>}</block></function>
<comment type="line">// @stereotype Get</comment>
<function st:stereotype="get"><type><name>int</name></type> <name>f</name><parameter_list>()</parameter_list> <block>{<block_content/>}</block></function>
<comment type="line">// @stereotype get</comment>
<decl_stmt><decl><type><name>int</name></type> <name>g</name></decl>;</decl_stmt>
<comment type="line">// @stereotype property</comment>
<comment type="line">// a note between the expectation and the element</comment>
<friend>friend <function st:stereotype="property"><type><name>int</name></type> <name>h</name><parameter_list>()</parameter_list> <block>{<block_content/>}</block></function></friend>
<comment type="line">// @stereotype get @spec</comment>
<function st:stereotype="get"><type><name>int</name></type> <name>i</name><parameter_list>()</parameter_list> <block>{<block_content/>}</block></function>
<comment type="line">// @stereotype set</comment>
<function><type><name>void</name></type> <name>j</name><parameter_list>()</parameter_list> <block>{<block_content/>}</block></function>
<comment type="line">// @stereotype empty</comment>
<typedef>typedef <type><struct st:stereotype="empty">struct <block>{}</block></struct></type> <name>K</name>;</typedef>
</unit>

<unit revision="1.0.0" language="Java" filename="second.java"><comment type="line">// @stereotype empty</comment>
<class st:stereotype="empty">class <name>A</name> <block>{ }</block></class>
</unit>

<unit revision="1.0.0" language="C#" filename="third.cs"><class>class <name>B</name> <block>{<comment type="line">// @stereotype get</comment>
<expr_stmt st:stereotype="get"><expr><specifier>public</specifier> <name>int</name> <name>Count</name> <operator>=&gt;</operator> <name>count</name></expr>;</expr_stmt> }</block></class>
</unit>

</unit>
]=])

    set(EXPECTED_REPORT [=[
sample.cpp:3: wrong stereotype: expected {set}, got {get}
sample.cpp:5: unexpected pass, remove @defect rules-B1: expected {get}, got {get}
sample.cpp:9: spec rules-B3 differs: expected {set}, got {get}
sample.cpp:11: malformed expectation: malformed label Get
sample.cpp:13: wrong stereotype: expected {get}, got no annotatable element after the comment
sample.cpp:18: malformed expectation: tag without a review id
sample.cpp:20: wrong stereotype: expected {set}, got no stereotype
13 expectations: 5 hold, 1 expected failures, 1 spec (0 hold), 6 problems
]=])

    file(WRITE "${WORK_DIR}/sample.xml" "${ARCHIVE}")
    expect_run(1 "${EXPECTED_REPORT}" comments sample.xml)
endfunction()

# The mode table writes the table of a stereocode output archive of srcml --position, finds no difference from the same table,
# and reports each difference from a reference that changes a row, lacks a row, holds an extra row, and ends its lines with CR LF
#
# The archive holds these files, labeled with stereocode -t:
#   shapes.hpp  typedef struct { int x; int getX() const { return x; } } Point;
#               class Box { int w; public: bool operator ==(const Box& other) const { return w == other.w; }
#                           int width() const { return w; } };
#               int twice(int v) { return 2 * v; }
#   Pair.java   class Pair<A,
#                          B> { private A first; A getFirst() { return first; } }
function(check_table)
    set(ARCHIVE [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:pos="http://www.srcML.org/srcML/position" revision="1.0.0" pos:tabs="8">

<unit xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="shapes.hpp" pos:tabs="8"><typedef pos:start="1:1" pos:end="4:8">typedef <type pos:start="1:9" pos:end="4:1"><struct pos:start="1:9" pos:end="4:1" st:stereotype="data-provider data-struct small-struct">struct <block pos:start="1:16" pos:end="4:1">{<public type="default" pos:start="1:17" pos:end="4:0">
    <decl_stmt pos:start="2:5" pos:end="2:10"><decl pos:start="2:5" pos:end="2:9"><type pos:start="2:5" pos:end="2:7"><name pos:start="2:5" pos:end="2:7">int</name></type> <name pos:start="2:9" pos:end="2:9">x</name></decl>;</decl_stmt>
    <function pos:start="3:5" pos:end="3:34" st:stereotype="get"><type pos:start="3:5" pos:end="3:7"><name pos:start="3:5" pos:end="3:7">int</name></type> <name pos:start="3:9" pos:end="3:12">getX</name><parameter_list pos:start="3:13" pos:end="3:14">()</parameter_list> <specifier pos:start="3:16" pos:end="3:20">const</specifier> <block pos:start="3:22" pos:end="3:34">{<block_content pos:start="3:23" pos:end="3:33"> <return pos:start="3:24" pos:end="3:32">return <expr pos:start="3:31" pos:end="3:31"><name pos:start="3:31" pos:end="3:31">x</name></expr>;</return> </block_content>}</block></function>
</public>}</block></struct></type> <name pos:start="4:3" pos:end="4:7">Point</name>;</typedef>

<class pos:start="6:1" pos:end="11:2" st:stereotype="data-provider small-class">class <name pos:start="6:7" pos:end="6:9">Box</name> <block pos:start="6:11" pos:end="11:1">{<private type="default" pos:start="6:12" pos:end="8:0">
    <decl_stmt pos:start="7:5" pos:end="7:10"><decl pos:start="7:5" pos:end="7:9"><type pos:start="7:5" pos:end="7:7"><name pos:start="7:5" pos:end="7:7">int</name></type> <name pos:start="7:9" pos:end="7:9">w</name></decl>;</decl_stmt>
</private><public pos:start="8:1" pos:end="11:0">public:
    <function type="operator" pos:start="9:5" pos:end="9:69" st:stereotype="predicate"><type pos:start="9:5" pos:end="9:8"><name pos:start="9:5" pos:end="9:8">bool</name></type> <name pos:start="9:10" pos:end="9:20">operator <name pos:start="9:19" pos:end="9:20">==</name></name><parameter_list pos:start="9:21" pos:end="9:38">(<parameter pos:start="9:22" pos:end="9:37"><decl pos:start="9:22" pos:end="9:37"><type pos:start="9:22" pos:end="9:31"><specifier pos:start="9:22" pos:end="9:26">const</specifier> <name pos:start="9:28" pos:end="9:30">Box</name><modifier pos:start="9:31" pos:end="9:31">&amp;</modifier></type> <name pos:start="9:33" pos:end="9:37">other</name></decl></parameter>)</parameter_list> <specifier pos:start="9:40" pos:end="9:44">const</specifier> <block pos:start="9:46" pos:end="9:69">{<block_content pos:start="9:47" pos:end="9:68"> <return pos:start="9:48" pos:end="9:67">return <expr pos:start="9:55" pos:end="9:66"><name pos:start="9:55" pos:end="9:55">w</name> <operator pos:start="9:57" pos:end="9:58">==</operator> <name pos:start="9:60" pos:end="9:66"><name pos:start="9:60" pos:end="9:64">other</name><operator pos:start="9:65" pos:end="9:65">.</operator><name pos:start="9:66" pos:end="9:66">w</name></name></expr>;</return> </block_content>}</block></function>
    <function pos:start="10:5" pos:end="10:35" st:stereotype="get"><type pos:start="10:5" pos:end="10:7"><name pos:start="10:5" pos:end="10:7">int</name></type> <name pos:start="10:9" pos:end="10:13">width</name><parameter_list pos:start="10:14" pos:end="10:15">()</parameter_list> <specifier pos:start="10:17" pos:end="10:21">const</specifier> <block pos:start="10:23" pos:end="10:35">{<block_content pos:start="10:24" pos:end="10:34"> <return pos:start="10:25" pos:end="10:33">return <expr pos:start="10:32" pos:end="10:32"><name pos:start="10:32" pos:end="10:32">w</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>

<function pos:start="13:1" pos:end="13:34"><type pos:start="13:1" pos:end="13:3"><name pos:start="13:1" pos:end="13:3">int</name></type> <name pos:start="13:5" pos:end="13:9">twice</name><parameter_list pos:start="13:10" pos:end="13:16">(<parameter pos:start="13:11" pos:end="13:15"><decl pos:start="13:11" pos:end="13:15"><type pos:start="13:11" pos:end="13:13"><name pos:start="13:11" pos:end="13:13">int</name></type> <name pos:start="13:15" pos:end="13:15">v</name></decl></parameter>)</parameter_list> <block pos:start="13:18" pos:end="13:34">{<block_content pos:start="13:19" pos:end="13:33"> <return pos:start="13:20" pos:end="13:32">return <expr pos:start="13:27" pos:end="13:31"><literal type="number" pos:start="13:27" pos:end="13:27">2</literal> <operator pos:start="13:29" pos:end="13:29">*</operator> <name pos:start="13:31" pos:end="13:31">v</name></expr>;</return> </block_content>}</block></function>
</unit>

<unit xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="Java" filename="Pair.java" pos:tabs="8"><class pos:start="1:1" pos:end="6:1" st:stereotype="data-provider boundary data-class small-class">class <name pos:start="1:7" pos:end="2:13"><name pos:start="1:7" pos:end="1:10">Pair</name><parameter_list type="generic" pos:start="1:11" pos:end="2:13">&lt;<parameter pos:start="1:12" pos:end="1:12"><name pos:start="1:12" pos:end="1:12">A</name></parameter>,
           <parameter pos:start="2:12" pos:end="2:12"><name pos:start="2:12" pos:end="2:12">B</name></parameter>&gt;</parameter_list></name> <block pos:start="2:15" pos:end="6:1">{
    <decl_stmt pos:start="3:5" pos:end="3:20"><decl pos:start="3:5" pos:end="3:19"><type pos:start="3:5" pos:end="3:13"><specifier pos:start="3:5" pos:end="3:11">private</specifier> <name pos:start="3:13" pos:end="3:13">A</name></type> <name pos:start="3:15" pos:end="3:19">first</name></decl>;</decl_stmt>

    <function pos:start="5:5" pos:end="5:34" st:stereotype="get collaborator"><type pos:start="5:5" pos:end="5:5"><name pos:start="5:5" pos:end="5:5">A</name></type> <name pos:start="5:7" pos:end="5:14">getFirst</name><parameter_list pos:start="5:15" pos:end="5:16">()</parameter_list> <block pos:start="5:18" pos:end="5:34">{<block_content pos:start="5:19" pos:end="5:33"> <return pos:start="5:20" pos:end="5:32">return <expr pos:start="5:27" pos:end="5:31"><name pos:start="5:27" pos:end="5:31">first</name></expr>;</return> </block_content>}</block></function>
}</block></class>
</unit>

</unit>
]=])

    string(JOIN "\n" EXPECTED_TABLE
        "file\tline\telement\tname\tstereotypes"
        "shapes.hpp\t1\tstruct\t\tdata-provider data-struct small-struct"
        "shapes.hpp\t3\tfunction\tgetX\tget"
        "shapes.hpp\t6\tclass\tBox\tdata-provider small-class"
        "shapes.hpp\t9\tfunction\toperator ==\tpredicate"
        "shapes.hpp\t10\tfunction\twidth\tget"
        "Pair.java\t1\tclass\tPair<A, B>\tdata-provider boundary data-class small-class"
        "Pair.java\t5\tfunction\tgetFirst\tget collaborator"
        ""
    )

    string(JOIN "\r\n" REFERENCE_TABLE
        "file\tline\telement\tname\tstereotypes"
        "shapes.hpp\t1\tstruct\t\tdata-provider data-struct small-struct"
        "shapes.hpp\t3\tfunction\tgetX\tget"
        "shapes.hpp\t6\tclass\tBox\tdata-provider small-class"
        "shapes.hpp\t10\tfunction\twidth\tset"
        "shapes.hpp\t13\tfunction\ttwice\tproperty"
        "Pair.java\t1\tclass\tPair<A, B>\tdata-provider boundary data-class small-class"
        "Pair.java\t5\tfunction\tgetFirst\tget collaborator"
        ""
    )

    string(JOIN "\n" EXPECTED_REPORT
        "The first rows that only the reference (-) or only the output (+) holds, with their lines:"
        "+ 5: shapes.hpp\t9\tfunction\toperator ==\tpredicate"
        "- 5: shapes.hpp\t10\tfunction\twidth\tset"
        "+ 6: shapes.hpp\t10\tfunction\twidth\tget"
        "- 6: shapes.hpp\t13\tfunction\ttwice\tproperty"
        "2 of the 8 rows of the reference and 2 of the 8 rows of the output differ"
        ""
    )

    file(WRITE "${WORK_DIR}/sample.xml" "${ARCHIVE}")
    file(WRITE "${WORK_DIR}/expected.tsv" "${EXPECTED_TABLE}")
    file(WRITE "${WORK_DIR}/reference.tsv" "${REFERENCE_TABLE}")

    expect_run(0 "" table sample.xml table.tsv)
    file(READ "${WORK_DIR}/table.tsv" TABLE_CONTENT)
    if(NOT TABLE_CONTENT STREQUAL EXPECTED_TABLE)
        message(FATAL_ERROR "The table of the sample differs from the expected table:\n${TABLE_CONTENT}")
    endif()

    expect_run(0 "" table sample.xml table.tsv expected.tsv)
    expect_run(1 "${EXPECTED_REPORT}" table sample.xml table.tsv reference.tsv)
endfunction()

run_check()
