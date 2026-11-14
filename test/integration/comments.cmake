# SPDX-License-Identifier: GPL-3.0-only
# @file comments.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Integration checks of the stereotype comments of -m
# test/CMakeLists.txt registers each function check_<name> as the test integration/comments/<name>

include(${CMAKE_CURRENT_LIST_DIR}/run_and_check.cmake)

# Fails unless the archive without its stereotype comments equals the expected archive
function(expect_without_comments fileName expected)
    file(READ "${fileName}" commented)
    string(REGEX REPLACE "<comment type=\"line\">// @stereotype [a-z -]*</comment>\n[ \t]*" "" uncommented "${commented}")
    if(NOT uncommented STREQUAL expected)
        message(FATAL_ERROR "Without its stereotype comments, ${fileName} differs from the archive without -m:\n${uncommented}")
    endif()
endfunction()

# With -m, a line comment "// @stereotype <labels>" precedes each labeled element of the srcML output archive, at the indentation of the element
# Without the comments, the archive equals the archive of a run without -m, for one or three worker threads, also when -o names the input
# -m with -d stops with an error and writes no output
#
# The units of the archive besides sample.cpp:
#   class Counter {                              void log(int value);
#       private int count;
#       public int getCount() { return count; }
#       public void reset() { count = 0; }
#   }
function(check_stereotype_comments)
    set(SAMPLE_COMMENTED_OUTPUT "${SAMPLE_OUTPUT}")
    string(REPLACE "filename=\"sample.cpp\"><class"
                   "filename=\"sample.cpp\"><comment type=\"line\">// @stereotype data-provider data-class small-class</comment>\n<class"
                   SAMPLE_COMMENTED_OUTPUT "${SAMPLE_COMMENTED_OUTPUT}")
    string(REPLACE "public:\n    <function"
                   "public:\n    <comment type=\"line\">// @stereotype get</comment>\n    <function"
                   SAMPLE_COMMENTED_OUTPUT "${SAMPLE_COMMENTED_OUTPUT}")
    string(REPLACE "\n\n<function"
                   "\n\n<comment type=\"line\">// @stereotype property</comment>\n<function"
                   SAMPLE_COMMENTED_OUTPUT "${SAMPLE_COMMENTED_OUTPUT}")

    string(REGEX REPLACE "^<\\?xml[^\n]*\n" "" SAMPLE_UNIT "${SAMPLE_INPUT}")
    string(REPLACE "<unit xmlns=\"http://www.srcML.org/srcML/src\" revision=\"1.0.0\"" "<unit revision=\"1.0.0\"" SAMPLE_UNIT "${SAMPLE_UNIT}")
    set(ARCHIVE_INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

@SAMPLE_UNIT@
<unit revision="1.0.0" language="Java" filename="Counter.java" hash="ce0fd855284b0459f4dba23ef09743db6baf9003"><class>class <name>Counter</name> <block>{
    <decl_stmt><decl><type><specifier>private</specifier> <name>int</name></type> <name>count</name></decl>;</decl_stmt>
    <function><type><specifier>public</specifier> <name>int</name></type> <name>getCount</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>count</name></expr>;</return> </block_content>}</block></function>
    <function><type><specifier>public</specifier> <name>void</name></type> <name>reset</name><parameter_list>()</parameter_list> <block>{<block_content> <expr_stmt><expr><name>count</name> <operator>=</operator> <literal type="number">0</literal></expr>;</expr_stmt> </block_content>}</block></function>
}</block></class>
</unit>

<unit revision="1.0.0" language="C++" filename="log.cpp" hash="17354119c4e3270ef741a81d932c285957b1350c"><function_decl><type><name>void</name></type> <name>log</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>value</name></decl></parameter>)</parameter_list>;</function_decl>
</unit>

</unit>
]=])
    string(CONFIGURE "${ARCHIVE_INPUT}" ARCHIVE_INPUT @ONLY)

    file(WRITE "${WORK_DIR}/input.xml" "${SAMPLE_INPUT}")
    file(WRITE "${WORK_DIR}/archive.xml" "${ARCHIVE_INPUT}")

    run_stereocode(--help)
    expect_exit(0)
    if(NOT RUN_OUTPUT MATCHES "\n  -m, +--comment +Inserts a // @stereotype <labels> comment \\(# @stereotype <labels>[ \n]+in Python\\)[ \n]+before each stereotyped")
        message(FATAL_ERROR "The help does not describe -m:\n${RUN_OUTPUT}")
    endif()

    run_stereocode(input.xml -f -m)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${SAMPLE_COMMENTED_OUTPUT}")

    file(WRITE "${WORK_DIR}/inplace.xml" "${SAMPLE_INPUT}")
    run_stereocode(inplace.xml -f -m -o inplace.xml)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/inplace.xml" "${SAMPLE_COMMENTED_OUTPUT}")
    expect_missing("${WORK_DIR}/inplace.xml.tmp")

    # An archive of several units, one of them without labels, with one and three worker threads
    run_stereocode(archive.xml -f -o plain.xml)
    expect_exit(0)
    file(READ "${WORK_DIR}/plain.xml" PLAIN_OUTPUT)
    foreach(threads 1 3)
        run_stereocode(archive.xml -f -m -j ${threads} -o commented.${threads}.xml)
        expect_exit(0)
        expect_without_comments("${WORK_DIR}/commented.${threads}.xml" "${PLAIN_OUTPUT}")
    endforeach()
    file(READ "${WORK_DIR}/commented.1.xml" COMMENTED_OUTPUT)
    expect_file_content("${WORK_DIR}/commented.3.xml" "${COMMENTED_OUTPUT}")
    foreach(comment "\n    <comment type=\"line\">// @stereotype get</comment>\n    <function st:stereotype=\"get\"><type><specifier>public</specifier> <name>int</name></type> <name>getCount</name>"
                    "\n    <comment type=\"line\">// @stereotype set</comment>\n    <function st:stereotype=\"set\"><type><specifier>public</specifier> <name>void</name></type> <name>reset</name>"
                    "filename=\"Counter.java\" hash=\"ce0fd855284b0459f4dba23ef09743db6baf9003\"><comment type=\"line\">// @stereotype data-class small-class</comment>\n<class")
        string(FIND "${COMMENTED_OUTPUT}" "${comment}" position)
        if(position EQUAL -1)
            message(FATAL_ERROR "The archive with comments lacks ${comment}:\n${COMMENTED_OUTPUT}")
        endif()
    endforeach()

    # Without the srcML output archive, the comments have no place, so -m and -d exclude each other
    run_stereocode(input.xml -m -d -r -o excluded.xml)
    expect_error()
    if(NOT RUN_ERROR MATCHES "--comment \\(-m\\).*--no-srcml-output \\(-d\\)")
        message(FATAL_ERROR "The error does not name -m and -d: ${RUN_ERROR}")
    endif()
    foreach(output excluded.xml excluded.xml.tmp excluded.stereotypes.csv excluded.stereotypes.csv.tmp)
        expect_missing("${WORK_DIR}/${output}")
    endforeach()
endfunction()

# Runs a command in the work folder and fails unless it exits with 0, and sets COMMAND_OUTPUT to its standard output
function(run_step)
    execute_process(
        COMMAND ${ARGN}
        WORKING_DIRECTORY "${WORK_DIR}"
        RESULT_VARIABLE exitCode
        OUTPUT_VARIABLE standardOutput
        ERROR_VARIABLE standardError
        TIMEOUT 60
    )
    if(NOT exitCode STREQUAL "0")
        message(FATAL_ERROR "${ARGN} exited with '${exitCode}'\n${standardError}")
    endif()
    set(COMMAND_OUTPUT "${standardOutput}" PARENT_SCOPE)
endfunction()

# Converts the source file <folder>/<fileName> of the work folder with srcml, runs stereocode -m with the given options on it, and sets
# COMMAND_OUTPUT to the source of the srcML output archive from srcml --output-src
# A second run of stereocode -m on the output archive must give the same archive
function(run_comments folder fileName)
    run_step("${SRCML}" "${folder}/${fileName}" -o "${folder}/case.xml")
    run_step("${STEREOCODE}" "${folder}/case.xml" -o "${folder}/case.stereotypes.xml" -m ${ARGN})
    run_step("${STEREOCODE}" "${folder}/case.stereotypes.xml" -o "${folder}/case.twice.xml" -m ${ARGN})
    file(READ "${WORK_DIR}/${folder}/case.stereotypes.xml" once)
    file(READ "${WORK_DIR}/${folder}/case.twice.xml" twice)
    if(NOT twice STREQUAL once)
        message(FATAL_ERROR "A second run of stereocode -m changes its output ${WORK_DIR}/${folder}/case.stereotypes.xml:\n${twice}")
    endif()
    run_step("${SRCML}" --output-src "${folder}/case.stereotypes.xml")
    set(COMMAND_OUTPUT "${COMMAND_OUTPUT}" PARENT_SCOPE)
endfunction()

# The comments of stereocode -m give back a labeled case of test/stereotypes in which every labeled element has its "// @stereotype" comment,
# which starts with # in a Python case
# The check converts a source with srcml, runs stereocode -m -f -i -t -e -u, and converts the srcML output archive back with
# srcml --output-src, which must give the case for the case without its comment lines, for the case as it is, for the case with
# a comment of an earlier run stacked above each of its comments, and for a source given after the case, whose elements follow
# other code on their lines
# The runs on the case as it is and on the stacked case show that the comment of each element takes the place of the comments
# before it instead of adding another
function(expect_round_trip caseFile)
    get_filename_component(CASE "${CMAKE_CURRENT_LIST_DIR}/../stereotypes/${caseFile}" ABSOLUTE)
    get_filename_component(CASE_FILE "${CASE}" NAME)
    file(READ "${CASE}" LABELED)

    set(COMMENT "// @stereotype")
    if(CASE_FILE MATCHES "\\.py$")
        set(COMMENT "# @stereotype")
    endif()

    # The line break before each comment line goes with the comment, so a newline in front covers a comment on the first line
    string(REGEX REPLACE "\n[ \t]*${COMMENT}[^\n]*" "" UNLABELED "\n${LABELED}")
    string(SUBSTRING "${UNLABELED}" 1 -1 UNLABELED)
    if(UNLABELED STREQUAL LABELED)
        message(FATAL_ERROR "${CASE} has no \"${COMMENT}\" comment")
    endif()
    string(REGEX REPLACE "\n([ \t]*)${COMMENT}" "\n\\1${COMMENT} command\n\\1${COMMENT}" STACKED "\n${LABELED}")
    string(SUBSTRING "${STACKED}" 1 -1 STACKED)

    set(variants unlabeled labeled stacked)
    if(ARGC GREATER 1)
        set(JOINED "${ARGV1}")
        list(APPEND variants joined)
    endif()

    # Each variant writes the source of the variable with its name in capitals
    foreach(variant IN LISTS variants)
        string(TOUPPER "${variant}" SOURCE)
        file(MAKE_DIRECTORY "${WORK_DIR}/${variant}")
        file(WRITE "${WORK_DIR}/${variant}/${CASE_FILE}" "${${SOURCE}}")

        run_comments(${variant} "${CASE_FILE}" -f -i -t -e -u)
        if(NOT COMMAND_OUTPUT STREQUAL LABELED)
            file(WRITE "${WORK_DIR}/${variant}/commented.${CASE_FILE}" "${COMMAND_OUTPUT}")
            message(FATAL_ERROR "For the ${variant} case, the source of the output of stereocode -m differs from ${CASE}:\n${COMMAND_OUTPUT}")
        endif()
    endforeach()
endfunction()

# The comments of -m give back the C++ case stereotypes/cpp/comment_output.cpp
function(check_round_trip_cpp)
    expect_round_trip(cpp/comment_output.cpp)
endfunction()

# The comments of -m give back the C# case stereotypes/csharp/comment_output.cs
function(check_round_trip_csharp)
    expect_round_trip(csharp/comment_output.cs)
endfunction()

# The comments of -m give back the Java case stereotypes/java/comment_output.java
function(check_round_trip_java)
    expect_round_trip(java/comment_output.java)
endfunction()

# The comments of -m give back the Python case stereotypes/python/comment_output.py
function(check_round_trip_python)
    expect_round_trip(python/comment_output.py)
endfunction()

# The comments of -m give back the TypeScript case stereotypes/javascript/comment_output.ts
function(check_round_trip_javascript)
    expect_round_trip(javascript/comment_output.ts)
endfunction()

# Without -f and -t, stereocode -m labels neither the free functions twice and square of the C++ case stereotypes/cpp/comment_output.cpp
# nor its struct Size with the method getWidth, so their comments of a run with every option go
# The class Account and its methods keep their stereotypes and their comments, since a friend function is no method of the class
function(check_stale_comments)
    get_filename_component(CASE "${CMAKE_CURRENT_LIST_DIR}/../stereotypes/cpp/comment_output.cpp" ABSOLUTE)
    file(READ "${CASE}" LABELED)
    string(REGEX REPLACE "[ \t]*// @stereotype [^\n]*\n([ \t]*(friend int twice|int square|typedef struct|int getWidth))" "\\1" EXPECTED "${LABELED}")

    file(WRITE "${WORK_DIR}/stale/comment_output.cpp" "${LABELED}")
    run_comments(stale comment_output.cpp)
    if(NOT COMMAND_OUTPUT STREQUAL EXPECTED)
        message(FATAL_ERROR "Without -f and -t, the source of the output of stereocode -m differs from the expected source:\n${COMMAND_OUTPUT}")
    endif()
endfunction()

# Without -f, stereocode -m labels no element of a unit that holds only free functions and a variable, so the stereotype comment directly
# above a function goes, while a comment of another kind and the stereotype comments above the variable and above an empty line stay
function(check_unit_without_labels)
    file(WRITE "${WORK_DIR}/plain/square.cpp" [=[
// @stereotype data-provider
int calls;

// Squares a value
// @stereotype property
int square(int value) { return value * value; }

// @stereotype property

int cube(int value) { return value * value * value; }
]=])
    set(EXPECTED [=[
// @stereotype data-provider
int calls;

// Squares a value
int square(int value) { return value * value; }

// @stereotype property

int cube(int value) { return value * value * value; }
]=])

    run_comments(plain square.cpp)
    if(NOT COMMAND_OUTPUT STREQUAL EXPECTED)
        message(FATAL_ERROR "The source of the output of stereocode -m for a unit without labels differs from the expected source:\n${COMMAND_OUTPUT}")
    endif()
endfunction()

# The comments of -m give the C++ case stereotypes/cpp/comment_after_code.cpp from a source whose methods and functions follow other code
# on their lines, such as a field, an earlier method, or a block comment
function(check_after_code_cpp)
    expect_round_trip(cpp/comment_after_code.cpp [=[
// stereocode -m moves each element after other code on its line below its "// @stereotype" comment, as in this file

class Point {
    int x; int y;
public:
    int getX() const { return x; } int getY() const { return y; }
    /* Sets x
       to a value */ void setX(int value) { x = value; }
};

struct Size { int width; int getWidth() const { return width; } };

int counter; int next() { return ++counter; }
]=])
endfunction()

# The comments of -m give the C# case stereotypes/csharp/comment_after_code.cs from a source whose accessors and methods follow other code
# on their lines
function(check_after_code_csharp)
    expect_round_trip(csharp/comment_after_code.cs [=[
// stereocode -m moves each element after other code on its line below its "// @stereotype" comment, as in this file

public class Account
{
    private int balance; private int limit;
    public int Balance { get { return balance; } set { balance = value; } }
    public int GetLimit() { return limit; } public void SetLimit(int value) { limit = value; }
}
]=])
endfunction()

# The comments of -m give the Java case stereotypes/java/comment_after_code.java from a source whose methods follow other code on their lines
function(check_after_code_java)
    expect_round_trip(java/comment_after_code.java [=[
// stereocode -m moves each element after other code on its line below its "// @stereotype" comment, as in this file

class Outer {
    private int a;
    static class Nested { int b; int getB() { return b; } }
    int getA() { return a; } void setA(int value) { a = value; }
}
]=])
endfunction()

run_check()
