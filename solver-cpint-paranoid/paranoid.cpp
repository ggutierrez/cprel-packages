#include <cudf/model.hh>
#include <gecode/kernel.hh>
#include <gecode/int.hh>
#include <gecode/search.hh>
#include <iostream>
#include <string>
#include <sstream>
#include <sstream>
#include <fstream>
#include <unordered_map>

using std::cout;
using std::endl;
using Gecode::Space;
using Gecode::Home;
using Gecode::IntVarArgs;

void nonemin(Home home, const IntVarArgs& x);


class ParanoidSolver : public Gecode::Space {
private:
  // packages in the system
  Gecode::IntVarArray packages_;
  // virtual packages in the system
  std::vector<Gecode::IntVar> virtuals_;
  // optimization value
  Gecode::IntVar opt_;
public:
  /// Constructor
  ParanoidSolver(int packages)
    : packages_(*this,packages,0,1)
    , opt_(*this,Gecode::Int::Limits::min,Gecode::Int::Limits::max)
  {}
  /// Copy constructor
  ParanoidSolver(bool share, ParanoidSolver& other)
    : Gecode::Space(share,other) {
    packages_.update(*this,share,other.packages_);
    opt_.update(*this,share,other.opt_);
    for (auto i = virtuals_.size(); i--;) 
      virtuals_[i].update(*this, share, other.virtuals_[i]);
  }
  /// Copy
  Gecode::Space* copy(bool share) {
    return new ParanoidSolver(share,*this);
  }
  /// Optimization
  virtual void constrain(const Gecode::Space& sol_) {
    const ParanoidSolver& sol = static_cast<const ParanoidSolver&>(sol_);
    // Maximization
    Gecode::rel(*this,opt_,Gecode::IRT_LE,sol.opt_);
  }
  /// Return the value of the optimization
  int optimization(void) const {
    return opt_.min();
  }
  void print(std::ostream& os) const {
    os << "Optimization " << opt_ << endl;
  }
  /// Post a dependency between a package and a virtual package
  void dependOnVirtual(int p, int q) {
    Gecode::IntArgs a(2);
    Gecode::IntVarArgs x(2);
    a[0] = -1; a[1] = 1;
    x[0] = packages_[p];
    x[1] = virtuals_[q];
    Gecode::linear(*this,a,x,Gecode::IRT_GQ,0);
  }
  /// Creates a virtual package in the solver that can be provided by
  /// any package in \a disj
  int createVirtual(const std::vector<int>& disj) {
    int index = virtuals_.size();
    Gecode::IntVar v(*this,0,1);
    virtuals_.push_back(v);

    // prepare the constraint that will install the virtual if at
    // least one of the packages in the disjunctions is installed.
    Gecode::IntArgs a(disj.size() + 1);
    Gecode::IntVarArgs x(disj.size() + 1);
    int i = 0;
    for (int pi : disj) {
      x[i] = packages_[pi];
      a[i] = 1;
      i++;
    }
    a[i] = -1;
    x[i] = v;
    Gecode::linear(*this,a,x,Gecode::IRT_GQ,0);
    
    // prepare the constraint that will make the virtual installed if
    // any of its providers is installed.
    for (int pi : disj) {
      Gecode::IntArgs a(2);
      Gecode::IntVarArgs x(2);
      a[0] = -1; a[1] = 1;
      x[0] = packages_[pi];
      x[1] = v;
      Gecode::linear(*this,a,x,Gecode::IRT_GQ,0);
    }
    
    return index;
  }
  /// Post a constraint stating that p depends on any in disj
  void depend(int p, const std::vector<int>& disj) {
    Gecode::IntArgs a(disj.size() + 1);
    Gecode::IntVarArgs x(disj.size() + 1);
    int i = 0;
    for (int pi : disj) {
      x[i] = packages_[pi];
      a[i] = 1;
      i++;
    }
    a[i] = -1;
    x[i] = packages_[p];

    Gecode::linear(*this,a,x,Gecode::IRT_GQ,0);
  }
  /// Post the constraint that p conflicts with q
  void conflict(int p, int q) {
    Gecode::IntArgs a(2);
    Gecode::IntVarArgs x(2);
    a[0] = -1; a[1] = -1;
    x[0] = packages_[p];
    x[1] = packages_[q];
    Gecode::linear(*this,a,x,Gecode::IRT_GQ,-1);
  }
  void install(const std::vector<int>& disj) {
    Gecode::IntVarArgs x(disj.size());
    int i = 0;
    for (int pi : disj) {
      x[i] = packages_[pi];
      i++;
    }
    Gecode::linear(*this,x,Gecode::IRT_GQ,1);
  }
  void setOptimize(const std::vector<int>& coeff) {
    Gecode::linear(*this,coeff,packages_,Gecode::IRT_EQ,opt_);
  }
  /// Returns the status (true: installed, false otherwise) of a package in the solver
  bool packageInstalled(int p) const {
    if (!packages_[p].assigned()) return false;
    return (bool)packages_[p].min();
  }
  /// Prints the virtuals that got installed at the end of the solving
  void virtualsInstalled(void) const {
    int i = 0;
    for (const Gecode::IntVar& v : virtuals_) {
      if (v.assigned()) 
        cout << "Virtual " << i << ": installed" << endl;
      i++;
    }
  }
  void setBrancher(const std::vector<int>& coeff) {
    using namespace Gecode;
    IntVarArgs must, fair, bad;
    int var = 0;
    for (int w : coeff) {
      if (w == 1)
        bad << packages_[var];
      else if (w == -1)
        fair << packages_[var];
      else
        must << packages_[var];
      var++;
    }

    cout << "Virtuals: " << virtuals_.size() << endl;
    cout << "Must " << must.size() << endl;
    cout << "Fair " << fair.size() << endl;
    cout << "Bad " << bad.size() << endl;

    branch(*this,bad,INT_VAR_DEGREE_MAX,INT_VAL_MIN);
    branch(*this,fair,INT_VAR_DEGREE_MAX,INT_VAL_MAX);   
    branch(*this,must,INT_VAR_DEGREE_MAX,INT_VAL_MAX);
   

    //nonemin(*this,packages_);
  }
  void problemInfo(void) {
    for (auto x : packages_) {
      cout << "A variable: " << x << " degree " << x.degree() << endl;
    }
  }
};

class CUDFVersionedPackage;

class Paranoid : public CUDFTools::Model {
private:
  /// CP model
  ParanoidSolver *solver_;
  /// Data structure for repeated packages
  std::unordered_map<std::string, int> definedDisj_;
public:
    // Objects of this class are non-copyable
  Paranoid() = delete;
  Paranoid(const Paranoid&) = delete;
  Paranoid& operator = (const Paranoid&) = delete;
  /// Constructor from a input specification in \a cudf
  Paranoid(const char* cudf)
    : CUDFTools::Model(cudf), solver_(NULL)
  {    
    // The constructor of the superclass will read everything in cudf,
    // making the number of packages available.
    solver_ = new ParanoidSolver(packages().size());
    
    // The call to these methods will make the interpretation of the
    // constraints. This will indirectly make the methods that we
    // override to be called.
    loadUniverse();
    interpretRequest();

    //solver_->problemInfo();

    cout << "Finished construction" << endl;
  }
  /// Destructor
  virtual ~Paranoid(void) {}
  void objective(void) {
    std::vector<int> coeffs(packages().size());
    
    // try to keep packages that are uninstalled already uninstalled
    for (CUDFVersionedPackage *p : uninstalledPackages())
      coeffs[rank(p)] = 1;
    
    for (CUDFVersionedPackage *p : installedPackages())
      coeffs[rank(p)] =  -(countVersions(p) == 1 ? 
                           (packages().size()) : 
                           1);
    solver_->setOptimize(coeffs);
    solver_->setBrancher(coeffs);
  }
  /// Transforms the disjunction of packages \a disj into packages identifiers
  std::vector<int> toPackageIds(const std::vector<CUDFVersionedPackage*>& disj) {
    std::vector<int> r;
    r.reserve(disj.size());
    
    for (CUDFVersionedPackage *p : disj)
      r.push_back(rank(p));
    
    return r;
  }
  /// Returns the package identifier of \a p
  int toPackageId(CUDFVersionedPackage *p) const  {
    return rank(p);
  }
  /// Transform \a disj into a cannonical form
  void makeCanonic(std::vector<int>& disj) {
    std::sort(std::begin(disj),std::end(disj));
  }
  /// Returns a  key for the canonic version of \a disj
  std::string makeKey(std::vector<int>& disj) {
    makeCanonic(disj);
    std::string key;
    std::stringstream ss(key);
    for (int p : disj)
      ss << p;
    return ss.str();
  }
  /**
   * \brief Handle the disjunction \a disj.
   *
   * If an equivalent disjunctio has been already added then this
   * method returns the index of the virtual package corresponding to
   * it. Otherwise it creates, registers and return a new virtual
   * package.
   */
  int lookUpOrAdd(std::vector<int>& disj) {
    std::string key(makeKey(disj));
    auto f = definedDisj_.find(key);
    if (f != definedDisj_.end()) {
      // a disjunction like this already exists.
      return f->second;;
    }
    
    // Create the disjunction in the solver
    int vp = solver_->createVirtual(disj);
    definedDisj_[key] = vp;
    return vp;
  }

  /// Add a dependency between package \a p on one of the packages in
  /// \a disj
  virtual void depend(CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& disj) {
    
    // convert the disjuntion into package identifiers
    std::vector<int> d = toPackageIds(disj);

    if (disj.size() == 1) {
      solver_->depend(toPackageId(p), d);
      return;
    }
    
    int disjId = lookUpOrAdd(d);
    solver_->dependOnVirtual(toPackageId(p),disjId);
  }
  /// Add a conflict between package \a p and package \a q
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    solver_->conflict(rank(p), rank(q));
    if (foundInstalled(p) && foundInstalled(q)) {
      //cout << "Existent conflict " << rank(p) << " with " << rank(q) << std::endl;
      //cout << "Existent conflict " << versionedName(p) << " with " << versionedName(q) << std::endl;
    }
  }
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  virtual void keep(int , CUDFVersionedPackage *, const std::vector<CUDFVersionedPackage*>&) {
    
  }
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const std::vector<CUDFVersionedPackage*>& disj) {
    std::vector<int> d;
    d.reserve(disj.size());
    for (CUDFVersionedPackage *p : disj)
      d.push_back(rank(p));
    
    solver_->install(d);
  }
  void solutionStats(std::ostream& os, const ParanoidSolver& sol) const {
    // get the number of packages that where removed and installed
    int installed = 0, removed = 0;
    for (CUDFVersionedPackage *p : packages()) {
      int id = toPackageId(p);
      bool currentlyInstalled = sol.packageInstalled(id);
      bool wasinstalled = foundInstalled(p);
      if (currentlyInstalled)
        installed++;
      else 
        if (wasinstalled)
          removed++;
    }
    os << "Installed: " << installed << endl
       << "Removed: " << removed << endl
       << "Optimization: " << sol.optimization() << endl;
  }
  void solve(void) {
    objective();
    
    Gecode::BAB<ParanoidSolver> e(solver_);
    std::cout << "Search will start" << std::endl;
    
    while (Gecode::Space* s = e.next()) {
      ParanoidSolver *sol = static_cast<ParanoidSolver*>(s);
      solutionStats(std::cout,*sol);
      //static_cast<ParanoidSolver*>(s)->virtualsInstalled();
      //cout << "solution" << endl;
      delete s;
    }    
  }
  /**
   * \brief Read a solution to the current problem from \a sol and
   * returnes it
   */
  static std::vector<int> readSolution(std::istream& sol) {
    std::vector<int> s;
    std::string line;
    int numLines = 0;
    while (sol.good()) {
      std::getline(sol,line);
      assert(!line.empty());
      std::stringstream st(line);
      int p; st >> p;
      s.push_back(p);
      numLines++;
    }
    cout << "Readed " << numLines << " packages installed";
    return s;
  }
  /**
   * \brief Post solution
   *
   * Takes all the packages in \a sol and ask the solver to install them.
   */
  void postSolution(const std::vector<int>& sol) {
    for (int p : sol)
      solver_->install({p});
  }
};


int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }
  
  std::fstream sol(argv[2]);
  if (!sol.good()) {
    std::cout << "Unable to open solution file" << std::endl;
    exit(1);
  }
  
  Paranoid model(argv[1]);
  std::vector<int> s = Paranoid::readSolution(sol);
  model.postSolution(s);
  model.solve();
  return 0;
}
