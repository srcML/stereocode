# SPDX-License-Identifier: GPL-3.0-only
# @file CMakeLists.txt
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Every test of Stereocode, with the label of its kind: unit, integration, stereotypes, or systems

# The unit tests of the library in one program, with doctest 2.5.3 from https://github.com/doctest/doctest/releases/tag/v2.5.3
file(GLOB UNIT_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp")
add_executable(unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE stereocode_lib)
# A captured C string then shows its text in the report of a failed check
target_compile_definitions(unit_tests PRIVATE DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING)
add_test(NAME unit COMMAND unit_tests)
set_tests_properties(unit PROPERTIES LABELS unit)

# The helper program checks the "// @stereotype" comments of the stereotype cases and writes the tables of the system tests
add_executable(check_stereotypes stereotypes/check_stereotypes.cpp)
target_link_libraries(check_stereotypes PRIVATE LibXml2::LibXml2)
target_compile_options(check_stereotypes PRIVATE ${STEREOCODE_WARNINGS})

# Registers each check of <folder>/<topic>.cmake as the test <folder>/<topic>/<check>, labeled <folder>, with the definitions after DEFINE
function(add_script_checks folder topic)
    cmake_parse_arguments(PARSE_ARGV 2 ARG "" "" "CHECKS;DEFINE")
    foreach(CHECK IN LISTS ARG_CHECKS)
        add_test(
            NAME "${folder}/${topic}/${CHECK}"
            COMMAND ${CMAKE_COMMAND}
                -DCHECK=${CHECK}
                -DSTEREOCODE=$<TARGET_FILE:stereocode>
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/${folder}/${topic}/${CHECK}
                ${ARG_DEFINE}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/${topic}.cmake
        )
        set_tests_properties("${folder}/${topic}/${CHECK}" PROPERTIES LABELS ${folder})
    endforeach()
endfunction()

add_script_checks(integration options CHECKS version_without_input rejected_arguments help_footer option_names user_list_files
                                             readme_options
                  DEFINE -DREADME=${PROJECT_SOURCE_DIR}/README.md)
add_script_checks(integration input CHECKS no_filename no_language invalid_input damaged_input archive_without_code damaged_markup
                                           escape_in_expression declaration_pairs long_expression labeled_input)
add_script_checks(integration output CHECKS output_is_input unwritable_output position_archive destructor_position)
add_script_checks(integration reports CHECKS free_function_csv csv_without_srcml_output csv_beside_output csv_type_rows verbose_csv
                                             verbose_csv_details txt_report)
add_script_checks(integration types CHECKS nested_types python_local_code unlabeled_enums anonymous_class_objects typedef_types csharp_property
                                           large_class_threshold return_type_macros)
add_script_checks(integration threads CHECKS thread_counts)

# The preloaded library makes std::thread::hardware_concurrency() return 0, which glibc allows
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_library(zero_processors MODULE integration/zero_processors.cpp)
    target_compile_options(zero_processors PRIVATE ${STEREOCODE_WARNINGS})
    add_script_checks(integration threads CHECKS zero_processors DEFINE -DPRELOAD=$<TARGET_FILE:zero_processors>)
endif()

# The comment round trips, the stereotype cases, and the system tests turn source code into srcML with the srcml program
find_program(SRCML_EXECUTABLE srcml HINTS "$ENV{SystemDrive}/srcML/bin" "/usr/local/bin")

add_script_checks(integration comments CHECKS stereotype_comments)
if(SRCML_EXECUTABLE)
    add_script_checks(integration comments CHECKS round_trip_cpp round_trip_csharp round_trip_java round_trip_python round_trip_javascript stale_comments
                                                  unit_without_labels after_code_cpp after_code_csharp after_code_java
                      DEFINE -DSRCML=${SRCML_EXECUTABLE})
endif()
add_script_checks(stereotypes check_stereotypes CHECKS comments unchecked_elements table DEFINE -DCHECK_STEREOTYPES=$<TARGET_FILE:check_stereotypes>)

if(NOT SRCML_EXECUTABLE)
    if(STEREOCODE_REQUIRE_SRCML)
        message(FATAL_ERROR "The srcml program is missing, and the comment round trips, the stereotype cases, and the system tests need it")
    endif()
    message(STATUS "Comment round trips, stereotype cases, and system tests skipped: they need the srcml program")
    return()
endif()

# Each case of stereotypes/<language>/ is the test stereotypes/<language>/<case>, which compares its "// @stereotype" comments with the labels
# A case is one source file, or a folder whose files form one archive, and any other entry stops the configuration, since it would never run
include(stereotypes/case_files.cmake)
list(TRANSFORM CASE_LANGUAGES REPLACE "(.+)" "${CMAKE_CURRENT_SOURCE_DIR}/stereotypes/\\1/*" OUTPUT_VARIABLE CASE_GLOBS)
file(GLOB STEREOTYPE_CASES CONFIGURE_DEPENDS LIST_DIRECTORIES true ${CASE_GLOBS})

foreach(CASE_PATH ${STEREOTYPE_CASES})
    if(NOT IS_DIRECTORY "${CASE_PATH}" AND NOT CASE_PATH MATCHES "${CASE_FILE}")
        message(FATAL_ERROR "${CASE_PATH} is neither a source file of a stereotype case nor a folder of one")
    endif()

    file(RELATIVE_PATH CASE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/stereotypes" "${CASE_PATH}")  # e.g., cpp/method_accessors.cpp
    string(REGEX REPLACE "${CASE_FILE}" "" CASE_NAME "${CASE_NAME}")                         # e.g., cpp/method_accessors

    add_test(
        NAME "stereotypes/${CASE_NAME}"
        COMMAND ${CMAKE_COMMAND}
            -DSRCML=${SRCML_EXECUTABLE}
            -DSTEREOCODE=$<TARGET_FILE:stereocode>
            -DCHECK_STEREOTYPES=$<TARGET_FILE:check_stereotypes>
            -DCASE=${CASE_PATH}
            -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/stereotypes/${CASE_NAME}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/stereotypes/run_case.cmake
    )
    set_tests_properties("stereotypes/${CASE_NAME}" PROPERTIES LABELS stereotypes TIMEOUT 120)
endforeach()

# Each zip of stereotypes/systems/ is the test stereotypes/systems/<system>, which compares the labels of the system with the table <system>.tsv
file(GLOB SYSTEM_ZIPS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/stereotypes/systems/*.zip")
set(UPDATE_COMMANDS "")

foreach(ZIP ${SYSTEM_ZIPS})
    get_filename_component(SYSTEM "${ZIP}" NAME_WLE)

    # The headers of Notepad++ hold C++ classes, which srcml parses as C code without the option
    set(SRCML_OPTIONS "")
    if(SYSTEM MATCHES "^notepad-plus-plus-")
        set(SRCML_OPTIONS --register-ext=h=C++)
    endif()

    set(SYSTEM_ARGUMENTS
        -DSRCML=${SRCML_EXECUTABLE}
        -DSRCML_OPTIONS=${SRCML_OPTIONS}
        -DSTEREOCODE=$<TARGET_FILE:stereocode>
        -DCHECK_STEREOTYPES=$<TARGET_FILE:check_stereotypes>
        -DZIP=${ZIP}
        -DREFERENCE=${CMAKE_CURRENT_SOURCE_DIR}/stereotypes/systems/${SYSTEM}.tsv
    )
    add_test(
        NAME "stereotypes/systems/${SYSTEM}"
        COMMAND ${CMAKE_COMMAND} ${SYSTEM_ARGUMENTS}
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/stereotypes/systems/${SYSTEM}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/stereotypes/run_system.cmake
    )
    # A system takes far longer than a case, so the label systems lets ctest -LE systems leave the systems out
    # Each system runs on up to 8 threads with up to 1 GB of memory, and the lock keeps ctest -j from running two systems at once
    set_tests_properties("stereotypes/systems/${SYSTEM}" PROPERTIES LABELS systems TIMEOUT 600 RESOURCE_LOCK system_tests)

    list(APPEND UPDATE_COMMANDS
        COMMAND ${CMAKE_COMMAND} ${SYSTEM_ARGUMENTS}
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/update_system_tables/${SYSTEM}
            -DUPDATE=ON
            -P ${CMAKE_CURRENT_SOURCE_DIR}/stereotypes/run_system.cmake
    )
endforeach()

# An intended change of the labels replaces the tables of stereotypes/systems/ through this target, from the zips beside them
add_custom_target(update_system_tables ${UPDATE_COMMANDS} COMMENT "Writing the tables of the system tests" VERBATIM)
add_dependencies(update_system_tables stereocode check_stereotypes)
