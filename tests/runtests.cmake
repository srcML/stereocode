# Remove generated report files (If they exist already)
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -f ${TEST_FILE}.xml.report.txt)

# Run stereocode on the test file
execute_process(COMMAND ${STEREOCODE} ${TEST_FILE}.xml -d)

# Compare the BASE report file to the generated report file
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_FILE}.xml.BASE.txt ${TEST_FILE}.xml.report.txt COMMAND_ERROR_IS_FATAL ANY)