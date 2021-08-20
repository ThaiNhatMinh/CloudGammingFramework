file(GLOB HEADERS "*.h" "*.hh")
file(GLOB SOURCES "*.cpp")

add_library(common2 STATIC ${SOURCES})
if(WIN32)
    target_link_libraries(common2 PUBLIC wsock32 ws2_32)
endif()