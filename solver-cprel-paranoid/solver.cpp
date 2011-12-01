#include <solver-cprel-paranoid/solver.hh>

using Gecode::Home;
void stableProvides(Home home, CPRelVar inst, CPRelVar provides);
void provides(Space& home, CPRelVar installation, CPRelVar provides, GRelation virtuals);
void dependencies(Space& home, CPRelVar installation, CPRelVar deps);
void conflicts(Space& home, CPRelVar installation, CPRelVar confs);

namespace CUDFTools {
  ParanoidSolver::ParanoidSolver(int concretePackages)
    : solution_(false)
    , deps_(*this,GRelation(2),GRelation::create_full(2))
    , provides_(*this,GRelation(2),GRelation::create_full(2))
    , conflicts_(*this,GRelation(2),GRelation::create_full(2))
    , install_(*this,GRelation(1),GRelation::create_full(1))
    , concretePackages_(concretePackages)
    , virtualPackages_(0)
    , deps0_(2), confs0_(2), provs0_(2), install0_(1), virtuals_(1)
  {}

  ParanoidSolver::ParanoidSolver(bool share, ParanoidSolver& other)
    : Space(share,other)
    , solution_(other.solution_)
    , concretePackages_(other.concretePackages_)
    , virtualPackages_(other.virtualPackages_)
    , deps0_(other.deps0_), confs0_(other.confs0_)
    , provs0_(other.provs0_), install0_(other.install0_)
    , virtuals_(other.virtuals_)
  {
    deps_.update(*this,share,other.deps_);
    provides_.update(*this,share,other.provides_);
    conflicts_.update(*this,share,other.conflicts_);
    install_.update(*this,share,other.install_);
  }

  Space* ParanoidSolver::copy(bool share) {
    return new ParanoidSolver(share,*this);
  }
 
  void ParanoidSolver::constrain(const Space& sol_) {
    
  }

  void ParanoidSolver::initVariables(void) {
    include(*this,deps_,deps0_);
    include(*this,conflicts_,confs0_);
    exclude(*this,provides_,provs0_.complement());
    include(*this,install_,install0_);

    // To compute the upper bound of the installation we consider the
    // concrete and virtual packages.
    GRelation all(1);
    for (int i = 0; i < concretePackages_; i++)
      all.add(Tuple({i}));
    for (int i = 0; i < virtualPackages_; i++)
      all.add(Tuple({concretePackages_+i}));
    exclude(*this,install_,all.complement());
  }

  void ParanoidSolver::postConstraints(void) {
    provides(*this,install_,provides_,virtuals_);
    dependencies(*this,install_,deps_);
    conflicts(*this,install_,conflicts_);
  }

  void ParanoidSolver::setBrancher(void) {
    stableProvides(*this,install_,provides_); 
  }
  
  void ParanoidSolver::print(std::ostream& os) const {
    (void)os;
  }
  void ParanoidSolver::setOptimize(const vector<int>& coeff) {
    (void) coeff;
  }

  int ParanoidSolver::optimization(void) const {
    return 0;
  }


  bool ParanoidSolver::packageInstalled(int p) const {
    GRelation r(1);
    r.add(Tuple({p}));
    return r.subsetEq(install_.glb());
  }

  void ParanoidSolver::knownProviders(int p) const {
    GRelation r(1);
    r.add(Tuple({p}));
    GRelation providers = r.timesULeft(1).intersect(provides_.glb());
    cout << "the providers " << providers << endl;
  }

  void ParanoidSolver::virtualsInstalled(void) const {
  
  }

  void ParanoidSolver::problemInfo(void) const {
    cout << "Prolem information:" << endl
         << "\tDependencies: " << deps_.glb().cardinality()
         << "\tProvides: " << provides_.lub().cardinality()
         << "\tConflicts: " << conflicts_.glb().cardinality()
         << "\tInstall: " << install_.glb().cardinality()
         << "..." << install_.lub().cardinality()
         << "\tVirtual packages: " << virtualPackages_
         << "\tConcrete packages: " << concretePackages_
         << endl;
  }

  vector<int> ParanoidSolver::installedPackages(void) const {
    vector<int> current;
    current.reserve(install_.glb().cardinality());
    GRelation i(install_.glb());
    while (!i.empty()) {
      Tuple t = i.pickOneTuple();
      GRelation r(i.arity());
      r.add(t);
      i.differenceAssign(r);
      current.push_back(t.value().at(0));
    }
    return current;
  }

}
