#include <solver-cprel-paranoid/prop/conflicts.hh>

void conflicts(Gecode::Space& home, CPRelVar installation, CPRelVar confs) {
  if (home.failed()) return;
  
  MPG::CPRel::CPRelView inst(installation);
  MPG::CPRel::CPRelView c(confs);
  GECODE_ES_FAIL((CPRelPkg::Conflicts::post(home,inst,c)));
}

