file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(planet STATIC ${SOURCES})
target_link_libraries(planet ipc)
