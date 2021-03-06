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
class ExistingInstall : public Brancher {
protected:
  /// Installation
  CPRelView inst_;
  /// Provides
  CPRelView provides_;
  /// Installed in the system
  GRelation installed_;
public:
  /// Constructor for a brancher on variable \a x
  ExistingInstall(Home home, CPRelView inst, CPRelView provides, GRelation installed)
    : Brancher(home), inst_(inst), provides_(provides), installed_(installed) {

    // Handle memory for installed_
    home.notice(*this,Gecode::AP_DISPOSE);
  }
    /// Brancher posting
    static void post(Home home, CPRelView inst, CPRelView provides, GRelation installed) {
      (void) new (home) ExistingInstall(home,inst, provides, installed);
    }
    /// Constructor for clonning
    ExistingInstall(Space& home, bool share, ExistingInstall& b)
      : Brancher(home,share,b), installed_(b.installed_) {
      inst_.update(home,share,b.inst_);
      provides_.update(home,share,b.provides_);
    }
    /// Brancher copying
    virtual Brancher* copy(Space& home, bool share) {
      return new (home) ExistingInstall(home,share,*this);
    }
    /// Brancher disposal
    virtual size_t dispose(Space& home) {
      installed_.~GRelation();
      home.ignore(*this,Gecode::AP_DISPOSE);
      (void) Brancher::dispose(home);
      return sizeof(*this);
    }
    /// Returns the status of the brancher
    virtual bool status(const Space&) const {
      cout << "**> Installed size: " << installed_.cardinality() << endl;
      cout << "**> Inst size: " << inst_.glb().cardinality() << endl;
      cout << "**> Common with inst " <<  inst_.glb().intersect(installed_).cardinality()
           << endl;
      if (installed_.difference(inst_.glb()).empty())
        return false;
      return true;
    }
    
  /// Creates a choice by selecting a tuple from the unknown of the variable
  virtual Choice* choice(Space&) {
    GRelation candidates = installed_.difference(inst_.glb());
    
    // Select one of the candidates to install
    Tuple choosen = candidates.pickOneTuple();
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
	std::cout << "-> Brancher adding: " << r << std::endl;
	return Gecode::me_failed(inst_.include(home,r)) ? ES_FAILED : ES_OK;
      } else {
	//std::cout << "-> Brancher removing: " << r << std::endl;
	return Gecode::me_failed(inst_.exclude(home,r)) ? ES_FAILED : ES_OK;
      }
    }
  };

void existingInstall(Home home, CPRelVar inst, CPRelVar provides, GRelation installed) {
    if (home.failed()) return;
    ExistingInstall::post(home,inst,provides,installed);
  }

