#ifndef __CPREL_PACKAGES_SOLVER_PROP_DEPENDENCIES_HH__
#define __CPREL_PACKAGES_SOLVER_PROP_DEPENDENCIES_HH__

#include <cprel/cprel.hh>

namespace CPRelPkg {

  /**
   * \brief Propagates: \f$ \f$
   * \ingroup PkgProp
   */
  class Dependencies : public Gecode::Propagator {
  protected:
    /// Installation
    MPG::CPRel::CPRelView inst_;
    /// Dependencies
    MPG::CPRel::CPRelView dependencies_;
  public:
    /// Constructor for the propagator \f$ equal(left,right) \f$
    Dependencies(Gecode::Home home,  MPG::CPRel::CPRelView inst,  MPG::CPRel::CPRelView dep)
      : Gecode::Propagator(home), inst_(inst), dependencies_(dep) {
      inst_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
      dependencies_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
    }
    /// Propagator posting
    static Gecode::ExecStatus post(Gecode::Home home,
				    MPG::CPRel::CPRelView inst,  MPG::CPRel::CPRelView dep) {
      (void) new (home) Dependencies(home,inst,dep);
      return Gecode::ES_OK;
    }
    /// Propagator disposal
    virtual size_t dispose(Gecode::Space& home) {
      inst_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      dependencies_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    /// Copy constructor
    Dependencies(Gecode::Space& home, bool share, Dependencies& p)
      : Gecode::Propagator(home,share,p) {
      inst_.update(home,share,p.inst_);
      dependencies_.update(home,share,p.dependencies_);
    }
    /// Copy
    virtual Gecode::Propagator* copy(Gecode::Space& home, bool share) {
      return new (home) Dependencies(home,share,*this);
    }
    /// Cost
    virtual Gecode::PropCost cost(const Gecode::Space&,
				  const Gecode::ModEventDelta&) const {
      return Gecode::PropCost::binary(Gecode::PropCost::LO);
    }
    /// Main propagation algorithm
    virtual Gecode::ExecStatus propagate(Gecode::Space& home,
					 const Gecode::ModEventDelta&)  {
      std::cout << "Propagating dependencies" << std::endl; 
      if (inst_.assigned() && dependencies_.assigned())
	return home.ES_SUBSUMED(*this);
      return Gecode::ES_FIX;
    }
  };
}

#endif
