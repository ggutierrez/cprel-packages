#include <cudf/model.hh>
#include <solver-cprel-paranoid/solver.hh>

#include <iostream>
#include <string>
#include <sstream>
#include <sstream>
#include <fstream>
#include <unordered_map>

using std::cout;
using std::endl;
using std::ostream;
using std::unordered_map;
using std::string;
using Gecode::Space;
using Gecode::BAB;

class CUDFVersionedPackage;

class Paranoid : public CUDFTools::Model {
private:
  /// CP model
  CUDFTools::ParanoidSolver *solver_;
  /// Data structure for repeated packages
  unordered_map<string, int> definedDisj_;
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
    solver_ = new CUDFTools::ParanoidSolver(packages().size());
    
    // The call to these methods will make the interpretation of the
    // constraints. This will indirectly make the methods that we
    // override to be called.
    loadUniverse();
    interpretRequest();

    solver_->initVariables();
    cout << "Finished construction" << endl;
  }
  /// Destructor
  virtual ~Paranoid(void) {
    delete solver_;
  }
  void objective(void) {
    
  }
  /// Transforms the disjunction of packages \a disj into packages identifiers
  vector<int> toPackageIds(const vector<CUDFVersionedPackage*>& disj) {
    vector<int> r;
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
  void makeCanonic(vector<int>& disj) {
    std::sort(std::begin(disj),std::end(disj));
  }
  /// Returns a  key for the canonic version of \a disj
  string makeKey(vector<int>& disj) {
    makeCanonic(disj);
    string key;
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
  int lookUpOrAdd(vector<int>& disj) {
    string key(makeKey(disj));
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
  virtual void depend(CUDFVersionedPackage *p, const vector<CUDFVersionedPackage*>& disj) {
    
    // convert the disjuntion into package identifiers
    vector<int> d = toPackageIds(disj);

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
  virtual void keep(int , CUDFVersionedPackage *, const vector<CUDFVersionedPackage*>&) {
    
  }
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const vector<CUDFVersionedPackage*>& disj) {
    // convert the disjuntion into package identifiers
    vector<int> d = toPackageIds(disj);
    if (d.size() == 1) {
      solver_->install(d.at(0));
      return;
    }
    int disjId = lookUpOrAdd(d);
    cout << "The package " << disjId << " needs to be installed" << endl;
    solver_->install(disjId);
  }
  void solutionInfo(ostream& os, const CUDFTools::ParanoidSolver& sol) const {
    vector<int> installed = sol.installedPackages();
    for (int p : installed) {
      os << "Package installed " << p << endl;
    }
  }
  void solutionStats(ostream& os, const CUDFTools::ParanoidSolver& sol) const {
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
    solver_->setBrancher();

    BAB<CUDFTools::ParanoidSolver> e(solver_);
    cout << "--- Search will start" << endl;
    
    while (Space* s = e.next()) {
      cout << "*** Solution" << endl;
      CUDFTools::ParanoidSolver *sol 
        = static_cast<CUDFTools::ParanoidSolver*>(s);
      //solutionInfo(cout,*sol);
      solutionStats(cout,*sol);
      //static_cast<ParanoidSolver*>(s)->virtualsInstalled();
      cout << "*** Solution END ***" << endl;
      delete s;
    }    
  }
  /**
   * \brief Read a solution to the current problem from \a sol and
   * returnes it
   */
  static vector<int> readSolution(std::istream& sol) {
    vector<int> s;
    string line;
    int numLines = 0;
    while (sol.good()) {
      std::getline(sol,line);
      assert(!line.empty());
      std::stringstream st(line);
      int p; st >> p;
      s.push_back(p);
      numLines++;
    }
    return s;
  }
  /**
   * \brief Post solution
   *
   * Takes all the packages in \a sol and ask the solver to install them.
   */
  void postSolution(const std::vector<int>& sol) {
    for (auto p = begin(sol); p != end(sol); ++p) {
      solver_->install({*p});
    }
  }
  void problemInfo(void) const {
    solver_->problemInfo();
  }
};

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }
  
  // load the solution file
  std::fstream sol(argv[2]);
  if (!sol.good()) {
    cout << "Unable to open solution file" << endl;
    exit(1);
  }
  
  Paranoid model(argv[1]);
  vector<int> s = Paranoid::readSolution(sol);
  model.postSolution(s);
  model.problemInfo();
  model.solve();
  return 0;
}
