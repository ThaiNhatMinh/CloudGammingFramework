file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(cgf STATIC ${SOURCES})
target_link_libraries(cgf planet)