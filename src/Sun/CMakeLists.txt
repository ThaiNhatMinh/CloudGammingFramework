file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_executable(Sun ${SOURCES})
target_link_libraries(Sun common ipc)
