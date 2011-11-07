#ifndef __CPREL_PACKAGES_SOLVER_SOLVER_HH__
#define __CPREL_PACKAGES_SOLVER_SOLVER_HH__

#include <gecode/set.hh>
#include <gecode/search.hh>
#include <cprel/cprel.hh>
#include <solversupport/reader.hh> // for ProblemDesc

namespace CPRelPkg {
  class Solver : public Gecode::Space {
  private:
    /// Relation variables
    MPG::CPRelVar dependencies_;
    MPG::CPRelVar conflicts_;
    MPG::CPRelVar provides_;
    MPG::CPRelVar inst_;
    /// A relation storing the concrete packages
    MPG::GRelation concretes_;
  public:
    /// Constructor
    Solver(const ProblemDesc& problem);
    /// Constructor for clonning \a s
    Solver(bool share, Solver& s);
    /// Copy during clonning
    Gecode::Space* copy(bool share);
    /// Print solution to stream \a os
    void print(std::ostream& os) const;
    /// Return a relation with the concrete packages
    MPG::GRelation getConcretes(void) const;
	
  };

  void stableProvides(Gecode::Home home, MPG::CPRelVar inst, MPG::CPRelVar provides);
}

#endif
