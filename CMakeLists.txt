# CMake file to build and run natively on macOS, Linux or Windows systems.
# 
# EpoxyDuino Core from https://github.com/bxparks/EpoxyDuino MIT License
#
# [X] EpoxyDuino Core EPOXY_CORE_AVR & EPOXY_CORE_ESP8266 (-DUSE_ESP8266_CORE=ON)
# [X] Examples
# [ ] Libraries
# [ ] Tests (Requires: AUnit and AceCommon from https://github.com/bxparks/)  
#  
# MIT License

cmake_minimum_required( VERSION 3.18 )

project( EpoxyDuino LANGUAGES CXX )

# Set C++ standard
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED TRUE )

# Set compiler identification
if( NOT BUILD_COMPILER )
   set( BUILD_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}" )
   message( STATUS "C++ compiler: " ${BUILD_COMPILER} )
endif()

# If macOS builds a Mach-O universal binary with 2 architectures: x86_64 and arm64 for Apple Silicon processors
if ( APPLE ) 
   set( CMAKE_OSX_ARCHITECTURES "x86_64;arm64" )
   set( CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>" )
endif()

# Set build type to "Debug"
set( CMAKE_BUILD_TYPE "Debug" )

# Ensures all binaries are placed in the same output directory
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR} ) # Dynamic
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR} ) # Static

# Setting build type to "debug" add only -g 
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG" )

# Set C++ compiler flags
if ( CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic" )
   set( COMPILE_FLAGS "-x c++" )
elseif ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
   set( MSVC_DISABLED_WARNINGS_LIST
      "C4996" # warning C4996: may be unsafe/disable deprecation
      "C4244" # warning C4244: conversion from 'LONGLONG' to 'unsigned long'
      "C4267" # warning C4267: conversion from 'size_t' to 'unsigned int'
   )
   string( REPLACE "C" " -wd" MSVC_DISABLED_WARNINGS_STR ${MSVC_DISABLED_WARNINGS_LIST} )
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 ${MSVC_DISABLED_WARNINGS_STR}" )
   set( COMPILE_FLAGS "" )
endif()

# Add EpoxyDuino Core source files
aux_source_directory( cores/epoxy EPOXY_CORE )

# Compile EpoxyDuino Core as objects
add_library( ${PROJECT_NAME}-obj OBJECT ${EPOXY_CORE} )

# Add include directory to use #include <file.h> or "file.h"
target_include_directories( ${PROJECT_NAME}-obj PUBLIC cores/epoxy )

# Pre-processor macros to define EPOXY_DUINO, and ARDUINO=100 and UNIX_HOST_DUINO for backwards compatibility
set( COMPILE_DEFINES EPOXY_DUINO ARDUINO=100 UNIX_HOST_DUINO )

# Check if USE_ESP8266_CORE is defined; convert it to uppercase if set
if( DEFINED USE_ESP8266_CORE )
   string( TOUPPER "${USE_ESP8266_CORE}" USE_ESP8266_CORE_UPPER )
else() # Otherwise, default to "OFF"
   set( USE_ESP8266_CORE_UPPER "OFF" )
endif()

# Ensure USE_ESP8266_CORE_UPPER has a valid boolean value ("ON" or "OFF")
if( NOT USE_ESP8266_CORE_UPPER STREQUAL "ON" AND NOT USE_ESP8266_CORE_UPPER STREQUAL "OFF" )
   # If the value is invalid, terminate with an error message
   message( FATAL_ERROR "-D USE_ESP8266_CORE value must be equal to ON or OFF. Received: ${USE_ESP8266_CORE}" )
endif()

# Append macro definition based on the selected EpoxyDuino Core
if(USE_ESP8266_CORE_UPPER STREQUAL "ON")
   list( APPEND COMPILE_DEFINES EPOXY_CORE_ESP8266 )
else()  # Otherwise, default to EPOXY_CORE_AVR
   list( APPEND COMPILE_DEFINES EPOXY_CORE_AVR )
endif()

# Clear cached value to prevent persistence across runs
unset( USE_ESP8266_CORE CACHE )

# Display defined pre-processor macros
set( FORMATTED_COMPILE_DEFINES "" )
foreach( def IN LISTS COMPILE_DEFINES )
   set( FORMATTED_COMPILE_DEFINES "${FORMATTED_COMPILE_DEFINES}-D ${def} " )
endforeach()
string( STRIP "${FORMATTED_COMPILE_DEFINES}" FORMATTED_COMPILE_DEFINES )
message( STATUS "Defined pre-processor macros: ${FORMATTED_COMPILE_DEFINES}" )

# Set preprocessor definitions for compiling object targets
target_compile_definitions( ${PROJECT_NAME}-obj PRIVATE ${COMPILE_DEFINES} )

# Enable position-independent code (PIC) for object target (required for shared libraries)
set_target_properties( ${PROJECT_NAME}-obj PROPERTIES POSITION_INDEPENDENT_CODE ON )

# Add a static library target built from object files
add_library( ${PROJECT_NAME}-static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}-obj> )

# Prevent the automatic "lib" prefix on library names
set_target_properties( ${PROJECT_NAME}-static PROPERTIES PREFIX "" )

# Set the library name to match the project name, with an OS-dependent extension like .a or .lib
set_target_properties( ${PROJECT_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_NAME} )

# Build examples
file( GLOB CHILD_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/examples ${CMAKE_SOURCE_DIR}/examples/* )

foreach(DIR ${CHILD_DIRS})
   file( GLOB INO_FILES "${CMAKE_SOURCE_DIR}/examples/${DIR}/*.ino" )
   file( GLOB CPP_FILES "${CMAKE_SOURCE_DIR}/examples/${DIR}/*.cpp" )

   foreach(INO ${INO_FILES})
      get_filename_component( EXEC_NAME ${INO} NAME_WE )
      add_executable( ${EXEC_NAME} ${INO} ${CPP_FILES} )
      set_source_files_properties( ${INO} PROPERTIES LANGUAGE CXX COMPILE_FLAGS "${COMPILE_FLAGS}" )
      target_include_directories( ${EXEC_NAME} PUBLIC cores/epoxy )
      target_compile_definitions( ${EXEC_NAME} PRIVATE ${COMPILE_DEFINES} )
      target_link_libraries( ${EXEC_NAME} PRIVATE ${PROJECT_NAME}-static )
   endforeach()
endforeach()
