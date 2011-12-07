##########################################################################
# Find glpk
##########################################################################
find_library(GLPK_LIB glpk)
find_path(GLPK_HDR glpk.h)
if(GLPK_LIB AND GLPK_HDR)
  message(STATUS "Using glpk from: ${GLPK_LIB}")
else()
  message(FATAL_ERROR "glpk is needed to build this project")
endif()
include_directories(${GLPK_HDR})

## Paranoid solver
add_executable(lp-paranoid paranoid.cpp)
target_link_libraries(lp-paranoid
  unsaparser
  ${GLPK_LIB}
)
