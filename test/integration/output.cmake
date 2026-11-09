# SPDX-License-Identifier: GPL-3.0-only
# @file output.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Integration checks of the srcML output archive, with its place, a failure to write it, and the positions of srcml --position
# test/CMakeLists.txt registers each function check_<name> as the test integration/output/<name>

include(${CMAKE_CURRENT_LIST_DIR}/run_and_check.cmake)

# An output file equal to the input archive receives the annotated archive, and the input is read in full first
function(check_output_is_input)
    file(WRITE "${WORK_DIR}/input.xml" "${SAMPLE_INPUT}")

    run_stereocode(input.xml -f -o input.xml)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.xml" "${SAMPLE_OUTPUT}")
    expect_missing("${WORK_DIR}/input.xml.tmp")
endfunction()

# An output archive that cannot be created stops the run with an error that names the file
function(check_unwritable_output)
    file(WRITE "${WORK_DIR}/input.xml" "${SAMPLE_INPUT}")

    run_stereocode(input.xml -f -o missing/output.xml)
    expect_error()
    if(NOT "${RUN_ERROR}" MATCHES "missing/output.xml")
        message(FATAL_ERROR "The error does not name the output file: ${RUN_ERROR}")
    endif()
    expect_missing("${WORK_DIR}/missing")
endfunction()

# The output of an archive that srcml --position writes keeps the namespace and the attributes of the positions,
# with each stereotype after the positions of its element
#
# The C++ file counter.cpp:
#   class Counter {
#       int count;
#   public:
#       Counter() { count = 0; }
#       ~Counter() { }
#       int get() const { return count; }
#       void set(int value) { count = value; }
#   };
#
#   int twice(int value) { return 2 * value; }
function(check_position_archive)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:pos="http://www.srcML.org/srcML/position" revision="1.0.0" language="C++" filename="counter.cpp" pos:tabs="8"><class pos:start="1:1" pos:end="8:2">class <name pos:start="1:7" pos:end="1:13">Counter</name> <block pos:start="1:15" pos:end="8:1">{<private type="default" pos:start="1:16" pos:end="3:0">
    <decl_stmt pos:start="2:5" pos:end="2:14"><decl pos:start="2:5" pos:end="2:13"><type pos:start="2:5" pos:end="2:7"><name pos:start="2:5" pos:end="2:7">int</name></type> <name pos:start="2:9" pos:end="2:13">count</name></decl>;</decl_stmt>
</private><public pos:start="3:1" pos:end="8:0">public:
    <constructor pos:start="4:5" pos:end="4:28"><name pos:start="4:5" pos:end="4:11">Counter</name><parameter_list pos:start="4:12" pos:end="4:13">()</parameter_list> <block pos:start="4:15" pos:end="4:28">{<block_content pos:start="4:16" pos:end="4:27"> <expr_stmt pos:start="4:17" pos:end="4:26"><expr pos:start="4:17" pos:end="4:25"><name pos:start="4:17" pos:end="4:21">count</name> <operator pos:start="4:23" pos:end="4:23">=</operator> <literal type="number" pos:start="4:25" pos:end="4:25">0</literal></expr>;</expr_stmt> </block_content>}</block></constructor>
    <destructor pos:start="5:5" pos:end="5:18"><name pos:start="5:5" pos:end="5:12">~Counter</name><parameter_list pos:start="5:13" pos:end="5:14">()</parameter_list> <block pos:start="5:16" pos:end="5:18">{<block_content pos:start="5:17" pos:end="5:17"> </block_content>}</block></destructor>
    <function pos:start="6:5" pos:end="6:37"><type pos:start="6:5" pos:end="6:7"><name pos:start="6:5" pos:end="6:7">int</name></type> <name pos:start="6:9" pos:end="6:11">get</name><parameter_list pos:start="6:12" pos:end="6:13">()</parameter_list> <specifier pos:start="6:15" pos:end="6:19">const</specifier> <block pos:start="6:21" pos:end="6:37">{<block_content pos:start="6:22" pos:end="6:36"> <return pos:start="6:23" pos:end="6:35">return <expr pos:start="6:30" pos:end="6:34"><name pos:start="6:30" pos:end="6:34">count</name></expr>;</return> </block_content>}</block></function>
    <function pos:start="7:5" pos:end="7:42"><type pos:start="7:5" pos:end="7:8"><name pos:start="7:5" pos:end="7:8">void</name></type> <name pos:start="7:10" pos:end="7:12">set</name><parameter_list pos:start="7:13" pos:end="7:23">(<parameter pos:start="7:14" pos:end="7:22"><decl pos:start="7:14" pos:end="7:22"><type pos:start="7:14" pos:end="7:16"><name pos:start="7:14" pos:end="7:16">int</name></type> <name pos:start="7:18" pos:end="7:22">value</name></decl></parameter>)</parameter_list> <block pos:start="7:25" pos:end="7:42">{<block_content pos:start="7:26" pos:end="7:41"> <expr_stmt pos:start="7:27" pos:end="7:40"><expr pos:start="7:27" pos:end="7:39"><name pos:start="7:27" pos:end="7:31">count</name> <operator pos:start="7:33" pos:end="7:33">=</operator> <name pos:start="7:35" pos:end="7:39">value</name></expr>;</expr_stmt> </block_content>}</block></function>
</public>}</block>;</class>

<function pos:start="10:1" pos:end="10:42"><type pos:start="10:1" pos:end="10:3"><name pos:start="10:1" pos:end="10:3">int</name></type> <name pos:start="10:5" pos:end="10:9">twice</name><parameter_list pos:start="10:10" pos:end="10:20">(<parameter pos:start="10:11" pos:end="10:19"><decl pos:start="10:11" pos:end="10:19"><type pos:start="10:11" pos:end="10:13"><name pos:start="10:11" pos:end="10:13">int</name></type> <name pos:start="10:15" pos:end="10:19">value</name></decl></parameter>)</parameter_list> <block pos:start="10:22" pos:end="10:42">{<block_content pos:start="10:23" pos:end="10:41"> <return pos:start="10:24" pos:end="10:40">return <expr pos:start="10:31" pos:end="10:39"><literal type="number" pos:start="10:31" pos:end="10:31">2</literal> <operator pos:start="10:33" pos:end="10:33">*</operator> <name pos:start="10:35" pos:end="10:39">value</name></expr>;</return> </block_content>}</block></function>
</unit>
]=])

    set(EXPECTED_OUTPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:pos="http://www.srcML.org/srcML/position" xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="counter.cpp" pos:tabs="8"><class pos:start="1:1" pos:end="8:2" st:stereotype="data-class small-class">class <name pos:start="1:7" pos:end="1:13">Counter</name> <block pos:start="1:15" pos:end="8:1">{<private type="default" pos:start="1:16" pos:end="3:0">
    <decl_stmt pos:start="2:5" pos:end="2:14"><decl pos:start="2:5" pos:end="2:13"><type pos:start="2:5" pos:end="2:7"><name pos:start="2:5" pos:end="2:7">int</name></type> <name pos:start="2:9" pos:end="2:13">count</name></decl>;</decl_stmt>
</private><public pos:start="3:1" pos:end="8:0">public:
    <constructor pos:start="4:5" pos:end="4:28" st:stereotype="constructor"><name pos:start="4:5" pos:end="4:11">Counter</name><parameter_list pos:start="4:12" pos:end="4:13">()</parameter_list> <block pos:start="4:15" pos:end="4:28">{<block_content pos:start="4:16" pos:end="4:27"> <expr_stmt pos:start="4:17" pos:end="4:26"><expr pos:start="4:17" pos:end="4:25"><name pos:start="4:17" pos:end="4:21">count</name> <operator pos:start="4:23" pos:end="4:23">=</operator> <literal type="number" pos:start="4:25" pos:end="4:25">0</literal></expr>;</expr_stmt> </block_content>}</block></constructor>
    <destructor pos:start="5:5" pos:end="5:18" st:stereotype="destructor"><name pos:start="5:5" pos:end="5:12">~Counter</name><parameter_list pos:start="5:13" pos:end="5:14">()</parameter_list> <block pos:start="5:16" pos:end="5:18">{<block_content pos:start="5:17" pos:end="5:17"> </block_content>}</block></destructor>
    <function pos:start="6:5" pos:end="6:37" st:stereotype="get"><type pos:start="6:5" pos:end="6:7"><name pos:start="6:5" pos:end="6:7">int</name></type> <name pos:start="6:9" pos:end="6:11">get</name><parameter_list pos:start="6:12" pos:end="6:13">()</parameter_list> <specifier pos:start="6:15" pos:end="6:19">const</specifier> <block pos:start="6:21" pos:end="6:37">{<block_content pos:start="6:22" pos:end="6:36"> <return pos:start="6:23" pos:end="6:35">return <expr pos:start="6:30" pos:end="6:34"><name pos:start="6:30" pos:end="6:34">count</name></expr>;</return> </block_content>}</block></function>
    <function pos:start="7:5" pos:end="7:42" st:stereotype="set"><type pos:start="7:5" pos:end="7:8"><name pos:start="7:5" pos:end="7:8">void</name></type> <name pos:start="7:10" pos:end="7:12">set</name><parameter_list pos:start="7:13" pos:end="7:23">(<parameter pos:start="7:14" pos:end="7:22"><decl pos:start="7:14" pos:end="7:22"><type pos:start="7:14" pos:end="7:16"><name pos:start="7:14" pos:end="7:16">int</name></type> <name pos:start="7:18" pos:end="7:22">value</name></decl></parameter>)</parameter_list> <block pos:start="7:25" pos:end="7:42">{<block_content pos:start="7:26" pos:end="7:41"> <expr_stmt pos:start="7:27" pos:end="7:40"><expr pos:start="7:27" pos:end="7:39"><name pos:start="7:27" pos:end="7:31">count</name> <operator pos:start="7:33" pos:end="7:33">=</operator> <name pos:start="7:35" pos:end="7:39">value</name></expr>;</expr_stmt> </block_content>}</block></function>
</public>}</block>;</class>

<function pos:start="10:1" pos:end="10:42" st:stereotype="property"><type pos:start="10:1" pos:end="10:3"><name pos:start="10:1" pos:end="10:3">int</name></type> <name pos:start="10:5" pos:end="10:9">twice</name><parameter_list pos:start="10:10" pos:end="10:20">(<parameter pos:start="10:11" pos:end="10:19"><decl pos:start="10:11" pos:end="10:19"><type pos:start="10:11" pos:end="10:13"><name pos:start="10:11" pos:end="10:13">int</name></type> <name pos:start="10:15" pos:end="10:19">value</name></decl></parameter>)</parameter_list> <block pos:start="10:22" pos:end="10:42">{<block_content pos:start="10:23" pos:end="10:41"> <return pos:start="10:24" pos:end="10:40">return <expr pos:start="10:31" pos:end="10:39"><literal type="number" pos:start="10:31" pos:end="10:31">2</literal> <operator pos:start="10:33" pos:end="10:33">*</operator> <name pos:start="10:35" pos:end="10:39">value</name></expr>;</return> </block_content>}</block></function>
</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${EXPECTED_OUTPUT}")
endfunction()

# A destructor keeps its stereotype in an archive that srcml --position writes, where its start tag holds attributes
#
# The C++ file:
#   class Buffer {
#       int* data;
#   public:
#       Buffer() { data = new int[8]; }
#       ~Buffer() { delete[] data; }
#   };
function(check_destructor_position)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:pos="http://www.srcML.org/srcML/position" revision="1.0.0" language="C++" filename="buffer.cpp" pos:tabs="8"><class pos:start="1:1" pos:end="6:2">class <name pos:start="1:7" pos:end="1:12">Buffer</name> <block pos:start="1:14" pos:end="6:1">{<private type="default" pos:start="1:15" pos:end="3:0">
    <decl_stmt pos:start="2:5" pos:end="2:14"><decl pos:start="2:5" pos:end="2:13"><type pos:start="2:5" pos:end="2:8"><name pos:start="2:5" pos:end="2:7">int</name><modifier pos:start="2:8" pos:end="2:8">*</modifier></type> <name pos:start="2:10" pos:end="2:13">data</name></decl>;</decl_stmt>
</private><public pos:start="3:1" pos:end="6:0">public:
    <constructor pos:start="4:5" pos:end="4:35"><name pos:start="4:5" pos:end="4:10">Buffer</name><parameter_list pos:start="4:11" pos:end="4:12">()</parameter_list> <block pos:start="4:14" pos:end="4:35">{<block_content pos:start="4:15" pos:end="4:34"> <expr_stmt pos:start="4:16" pos:end="4:33"><expr pos:start="4:16" pos:end="4:32"><name pos:start="4:16" pos:end="4:19">data</name> <operator pos:start="4:21" pos:end="4:21">=</operator> <operator pos:start="4:23" pos:end="4:25">new</operator> <name pos:start="4:27" pos:end="4:32"><name pos:start="4:27" pos:end="4:29">int</name><index pos:start="4:30" pos:end="4:32">[<expr pos:start="4:31" pos:end="4:31"><literal type="number" pos:start="4:31" pos:end="4:31">8</literal></expr>]</index></name></expr>;</expr_stmt> </block_content>}</block></constructor>
    <destructor pos:start="5:5" pos:end="5:32"><name pos:start="5:5" pos:end="5:11">~Buffer</name><parameter_list pos:start="5:12" pos:end="5:13">()</parameter_list> <block pos:start="5:15" pos:end="5:32">{<block_content pos:start="5:16" pos:end="5:31"> <expr_stmt pos:start="5:17" pos:end="5:30"><expr pos:start="5:17" pos:end="5:29"><operator pos:start="5:17" pos:end="5:22">delete</operator><index pos:start="5:23" pos:end="5:24">[]</index> <name pos:start="5:26" pos:end="5:29">data</name></expr>;</expr_stmt> </block_content>}</block></destructor>
</public>}</block>;</class>
</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml)
    expect_exit(0)
    file(READ "${WORK_DIR}/input.stereotypes.xml" output)
    if(NOT output MATCHES "<destructor pos:start=\"5:5\" pos:end=\"5:32\" st:stereotype=\"destructor\">")
        message(FATAL_ERROR "The destructor did not get the stereotype destructor:\n${output}")
    endif()
endfunction()

run_check()
