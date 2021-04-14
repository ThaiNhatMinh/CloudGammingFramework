cmake_minimum_required(VERSION 3.0)
project(CloudGamming VERSION 1.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories($ENV{UniversalCRTSdkDir}/Include/ucrt)
include_directories($ENV{UniversalCRTSdkDir}/Include/10.0.10056.0/ucrt)
include_directories(Lib)

find_library(DETOURS_LIB detours HINTS "./Lib")

# add_compile_definitions(LOG_TO_FILE)

add_subdirectory(DirectXHook)
add_subdirectory(GameLoader)