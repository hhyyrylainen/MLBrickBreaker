# All source files must be listed here
set(SOURCES
  logic/Game.h logic/Game.cpp
  logic/gdlibrary.cpp
  logic/Match.cpp logic/Match.h logic/Definitions.h logic/GameElements.h logic/Input.cpp logic/Input.h)

# library and compile option setup
add_library(mlbb SHARED ${SOURCES})

if(WIN32)
  target_link_libraries(mlbb PRIVATE optimized libgodot-cpp.windows.release.${BITNESS}.lib
    debug libgodot-cpp.windows.debug.${BITNESS}.lib)

  # Just to make sure things are consistent with the scons stuff
  target_compile_definitions(mlbb PRIVATE WINDOWS _CRT_SECURE_NO_WARNINGS)
  
  if(MSVC)
    # target_compile_options(mlbb PRIVATE)
  else()
    # target_compile_options(mlbb PRIVATE)
  endif()
  
else()

  target_compile_options(mlbb PRIVATE -Wall -Wextra -Werror
    -Wno-parentheses -Wno-unused-parameter)
  
  # TODO: mac support    
  target_link_libraries(mlbb PRIVATE optimized libgodot-cpp.linux.release.${BITNESS}.a
    debug libgodot-cpp.linux.debug.${BITNESS}.a)  
endif()

set_target_properties(mlbb PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF)

target_include_directories(mlbb PUBLIC ./)
