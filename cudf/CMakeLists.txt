##########################################################################
# Bison / Flex
##########################################################################
find_package(Bison REQUIRED)
find_package(Flex REQUIRED)
BISON_TARGET(cudfparser cudf.y 
  ${CMAKE_CURRENT_BINARY_DIR}/cudf.tab.cpp COMPILE_FLAGS "--name-prefix=cudf")

FLEX_TARGET(cudflexer cudf.l
  ${CMAKE_CURRENT_BINARY_DIR}/cudflexer.cpp COMPILE_FLAGS " --prefix=cudf")
 ADD_FLEX_BISON_DEPENDENCY(cudflexer cudfparser)

add_library(unsaparser 
  model.cpp
  virtual_model.cpp
  cudf_tools.cpp
  cudf_hash_table.cpp
  ${BISON_cudfparser_OUTPUTS}
  ${FLEX_cudflexer_OUTPUTS})
