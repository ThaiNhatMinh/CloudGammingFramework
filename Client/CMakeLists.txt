file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_executable(Client ${SOURCES})
target_link_libraries(Client PRIVATE common)
target_link_libraries(Client PRIVATE glfw)
target_link_libraries(Client PRIVATE glad)
target_link_libraries(Client PRIVATE imgui)
find_package(OpenGL REQUIRED)
target_link_libraries(Client PUBLIC opengl32)

message("Platform ${CMAKE_GENERATOR_PLATFORM}")