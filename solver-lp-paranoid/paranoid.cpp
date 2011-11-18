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
  
  enum LP_REL {
    LP_REL_GEQ, // inequality >=
    LP_REL_LEQ, // inequality <=
    LP_REL_EQ   // Equation
  };
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
      glp_set_col_name(lp_, i, versionedName(p));
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
  /**
   * \brief Adds a constraint to the model with relation \a r and \a rhs.
   *
   */
  void addConstraint(LP_REL r, double rhs) {
    if (coeff_rank_ > 1) {
      int irow = glp_add_rows(lp_,1);
      switch (r) {
      case LP_REL_GEQ:
        glp_set_row_bnds(lp_, irow, GLP_LO, rhs, 0);
        break;
      case LP_REL_LEQ:
        glp_set_row_bnds(lp_, irow, GLP_UP, 0, rhs);
        break;
      case LP_REL_EQ:
        glp_set_row_bnds(lp_, irow, GLP_FX, rhs, rhs);
        break;
      }  
      glp_set_mat_row(lp_, irow, coeff_rank_ - 1, &index_[0], &coefficient_[0]);
    }
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
    addConstraint(LP_REL_GEQ, 0);
    // restore the rank
    coeff_rank_ = 1;
  }
  /// Add a conflict between package \a p and package \a q
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    coeff_rank_ = 1;
    setCoeff(p,-1);
    setCoeff(q,-1);
    
    // add the constraint
    addConstraint(LP_REL_GEQ,-1);
    // restore the rank
    coeff_rank_ = 1;
  }
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  virtual void keep(int kcst, CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& pkgs) {
    
  }
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const std::vector<CUDFVersionedPackage*>& disj) {
    coeff_rank_ = 1;
    for (CUDFVersionedPackage *d : disj) 
      if (getCoeff(d) == 0) 
        setCoeff(d,1);
    
    // add the constraint
    addConstraint(LP_REL_GEQ, 1);    
    // restore the rank
    coeff_rank_ = 1;
  }
  /**
   * \brief Solve the actual problem.
   *
   * Returns \a true if a solution was found, \a false otherwise.
   */
  bool solve(void) {
    //glp_write_lp(lp_, NULL, "glpk.lp");
    
    //  glp_smcp simplex_params;
    glp_iocp mip_params;
    
    glp_init_iocp(&mip_params);
    mip_params.binarize = GLP_ON;
    mip_params.gmi_cuts = GLP_ON;
    mip_params.mir_cuts = GLP_ON;
    mip_params.cov_cuts = GLP_ON;
    mip_params.clq_cuts = GLP_ON;
    mip_params.presolve = GLP_ON;
   
   
    glp_cpx_basis(lp_);

    int status = glp_intopt(lp_, &mip_params);
    return status == 0 ? true : false;
  }
  
  /// Retrieve the status of package \a p after the solving process 
  double getStatus(CUDFVersionedPackage *p) const {
    return glp_mip_col_val(lp_, rank(p) + 1);
  }
  /// Outputs the solution to \a os
  void printStats(std::ostream& os) {
    int installed = 0, newInstalled = 0, removed = 0;
    for (CUDFVersionedPackage *p : packages()) {
      if (getStatus(p) > 0.7) {
        installed++;
        if (!foundInstalled(p))
          newInstalled++;
      } else {
        if (foundInstalled(p))
          removed++;
      }
    }
    
    os << "Statistics: " << endl
       << "\tInstalled: " << installed << endl
       << "\tNew installed: " << newInstalled << endl
       << "\tRemoved: " << removed << endl;
      
  }

  /// Outputs the solution to \a os in CUDF format
  void printSolutionCUDF(std::ostream& os) {
    for (CUDFVersionedPackage *p : packages())
      if (getStatus(p) > 0.7) {
        os << "package: " << name(p) << endl
           << "version: " << version(p) << endl
           << "installed: true" << endl << endl;
      } 
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }

  Paranoid model(argv[1]);
  if (model.solve()) {
    cout << "Solution was found" << endl;
    model.printStats(cout);
    //model.printSolutionCUDF(cout);
  } else {
    cout << "Problem is unsatisfiable" << endl;
  }
  
  return 0;
}
