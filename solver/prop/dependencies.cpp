#include <solver/prop/dependencies.hh>

namespace CPRelPkg {
  void dependencies(Gecode::Space& home, MPG::CPRelVar installation, MPG::CPRelVar dependencies) {
    if (home.failed()) return;
    
    MPG::CPRel::CPRelView inst(installation);
    MPG::CPRel::CPRelView deps(dependencies);
    GECODE_ES_FAIL((Dependencies::post(home,inst,deps)));
  }
}
