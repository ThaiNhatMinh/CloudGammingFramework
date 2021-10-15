file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(cgf STATIC ${SOURCES})
find_package(OpenGL REQUIRED)
target_link_libraries(cgf planet satellite opengl32 ${OPENGL_LIBRARIES})