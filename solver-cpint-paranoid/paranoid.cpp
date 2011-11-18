#include <cudf/model.hh>
#include <gecode/kernel.hh>
#include <gecode/int.hh>
#include <iostream>

using std::cout;
using std::endl;
using Gecode::Space;

class ParanoidSolver : public Gecode::Space {
private:
  // packages in the system
  Gecode::IntVarArray packages_;
  // optimization value
  Gecode::IntVar opt_;
public:
  /// Constructor
  ParanoidSolver(int packages)
    : packages_(*this,packages,0,1)
    , opt_(*this,Gecode::Int::Limits::min,Gecode::Int::Limits::max)
  {}
  /// Copy constructor
  ParanoidSolver(bool share, ParanoidSolver& other) {
    packages_.update(*this,share,other.packages_);
  }
  /// Copy
  Gecode::Space* copy(bool share) {
    return new ParanoidSolver(share,*this);
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
  void setOptimize(const std::vector<int>& coeffs) {
    
  }
};

class CUDFVersionedPackage;

class Paranoid : public CUDFTools::Model {
private:
  /// CP model
  ParanoidSolver *solver_;
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
  }

  /// Add a dependency between package \a p on one of the packages in
  /// \a disj
  virtual void depend(CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& disj) {
    std::vector<int> a;
    a.reserve(disj.size());
    for (CUDFVersionedPackage *d : disj)
      a.push_back(rank(d));
    solver_->depend(rank(p), a);
  }
  /// Add a conflict between package \a p and package \a q
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    solver_->conflict(rank(p), rank(q));
  }
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  virtual void keep(int kcst, CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& pkgs) {
    
  }
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const std::vector<CUDFVersionedPackage*>& disj) {
    std::vector<int> d;
    d.reserve(disj.size());
    for (CUDFVersionedPackage *p : disj)
      d.push_back(rank(p));
    
    solver_->install(d);
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }
  
  Paranoid model(argv[1]);
    
  return 0;
}
