file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(common STATIC ${SOURCES})
if(WIN32)
    target_link_libraries(common wsock32 ws2_32)
endif()