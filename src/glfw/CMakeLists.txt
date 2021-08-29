file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(glfwrapper STATIC ${SOURCES})
target_link_libraries(glfwrapper glfw)