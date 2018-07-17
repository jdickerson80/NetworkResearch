cmake_minimum_required( VERSION 2.8 )

# Name the project
project( WCEnabler )

# Find all of the libraries needed to build project
find_package( Threads )

# Set the active build type
#set( CMAKE_BUILD_TYPE Release )
set( CMAKE_BUILD_TYPE Debug )

# Add compiler flags
set( CMAKE_CXX_FLAGS "-Wall" )

# Bring in the project headers
include_directories( ${CMAKE_SOURCE_DIR}/* )

# Add all of the files to the build
file( GLOB_RECURSE Headers "*.h" )
file( GLOB_RECURSE Sources "*.c" "*.cpp" )

# Add the executable output
add_executable( ${PROJECT_NAME} ${Headers} ${Sources} )

# Setup all of the link libraries
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} )
#target_link_libraries( ${PROJECT_NAME} ${BLUEZ_LIBRARIES} )
