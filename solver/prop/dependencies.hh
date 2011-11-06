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
      std::cout << "Dependencies(ctor)" << std::endl; 
      inst_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
      dependencies_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
      std::cout << "Dependencies(ctor)*" << std::endl; 
    }
    /// Propagator posting
    static Gecode::ExecStatus post(Gecode::Home home,
				    MPG::CPRel::CPRelView inst,  MPG::CPRel::CPRelView dep) {
      std::cout << "Dependencies(post)" << std::endl; 
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
      using namespace Gecode;
      //std::cout << "Propagating dependencies" << std::endl; 

      // Every package known to be installed must have all its
      // dependencies installed
      auto installed = inst_.glb();
      
      {
	// Create a relation with all the possible dependencies for
	// installed, then intersect it with the dependencies that are
	// known in the system. This will give us the dependencies
	// that we need to add for this current set of installed
	// packages.
	auto neededDeps = installed.timesURight(1).intersect(dependencies_.glb());
	
	// Add the needed dependencies to the installation. for this
	// we need to project on the right most column and include in
	// inst_
	auto toInclude = neededDeps.project(1);
	GECODE_ME_CHECK(inst_.include(home,toInclude));
      }

      { // The following code is correct but it has a lot impact on
	// the solver running time for now. Probably it will be better
	// to execute it only when needed (another event change on the
	// variable?)

	/*
	// All the packages that have as dependencies packages that
	// cannot be installed are not installable.
	auto notInstallable = inst_.oob();
	// get the unsatisfiable dependencies
	auto unsatDependencies = notInstallable.timesULeft(1).intersect(dependencies_.glb());
	// the packages to remove are present in the left column of
	// the computed relation.
	auto toExclude = unsatDependencies.shiftRight(1);
	GECODE_ME_CHECK(inst_.exclude(home,toExclude));
       */}
      
      
      if (inst_.assigned() && dependencies_.assigned())
	return home.ES_SUBSUMED(*this);
      return Gecode::ES_NOFIX;
    }
  };
  void dependencies(Gecode::Space& home, MPG::CPRelVar installation, MPG::CPRelVar dependencies);
}

#endif
