file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(Sun STATIC ${SOURCES})
