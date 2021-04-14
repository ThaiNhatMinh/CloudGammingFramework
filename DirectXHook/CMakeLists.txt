file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(Hook SHARED ${SOURCES})
target_link_libraries(Hook PRIVATE ${DETOURS_LIB})
target_link_libraries(Hook PUBLIC d3d11)
target_link_libraries(Hook PUBLIC dxgi dxguid)
set_target_properties(Hook PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
