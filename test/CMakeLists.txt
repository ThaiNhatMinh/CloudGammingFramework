file(GLOB SOURCES "*.cpp")
include(Dart)

foreach(test_file ${SOURCES})
    string(REPLACE ".cpp" "" EXEC_FILE ${test_file})
    get_filename_component(ITEM_NAME ${EXEC_FILE} NAME)
    add_executable(${ITEM_NAME} ${test_file})
    target_link_libraries(${ITEM_NAME} ipc SunForTest satellite planet)
    add_test(${ITEM_NAME} ${ITEM_NAME})
endforeach(test_file ${SOURCE})
