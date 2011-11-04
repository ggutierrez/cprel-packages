#ifndef __CPREL_PACKAGES_SOLVERSUPPORT_PKG_BUILTINS_HH__
#define __CPREL_PACKAGES_SOLVERSUPPORT_PKG_BUILTINS_HH__

#include <rel/grelation.hh>
#include <solversupport/reader.hh>

namespace CPRelPkg {
  /**
   * \brief In a ternary relation specifying the packages of
   * installation, returns a sub relation with properties \a keep and
   * \install
   */
  inline 
  MPG::GRelation filterPackages(const MPG::GRelation& packages, int keep, int install) {
    MPG::GRelation tmp(2);
    tmp.add(MPG::Tuple({keep,install}));
    
    auto allPossible = tmp.timesULeft(1);
    auto filtered = allPossible.intersect(packages);
    return filtered;
  }
  /**
   * \brief Checks if the packages contained in \a installation are
   * stable with respect to the \a dependencies relation.
   */
  inline
  bool isStableDependencies(const MPG::GRelation& installation, const MPG::GRelation& dependencies) {
    // we need all the packages in the right most column to be installed
    auto depsNeeded = installation.timesURight(1).intersect(dependencies);
    auto packagesNeeded = depsNeeded.project(1);
    std::cout << "Packages that are not installed " << packagesNeeded.difference(installation) << std::endl; 
    return packagesNeeded.difference(installation).empty();
  }

  /**
   * \brief Checks if \a problem is dependency stable.
   *
   * For a problem to stisfy this property, all the packages that are
   * reported as installed must have all their dependencies also
   * installed.
   */
  inline
  bool isStableDependencies(const CPRelPkg::ProblemDesc& problem) {
    auto inst = std::get<0>(problem);
    auto deps = std::get<1>(problem);
    // 1. Every package installed has its dependencies installed
    //  Get the unary relation with packages
    auto installed = filterPackages(inst,0,1).shiftRight(2);
    //  Get the packages that must be uninstalled
    auto mustUninstall = filterPackages(inst,1,0).shiftRight(2);
    //   we are not interested in the dependencies of the packages that
    //   must be uninstalled
    auto depsMustUninstall = mustUninstall.timesURight(1).intersect(deps);
    //   dependencies that we care about
    auto depsToCheck = deps.difference(depsMustUninstall);
    return isStableDependencies(installed,depsToCheck);
  }

  /**
   * \brief Checks if the packages contained in \a installation are
   * stable with respect to the \a provides relation.
   */
  inline
  bool isStableProvides(const MPG::GRelation& installation, const MPG::GRelation& provides) {
    // we need all the packages in the installation to be provided
    auto providesInInstallation = installation.times(installation).intersect(provides);
    auto provided = providesInInstallation.shiftRight(1);

    //std::cout << "Provided " << provided.difference(installation) << std::endl;
    //std::cout << "Provided2 " << installation.difference(provided) << std::endl;
    
    return provided.eq(installation);
  }
}

#endif
