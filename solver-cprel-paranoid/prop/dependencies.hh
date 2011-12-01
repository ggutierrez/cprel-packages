#ifndef __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_PROP_DEPENDENCIES_HH__
#define __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_PROP_DEPENDENCIES_HH__

#include <cprel/cprel.hh>

using MPG::CPRelVar;
using MPG::CPRel::CPRelView;
using MPG::GRelation;
using MPG::GRelation;
using Gecode::Home;
using Gecode::Space;

namespace CPRelPkg {

  class Dependencies : public Gecode::Propagator {
  protected:
    /// Installation
    CPRelView inst_;
    /// Dependencies
    CPRelView dependencies_;
    /// Virtual packages
    GRelation virtuals_;
 public:
    /// Constructor for the propagator \f$ equal(left,right) \f$
    Dependencies(Home home,  CPRelView inst,  CPRelView dependencies, GRelation virtuals)
      : Gecode::Propagator(home), inst_(inst), dependencies_(dependencies), virtuals_(virtuals) {
      inst_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
      dependencies_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
    }
    /// Propagator posting
    static Gecode::ExecStatus post(Home home, CPRelView inst,  CPRelView dependencies, GRelation virtuals) {
      (void) new (home) Dependencies(home,inst,dependencies,virtuals);
      return Gecode::ES_OK;
    }
    /// Propagator disposal
    virtual size_t dispose(Gecode::Space& home) {
      inst_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      dependencies_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      // Todo: remeber to do something with virtuals_
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    /// Copy constructor
    Dependencies(Gecode::Space& home, bool share, Dependencies& p)
      : Gecode::Propagator(home,share,p), virtuals_(p.virtuals_) {
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
    virtual Gecode::ExecStatus propagate(Space& home, const Gecode::ModEventDelta& med)  {
      using namespace Gecode;

      {
        cout << "Running dependencies propagator" << endl;
      }
      { // 1 Whatever is installed needs to have all its dependencies
        // installed
        GRelation installed = inst_.glb();
        GRelation depsNeeded = installed.timesURight(1).intersect(dependencies_.glb());
        GRelation pkgsNeeded = depsNeeded.project(1);
        GECODE_ME_CHECK(inst_.include(home,pkgsNeeded));
      }
      if (inst_.assigned() && dependencies_.assigned())
	return home.ES_SUBSUMED(*this);
      return Gecode::ES_NOFIX;
    }
  };
  void dependencies(Gecode::Space& home, CPRelVar installation, CPRelVar deps, GRelation virtuals);
}

#endif
