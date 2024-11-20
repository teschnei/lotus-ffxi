cmake_minimum_required( VERSION 3.13 FATAL_ERROR )
cmake_policy( VERSION 3.13 )

project(lotus-ffxi)

###############################
## output
###############################
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

add_subdirectory( "lotus-engine" )
add_subdirectory( "ffxi" )
