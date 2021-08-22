file(GLOB SOURCES "*.cpp")

# add_executable(pipeserver "NamedPipek.cpp")
# add_executable(pipeclient "NamedPipeClient.cpp")

foreach(test_file ${SOURCES})
    string(REPLACE ".cpp" "" EXEC_FILE ${test_file})
    get_filename_component(ITEM_NAME ${EXEC_FILE} NAME)
    add_executable(${ITEM_NAME} ${test_file})
    target_link_libraries(${ITEM_NAME} ipc)
endforeach(test_file ${SOURCE})
