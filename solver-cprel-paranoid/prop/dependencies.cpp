#include <solver-cprel-paranoid/prop/dependencies.hh>

void dependencies(Gecode::Space& home, CPRelVar installation, CPRelVar deps, GRelation virtuals) {
  if (home.failed()) return;
  
  MPG::CPRel::CPRelView inst(installation);
  MPG::CPRel::CPRelView d(deps);
  GECODE_ES_FAIL((CPRelPkg::Dependencies::post(home,inst,d,virtuals)));
}

