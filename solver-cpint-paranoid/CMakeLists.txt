add_executable(cpint-paranoid
  paranoid.cpp
  brancher.cpp
)

target_link_libraries(cpint-paranoid 
  unsaparser
  ${GECODE_LIBRARIES}
)
