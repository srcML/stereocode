# Remove generated XML files (If they exist already)
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -f ${TEST_FILE}.stereotypes.xml)

# Run stereocode on the test file
execute_process(COMMAND ${STEREOCODE} ${TEST_FILE}.xml)

# Compare the BASE report file to the generated XML file
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_FILE}.BASE.xml ${TEST_FILE}.stereotypes.xml COMMAND_ERROR_IS_FATAL ANY)