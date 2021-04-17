file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp" "src/*.cpp" "src/*.c")

add_library(glad STATIC ${SOURCES})