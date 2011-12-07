include_directories(${CMAKE_SOURCE_DIR}/libgexf)
include_directories(${LIBXML2_INCLUDE_DIR})

add_executable(pkgviz pkgviz.cpp)
target_link_libraries(
  pkgviz 
  unsaparser
  gexf
)
