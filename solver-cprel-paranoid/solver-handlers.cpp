#include <solver-cprel-paranoid/solver.hh>

namespace CUDFTools {

  void ParanoidSolver::dependOnVirtual(int p, int q) {
    deps0_.add(Tuple({p,q}));
  }

  int ParanoidSolver::createVirtual(const vector<int>& disj) {
    // here we just return a new virtual package identifier
    int nextVirtual = concretePackages_ + virtualPackages_;
    virtualPackages_++;
   
    // here we post the constraint that the new vrtual package is
    // provided by any of the packages in the disjuncion.

    // here we put the constraint that if any package in the
    // disjunction is installed then the virtual package gets
    // installed.

    // the new virtual is provided by any of the packages of the
    // disjunction
    for (int p : disj)
      provs0_.add(Tuple({p,nextVirtual}));

    // it is not possible to have one of the providers installed
    // without having the virtual installed
    for (int p: disj)
      deps0_.add(Tuple({p,nextVirtual}));

    // add the new virtual package to the virtuals_ relation
    virtuals_.add(Tuple({nextVirtual}));

    return nextVirtual;
  }

  void ParanoidSolver::depend(int p, const vector<int>& disj) {
    if (disj.size() != 1)
      cout << "Ouch, fix me!" << endl;
    deps0_.add(Tuple({p,disj.at(0)}));
  }

  void ParanoidSolver::conflict(int p, int q) {
    confs0_.add(Tuple({p,q}));
  }

  void ParanoidSolver::install(int p) {
    Tuple t({p});
    install0_.add(t);
    include(*this,install_,install0_);
  }
}
