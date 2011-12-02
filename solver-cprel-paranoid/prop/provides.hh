#ifndef __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_PROP_PROVIDES_HH__
#define __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_PROP_PROVIDES_HH__

#include <cprel/cprel.hh>

using MPG::CPRelVar;
using MPG::CPRel::CPRelView;
using MPG::GRelation;
using MPG::GRelation;
using Gecode::Home;
using Gecode::Space;

namespace CPRelPkg {

  class Provides : public Gecode::Propagator {
  protected:
    /// Installation
    CPRelView inst_;
    /// Provides
    CPRelView provides_;
    /// Virtual packages
    GRelation virtuals_;
  public:
    /// Constructor for the propagator \f$ equal(left,right) \f$
    Provides(Home home,  CPRelView inst,  CPRelView dep, GRelation virtuals)
      : Gecode::Propagator(home), inst_(inst), provides_(dep), virtuals_(virtuals) {
      inst_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
      provides_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
    }
    /// Propagator posting
    static Gecode::ExecStatus post(Home home, CPRelView inst,  CPRelView dep, GRelation virtuals) {
      (void) new (home) Provides(home,inst,dep,virtuals);
      return Gecode::ES_OK;
    }
    /// Propagator disposal
    virtual size_t dispose(Gecode::Space& home) {
      inst_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      provides_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      // what should i do with virtuals here???
    
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    /// Copy constructor
    Provides(Gecode::Space& home, bool share, Provides& p)
      : Gecode::Propagator(home,share,p), virtuals_(p.virtuals_) {
      inst_.update(home,share,p.inst_);
      provides_.update(home,share,p.provides_);
    }
    /// Copy
    virtual Gecode::Propagator* copy(Gecode::Space& home, bool share) {
      return new (home) Provides(home,share,*this);
    }
    /// Cost
    virtual Gecode::PropCost cost(const Gecode::Space&,
				  const Gecode::ModEventDelta&) const {
      // This constraint should always have a bigger cost than the
      // dependency propagator. The reason for this is that, as it is
      // implemented now the dependency propagator does not compute a
      // fix point every time it is run. Having a higher cost here
      // will guarantee that it is always executed when we have gotten
      // the most out of the dependencies.
      return Gecode::PropCost::ternary(Gecode::PropCost::HI);
    }
    /// Main propagation algorithm
    virtual Gecode::ExecStatus propagate(Space& home, const Gecode::ModEventDelta&)  {
      using namespace Gecode;

      {
        //cout << "Running provides propagator" << endl;
      }
     
      { // 1. All the packages provided by installed packages should
        // become known in the provides relation. If a package p is
        // installed then all the packages it provides should become
        // installed. This constraint will add virtual packages to the
        // installation.  Note that if there is a virtual package
        // installed the intersection will remove it as it is not
        // possible to have (p,q) in the provides relation being p a
        // virtual.
        GRelation provided = inst_.glb().timesURight(1).intersect(provides_.lub());
        GECODE_ME_CHECK(provides_.include(home,provided));

	// Every package in the installation needs at least one
	// provider. If this does not hold then this constraint must
	// fail.  
        GRelation virtualsPossible = provides_.lub().project(1);
        GRelation virtualsNeeded = inst_.glb().intersect(virtuals_);
        if (!virtualsNeeded.subsetEq(virtualsPossible)) {
          cout << "Failed, impossible to install needed virtuals" << endl;
          cout << "The following packages are needed but cannot be provided: "
               << virtualsNeeded.difference(virtualsPossible);
          return ES_FAILED;
        }
      
        // If there is only one package that can provide it then that
	// package must be installed to satisfy the constraint.
        GRelation possibleProviders = virtualsNeeded.timesULeft(1).intersect(provides_.lub());
        GRelation uniqueProviders = possibleProviders.unique(1);
        GRelation toIncludeProv = uniqueProviders.intersect(possibleProviders);
        GECODE_ME_CHECK(provides_.include(home,toIncludeProv));
        // The inclusion of the provided relation should cause the
        // installation to be affected so this propagator does not
        // compute a fix point.
        //GRelation toIncludeInst = toIncludeProv.shiftRight(1);
        //GECODE_ME_CHECK(inst_.include(home,toIncludeInst));
      }
      { // 4 Anything that cannot be installed should not be in the
        // provides relation.
        
        // a possible slow version
        GRelation notInstallable = inst_.oob();
        GRelation cannotProvide = notInstallable.timesURight(1);
        GECODE_ME_CHECK(provides_.exclude(home,cannotProvide));

        GRelation uninstPossibleProviders = provides_.lub().shiftRight(1).intersect(inst_.oob());
        GRelation invalidProvides = uninstPossibleProviders.timesURight(1);
        GECODE_ME_CHECK(provides_.exclude(home,invalidProvides));
      }      
      { // 2. every provide relation known must be reflected in the
        // installation. 
        
        // TODO: Removing what is only possible instead of removing
        // the whole relation is done because some packages are not in
        // the installation upper bound. I have to investigate why
        // this is happening.
        GRelation knownVirtuals = provides_.glb().project(1);
        GECODE_ME_CHECK(inst_.include(home,knownVirtuals.intersect(inst_.lub())));
        GRelation knownProviders = provides_.glb().shiftRight(1);
        GECODE_ME_CHECK(inst_.include(home,knownProviders.intersect(inst_.lub())));
      }
      { // 3. When something is no longer possible in provides then it
        // has to be removed from the installation.
        GRelation possibleToProvide = provides_.lub().project(1);
        GRelation possibleToInstall = inst_.lub().intersect(virtuals_);
        GRelation cannotInstall = possibleToInstall.difference(possibleToProvide);
        GECODE_ME_CHECK(inst_.exclude(home,cannotInstall));        
      }
      if (inst_.assigned() && provides_.assigned())
	return home.ES_SUBSUMED(*this);
      return Gecode::ES_NOFIX;
    }
  };
void provides(Gecode::Space& home, CPRelVar installation, CPRelVar provides, GRelation virtuals);
}

#endif
