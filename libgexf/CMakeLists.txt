##########################################################################
# libxml
##########################################################################
find_package(libxml2)
if(LIBXML2_FOUND) 
  message(STATUS "Using libxml2 library from: ${LIBXML2_LIBRARIES}")
  message(STATUS "Using libxml2 includes from: ${LIBXML2_INCLUDE_DIR}")
  include_directories(${LIBXML2_INCLUDE_DIR})
else()
  message(FATAL_ERROR "libxml2 library is required")
endif()

include_directories(${CMAKE_SOURCE_DIR}/libgexf)

set(GEXF_HDRS
libgexf/abstractiter.h    libgexf/conv.h            libgexf/edgeiter.h        libgexf/gexf.h            libgexf/legacywriter.h    libgexf/nodeiter.h        libgexf/undirectedgraph.h libgexf/abstractparser.h  libgexf/data.h            libgexf/exceptions.h      libgexf/gexfparser.h      libgexf/libgexf.h   libgexf/rngvalidator.h libgexf/attributeiter.h   libgexf/directedgraph.h   libgexf/filereader.h libgexf/graph.h           libgexf/memoryvalidator.h libgexf/schemavalidator.h libgexf/attvalueiter.h libgexf/dynamicgraph.h    libgexf/filewriter.h      libgexf/legacyparser.h    libgexf/metadata.h  libgexf/typedefs.h
)

set(GEXF_SRCS
libgexf/abstractparser.cpp  libgexf/conv.cpp            libgexf/dynamicgraph.cpp    libgexf/filewriter.cpp      libgexf/graph.cpp           libgexf/memoryvalidator.cpp libgexf/rngvalidator.cpp
libgexf/attributeiter.cpp   libgexf/data.cpp            libgexf/edgeiter.cpp        libgexf/gexf.cpp            libgexf/legacyparser.cpp    libgexf/metadata.cpp        libgexf/schemavalidator.cpp
libgexf/attvalueiter.cpp    libgexf/directedgraph.cpp   libgexf/filereader.cpp      libgexf/gexfparser.cpp      libgexf/legacywriter.cpp    libgexf/nodeiter.cpp        libgexf/undirectedgraph.cpp
)

add_library(gexf ${GEXF_SRCS} ${GEXF_HDRS})
target_link_libraries(gexf ${LIBXML2_LIBRARIES})

add_executable(test test.cpp)
target_link_libraries(test gexf)
