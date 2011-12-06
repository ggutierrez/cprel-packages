#include <solver-cprel-paranoid/solver.hh>

using Gecode::Space;
using std::vector;


namespace CUDFTools {
  void ParanoidSolver::constrain(const Space& sol_) {
    
  }
  
  void ParanoidSolver::setOptimize(const vector<int>& coeff) {
    (void) coeff;
  }
  
  int ParanoidSolver::optimization(void) const {
    return 0;
  }
}
