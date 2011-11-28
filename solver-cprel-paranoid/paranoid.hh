#ifndef __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_PARANOID_HH
#define __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_PARANOID_HH

#include <cudf/model.hh>
#include <solver-cprel-paranoid/solver.hh>

#include <iostream>
#include <string>
#include <sstream>
#include <sstream>
#include <fstream>
#include <unordered_map>

using std::ostream;
using std::unordered_map;
using std::string;

class CUDFVersionedPackage;

class Paranoid : public CUDFTools::Model {
private:
  /// CP model
  CUDFTools::ParanoidSolver *solver_;
  /// Data structure for repeated packages
  unordered_map<string, int> definedDisj_;
  /// Requested packages and possible providers
  unordered_map<int, vector<CUDFVersionedPackage*>> request_;
  
public:
    // Objects of this class are non-copyable
  Paranoid() = delete;
  Paranoid(const Paranoid&) = delete;
  Paranoid& operator = (const Paranoid&) = delete;
  /// Constructor from a input specification in \a cudf

  Paranoid(const char* cudf);
  /// Destructor
  virtual ~Paranoid(void);
  void objective(void);
  /// Transforms the disjunction of packages \a disj into packages identifiers
  vector<int> toPackageIds(const vector<CUDFVersionedPackage*>& disj);
  /// Returns the package identifier of \a p
  int toPackageId(CUDFVersionedPackage *p) const;
  /// Transform \a disj into a cannonical form
  void makeCanonic(vector<int>& disj);
  /// Returns a  key for the canonic version of \a disj
  string makeKey(vector<int>& disj);
  /**
   * \brief Handle the disjunction \a disj.
   *
   * If an equivalent disjunctio has been already added then this
   * method returns the index of the virtual package corresponding to
   * it. Otherwise it creates, registers and return a new virtual
   * package.
   */
  int lookUpOrAdd(vector<int>& disj);

  /// Add a dependency between package \a p on one of the packages in
  /// \a disj
  virtual void depend(CUDFVersionedPackage *p, const vector<CUDFVersionedPackage*>& disj);
  /// Add a conflict between package \a p and package \a q
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q);
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  virtual void keep(int , CUDFVersionedPackage *, const vector<CUDFVersionedPackage*>&);
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const vector<CUDFVersionedPackage*>& disj);
  void solutionInfo(ostream& os, const CUDFTools::ParanoidSolver& sol) const;
  void solutionStats(ostream& os, const CUDFTools::ParanoidSolver& sol) const;
  void solve(void);
  /**
   * \brief Read a solution to the current problem from \a sol and
   * returnes it
   */
  static vector<int> readSolution(std::istream& sol);
  /**
   * \brief Post solution
   *
   * Takes all the packages in \a sol and ask the solver to install them.
   */
  void postSolution(const std::vector<int>& sol);
  void problemInfo(void) const;
};

#endif


