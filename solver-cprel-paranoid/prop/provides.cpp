#include <solver-cprel-paranoid/prop/provides.hh>

void provides(Gecode::Space& home, CPRelVar installation, CPRelVar provides, GRelation virtuals) {
    if (home.failed()) return;
    
    MPG::CPRel::CPRelView inst(installation);
    MPG::CPRel::CPRelView pvds(provides);
    GECODE_ES_FAIL((CPRelPkg::Provides::post(home,inst,provides,virtuals)));
  }

