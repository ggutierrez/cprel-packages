#ifndef __CPREL_PACKAGES_SOLVER_PROP_CONFLICTS_HH__
#define __CPREL_PACKAGES_SOLVER_PROP_CONFLICTS_HH__

#include <cprel/cprel.hh>

namespace CPRelPkg {

  /**
   * \brief Propagates: \f$ \f$
   * \ingroup PkgProp
   */
  class Conflicts : public Gecode::Propagator {
  protected:
    /// Installation
    MPG::CPRel::CPRelView inst_;
    /// Conflicts
    MPG::CPRel::CPRelView conflicts_;
  public:
    /// Constructor for the propagator \f$ equal(left,right) \f$
    Conflicts(Gecode::Home home,  MPG::CPRel::CPRelView inst,  MPG::CPRel::CPRelView confs)
      : Gecode::Propagator(home), inst_(inst), conflicts_(confs) {
      inst_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
      conflicts_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
    }
    /// Propagator posting
    static Gecode::ExecStatus post(Gecode::Home home,
				    MPG::CPRel::CPRelView inst,  MPG::CPRel::CPRelView confs) {
      (void) new (home) Conflicts(home,inst,confs);
      return Gecode::ES_OK;
    }
    /// Propagator disposal
    virtual size_t dispose(Gecode::Space& home) {
      inst_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      conflicts_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    /// Copy constructor
    Conflicts(Gecode::Space& home, bool share, Conflicts& p)
      : Gecode::Propagator(home,share,p) {
      inst_.update(home,share,p.inst_);
      conflicts_.update(home,share,p.conflicts_);
    }
    /// Copy
    virtual Gecode::Propagator* copy(Gecode::Space& home, bool share) {
      return new (home) Conflicts(home,share,*this);
    }
    /// Cost
    virtual Gecode::PropCost cost(const Gecode::Space&,
				  const Gecode::ModEventDelta&) const {
      return Gecode::PropCost::binary(Gecode::PropCost::LO);
    }
    /// Main propagation algorithm
    virtual Gecode::ExecStatus propagate(Gecode::Space& home,
					 const Gecode::ModEventDelta& med)  {
      using namespace Gecode;
      //std::cout << "Propagating conflicts" << std::endl; 
      
      {
	auto m = MPG::CPRel::CPRelView::me(med);
	if (m != MPG::CPRel::ME_CPREL_MAX) {
	  // Create a relation with the installed packages and all the
	  // packages they conflict with.
	  auto installed = inst_.glb();
	  auto conflicting = installed.timesURight(1).intersect(conflicts_.glb());
	  auto conflictingPackages = conflicting.project(1);
	  GECODE_ME_CHECK(inst_.exclude(home,conflictingPackages));
	}
      }
            
      if (inst_.assigned() && conflicts_.assigned())
	return home.ES_SUBSUMED(*this);
      return Gecode::ES_FIX;
    }
  };
  void conflicts(Gecode::Space& home, MPG::CPRelVar installation, MPG::CPRelVar conflicts);
}

#endif
