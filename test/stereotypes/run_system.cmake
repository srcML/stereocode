# SPDX-License-Identifier: GPL-3.0-only
# @file run_system.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Runs one system test: srcml --position on the zip, stereocode with -f -i -t -e -u, and check_stereotypes table against REFERENCE
# Inputs: SRCML, SRCML_OPTIONS (may be empty), STEREOCODE, CHECK_STEREOTYPES, ZIP, REFERENCE, WORK_DIR, and UPDATE (ON to write the reference)

get_filename_component(SYSTEM "${ZIP}" NAME_WLE)
set(ARCHIVE "${WORK_DIR}/${SYSTEM}.xml")
set(OUTPUT "${WORK_DIR}/${SYSTEM}.stereotypes.xml")
set(SRCML_LOG "${WORK_DIR}/srcml.log")

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

# srcml exits with 1 for the files of a zip in languages that it does not parse, so only a missing or empty archive fails the run
execute_process(
    COMMAND "${SRCML}" --quiet --position ${SRCML_OPTIONS} "${ZIP}" -o "${ARCHIVE}"
    RESULT_VARIABLE SRCML_EXIT_CODE
    ERROR_FILE "${SRCML_LOG}"
)
set(ARCHIVE_SIZE 0)
if(EXISTS "${ARCHIVE}")
    file(SIZE "${ARCHIVE}" ARCHIVE_SIZE)
endif()
if(NOT SRCML_EXIT_CODE MATCHES "^[01]$" OR ARCHIVE_SIZE EQUAL 0)
    message(FATAL_ERROR "srcml failed with exit code '${SRCML_EXIT_CODE}', see ${SRCML_LOG}")
endif()

execute_process(
    COMMAND "${STEREOCODE}" "${ARCHIVE}" -o "${OUTPUT}" -f -i -t -e -u
    RESULT_VARIABLE STEREOCODE_EXIT_CODE
)
if(NOT STEREOCODE_EXIT_CODE EQUAL 0)
    message(FATAL_ERROR "stereocode failed with exit code '${STEREOCODE_EXIT_CODE}'")
endif()

if(UPDATE)
    execute_process(COMMAND "${CHECK_STEREOTYPES}" table "${OUTPUT}" "${REFERENCE}" RESULT_VARIABLE TABLE_EXIT_CODE)
else()
    set(TABLE_FILE "${WORK_DIR}/${SYSTEM}.tsv")
    execute_process(COMMAND "${CHECK_STEREOTYPES}" table "${OUTPUT}" "${TABLE_FILE}" "${REFERENCE}" RESULT_VARIABLE TABLE_EXIT_CODE)
    if(TABLE_EXIT_CODE EQUAL 1)
        message(FATAL_ERROR "The table ${TABLE_FILE} differs from ${REFERENCE}. "
                            "For an intended change, the target update_system_tables writes the new tables.")
    endif()
endif()
if(NOT TABLE_EXIT_CODE EQUAL 0)
    message(FATAL_ERROR "check_stereotypes table failed with exit code '${TABLE_EXIT_CODE}'")
endif()

# The archives of a system take hundreds of megabytes, so a run that passes keeps only its table
file(REMOVE "${ARCHIVE}" "${OUTPUT}" "${SRCML_LOG}")
