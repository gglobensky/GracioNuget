cmake_minimum_required(VERSION 3.15)
project(GracioNative LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the DLL output name
set(TARGET_NAME "gracio_native")

# Include directories
include_directories(native/include)

# Source files
set(SOURCES 
    native/src/gracio_core.cpp 
    native/src/gracio_api.cpp
)

# Create the Shared Library (DLL)
add_library(${TARGET_NAME} SHARED ${SOURCES})

# Windows specific: Ensure symbols are exported correctly
set_target_properties(${TARGET_NAME} PROPERTIES 
    PREFIX "" 
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out/bin/native"
)

message(STATUS "Gracio Native build configured for ${CMAKE_SYSTEM_PROCESSOR}")
