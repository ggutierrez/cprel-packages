#include <gecode/search.hh>
#include <rel/grelation.hh> 
#include <cprel/cprel.hh>

using MPG::CPRelVar;
using MPG::CPRel::CPRelView;
using MPG::GRelation;
using MPG::Tuple;


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
class MinimalChanges : public Brancher {
protected:
  /// Installation
  CPRelView inst_;
  /// Provides
  CPRelView provides_;
  /// Installed in the system
  GRelation installed_;
  /// Simple, tuple-based relation choice
  class RelChoice : public Choice {
  public:
    /// Tuple to branch on
    Tuple t_;
    /// Constructor
    RelChoice(const MinimalChanges& b, const Tuple& t)
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
  MinimalChanges(Home home, CPRelView inst, CPRelView provides, GRelation installed)
    : Brancher(home), inst_(inst), provides_(provides), installed_(installed) {}
    /// Brancher posting
    static void post(Home home, CPRelView inst, CPRelView provides, GRelation installed) {
      (void) new (home) MinimalChanges(home,inst, provides, installed);
    }
    /// Constructor for clonning
    MinimalChanges(Space& home, bool share, MinimalChanges& b)
      : Brancher(home,share,b), installed_(b.installed_) {
      inst_.update(home,share,b.inst_);
      provides_.update(home,share,b.provides_);
    }
    /// Brancher copying
    virtual Brancher* copy(Space& home, bool share) {
      return new (home) MinimalChanges(home,share,*this);
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
      // This is repeated work. We already computed this for deciding
      // if the brancher should be still active or not. A better
      // approach is to keep a ground relation that stores the
      // subrelation computed at status and then to use it here.

      auto possibleProvides = inst_.glb().timesULeft(1).intersect(provides_.lub());
      auto alreadyProvided = provides_.glb().exists(1) ;      
      auto needed_ = possibleProvides.difference(alreadyProvided);

      // out of the ones that are needed we will choose the ones that
      // are present in the installed_ relation.
      Tuple choosen = needed_.pickOneTuple();

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

void minimalChanges(Home home, CPRelVar inst, CPRelVar provides, GRelation installed) {
    if (home.failed()) return;
    MinimalChanges::post(home,inst,provides,installed);
  }
