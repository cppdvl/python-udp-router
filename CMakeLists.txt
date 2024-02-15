cmake_minimum_required(VERSION 3.12)

# Project name and a few useful settings. Other commands can pick up the results
project(router
        VERSION 1.0
        DESCRIPTION "A stream router" 
        LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the executable
add_executable(${PROJECT_NAME} streamrouter.cpp)

# Set the compile options you want
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:-Og -g>
    -Wall -Wextra)

# Set the default build type to Release if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui, ccmake
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

