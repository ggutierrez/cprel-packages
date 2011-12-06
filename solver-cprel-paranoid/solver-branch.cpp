#include <solver-cprel-paranoid/solver.hh>

using Gecode::Home;

void stableProvides(Home home, CPRelVar inst, CPRelVar provides);
void minimalChanges(Home home, CPRelVar inst, CPRelVar provides, GRelation installed);
void existingInstall(Home home, CPRelVar inst, CPRelVar provides, GRelation installed);


namespace CUDFTools {  
  void ParanoidSolver::installedPackages(const vector<int>& inst) {
    for (int p : inst)
      installed_.add(Tuple({p}));
    cout << "readed " << installed_.cardinality() 
         << " installed package for heuristic" << endl;
  }

  void ParanoidSolver::setBrancher(void) {
    //stableProvides(*this,install_,provides_); 
    //minimalChanges(*this,install_,provides_,installed_);
    existingInstall(*this,install_,provides_,installed_);
  }
}
