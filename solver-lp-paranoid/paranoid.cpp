#include <cudf/model.hh>
#include <glpk.h>
#include <iostream>

using std::cout;
using std::endl;

class CUDFVersionedPackage;

class Paranoid : public CUDFTools::Model {
private:
  /// LP model
  glp_prob *lp_;
  /// Number of variables in the model
  int variables_;
  /// Coefficient array for every constraint
  std::vector<double> coefficient_;
  /// Index array for every constraint
  std::vector<int> index_;
  int coeff_rank_;
public:
  // Objects of this class are non-copyable
  Paranoid() = delete;
  Paranoid(const Paranoid&) = delete;
  Paranoid& operator = (const Paranoid&) = delete;
  /// Constructor from a input specification in \a cudf
  Paranoid(const char* cudf)
    : CUDFTools::Model(cudf), lp_(NULL)
  {    
    // The constructor of the superclass will read everything in cudf,
    // making the number of packages available.
    variables_ = packages().size();
    lp_ = glp_create_prob();
    glp_add_cols(lp_,variables_);
    
    // reserve some space in the vectors for the constraints
    coefficient_.resize(variables_ + 1);
    index_.resize(variables_ + 1);
   
    // The call to these methods will make the interpretation of the
    // constraints. This will indirectly make the methods that we
    // override to be called.
    loadUniverse();
    interpretRequest();
   
    // set the objective function
    objective();
    cout << "Number of variables: " << variables_ << endl;
  }
  /// Destructor
  virtual ~Paranoid(void) {}
  // Add the objective function to the problem
  void objective(void) {
    // is a minimization
    glp_set_obj_dir(lp_,GLP_MIN);
    
    // try to keep packages that are uninstalled already uninstalled
    for (CUDFVersionedPackage *p : uninstalledPackages())
      glp_set_obj_coef(lp_, rank(p) + 1, 1);
    
    for (CUDFVersionedPackage *p : installedPackages())
      glp_set_obj_coef(lp_, rank(p) + 1, -(countVersions(p) == 1 ? variables_ : 1));
 
    // Add the column names and the bounds for the variables of the objective function
    for (CUDFVersionedPackage *p : packages()) {
      int i = rank(p) + 1;
      glp_set_col_bnds(lp_, i, GLP_DB, 0, 1); // set bounds to [0, 1]
      //glp_set_col_name(lp_, i,  )
      glp_set_col_kind(lp_, i, GLP_BV);
    }
  }
  /// Returns a coefficient for this package in the current constraint, zero if there is no one.
  int getCoeff(CUDFVersionedPackage *p) {
    int prank = rank(p) + 1;
    for (int i = 1; i < coeff_rank_; i++)
      if (index_.at(i) == prank)
        return coefficient_.at(i);
    return 0;
  }
  void setCoeff(CUDFVersionedPackage *p, double value) {
    coefficient_[coeff_rank_] = value;
    index_[coeff_rank_] = rank(p) + 1;
    coeff_rank_++;
  }
  /// Add a dependency between package \a p on one of the packages in
  /// \a disj
  virtual void depend(CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& disj) {
    //cout << name(p) << "*" << version(p)  << endl;
    coeff_rank_ = 1;
    for (CUDFVersionedPackage *d : disj) 
      if (getCoeff(d) == 0) 
        setCoeff(d,1);
    setCoeff(p,-1);
    
    // add the constraint
    if (coeff_rank_ > 1) {
      int irow = glp_add_rows(lp_,1);
      glp_set_row_bnds(lp_, irow, GLP_LO, -1, 0);
      glp_set_mat_row(lp_, irow, coeff_rank_ - 1, &index_[0], &coefficient_[0]);
    }
    // restore the rank
    coeff_rank_ = 1;
  }
  /// Add a conflict between package \a p and package \a q
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    
  }
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  virtual void keep(int kcst, CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& pkgs) {

  }
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const std::vector<CUDFVersionedPackage*>& disj) {

  }
  /// Solve the actual problem
  void solve(void) {
    glp_write_lp(lp_, NULL, "glpk.lp");
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }

  Paranoid model(argv[1]);
  model.solve();
  cout << "Constructed model" << endl;
  return 0;
}
