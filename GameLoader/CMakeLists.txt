file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_executable(GameLoader ${SOURCES})
target_link_libraries(GameLoader PRIVATE ${DETOURS_LIB})
set_target_properties(GameLoader PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

