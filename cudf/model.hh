#ifndef __CPREL_PACKAGES_CUDF_MODEL_HH__
#define __CPREL_PACKAGES_CUDF_MODEL_HH__

#include <istream>
#include <vector>

// forward declaration to avoid inclusion of cudf.hh
class CUDFVersionedPackage;

namespace CUDFTools {
  /**
   * \brief This class abstracts the notion of a model for solving an
   * instance of the installability problem.
   *
   * The idea is that every method is called when the parser detects a
   * dependency or a conflict relation.
   */
  class Model { 
  private:
    /// Number of dependencies
    int dependencies_;
    /// Number of conflicts
    int conflicts_;
  public:
    ///Empty constructor
    Model(void);
    /// Create a model from an input cudf file with name \a fname
    Model(const char* fname);
    /// Add a dependency between package \a p on one of the packages in
    /// \a disj
    void depend(CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& disj);
    /// Add a conflict between package \a p and package \a q
    void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q);
    /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
    void keep(int kcst, CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& pkgs);
  };
}
#endif
