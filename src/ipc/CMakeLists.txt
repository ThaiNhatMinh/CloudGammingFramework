file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(ipc STATIC ${SOURCES})
target_link_libraries(ipc common)