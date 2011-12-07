add_executable(cprel-paranoid 
  paranoid.cpp
  solver.cpp
  solver-handlers.cpp
  solver-opt.cpp
  solver-branch.cpp
  prop/dependencies.cpp
  prop/provides.cpp
  prop/conflicts.cpp

## Heuristics
  branch/stableProvides.cpp
  branch/minimalChanges.cpp
  branch/existingInstall.cpp
)
target_link_libraries(cprel-paranoid 
  unsaparser 
  ${GECODE_LIBRARIES}
  ${CPREL_LIBRARIES} 
)
