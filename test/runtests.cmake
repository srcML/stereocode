# Remove generated XML files (If they exist already)
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -f ${TEST_FILE}.stereotypes.xml)

# Run stereocode AND check if it crashed (free functions, interfaces, structs, enums, and unions are considered)
execute_process(
    COMMAND ${STEREOCODE} ${TEST_FILE}.xml -f -i -t -e -u
    RESULT_VARIABLE TOOL_EXIT_CODE
)

if(NOT TOOL_EXIT_CODE EQUAL 0)
    message(FATAL_ERROR "Test Failed: Stereocode crashed or failed with exit code '${TOOL_EXIT_CODE}'")
endif()

# Compare the BASE report file to the generated XML file
execute_process(
    COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_FILE}.BASE.xml ${TEST_FILE}.stereotypes.xml
    COMMAND_ERROR_IS_FATAL ANY
)