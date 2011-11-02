#include <solver/solver.hh>

namespace CPRelPkg {
  Solver::Solver(void)
  {
    
  }

  Solver::Solver(bool share, Solver& s)
    : Gecode::Space(share,s) {
    
  }

  Gecode::Space* Solver::copy(bool share) {
    return new Solver(share,*this);
  }

  void Solver::print(std::ostream& os) const {
    os << "Something" << std::endl; 
  }
}
