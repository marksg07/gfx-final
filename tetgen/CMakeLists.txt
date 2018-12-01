# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 2.6)

set(TET_MAJOR_VERSION 1)
set(TET_MINOR_VERSION 5)
set(TET_PATCH_VERSION 0)

INCLUDE(GNUInstallDirs)


#Add a shared library to the project using the specified source files.
add_library(tetlib SHARED tetgen.cxx predicates.cxx)
#Set properties on a target. 
#We use this here to set -DTETLIBRARY for when compiling the
#library
set_target_properties(tetlib PROPERTIES "COMPILE_DEFINITIONS" TETLIBRARY)
set_target_properties(tetlib PROPERTIES OUTPUT_NAME tet)
set_target_properties(tetlib PROPERTIES
    VERSION ${TET_MAJOR_VERSION}.${TET_MINOR_VERSION}.${TET_PATCH_VERSION}
    SOVERSION ${TET_MAJOR_VERSION}.${TET_MINOR_VERSION})

# Add an executable to the project using the specified source files.
add_definitions(-DTETLIBRARY)
add_executable(tetgen tetgen_main.cxx)
target_link_libraries(tetgen tetlib)

# Install
install(TARGETS tetgen DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS tetlib DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES tetgen.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
