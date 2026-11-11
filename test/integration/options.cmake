# SPDX-License-Identifier: GPL-3.0-only
# @file options.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Integration checks of the command line, with the names of the options, the user list files, and the option table of README.md
# test/CMakeLists.txt registers each function check_<name> as the test integration/options/<name>

include(${CMAKE_CURRENT_LIST_DIR}/run_and_check.cmake)

# --version prints the version without an input archive
function(check_version_without_input)
    run_stereocode(--version)
    expect_exit(0)
    if(NOT "${RUN_OUTPUT}" MATCHES "^Stereocode v[0-9]")
        message(FATAL_ERROR "Unexpected version output '${RUN_OUTPUT}'")
    endif()
endfunction()

# Fails unless the second folder holds the files of the first folder, each with the same content
function(expect_same_files expectedFolder actualFolder)
    file(GLOB expectedFiles RELATIVE "${WORK_DIR}/${expectedFolder}" "${WORK_DIR}/${expectedFolder}/*")
    file(GLOB actualFiles RELATIVE "${WORK_DIR}/${actualFolder}" "${WORK_DIR}/${actualFolder}/*")
    if(NOT expectedFiles OR NOT actualFiles STREQUAL expectedFiles)
        message(FATAL_ERROR "${actualFolder} holds '${actualFiles}', while ${expectedFolder} holds '${expectedFiles}'")
    endif()
    foreach(fileName ${expectedFiles})
        file(READ "${WORK_DIR}/${expectedFolder}/${fileName}" expected)
        expect_file_content("${WORK_DIR}/${actualFolder}/${fileName}" "${expected}")
    endforeach()
endfunction()

# Each name of an option gives the outputs of its other names, the older name --srcmlOutput of -d included
#
# The archive holds these files:
#   shapes.cpp   class Box { int w; public: int width() const { return w; } };
#                struct Point { int x; int getX() const { return x; } };
#                union Number { int i; int get() const { return i; } };
#                int twice(int v) { return 2 * v; }
#   Shape.cs     interface IShape { IShape Self() { return this; } }
#   Color.java   enum Color { RED; int code() { return 1; } }
function(check_option_names)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit revision="1.0.0" language="C++" filename="shapes.cpp"><class>class <name>Box</name> <block>{<private type="default"> <decl_stmt><decl><type><name>int</name></type> <name>w</name></decl>;</decl_stmt> </private><public>public: <function><type><name>int</name></type> <name>width</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function> </public>}</block>;</class>
<struct>struct <name>Point</name> <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function> </public>}</block>;</struct>
<union>union <name>Number</name> <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>i</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>i</name></expr>;</return> </block_content>}</block></function> </public>}</block>;</union>
<function><type><name>int</name></type> <name>twice</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">2</literal> <operator>*</operator> <name>v</name></expr>;</return> </block_content>}</block></function>
</unit>

<unit revision="1.0.0" language="C#" filename="Shape.cs"><interface>interface <name>IShape</name> <block>{ <function><type><name>IShape</name></type> <name>Self</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>this</name></expr>;</return> </block_content>}</block></function> }</block></interface>
</unit>

<unit revision="1.0.0" language="Java" filename="Color.java"><enum>enum <name>Color</name> <block>{ <decl><name>RED</name></decl>; <function><type><name>int</name></type> <name>code</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">1</literal></expr>;</return> </block_content>}</block></function> }</block></enum>
</unit>

</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")
    file(WRITE "${WORK_DIR}/primitives.txt" "Timestamp\n")
    file(WRITE "${WORK_DIR}/calls.txt" "log\n")
    file(WRITE "${WORK_DIR}/specifiers.txt" "MY_API\n")
    file(WRITE "${WORK_DIR}/queries.txt" "isNull\n")
    file(MAKE_DIRECTORY "${WORK_DIR}/short" "${WORK_DIR}/long" "${WORK_DIR}/d" "${WORK_DIR}/no-srcml-output" "${WORK_DIR}/srcmlOutput")

    # The archive gives each kind of type and the free function its own labels, and -b prints the lists and the threshold
    run_stereocode(input.xml -o short/out.xml -p primitives.txt -c calls.txt -s specifiers.txt -q queries.txt -l 4 -j 2
                   -f -i -u -e -t -r -x -b -m)
    expect_exit(0)
    set(SHORT_ERROR "${RUN_ERROR}")

    run_stereocode(input.xml --output-file long/out.xml --primitive-file primitives.txt --call-file calls.txt
                   --specifier-file specifiers.txt --query-file queries.txt --large-class 4 --threads 2
                   --free-function --interface --union --enum --struct --csv-report --txt-report --verbose --comment)
    expect_exit(0)
    if(NOT RUN_ERROR STREQUAL SHORT_ERROR)
        message(FATAL_ERROR "The long names print other lists or another threshold than the short names:\n${RUN_ERROR}\nExpected:\n${SHORT_ERROR}")
    endif()
    expect_same_files(short long)

    foreach(name -d --no-srcml-output --srcmlOutput)
        string(REGEX REPLACE "^-+" "" folder "${name}")
        run_stereocode(input.xml ${name} -r -o ${folder}/out.xml)
        expect_exit(0)
        expect_missing("${WORK_DIR}/${folder}/out.xml")
    endforeach()
    expect_same_files(d no-srcml-output)
    expect_same_files(d srcmlOutput)

    foreach(names "-v;--version" "-h;--help")
        list(GET names 0 shortName)
        list(GET names 1 longName)
        run_stereocode(${longName})
        expect_exit(0)
        set(longOutput "${RUN_OUTPUT}")
        run_stereocode(${shortName})
        expect_exit(0)
        if(NOT RUN_OUTPUT STREQUAL longOutput)
            message(FATAL_ERROR "${shortName} prints other text than ${longName}:\n${RUN_OUTPUT}\nExpected:\n${longOutput}")
        endif()
    endforeach()
endfunction()

# The files of -p, -c, -s, and -q must exist, and their readers skip comments and blank lines and trim each entry
# An entry is plain text, whatever characters it holds
# The entries of the files below are QString, logMessage, MY_API, and measure, and the verbose run lists them
# A section header, such as [C++], gives the entries after it to the units of its language, and C units take the C++ entries
# A line that names a language in another case, such as [c++], is an entry, and the run warns about it
#
# The C++ file:
#   class Label {
#       QString text;
#   public:
#       QString getText() const { return text; }
#   };
#
# The C++ file of the query methods, where a call of a query method on a data member reads the data member:
#   class Label {
#       QString text;
#   public:
#       int width() { return text.measure(); }
#   };
#
# The C file:
#   void report(void) { logMessage(); }
function(check_user_list_files)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="label.cpp"><class>class <name>Label</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>QString</name></type> <name>text</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>QString</name></type> <name>getText</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>text</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>
]=])

    set(QUERY_INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="label.cpp"><class>class <name>Label</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>QString</name></type> <name>text</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>width</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><call><name><name>text</name><operator>.</operator><name>measure</name></name><argument_list>()</argument_list></call></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>
]=])

    set(C_INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C" filename="report.c"><function><type><name>void</name></type> <name>report</name><parameter_list>(<parameter><decl><type><name>void</name></type></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><call><name>logMessage</name><argument_list>()</argument_list></call></expr>;</expr_stmt> </block_content>}</block></function>
</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")
    file(WRITE "${WORK_DIR}/query_input.xml" "${QUERY_INPUT}")
    file(WRITE "${WORK_DIR}/c_input.xml" "${C_INPUT}")

    # QString is another class unless the user lists it as a primitive
    run_stereocode(input.xml -o plain.xml)
    expect_exit(0)
    file(READ "${WORK_DIR}/plain.xml" plainOutput)
    if(NOT plainOutput MATCHES "st:stereotype=\"get collaborator\"")
        message(FATAL_ERROR "Expected getText to be a get collaborator without a primitives file:\n${plainOutput}")
    endif()

    file(WRITE "${WORK_DIR}/primitives.txt" "# Qt types\r\n\r\n  QString \r\n")
    file(WRITE "${WORK_DIR}/calls.txt" "# Logging\n\nlogMessage\t\n")
    file(WRITE "${WORK_DIR}/specifiers.txt" "MY_API\n\n# Export macros\n")
    file(WRITE "${WORK_DIR}/queries.txt" "# Qt\r\n measure\n")

    foreach(option -p -c -s -q)
        run_stereocode(input.xml ${option} missing.txt)
        expect_exit(105)
        if(NOT "${RUN_ERROR}" MATCHES "File does not exist: missing.txt")
            message(FATAL_ERROR "The error of ${option} does not name the missing file: ${RUN_ERROR}")
        endif()
        expect_missing("${WORK_DIR}/input.stereotypes.xml")
    endforeach()

    # A file that exists but cannot be read stops the run, unless the permissions do not keep the test from reading it, as for root
    file(WRITE "${WORK_DIR}/locked.txt" "QString\n")
    file(CHMOD "${WORK_DIR}/locked.txt" PERMISSIONS OWNER_WRITE)
    execute_process(COMMAND ${CMAKE_COMMAND} -E cat "${WORK_DIR}/locked.txt" RESULT_VARIABLE lockedReadable OUTPUT_QUIET ERROR_QUIET)
    if(NOT lockedReadable EQUAL 0)
        foreach(option -p -c -s -q)
            run_stereocode(input.xml ${option} locked.txt)
            expect_error()
            if(NOT "${RUN_ERROR}" MATCHES "cannot read the [a-z ]+ file locked.txt")
                message(FATAL_ERROR "The error of ${option} does not name the unreadable file: ${RUN_ERROR}")
            endif()
            expect_missing("${WORK_DIR}/input.stereotypes.xml")
        endforeach()
    endif()

    run_stereocode(input.xml -b -p primitives.txt -c calls.txt -s specifiers.txt -q queries.txt)
    expect_exit(0)
    file(READ "${WORK_DIR}/input.stereotypes.xml" output)
    if(NOT output MATCHES "<class st:stereotype=\"data-provider data-class small-class\">" OR NOT output MATCHES "<function st:stereotype=\"get\">")
        message(FATAL_ERROR "QString of the primitives file is not a primitive type:\n${output}")
    endif()
    foreach(entry QString logMessage MY_API measure)
        if(NOT "${RUN_ERROR}" MATCHES "\\[User-Defined\\]: ${entry}\n")
            message(FATAL_ERROR "The verbose output does not list ${entry} alone:\n${RUN_ERROR}")
        endif()
    endforeach()

    # A call of a listed query method on a data member reads the data member, while a call of another method changes it
    run_stereocode(query_input.xml -p primitives.txt -o query_plain.xml)
    expect_exit(0)
    file(READ "${WORK_DIR}/query_plain.xml" queryOutput)
    if(NOT queryOutput MATCHES "<function st:stereotype=\"property non-void-command\">")
        message(FATAL_ERROR "Expected width to call a method that changes the data member without a query methods file:\n${queryOutput}")
    endif()
    run_stereocode(query_input.xml -p primitives.txt -q queries.txt -o query_listed.xml)
    expect_exit(0)
    file(READ "${WORK_DIR}/query_listed.xml" queryOutput)
    if(NOT queryOutput MATCHES "<function st:stereotype=\"property\">")
        message(FATAL_ERROR "measure of the query methods file is not a query method:\n${queryOutput}")
    endif()

    # Specifiers with the characters of regular expressions are removed as written
    file(WRITE "${WORK_DIR}/regex_specifiers.txt" "*\n[[nodiscard]]\n(\n")
    run_stereocode(input.xml -s regex_specifiers.txt -o specifier.xml)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/specifier.xml" "${plainOutput}")

    # A primitive of the C# section leaves QString another class of the C++ unit, while one of the C++ section makes it a primitive type
    file(WRITE "${WORK_DIR}/csharp_primitives.txt" "# Qt types\n[C#]\nQString\n")
    run_stereocode(input.xml -p csharp_primitives.txt -o csharp_section.xml)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/csharp_section.xml" "${plainOutput}")

    file(WRITE "${WORK_DIR}/cpp_primitives.txt" "[C#]\nGuid\n\n  [C++] \r\nQString\n")
    run_stereocode(input.xml -b -p cpp_primitives.txt -o cpp_section.xml)
    expect_exit(0)
    file(READ "${WORK_DIR}/cpp_section.xml" sectionOutput)
    if(NOT sectionOutput MATCHES "<function st:stereotype=\"get\">")
        message(FATAL_ERROR "QString of the C++ section is not a primitive type of the C++ unit:\n${sectionOutput}")
    endif()
    if(NOT "${RUN_ERROR}" MATCHES "\\[User-Defined C\\+\\+\\]: QString\n" OR NOT "${RUN_ERROR}" MATCHES "\\[User-Defined C#\\]: Guid\n")
        message(FATAL_ERROR "The verbose output does not list QString under C++ and Guid under C#:\n${RUN_ERROR}")
    endif()

    # A call of the C++ section is ignored in a C unit, while a call of the C# section is not
    file(WRITE "${WORK_DIR}/cpp_calls.txt" "[C++]\nlogMessage\n")
    file(WRITE "${WORK_DIR}/csharp_calls.txt" "[C#]\nlogMessage\n")
    foreach(section cpp csharp)
        run_stereocode(c_input.xml -f -c ${section}_calls.txt -o c_${section}.xml)
        expect_exit(0)
    endforeach()
    file(READ "${WORK_DIR}/c_cpp.xml" cppSectionOutput)
    file(READ "${WORK_DIR}/c_csharp.xml" csharpSectionOutput)
    if(NOT cppSectionOutput MATCHES "<function st:stereotype=\"literal\">" OR NOT csharpSectionOutput MATCHES "<function st:stereotype=\"literal wrapper\">")
        message(FATAL_ERROR "The C unit does not take the calls of the C++ section alone:\n${cppSectionOutput}\n${csharpSectionOutput}")
    endif()

    # A header in another case is an entry for every language, which a warning names with its file
    file(WRITE "${WORK_DIR}/lower_primitives.txt" "[c++]\nQString\n[C#]\nGuid\n")
    run_stereocode(input.xml -p lower_primitives.txt -o lower_section.xml)
    expect_exit(0)
    if(NOT "${RUN_ERROR}" MATCHES "Warning: the line \\[c\\+\\+\\] of the primitive types file lower_primitives.txt is an entry" OR "${RUN_ERROR}" MATCHES "\\[C#\\] of")
        message(FATAL_ERROR "The run does not warn about the header [c++] alone: ${RUN_ERROR}")
    endif()
endfunction()

# Gets the names of options that a text gives after a space or a comma, as in "  -o,     --output-file TEXT"
function(option_names text result)
    string(REGEX MATCHALL "[ ,]--?[a-zA-Z][a-zA-Z-]*" names " ${text}")
    string(REGEX REPLACE "(^|;)[ ,]" "\\1" names "${names}")
    list(REMOVE_DUPLICATES names)
    set(${result} "${names}" PARENT_SCOPE)
endfunction()

# The options table of README.md names exactly the options of stereocode --help
# Each row of the table starts with the names of one option in code spans, such as | `-o`, `--output-file` |
function(check_readme_options)
    run_stereocode(--help)
    expect_exit(0)

    file(STRINGS "${README}" tableRows REGEX "^\\| `-")
    set(readmeOptions "")
    foreach(row IN LISTS tableRows)
        string(REGEX MATCH "^\\| ([^|]*)\\|" firstCell "${row}")
        string(REPLACE "`" "" firstCell "${CMAKE_MATCH_1}")
        option_names("${firstCell}" names)
        list(APPEND readmeOptions ${names})
    endforeach()

    # The help lists each option at the start of a line, before its description
    string(REGEX MATCHALL "\n  -[^\n]*" helpLines "${RUN_OUTPUT}")
    set(helpOptions "")
    foreach(line IN LISTS helpLines)
        string(REGEX MATCH "^\n  (-[a-zA-Z],[ ]+--[a-zA-Z-]+(, --[a-zA-Z-]+)*)" names "${line}")
        option_names("${CMAKE_MATCH_1}" names)
        list(APPEND helpOptions ${names})
    endforeach()

    list(SORT readmeOptions)
    list(SORT helpOptions)
    if(NOT readmeOptions STREQUAL helpOptions OR readmeOptions STREQUAL "")
        message(FATAL_ERROR "The options of README.md differ from those of stereocode --help\nREADME.md: ${readmeOptions}\n--help:    ${helpOptions}")
    endif()
endfunction()

run_check()
