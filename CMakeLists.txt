cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-support")

set(PROJECT_NAME "CPRel-packages")
set(PROJECT_VERSION "0.9.0")
project(${PROJECT_NAME})

##########################################################################
# Command line options
##########################################################################

##########################################################################
# Compiler information
##########################################################################
if(CMAKE_COMPILER_IS_GNUCC)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION)
  if (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)
    message(STATUS "GCC version ${GCC_VERSION}")
  else()
    message(FATAL_ERROR "GCC version 4.6 or greater is required")
  endif()
else()
  message(WARNING "Compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()
##########################################################################
# System information
##########################################################################
message(STATUS "Building for architecture: ${CMAKE_SYSTEM}")
##########################################################################
# Additional compiler flags
##########################################################################
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++0x" C0X_SUPPORT)
if(C0X_SUPPORT)
  message(STATUS "CXX has c++0x support")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++0x")
else()
  message(FATAL_ERROR "c++0x capable compiler is needed to build this project")
endif()

check_cxx_compiler_flag(-Wall FLAG_WALL)
if(FLAG_WALL)
  add_definitions(-Wall)
endif()

check_cxx_compiler_flag(-Wextra FLAG_WEXTRA)
if(FLAG_WEXTRA)
  add_definitions(-Wextra)
endif()
##########################################################################
# Boost
##########################################################################
set(BOOST_COMPONENTS)
list(APPEND BOOST_COMPONENTS system)
list(APPEND BOOST_COMPONENTS chrono)

find_package(Boost 1.47.0 COMPONENTS ${BOOST_COMPONENTS})
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "The following boost components are needed: ${BOOST_COMPONENTS}")
endif()
##########################################################################
# Gecode
##########################################################################
find_package(Gecode REQUIRED COMPONENTS kernel support int set driver flatzinc gist minimodel search)
include_directories(GECODE_INCLUDE_DIR)
##########################################################################
# CPRel
##########################################################################
find_package(CPRel REQUIRED)
include_directories(CPREL_INCLUDE_DIR)

##########################################################################
# This project products
##########################################################################
include_directories(${CMAKE_SOURCE_DIR})

# The visualizer library
add_subdirectory(libgexf)
# The unsa parser
add_subdirectory(cudf)
# The paranoid solver (cprel)
add_subdirectory(solver-cprel-paranoid)
# The paranoid solver (cpint)
add_subdirectory(solver-cpint-paranoid)
# The paranoid solver (lp)
add_subdirectory(solver-lp-paranoid)
# Misc tools
add_subdirectory(tools)

## Temporal, should be removed adter

#add_library(cprelsolver 
#  solver/solver.cpp
#  solver/prop/dependencies.cpp
#  solver/prop/provides.cpp
#  solver/prop/conflicts.cpp
#  solver/branch/stableProvides.cpp
#)

#add_executable(solver tools/solver.cpp)
#target_link_libraries(solver cprelsolver 
#  ${CUDF_LIB} ${KCUDF_LIB} 
#  ${CPREL_LIBRARIES} 
#  ${Boost_LIBRARIES} 
#  ${GECODE_LIBRARIES})

#add_executable(cudfstats tools/cudfstats.cpp)
#target_link_libraries(cudfstats unsaparser)
