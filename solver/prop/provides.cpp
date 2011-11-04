#include <solver/prop/provides.hh>
#undef fail

namespace CPRelPkg {
  void provides(Gecode::Space& home, MPG::CPRelVar installation, MPG::CPRelVar provides) {
    if (home.failed()) return;
    
    std::cout << "Posting provides" << std::endl; 
    MPG::CPRel::CPRelView inst(installation);
    MPG::CPRel::CPRelView pvds(provides);
    GECODE_ES_FAIL((Provides::post(home,inst,provides)));
  }
}
