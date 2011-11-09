#include <solver/solver.hh>
#include <gecode/search.hh>
#include <solversupport/pkg-builtins.hh>

using Gecode::Space;
using Gecode::Archive;
using Gecode::Brancher;
using Gecode::Choice;
using Gecode::ExecStatus;
using Gecode::Home;
using Gecode::ES_FAILED;
using Gecode::ES_OK;

namespace CPRelPkg {

  /**
   * \brief Simple and naive brancher based on tuple inclusion and exclusion.
   * \ingroup RelBranch
   */
  class NaiveBranch : public Brancher {
  protected:
    /// Installation
    MPG::CPRel::CPRelView inst_;
    /// Provides
    MPG::CPRel::CPRelView provides_;
    /// Last computed relation of needed provides
    //mutable MPG::GRelation needed_;
    /// Simple, tuple-based relation choice
    class RelChoice : public Choice {
    public:
      /// Tuple to branch on
      MPG::Tuple t_;
      /// Constructor
      RelChoice(const NaiveBranch& b, const MPG::Tuple& t)
	: Choice(b,2), t_(t) {}
      /// Returns the size of the object
      virtual size_t size(void) const {
	return sizeof(*this);
      }
      virtual void archive(Archive& e) const {
	Choice::archive(e);
	std::vector<int> t(t_.value());
	// first the arity of the tuple and then the tuple itself
	e << t_.arity();
	for (int i = 0; i < t_.arity(); i++) {
	  e << t[i];
	}
      }
    };
  public:
    /// Constructor for a brancher on variable \a x
    NaiveBranch(Home home, MPG::CPRel::CPRelView inst, MPG::CPRel::CPRelView provides)
      : Brancher(home), inst_(inst), provides_(provides) {}
    /// Brancher posting
    static void post(Home home, MPG::CPRel::CPRelView inst, MPG::CPRel::CPRelView provides) {
      (void) new (home) NaiveBranch(home,inst, provides);
    }
    /// Constructor for clonning
    NaiveBranch(Space& home, bool share, NaiveBranch& b)
      : Brancher(home,share,b) {
      inst_.update(home,share,b.inst_);
      provides_.update(home,share,b.provides_);
    }
    /// Brancher copying
    virtual Brancher* copy(Space& home, bool share) {
      return new (home) NaiveBranch(home,share,*this);
    }
    /// Brancher disposal
    virtual size_t dispose(Space& home) {
      (void) Brancher::dispose(home);
      return sizeof(*this);
    }
    /// Returns the status of the brancher
    virtual bool status(const Space&) const {
      // get the all the possible ways to provide the elements in inst
      auto possibleProvides = inst_.glb().timesULeft(1).intersect(provides_.lub());
      auto alreadyProvided = provides_.glb().exists(1);
      
      auto needed_ = possibleProvides.difference(alreadyProvided);
      //std::cout << "Needed provides: " << needed_ << std::endl; 
      if (needed_.empty())
	return false;
      return true;
    }
    /// Creates a choice by selecting a tuple from the unknown of the variable
    virtual Choice* choice(Space&) {
      // This is repreated work. We already computed this for deciding
      // if the brancher should be still active or not. A better
      // approach is to keep a ground relation that stores the
      // subrelation computed at status and then to use it here.

      auto possibleProvides = inst_.glb().timesULeft(1).intersect(provides_.lub());
      auto alreadyProvided = provides_.glb().exists(1) ;      
      auto needed_ = possibleProvides.difference(alreadyProvided);
      MPG::Tuple choosen = needed_.pickOneTuple();

      /*
      auto value = choosen.value();
      if (value.at(0) == value.at(1))
	std::cerr << "Bad choice!!! " << choosen << std::endl; 
      */
      // the choosen tuple cannot represent a concrete package
      // providing itself.


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
      MPG::GRelation r(ch.t_.arity());
      r.add(ch.t_);
      if (a == 0) {
	std::cout << "-> Brancher adding: " << r << std::endl;
	return Gecode::me_failed(provides_.include(home,r)) ? ES_FAILED : ES_OK;
      } else {
	std::cout << "-> Brancher removing: " << r << std::endl;
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
  void stableProvides(Home home, MPG::CPRelVar inst, MPG::CPRelVar provides) {
    if (home.failed()) return;
    NaiveBranch::post(home,inst,provides);
  }
}
