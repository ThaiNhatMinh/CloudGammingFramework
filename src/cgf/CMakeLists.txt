file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(cgf STATIC ${SOURCES})
