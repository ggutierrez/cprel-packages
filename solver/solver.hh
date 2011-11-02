#ifndef __CPREL_PACKAGES_SOLVER_SOLVER_HH__
#define __CPREL_PACKAGES_SOLVER_SOLVER_HH__

#include <gecode/int.hh>
#include <gecode/search.hh>


namespace CPRelPkg {

  class Solver : public Gecode::Space {
  public:
    /// Constructor
    Solver(void);
    /// Constructor for clonning \a s
    Solver(bool share, Solver& s);
    /// Copy during clonning
    Gecode::Space* copy(bool share);
    /// Print solution to stream \a os
    void print(std::ostream& os) const;
  };
}

#endif
