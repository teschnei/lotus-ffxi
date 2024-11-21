cmake_minimum_required( VERSION 3.28 FATAL_ERROR )

project(lotus-ffxi)

###############################
## output
###############################
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

add_subdirectory( "lotus-engine" )
add_subdirectory( "ffxi" )
