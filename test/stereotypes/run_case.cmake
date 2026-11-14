# SPDX-License-Identifier: GPL-3.0-only
# @file run_case.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Runs one stereotype case: srcml on the case, stereocode with -f -i -t -e -u, and check_stereotypes comments
# Inputs: SRCML, STEREOCODE, CHECK_STEREOTYPES, CASE (a source file or a folder), SOURCE_DIR, WORK_DIR

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

# The source files of a folder case go into one archive in sorted order; paths stay relative to the repository
if(IS_DIRECTORY "${CASE}")
    file(GLOB_RECURSE CASE_FILES LIST_DIRECTORIES false RELATIVE "${SOURCE_DIR}" "${CASE}/*")
    include("${CMAKE_CURRENT_LIST_DIR}/case_files.cmake")
    list(FILTER CASE_FILES INCLUDE REGEX "${CASE_FILE}")
    list(SORT CASE_FILES)
else()
    file(RELATIVE_PATH CASE_FILES "${SOURCE_DIR}" "${CASE}")
endif()

execute_process(
    COMMAND "${SRCML}" ${CASE_FILES} -o "${WORK_DIR}/case.xml"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE SRCML_EXIT_CODE
)
if(NOT SRCML_EXIT_CODE EQUAL 0)
    message(FATAL_ERROR "srcml failed with exit code '${SRCML_EXIT_CODE}'")
endif()

execute_process(
    COMMAND "${STEREOCODE}" "${WORK_DIR}/case.xml" -o "${WORK_DIR}/case.stereotypes.xml" -f -i -t -e -u
    RESULT_VARIABLE STEREOCODE_EXIT_CODE
)
if(NOT STEREOCODE_EXIT_CODE EQUAL 0)
    message(FATAL_ERROR "stereocode failed with exit code '${STEREOCODE_EXIT_CODE}'")
endif()

execute_process(
    COMMAND "${CHECK_STEREOTYPES}" comments "${WORK_DIR}/case.stereotypes.xml"
    RESULT_VARIABLE CHECKER_EXIT_CODE
)
if(NOT CHECKER_EXIT_CODE EQUAL 0)
    message(FATAL_ERROR "The expected stereotypes of the case do not hold")
endif()
