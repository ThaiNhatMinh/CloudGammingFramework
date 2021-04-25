file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.c" "src/*.cpp" "src/*.c")

add_compile_definitions(INTERCEPTION_STATIC)
add_library(Interception STATIC ${SOURCES})