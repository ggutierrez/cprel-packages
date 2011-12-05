#include <solver-cprel-paranoid/branch/choice.hh>
#include <gecode/search.hh>
#include <rel/grelation.hh> 
#include <cprel/cprel.hh>

using MPG::CPRelVar;
using MPG::CPRel::CPRelView;
using MPG::GRelation;
using MPG::Tuple;

using CUDFTools::RelChoice;

using Gecode::Space;
using Gecode::Archive;
using Gecode::Brancher;
using Gecode::Choice;
using Gecode::ExecStatus;
using Gecode::Home;
using Gecode::ES_FAILED;
using Gecode::ES_OK;

/**
 * \brief Simple and naive brancher based on tuple inclusion and exclusion.
 * \ingroup RelBranch
 */
class StableProvides : public Brancher {
protected:
  /// Installation
  CPRelView inst_;
  /// Provides
  CPRelView provides_;
  /// Last computed relation of needed provides
  mutable MPG::GRelation needed_;
public:
  /// Constructor for a brancher on variable \a x
  StableProvides(Home home, CPRelView inst, CPRelView provides)
    : Brancher(home), inst_(inst), provides_(provides), needed_(2) {
    // needed_ attribute handles memmory outside of the space
    home.notice(*this,Gecode::AP_DISPOSE);
  }
    /// Brancher posting
    static void post(Home home, CPRelView inst, CPRelView provides) {
      (void) new (home) StableProvides(home,inst, provides);
    }
    /// Constructor for clonning
    StableProvides(Space& home, bool share, StableProvides& b)
      : Brancher(home,share,b), needed_(2) {
      // The needed_ attribute is only optimization. It avoid to keep
      // the result in the status method and to use it to create the
      // choice without having to recompute it again. In every call to
      // the copy constructor this relation is initialized emtpy.

      inst_.update(home,share,b.inst_);
      provides_.update(home,share,b.provides_);
    }
    /// Brancher copying
    virtual Brancher* copy(Space& home, bool share) {
      return new (home) StableProvides(home,share,*this);
    }
    /// Brancher disposal
    virtual size_t dispose(Space& home) {
      needed_.~GRelation();
      home.ignore(*this,Gecode::AP_DISPOSE);
      (void) Brancher::dispose(home);
      return sizeof(*this);
    }
    /// Returns the status of the brancher
    virtual bool status(const Space&) const {
      // get the all the possible ways to provide the elements in inst
      auto possibleProvides = inst_.glb().timesULeft(1).intersect(provides_.lub());
      auto alreadyProvided = provides_.glb().exists(1);
      
      needed_.become(possibleProvides.difference(alreadyProvided));
      //std::cout << "Needed provides: " << needed_ << std::endl; 
      if (needed_.empty())
	return false;
      return true;
    }
    /// Creates a choice by selecting a tuple from the unknown of the variable
    virtual Choice* choice(Space&) {
      if (needed_.empty()) {
        cout << "There is a big problem in the stableProvides brancher " << endl;
      }
      Tuple choosen = needed_.pickOneTuple();

      return new RelChoice(*this,choosen);
    }
    virtual Choice* choice(const Space&, Archive& e) {
      assert(false);
      int arity;
      e >> arity;
      std::vector<int> t(arity,0);
      for (int i = 0; i < arity; i++) {
	int v;
	e >> v;
	t[i] = v;
      }
      MPG::Tuple x(t);
      return new RelChoice(*this,x);
    }

    /// Commit choice
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int a) {
      const RelChoice& ch = static_cast<const RelChoice&>(c);
      GRelation r(ch.t_.arity());
      r.add(ch.t_);
      if (a == 0) {
	//std::cout << "-> Brancher adding: " << r << std::endl;
	return Gecode::me_failed(provides_.include(home,r)) ? ES_FAILED : ES_OK;
      } else {
	//std::cout << "-> Brancher removing: " << r << std::endl;
	return Gecode::me_failed(provides_.exclude(home,r)) ? ES_FAILED : ES_OK;
      }
    }
  };

  /**
   * \brief Naive brancher on relation \a R
   * \ingroup RelBranch
   *
   * Branches on \a R by selecting a tuple in it and creating a choice point that
   * includes and excludes that tuple.
   */
  void stableProvides(Home home, CPRelVar inst, CPRelVar provides) {
    if (home.failed()) return;
    StableProvides::post(home,inst,provides);
  }

