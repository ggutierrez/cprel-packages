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
class MinimalChanges : public Brancher {
protected:
  /// Installation
  CPRelView inst_;
  /// Provides
  CPRelView provides_;
  /// Installed in the system
  GRelation installed_;
public:
  /// Constructor for a brancher on variable \a x
  MinimalChanges(Home home, CPRelView inst, CPRelView provides, GRelation installed)
    : Brancher(home), inst_(inst), provides_(provides), installed_(installed) {
  
    // The memory of the installed_ relation is outside the gecode heap.
    home.notice(*this,Gecode::AP_DISPOSE);
  }
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
      installed_.~GRelation();
      home.ignore(*this,Gecode::AP_DISPOSE);
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
  
  Tuple chooseInstalledProvider(Tuple candidate) const {
    auto v = candidate.value();
    int provider = v.at(0);
    int Virtual = v.at(1);
    
    // Choosen contains a tuple that can be used as a
    // provide. However this tuple might be the best if the provider
    // is not part of the original installation.
    
    // make a relation out of the choosen tuple
    GRelation candidates(2);
    candidates.add(candidate);
    // compute the possible ways of troviding the virtual of the
    // relation
    GRelation possibleProvides = candidates.project(1).timesULeft(1).intersect(provides_.lub());
    // Out of the possible providers, find the ones that are installed
    GRelation possibleProviders = possibleProvides.shiftRight(1);

    //cout << ">>> Need to provide " << possibleProvides << endl;
    
    // this is a safety test, if one of the possible providers is
    // already installed then it means that we are not performing
    // enough propagation because it should have been detected by
    // the provides constraint.
    if (!possibleProviders.intersect(inst_.glb()).empty()) {
      cout << " We have a big problem " << endl;
    }
    
    GRelation installedCandidates = possibleProviders.intersect(installed_);
    if (installedCandidates.empty()) {
      // the heuristic does not applies and we keep the choosen provides
      cout << ">>> Heuristic FAILED " << endl;
      return candidate;
    } else {
      // we prefer to provide the package with the installed one
      provider = installedCandidates.pickOneTuple().value().at(0);
      //cout << ">>> Heuristic OK " << installedCandidates << endl
      //     << ">>> Selecting " << provider << endl;
      return Tuple({provider,Virtual});
    }
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
    return new RelChoice(*this,chooseInstalledProvider(choosen));
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

