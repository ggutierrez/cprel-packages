#include <vector>
#include <queue>

#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/graph_traits.hpp>

#include <solver-cprel-paranoid/branch/choice.hh>
#include <gecode/search.hh>
#include <rel/grelation.hh> 
#include <cprel/cprel.hh>

using std::shared_ptr;
using std::vector;
using std::map;

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

/// List of packages (I do not like this design, acts like a global)
vector<int> packages_;

/**
 * \brief Simple and naive brancher based on tuple inclusion and exclusion.
 * \ingroup RelBranch
 */
class ExistingInstallDegree : public Brancher {
protected:
  /// Installation
  CPRelView inst_;
  /// Provides
  CPRelView provides_;
  /// Installed in the system
  GRelation installed_;
  /// Graph with relations
  std::shared_ptr<vector<vector<int>>> allRelations_;  
public:
  /// Constructor for a brancher on variable \a x
  ExistingInstallDegree(Home home, CPRelView inst, CPRelView provides,
                        GRelation installed, shared_ptr<vector<vector<int>>> allRelations)
    : Brancher(home)
    , inst_(inst)
    , provides_(provides)
    , installed_(installed)
    , allRelations_(allRelations) 
  {
    // Handle memory for installed_
    home.notice(*this,Gecode::AP_DISPOSE);
  }
  
  /// Brancher posting
  static void post(Home home, CPRelView inst, CPRelView provides,
                   GRelation installed, 
                   const shared_ptr<vector<vector<int>>> allRelations) {
    (void) new (home) ExistingInstallDegree(home,inst, provides, installed, allRelations);
  }
  /// Constructor for clonning
  ExistingInstallDegree(Space& home, bool share, ExistingInstallDegree& b)
    : Brancher(home,share,b)
    , installed_(b.installed_)
    , allRelations_(b.allRelations_)
  {
    inst_.update(home,share,b.inst_);
    provides_.update(home,share,b.provides_);
  }
    /// Brancher copying
    virtual Brancher* copy(Space& home, bool share) {
      return new (home) ExistingInstallDegree(home,share,*this);
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
  static void getPackages(const vector<int>& tuple) {
    packages_.push_back(tuple.at(0));
    //cout << "A tuple " << tuple.size() << endl;
  }
  /// Creates a choice by selecting a tuple from the unknown of the variable
  virtual Choice* choice(Space&) {
    /**
     * Predicate to insert nodes of the graph in a priority queue
     * according with the degree.
     */
    auto compare_ = [=](int lhs, int rhs) -> bool {
      GRelation relLhs(1), relRhs(1);
      relLhs.add(Tuple({lhs}));
      relRhs.add(Tuple({rhs}));

      int degLhs = boost::out_degree(lhs,*allRelations_);
      int degRhs = boost::out_degree(rhs,*allRelations_);
      return degLhs < degRhs;
    };
    
    // Candidate packages to install
    GRelation candidates = installed_.difference(inst_.glb());
    int selected = -1;
    {
      packages_.clear();
      candidates.visit(getPackages);
      std::priority_queue<int,vector<int>,decltype(compare_)>
        q(compare_);
      for (int p : packages_) {
        GRelation x(1);
        x.add(Tuple({p}));
        if (x.subsetEq(inst_.unk()))
          q.push(p);
      }
      cout << "Is empty?? " << q.empty() << endl;
      selected = q.top();
      //packages_.clear();
    }
      // From the candidates take the one with the higest degree in the graph 
    //    auto v = boost::vertices(*allRelations);
    // std::priority_queue<int,vector<int>,decltype(compare)>  q(v.first, v.second, compare);

    // Select one of the candidates to install
    //Tuple choosen = candidates.pickOneTuple();
    cout << "**>Selected: " << selected << " value "
         << boost::out_degree(selected,*allRelations_)
         << endl;
    return new RelChoice(*this,Tuple({selected}));
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

void existingInstallDegree(Home home, CPRelVar inst, CPRelVar provides, 
                           GRelation installed, 
                           const shared_ptr<vector<vector<int>>> allRelations) {
    if (home.failed()) return;
    
    auto compare = [=](int lhs, int rhs) -> bool {
      int degLhs = boost::out_degree(lhs,*allRelations);
      int degRhs = boost::out_degree(rhs,*allRelations);
      return degLhs < degRhs;
    };

    auto v = boost::vertices(*allRelations);
    std::priority_queue<int,vector<int>,decltype(compare)>  q(v.first, v.second, compare);

    cout << "Higest? " << q.top() << endl;
    cout << "Degree: " << boost::out_degree(q.top(),*allRelations);
    
    ExistingInstallDegree::post(home,inst,provides,installed,allRelations);
}

