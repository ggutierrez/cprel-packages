#include <solver/solver.hh>
#include <solversupport/pkg-builtins.hh>
#include <solver/prop/dependencies.hh>
#include <solver/prop/provides.hh>
#include <solver/prop/conflicts.hh>

namespace CPRelPkg {

  Solver::Solver(const ProblemDesc& problem)
    : concretes_(std::get<4>(problem)){
    
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

    {
      //std::cout << "Before posting mi: " << inst_.glb().cardinality() << std::endl; 
      //std::cout << "Before posting mu: " << inst_.unk().cardinality() << std::endl; 

      // Packages with Must#Install must be in the installation
      auto mi = filterPackages(std::get<0>(problem), 1, 1);
      MPG::include(*this,inst_,mi.shiftRight(2));

      // Packages with Must#install must be out of the installation
      auto mu = filterPackages(std::get<0>(problem), 1, 0);
      MPG::exclude(*this,inst_,mu.shiftRight(2));

      //std::cout << "After posting mi: " << inst_.glb().cardinality() << std::endl; 
      //std::cout << "After posting mu: " << inst_.unk().cardinality() << std::endl; 
    }

    {
      // the dependencies of all packages must be ensured
      dependencies(*this,inst_,dependencies_);
      provides(*this,inst_,provides_);
      conflicts(*this,inst_,conflicts_);
    }

    // branch
    //MPG::branch(*this,inst_);
    stableProvides(*this,inst_,provides_);
  }

  Solver::Solver(bool share, Solver& s)
    : Gecode::Space(share,s), concretes_(s.concretes_) {
    dependencies_.update(*this,share,s.dependencies_);
    conflicts_.update(*this,share,s.conflicts_);
    provides_.update(*this,share,s.provides_);
    inst_.update(*this,share,s.inst_);
  }

  Gecode::Space* Solver::copy(bool share) {
    return new Solver(share,*this);
  }

  MPG::GRelation Solver::getConcretes(void) const {
    return concretes_;
  }

  void Solver::print(std::ostream& os) const {
    
    os << "Solution " << std::endl
       << "\tInst_: " << inst_.glb() << " / " << inst_.unk().cardinality() << std::endl // 
      //<< "\tInst_Unk: " << inst_.unk() << " / " << inst_.unk() << std::endl
       << "\tProvides_: " << provides_.glb() << std::endl
       << std::endl; 
    
    /*
    os << "Solutions " << std::endl
       << "\tInst_: " << inst_.glb().cardinality() << " / " 
       << inst_.unk().cardinality() << std::endl
       << "\tProvides_: " << provides_.glb().cardinality() << " / " 
       << provides_.unk().cardinality() << std::endl
       << std::endl; 
    */
  }
}
