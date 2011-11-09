#ifndef __CPREL_PACKAGES_SOLVER_PROP_PROVIDES_HH__
#define __CPREL_PACKAGES_SOLVER_PROP_PROVIDES_HH__

#include <cprel/cprel.hh>
#include <solver/solver.hh>

namespace CPRelPkg {

  /**
   * \brief Propagates: \f$ \f$
   * \ingroup PkgProp
   */
  class Provides : public Gecode::Propagator {
  protected:
    /// Installation
    MPG::CPRel::CPRelView inst_;
    /// Provides
    MPG::CPRel::CPRelView provides_;
  public:
    /// Constructor for the propagator \f$ equal(left,right) \f$
    Provides(Gecode::Home home,  MPG::CPRel::CPRelView inst,  MPG::CPRel::CPRelView dep)
      : Gecode::Propagator(home), inst_(inst), provides_(dep) {
      inst_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
      provides_.subscribe(home,*this,MPG::CPRel::PC_CPREL_BND);
    }
    /// Propagator posting
    static Gecode::ExecStatus post(Gecode::Home home,
				    MPG::CPRel::CPRelView inst,  MPG::CPRel::CPRelView dep) {
      (void) new (home) Provides(home,inst,dep);
      return Gecode::ES_OK;
    }
    /// Propagator disposal
    virtual size_t dispose(Gecode::Space& home) {
      inst_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      provides_.cancel(home,*this,MPG::CPRel::PC_CPREL_BND);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    /// Copy constructor
    Provides(Gecode::Space& home, bool share, Provides& p)
      : Gecode::Propagator(home,share,p) {
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
      return Gecode::PropCost::ternary(Gecode::PropCost::LO);
    }
    /// Main propagation algorithm
    virtual Gecode::ExecStatus propagate(Gecode::Space& home,
					 const Gecode::ModEventDelta& med)  {
      using namespace Gecode;

      {
	// This part maintains the coherence between the installation
	// and the provides relation. For the moment this is tight to
	// the fact that when a provides tuple is known then the
	// concrete package of that tuple is installed. 

	// Compute what is known to be provided and include the
	// conrete packages in the installation.
	auto provided = provides_.glb();
	auto toInclude = provided.shiftRight(1);
	GECODE_ME_CHECK(inst_.include(home,toInclude));

	// Every concrete package that is installed is a provider of
	// itself and will appear in the provides relation.
	const Solver& sp = static_cast<const Solver&>(home);
	auto concretes = sp.getConcretes();
	auto installedConcretes = concretes.intersect(inst_.glb());
	auto providedByConcretes = installedConcretes.timesULeft(1).intersect(provides_.lub());
	GECODE_ME_CHECK(provides_.include(home,providedByConcretes));
      }
      
      {
	// Every package in the installation needs at least one
	// provider. If this does not hold then this constraint must
	// fail.
	auto possibleProvides = inst_.glb().timesULeft(1).intersect(provides_.lub());
	auto canBeProvided = possibleProvides.project(1);
	if (!inst_.glb().eq(canBeProvided)) {
	  std::cout << "There is no provider for " << inst_.glb().difference(canBeProvided) << std::endl; 
	  return ES_FAILED;
	}
      }

      {
	// The following propagation rule is only executed when the
	// change in the relation variables is an exclusion. This is
	// not as fine grained as I would like, because what we
	// catually need is to run on the removal osa subrelation from
	// the provides relation. Probably an advisor will do a better
	// job here.
	auto m = MPG::CPRel::CPRelView::me(med);
	if (m != MPG::CPRel::ME_CPREL_MIN) {
	  // When there is only one provider possible for a package that
	  // is needed then we have to ensure the installation of it.
	  auto needed = inst_.glb();
	  auto possibleProviders = needed.timesULeft(1).intersect(provides_.lub());
	  auto uniqueProviders = possibleProviders.unique(1);
	  auto toIncludeProv = uniqueProviders.intersect(possibleProviders);
	  auto toIncludeInst = toIncludeProv.shiftRight(1);
	  GECODE_ME_CHECK(inst_.include(home,toIncludeInst));
	  
	  // As result of the last statement we have to keep the
	  // consistency with the provides relation by including the new
	  // packages as providers.
	  GECODE_ME_CHECK(provides_.include(home,toIncludeProv));
	}
      }
      

      if (inst_.assigned() && provides_.assigned())
	return home.ES_SUBSUMED(*this);
      return Gecode::ES_NOFIX;
    }
  };
  void provides(Gecode::Space& home, MPG::CPRelVar installation, MPG::CPRelVar provides);
}

#endif
