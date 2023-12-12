cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(JuceVLC VERSION 0.9.2)

# we use inplace sources with a submodule
add_subdirectory(JUCE)

# we use lib extracted from dlls under windows
# and headers are retrieved from a submodule
add_library(libvlc SHARED IMPORTED)
set_target_properties(libvlc PROPERTIES
  IMPORTED_CONFIGURATIONS "Debug;Release"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/contrib/libvlc/lib64/libvlc.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/vlc/include;${CMAKE_SOURCE_DIR}/vlc/lib"
)

# we use lib extracted from dlls under windows
# and headers are retrieved from a submodule
add_library(libvlccore SHARED IMPORTED)
set_target_properties(libvlccore PROPERTIES
  IMPORTED_CONFIGURATIONS "Debug;Release"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/contrib/libvlc/lib64/libvlccore.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/vlc/include;${CMAKE_SOURCE_DIR}/vlc/lib"
)

add_subdirectory(src)

