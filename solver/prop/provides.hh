#ifndef __CPREL_PACKAGES_SOLVER_PROP_PROVIDES_HH__
#define __CPREL_PACKAGES_SOLVER_PROP_PROVIDES_HH__

#include <cprel/cprel.hh>

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
      return Gecode::PropCost::binary(Gecode::PropCost::LO);
    }
    /// Main propagation algorithm
    virtual Gecode::ExecStatus propagate(Gecode::Space& home,
					 const Gecode::ModEventDelta&)  {
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
      }
      /*
      {
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
      */

      if (inst_.assigned() && provides_.assigned())
	return home.ES_SUBSUMED(*this);
      return Gecode::ES_NOFIX;
    }
  };
  void provides(Gecode::Space& home, MPG::CPRelVar installation, MPG::CPRelVar provides);
}

#endif