# SPDX-License-Identifier: GPL-3.0-only
# @file run_and_check.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Steps shared by the integration checks: a fresh work folder, a run of stereocode, checks of the results, and a sample archive
#
# Each topic script runs with -DCHECK=<check> -DSTEREOCODE=<binary> -DWORK_DIR=<folder>, includes this file, and ends with run_check()

# Runs the function check_<CHECK> of the topic script in an empty work folder
function(run_check)
    if(NOT COMMAND "check_${CHECK}")
        message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE} has no check '${CHECK}'")
    endif()
    file(REMOVE_RECURSE "${WORK_DIR}")
    file(MAKE_DIRECTORY "${WORK_DIR}")
    cmake_language(CALL "check_${CHECK}")
endfunction()

# Runs stereocode in the work folder with the given arguments and sets RUN_EXIT, RUN_OUTPUT, and RUN_ERROR
# A timeout turns a hang into a failure
function(run_stereocode)
    execute_process(
        COMMAND ${STEREOCODE} ${ARGN}
        WORKING_DIRECTORY "${WORK_DIR}"
        RESULT_VARIABLE exitCode
        OUTPUT_VARIABLE standardOutput
        ERROR_VARIABLE standardError
        TIMEOUT 30
    )
    set(RUN_EXIT "${exitCode}" PARENT_SCOPE)
    set(RUN_OUTPUT "${standardOutput}" PARENT_SCOPE)
    set(RUN_ERROR "${standardError}" PARENT_SCOPE)
endfunction()

# Fails unless the last run exited with the expected code
function(expect_exit expected)
    if(NOT "${RUN_EXIT}" STREQUAL "${expected}")
        message(FATAL_ERROR "Expected exit code ${expected}, got '${RUN_EXIT}'\nstdout: ${RUN_OUTPUT}\nstderr: ${RUN_ERROR}")
    endif()
endfunction()

# Fails unless the last run failed with exit code 1 and a message on standard error
function(expect_error)
    expect_exit(1)
    if(NOT "${RUN_ERROR}" MATCHES "^Error: ")
        message(FATAL_ERROR "Expected an error message on standard error, got '${RUN_ERROR}'")
    endif()
endfunction()

# Fails unless the file holds exactly the expected text
function(expect_file_content fileName expected)
    if(NOT EXISTS "${fileName}")
        message(FATAL_ERROR "Missing file ${fileName}\nstdout: ${RUN_OUTPUT}\nstderr: ${RUN_ERROR}")
    endif()
    file(READ "${fileName}" actual)
    if(NOT "${actual}" STREQUAL "${expected}")
        message(FATAL_ERROR "Unexpected content of ${fileName}:\n${actual}\nExpected:\n${expected}")
    endif()
endfunction()

# Fails if the file or folder exists
function(expect_missing fileName)
    if(EXISTS "${fileName}")
        message(FATAL_ERROR "Unexpected file ${fileName}")
    endif()
endfunction()

# Fails unless the option with the value stops a run on input.xml with the message and writes no output
function(expect_rejected_value option value expectedMessage)
    run_stereocode(input.xml ${option} ${value} -o rejected.xml)
    if(RUN_EXIT STREQUAL "0" OR NOT RUN_ERROR MATCHES "${expectedMessage}")
        message(FATAL_ERROR "Expected ${option} ${value} to fail with '${expectedMessage}'\nexit: ${RUN_EXIT}\nstderr: ${RUN_ERROR}")
    endif()
    expect_missing("${WORK_DIR}/rejected.xml")
endfunction()

# The srcML of one C++ file with a class and a free function, and what stereocode -f -r writes for it
#
# The C++ file:
#   class A {
#       int x;
#   public:
#       int getX() const { return x; }
#   };
#
#   int twice(int v) { return 2 * v; }

set(SAMPLE_INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="sample.cpp"><class>class <name>A</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>

<function><type><name>int</name></type> <name>twice</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">2</literal> <operator>*</operator> <name>v</name></expr>;</return> </block_content>}</block></function>
</unit>
]=])

set(SAMPLE_OUTPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="sample.cpp"><class st:stereotype="data-provider data-class small-class">class <name>A</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</private><public>public:
    <function st:stereotype="get"><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>

<function st:stereotype="property"><type><name>int</name></type> <name>twice</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">2</literal> <operator>*</operator> <name>v</name></expr>;</return> </block_content>}</block></function>
</unit>
]=])

set(SAMPLE_CSV [=[
type_name,type_stereotype,function_name,function_stereotype
"A","data-provider data-class small-class","getX","get"
]=])

set(SAMPLE_FREE_FUNCTIONS_CSV [=[
type_name,type_stereotype,function_name,function_stereotype
"N/A","N/A","twice","property"
]=])
