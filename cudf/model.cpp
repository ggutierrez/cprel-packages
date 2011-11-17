#include <iostream>
#include <cudf/model.hh>
#include <cudf/cudf.hh>

using std::cout;
using std::cerr;
using std::endl;

namespace CUDFTools {

  /// Iterpretes the dependencies for package \a pkg and add them to \a model
  void dependencies(CUDFVersionedPackage *pkg, Model& model);
  
  /// Interpretes the conflicts for package \a pkg and add them to \a model
  void conflicts(CUDFVersionedPackage *pkg, Model& model);
  
  /// Interpretes the keep constraints for \a pkg and add them to \a model
  void keep(CUDFVersionedPackage *pkg, std::vector<bool>& handled, Model& model);

  /// Interpretes the constraints available from the universe and add them to \a model
  void universeConstraints(Model& model);
  
  /// Interpretes the request present in the parsed file and add it to \a model
  void handleRequest(Model& model);

  Model::Model(void) 
    : dependencies_(0), conflicts_(0) {}
  
  Model::Model(const char* fname) 
    : dependencies_(0), conflicts_(0) {
    // parse the file with unsa parser
    FILE *input = fopen(fname,"r");
    if (input == NULL) {
      cerr << "Error opening file: " << fname << endl;
      /// \todo rise exception
      exit(1);
    }
    
    switch(parse_cudf(input)) {
    case 0: break;
    case 1: cerr << "Invalid input" << endl; exit(1);
      break;
    case 2: cerr << "Parser memory issue" << endl; exit(1);
      break;
    }
    fclose(input);
    
    // handle the constraints present in the universe
    universeConstraints(*this);
    // handle the request
    handleRequest(*this);
  }

  void Model::depend(CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& disj) {
    dependencies_++;
    // for now just print the dependency
    cout << p->name << "," << p->version << " depends on: "
         << "\t\t{";
    for (CUDFVersionedPackage *d : disj) {
      cout << d->name << "," << d->version << " ";
    }
    cout << "}" << endl;
  }
  
  void Model::conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    conflicts_++;
    cout << p->name << "," << p->version << "\t\tconflicts with: " 
         << q->name << "," << q->version << endl;
  }

  void Model::keep(int kcst, CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& pkgs) {
    
  }
  
  void Model::install(const std::vector<CUDFVersionedPackage*>& disj) {
    cout << "Processed package <install> constraint" << endl;
  }

  // implementation of private functions to interpret the problem input

  void dependencies(CUDFVersionedPackage *pkg, Model& model) {
    const CUDFVersionedPackage& ipkg = *pkg;
    if (ipkg.depends == NULL) {
      return;
    }
    bool has_coeff = false;
    for (const CUDFVpkgList *anddep : *(ipkg.depends)) {
      // every _anddep_ is a conjuntion of disjuntions. We will traverse
      // now the disjunctions.
    
      // prepare storage for call the model
      std::vector<CUDFVersionedPackage*> disj;
      
      // store if there is a self-dependency
      bool self_depend = false;
      for (const CUDFVpkg *ordep : *anddep) {
        const CUDFVirtualPackage& vpackage = *(ordep->virtual_package);
        a_compptr comp = get_comparator(ordep->op);

        //cout << "\t\t" << vpackage.name << " {";
        // is there a concrete package that provides it?
        if (vpackage.all_versions.size() > 0) {
          for (CUDFVersionedPackage *jpkg : vpackage.all_versions)
            if (comp(jpkg->version,ordep->version)) {
              if (jpkg == pkg) { // there is a self dependency
                self_depend = true;
                has_coeff = false;
                break;
              } else {
                // add constraint
                has_coeff = true;
                //cout << jpkg->name << "," << jpkg->version << " ";
                disj.push_back(jpkg);
              }
            } 
        }

        // is there a provider for it?
        if (!self_depend && vpackage.providers.size() > 0) {
          for (CUDFVersionedPackage *jpkg : vpackage.providers)
            if (jpkg == pkg) { // there is a self dependency
              self_depend = true;
              has_coeff = false;
              break;
            } else {
              has_coeff = true;
              //cout << jpkg->name << "," << jpkg->version << " ";
              disj.push_back(jpkg);
            }
        }

        // is there a versioned provider?
        if (!self_depend) {
          for (auto& jpkg : vpackage.versioned_providers)
            if (self_depend) break;
            else if (comp(jpkg.first, ordep->version))
              for (CUDFVersionedPackage *kpkg : jpkg.second)
                if (kpkg == pkg) {
                  self_depend = true;
                  has_coeff = false;
                  break;
                } else {
                  has_coeff = true;
                  // add constraint
                  //cout << kpkg->name << "," << kpkg->version << " ";
                  disj.push_back(kpkg);
                }
        }
        //cout << "}" << endl;
      }
      if (has_coeff) {
        //cout << "\t\tA constraint was posted" << endl;
        model.depend(pkg,disj);
      } else if (!self_depend) {
        cout << "\t\tNo coeff and no self depend " << ipkg.name << "," << ipkg.version << endl;
      }
      //cout << endl;
    }
  }
  
  void conflicts(CUDFVersionedPackage *pkg, Model& model) {
    const CUDFVersionedPackage& ipkg = *pkg;
    if (ipkg.conflicts == NULL) {
      return;
    }
  
    for (CUDFVpkg *ipkgop : *(ipkg.conflicts)) {
      const CUDFVirtualPackage& vpackage = *(ipkgop->virtual_package);
      a_compptr comp = get_comparator(ipkgop->op);
    
      if (vpackage.all_versions.size() > 0) {
        // it conflicts with the right versions of the package
        for(CUDFVersionedPackage *jpkg : vpackage.all_versions) {
          if (jpkg != pkg && comp(jpkg->version, ipkgop->version)) {
            model.conflict(pkg, jpkg);
          }
        }
      }

      // as well as with all the providers
      if (vpackage.providers.size() > 0) {
        for(CUDFVersionedPackage *jpkg : vpackage.all_versions) {
          if (jpkg != pkg) {
            model.conflict(pkg,jpkg);
          }
        }
      }

      // as well as with all the versioned providers with the right
      // version
      for (auto& jpkg : vpackage.versioned_providers)
        if (comp(jpkg.first, ipkgop->version))
          for (CUDFVersionedPackage *kpkg : jpkg.second)
            if (kpkg != pkg) 
              model.conflict(pkg,kpkg);
    
    }

  }

  void keep(CUDFVersionedPackage *pkg, std::vector<bool>& handled, Model& model) {
    const CUDFVersionedPackage& ipkg = *pkg;
    std::vector<CUDFVersionedPackage*> concerned;
    switch(ipkg.keep) {
    case keep_none: break;
    case keep_feature: // Preserve all the provided features
      if (ipkg.provides != NULL) {
        for (CUDFVpkg *ipkgop : *(ipkg.provides) ) {
          CUDFVirtualPackage *vpackage = ipkgop->virtual_package;
          a_compptr comp = get_comparator(ipkgop->op);
          if (vpackage->all_versions.size() > 0)
            for (CUDFVersionedPackage *jpkg : vpackage->all_versions)
              if (comp(jpkg->version, ipkgop->version)) {
                concerned.push_back(jpkg);
              }
          if (vpackage->providers.size() > 0)
            for (CUDFVersionedPackage *jpkg  : vpackage->providers)
              concerned.push_back(jpkg);
          for (auto& jpkg  : vpackage->versioned_providers)
            if (comp(jpkg.first, ipkgop->version))
              for (CUDFVersionedPackage *kpkg : jpkg.second)
                concerned.push_back(kpkg);
          model.keep(keep_feature,pkg,concerned);
        }
      }
      break;
    case keep_package: // Preserve at least one version of the package
      if (ipkg.virtual_package->all_versions.size() > 0 &&
          !handled.at(ipkg.virtual_package->rank+1))
        {
          CUDFVirtualPackage *vpackage = ipkg.virtual_package;
          if (vpackage->all_versions.size() > 0)  // Should not make sense
            for (CUDFVersionedPackage *jpkg : vpackage->all_versions)
              concerned.push_back(jpkg);
          model.keep(keep_package,pkg,concerned);
          handled[ipkg.virtual_package->rank+1] = true;
        }
      break;
    case keep_version: // Preserve the current version
      concerned.push_back(pkg);
      model.keep(keep_version,pkg,concerned);
      break;
    }
  }

  void universeConstraints(Model& model) {
    std::vector<bool> handled(all_virtual_packages.size(), false);
    for (CUDFVersionedPackage *pkg : all_packages) {
      // handle dependencies
      dependencies(pkg,model);
      // handle conflicts
      conflicts(pkg,model);
      // handle keep, only if is installed
      if (pkg->installed)
        keep(pkg,handled,model);
    }
  }

  /// Process the install query represented by \a pkgop
  void install(CUDFVpkg *pkgop, Model& model) {
    CUDFVirtualPackage *vpackage = pkgop->virtual_package;
    a_compptr comp  = get_comparator(pkgop->op);
    bool has_pkg = false;
    
    // storage for the concerned packages
    std::vector<CUDFVersionedPackage*> concerned;
    if (vpackage->all_versions.size() > 0) // Install P = install one version of P
      for (CUDFVersionedPackage *ipkg : vpackage->all_versions)
        if (comp(ipkg->version, pkgop->version)) {
          has_pkg = true;
          concerned.push_back(ipkg);
        }
    if (vpackage->providers.size() > 0) // or install one of the providers of P
      for (CUDFVersionedPackage *jpkg : vpackage->providers) {
        has_pkg = true;
        concerned.push_back(jpkg);
      }
    // or install one of the providers with the right version
    for (auto&  jpkg : vpackage->versioned_providers)
      if (comp(jpkg.first, pkgop->version))
        for (CUDFVersionedPackage *kpkg : jpkg.second) {
          has_pkg = true;
          concerned.push_back(kpkg);
        }
    if (has_pkg)
      model.install(concerned);
    else {
      cerr << "Error: cannot install package" << endl;
      exit(1); // Cannot install this pkg
    }
 }

  /// Process the remove query represented by \a pkgop
  void remove(CUDFVpkg *pkgop, Model& model) {
    cout << "Processed package <remove> constraint" << endl;
  }

  void handleRequest(Model& model) {
    if (the_problem->install != NULL)
      for (CUDFVpkg *ipkgop : *(the_problem->install))
        install(ipkgop, model);
    if (the_problem->remove != NULL)
      for (CUDFVpkg *ipkgop : *(the_problem->remove))
        remove(ipkgop, model);
  }
}
