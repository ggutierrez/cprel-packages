#ifndef __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_PROP_CONFLICTS_HH__
#define __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_PROP_CONFLICTS_HH__

#include <cprel/cprel.hh>

using MPG::CPRelVar;
using MPG::CPRel::CPRelView;
using MPG::GRelation;
using MPG::GRelation;
using Gecode::Home;
using Gecode::Space;

namespace CPRelPkg {

  class Conflicts : public Gecode::Propagator {
  protected:
    /// Installation
    CPRelView inst_;
    /// Dependencies
    CPRelView conflicts_;
 public:
    /// Constructor for the propagator \f$ equal(left,right) \f$
    Conflicts(Home home,  CPRelView inst,  CPRelView conflicts)
      : Gecode::Propagator(home), inst_(inst), conflicts_(conflicts) {
      inst_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
      conflicts_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
    }
    /// Propagator posting
    static Gecode::ExecStatus post(Home home, CPRelView inst,  CPRelView conflicts) {
      (void) new (home) Conflicts(home,inst,conflicts);
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
    virtual Gecode::ExecStatus propagate(Space& home, const Gecode::ModEventDelta& med)  {
      using namespace Gecode;

      {
        cout << "Running conflicts propagator" << endl;
      }
      { // 1 Whatever is installed needs to have all its conflicts
        // removed
        GRelation installed = inst_.glb();
        GRelation rightConflicts = installed.timesURight(1).intersect(conflicts_.glb());
        GRelation rightPackages = rightConflicts.project(1);
        GECODE_ME_CHECK(inst_.exclude(home,rightPackages));

        GRelation leftConflicts = installed.timesULeft(1).intersect(conflicts_.glb());
        GRelation leftPackages = leftConflicts.shiftRight(1);
        GECODE_ME_CHECK(inst_.exclude(home,leftPackages));
        
      }
      if (inst_.assigned() && conflicts_.assigned())
	return home.ES_SUBSUMED(*this);
      return Gecode::ES_FIX;
    }
  };
  void conflicts(Gecode::Space& home, CPRelVar installation, CPRelVar confs);
}

#endif
