#include <solver-cprel-paranoid/prop/provides.hh>

  void provides(Gecode::Space& home, MPG::CPRelVar installation, MPG::CPRelVar provides) {
    if (home.failed()) return;
    
    MPG::CPRel::CPRelView inst(installation);
    MPG::CPRel::CPRelView pvds(provides);
    GECODE_ES_FAIL((CPRelPkg::Provides::post(home,inst,provides)));
  }

