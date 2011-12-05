#ifndef __CPREL_PACKAGES_CUDF_MODEL_HH__
#define __CPREL_PACKAGES_CUDF_MODEL_HH__

#include <istream>
#include <ostream>
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
  public:
    ///Empty constructor
    Model(void);
    /// Create a model from an input cudf file with name \a fname
    Model(const char* fname);
    /// Destructor
    ~Model(void);
    /// Load the constraints in the universe
    void loadUniverse(void);
    /// Interpret the request
    void interpretRequest(void);
    /// Add a dependency between package \a p on one of the packages in
    /// \a disj
    virtual void depend(CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& disj) = 0;
    /// Add a conflict between package \a p and package \a q
    virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) = 0;
    /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
    virtual void keep(int kcst, CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& pkgs) = 0;
    /// Handle the installation of one of the packages in \a disj
    virtual void install(const std::vector<CUDFVersionedPackage*>& disj) = 0;
    /// Returns the installed packages that were parsed
    const std::vector<CUDFVersionedPackage*>& installedPackages(void) const;
    /// Returns the uninstalled packages that were parsed
    const std::vector<CUDFVersionedPackage*>& uninstalledPackages(void) const;
    /// Returns the packages that were parsed
    const std::vector<CUDFVersionedPackage*>& packages(void) const;
    /*
      In orther to avoid the inclusion of cudf.hh by other files, I am
      providing these methods for packages here. The rationale behind
      this is to avoid the global variables created for the parsing to
      pollute other files.
     */
    /// Returns the package name of \a pkg
    const char* name(const CUDFVersionedPackage *pkg) const;
    /// Returns the package name of \a pkg
    const char* versionedName(const CUDFVersionedPackage *pkg) const;
    /// Returns the package version of \a pkg
    unsigned long long version(const CUDFVersionedPackage *pkg) const;
    /**
     * \brief Returns the rank of \a pkg.
     *
     * The rank is a consequtive number assigned to every package
     * during parsing.
     */
    int rank(const CUDFVersionedPackage *pkg) const;
    /**
     * \brief Returns the number of packages with other versions of \a pkg.
     *
     */
    int countVersions(const CUDFVersionedPackage *pkg) const;
    /**
     * \brief Returns if package \a pkg appears as installed in the input
     *
     */
    bool foundInstalled(const CUDFVersionedPackage *pkg) const;
    /** 
     * \brief Debug package information
     *
     * Prints in \a os the package names (with versions) and the rank
     * assigned during parsing.
    */
    void debugPackageRanks(std::ostream& os) const;
  };
 } 

#endif
