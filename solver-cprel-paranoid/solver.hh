#ifndef __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_SOLVER_HH
#define __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_SOLVER_HH

#include <vector>
#include <gecode/kernel.hh>
#include <gecode/kernel.hh>
#include <gecode/search.hh>
#include <rel/grelation.hh>
#include <cprel/cprel.hh>

using MPG::CPRelVar;
using MPG::GRelation;
using MPG::Tuple;
using std::vector;

namespace CUDFTools {
  class ParanoidSolver : public Space {
  private:
    /// Indicates if this represents a solution
    bool solution_;
    /// Dependencies
    CPRelVar deps_;
    /// Provides
    CPRelVar provides_;
    /// Conflicts
    CPRelVar conflicts_;
    /// Installation
    CPRelVar install_;
    /// Number of concrete packages
    int concretePackages_;
    /// Number of virtual packages
    int virtualPackages_;
    /// Ground relations used while the input is read
    GRelation deps0_;
    GRelation confs0_;
    GRelation provs0_;
    GRelation install0_;
  public:
  /// Constructor
  ParanoidSolver(int concretePackages);
  /// Copy constructor
  ParanoidSolver(bool share, ParanoidSolver& other);
    /// Copy
    Space* copy(bool share);
    /// Constraint variable initialization
    void initVariables(void);
    /// Constraint posting
    void postConstraints(void);
    /// Optimization
    virtual void constrain(const Space& sol_);
    /// Return the value of the optimization
    int optimization(void) const;
    /// Tries to perform some simplification of the variables
    void print(std::ostream& os) const;
    /// Post a dependency between a package and a virtual package
    void dependOnVirtual(int p, int q);
  /// Creates a virtual package in the solver that can be provided by
  /// any package in \a disj
    int createVirtual(const vector<int>& disj);
  /// Post a constraint stating that p depends on any in disj
    void depend(int p, const vector<int>& disj);
  /// Post the constraint that p conflicts with q
    void conflict(int p, int q);
    void install(int p);
    void setOptimize(const vector<int>& coeff);
  /// Returns the status (true: installed, false otherwise) of a package in the solver
    bool packageInstalled(int p) const;
  /// Prints the virtuals that got installed at the end of the solving
    void virtualsInstalled(void) const;
    void setBrancher(void);
    void problemInfo(void) const;
  /// Returns the packages that are currently installed by the solver
    vector<int> installedPackages(void) const;
    void knownProviders(int p) const;  
};

}

#endif
