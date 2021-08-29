file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(satellite STATIC ${SOURCES})
target_link_libraries(satellite ipc)
