# Remove generated files (if they exist already)
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -f ${TEST_FILE}.stereotypes.xml)
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -f ${TEST_FILE}.report.txt)

# Run stereocode on the test file
execute_process(COMMAND ${STEREOCODE} ${TEST_FILE}.xml)

# Compare the BASE file to the generated annotated file
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_FILE}.BASE.xml ${TEST_FILE}.stereotypes.xml COMMAND_ERROR_IS_FATAL ANY)