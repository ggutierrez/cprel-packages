#include <solver/prop/conflicts.hh>

namespace CPRelPkg {
  void conflicts(Gecode::Space& home, MPG::CPRelVar installation, MPG::CPRelVar conflicts) {
    if (home.failed()) return;
    
    MPG::CPRel::CPRelView inst(installation);
    MPG::CPRel::CPRelView confs(conflicts);
    GECODE_ES_FAIL((Conflicts::post(home,inst,confs)));
  }
}
