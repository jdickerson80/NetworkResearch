cmake_minimum_required( VERSION 2.8 )

# Name the project
project( BGAdaptor )

# Find all of the libraries needed to build project
find_package( Threads )

# Set the active build type
#set( CMAKE_BUILD_TYPE Release )
set( CMAKE_BUILD_TYPE Debug )

# Add compiler flags
set( CMAKE_CXX_FLAGS "-Wall -std=c++11" )

# Bring in the project headers
include_directories( ${CMAKE_SOURCE_DIR}/* )
include_directories( ../Common/ )

# Add all of the files to the build
file( GLOB_RECURSE Headers "*.h" )
file( GLOB_RECURSE Sources "*.c" "*.cpp" )
file( GLOB_RECURSE CommonHeaders ../Common/*.h )
file( GLOB_RECURSE CommonSource ../Common/*.cpp )

# Add the executable output
add_executable( ${PROJECT_NAME} ${Headers} ${Sources} ${CommonSource} ${CommonHeaders} )

# Setup all of the link libraries
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} )
#target_link_libraries( ${PROJECT_NAME} ${BLUEZ_LIBRARIES} )
