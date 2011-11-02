#include <solver/solver.hh>


namespace CPRelPkg {
  Solver::Solver(const ProblemDesc& problem) {
    dependencies_ = MPG::CPRelVar(*this,
				  std::get<1>(problem),
				  MPG::GRelation::create_full(2)
				  );
    conflicts_ = MPG::CPRelVar(*this,
			       std::get<2>(problem),
			       MPG::GRelation::create_full(2)
			       );
    provides_ = MPG::CPRelVar(*this,
			      MPG::GRelation(2),
			      std::get<3>(problem)
			      );
    inst_ = MPG::CPRelVar(*this,
			  MPG::GRelation(1),
			  std::get<0>(problem).shiftRight(2)
			  );

    // branch
    MPG::branch(*this,inst_);
  }

  Solver::Solver(bool share, Solver& s)
    : Gecode::Space(share,s) {
    dependencies_.update(*this,share,s.dependencies_);
    conflicts_.update(*this,share,s.conflicts_);
    provides_.update(*this,share,s.provides_);
    inst_.update(*this,share,s.inst_);
  }

  Gecode::Space* Solver::copy(bool share) {
    return new Solver(share,*this);
  }

  void Solver::print(std::ostream& os) const {
    os << "Cardinalities " << std::endl
       << "\tInst_: " << inst_.glb().cardinality() << " / " << inst_.unk().cardinality() << std::endl
       << "\tProvides_: " << provides_.glb().cardinality() << std::endl
       << std::endl; 
  }
}
