
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)

add_subdirectory(glad)
add_subdirectory(glfw)

file(GLOB IMGUI_SOURCES "imgui/*.cpp" "imgui/backends/imgui_impl_glfw.cpp" "imgui/backends/imgui_impl_opengl3.cpp")

include_directories(imgui)
add_library(imgui STATIC ${IMGUI_SOURCES})

include_directories(imgui/backends)
find_package(OpenGL REQUIRED)
add_executable(imgui_example "imgui/examples/example_glfw_opengl3/main.cpp")
target_link_libraries(imgui_example glfw imgui ${OPENGL_LIBRARIES})
