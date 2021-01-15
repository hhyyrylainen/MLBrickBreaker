# native module compile cmake file as scons compilation database doesn't work
cmake_minimum_required(VERSION 3.10)

project(MLBrickBreaker)

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
    "Set the build type, usually Release or RelWithDebInfo" FORCE)
endif(CMAKE_BUILD_TYPE STREQUAL "")

# cmake setup
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_DEBUG_POSTFIX .debug)
set(BITNESS 64)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(BITNESS 64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(BITNESS 32)
endif()

if(WIN32)
  if(BITNESS EQUAL 64)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/win64/$<$<CONFIG:Debug>:>")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/win64/$<$<CONFIG:Debug>:>")
  else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/win32/$<$<CONFIG:Debug>:>")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/win64/$<$<CONFIG:Debug>:>")
  endif()
elseif(UNIX)
  if(APPLE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/osx")    
  else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/x11")
  endif()
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endif()

# O3 build
string(REGEX REPLACE "([\\/\\-]O)2" "\\13"
  CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "([\\/\\-]O)2" "\\13"
  CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REGEX REPLACE "([\\/\\-]O)2" "\\13"
  CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
string(REGEX REPLACE "([\\/\\-]O)2" "\\13"
  CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

if(UNIX)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

# Godot scons build copied info

set(GODOT_HEADERS_PATH "third_party/godot-cpp/godot_headers/")
set(CPP_BINDINGS_PATH "third_party/godot-cpp/")

include_directories(${GODOT_HEADERS_PATH} "${CPP_BINDINGS_PATH}/include"
  "${CPP_BINDINGS_PATH}/include/core" "${CPP_BINDINGS_PATH}/include/gen")
link_directories("${CPP_BINDINGS_PATH}/bin")


# the actual code
add_subdirectory(src)
