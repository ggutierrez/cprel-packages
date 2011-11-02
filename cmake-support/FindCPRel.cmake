message(STATUS "Finding CPRel")

## This file has to be processed always after find_package(Gecode)
if(NOT GECODE_LIBRARIES)
  message(FATAL_ERROR "Gecode is needed by the CPRel domain")
endif()

##########################################################################
# Detection of the domain representation library
##########################################################################
find_library(BDDDOMAIN bdddomain)
find_path(BDDDOMAIN_HDRS bdddomain/rel-impl.hh)

## Additional folders created by CUDD need to be checked.
set(CUDD_DEPS cudd mtr epd st util)
set(CUDD_LIBRARIES)
foreach(COMPONENT ${CUDD_DEPS})
  find_path(CUDD_${COMPONENT} ${COMPONENT}/${COMPONENT}.h)
  if(CUDD_${COMPONENT})
    include_directories(${CUDD_${COMPONENT}}/${COMPONENT})
  else()
    message(FATAL_ERROR "  Component ${CUDD_${COMPONENT}} was not found")
  endif()
  find_library(CUDD_${COMPONENT}_LIB ${COMPONENT})
  if(CUDD_${COMPONENT}_LIB)
    message(STATUS "  ${COMPONENT}: ${CUDD_${COMPONENT}_LIB}")
    list(APPEND CUDD_LIBRARIES ${CUDD_${COMPONENT}_LIB})
  else()
    message(FATAL_ERROR "  Component ${CUDD_${COMPONENT}_LIB} was not found")
  endif() 
endforeach()

# find the object oriented API
find_path(CUDD_OBJ NAMES obj/cuddObj.hh)
find_library(CUDD_OBJ_LIB cuddobj)

if(CUDD_OBJ AND CUDD_OBJ_LIB)
  include_directories(${CUDD_OBJ})
  list(APPEND CUDD_LIBRARIES ${CUDD_OBJ_LIB})
else()
  message(FATAL_ERROR "Object API of CUDD was not found")
endif()

## Now, find CPRel 
set(NEEDED_CPREL_HDRS bdddomain/manager.hh cprel/cprel.hh rel/grelation.hh)
foreach(COMPONENT ${NEEDED_CPREL_HDRS})
  find_path(CPREL_${COMPONENT} ${COMPONENT})
  if(CPREL_${COMPONENT})
    include_directories(${CPREL_${COMPONENT}})
  else()
    message(STATUS "  Component ${CPREL_${COMPONENT}} was not found")
  endif()
endforeach()

set(NEEDED_CPREL_LIBS gecodecprel bdddomain)
foreach(COMPONENT ${NEEDED_CPREL_LIBS})
  find_library(CPREL_${COMPONENT}_LIB ${COMPONENT})
  if(CPREL_${COMPONENT}_LIB)
    list(APPEND CUDD_LIBRARIES ${CPREL_${COMPONENT}_LIB})
    message(STATUS "  ${COMPONENT} : ${CPREL_${COMPONENT}_LIB}")
  else()
    message(FATAL_ERROR "  Component ${CPREL_${COMPONENT}_LIB} was not found")
  endif()
endforeach()

#message(STATUS "Libraries: ${CUDD_LIBRARIES}")


