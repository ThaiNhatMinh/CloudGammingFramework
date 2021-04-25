file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(common STATIC ${SOURCES})
target_link_libraries(common PUBLIC Interception)
if(WIN32)
    target_link_libraries(common PUBLIC wsock32 ws2_32)
endif()