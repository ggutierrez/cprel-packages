#include <iostream>
#include <cudf/cudf.hh>

using namespace std;

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
  Model(void) 
    : dependencies_(0), conflicts_(0) {}
  /// Add a dependency between package \a p on one of the packages in
  /// \a disj
  void depend(CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& disj) {
    dependencies_++;
    // for now just print the dependency
    cout << p->name << "," << p->version << " depends on: "
         << "\t\t{";
    for (CUDFVersionedPackage *d : disj) {
      cout << d->name << "," << d->version << " ";
    }
    cout << "}" << endl;
  }
  /// Add a conflict between package \a p and package \a q
  void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    conflicts_++;
    cout << p->name << "," << p->version << "\t\tconflicts with: " 
         << q->name << "," << q->version << endl;
  }
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  void keep(CUDFKeepOp kcst, CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& pkgs) {
    
  }
};

/// Iterpretes the dependencies for package \a pkg and add them to \a model
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

void install(void) {
  if (the_problem->install == NULL) return;
  for (CUDFVpkg *ipkgop : *(the_problem->install)) {
    CUDFVirtualPackage *vpackage = ipkgop->virtual_package;
    a_compptr comp = get_comparator(ipkgop->op);
    bool has_pkg = false;
    cout << "{";
    // Install a package is install one version of it
    if (vpackage->all_versions.size() > 0) 
      for (CUDFVersionedPackage *ipkg : vpackage->all_versions) {
        if (comp(ipkg->version, ipkgop->version)) {
          has_pkg = true;
          // add constraint
          cout << ipkg->name << "," << ipkg->version << " ";
        }
      }
    
    // also we can install a provider for it
    if (vpackage->providers.size() > 0) {
      for (CUDFVersionedPackage *jpkg : vpackage->providers) {
        has_pkg = true;
        // add constraint
        cout << jpkg->name << "," << jpkg->version << " ";
      }
    }

    // or install one of the providers with the right version
    for (auto& jpkg : vpackage->versioned_providers)
      if (comp(jpkg.first,ipkgop->version))
        for (CUDFVersionedPackage *kpkg : jpkg.second) {
          // add constraint
          has_pkg = true;
          cout << kpkg->name << "," << kpkg->version << " ";
        }    
    cout << "}" << endl;
    if (has_pkg) {
      cout << "\tAdded constraint" << endl;
    } else {
      cout << "\tThere is no solution" << endl;
    }
  }
}
void handleRequest(void) {
  // handling the request invloves taking into account the install,
  // remove and upgrade statements.
  install();
}

/*
void printVirtuals(void) {
  for (CUDFVirtualPackage *vp : all_virtual_packages) {
    cout << "Package: " << vp->name << endl;
    // get all the versions that provide the package
    const auto& all = vp->all_versions;
    if (all.size() != 0) {
      // this is a package that exists in the universe
      for (CUDFVersionedPackage *provider : all) {
        cout << "\t" << provider->name << "," << provider->version << endl;
      }
    } else {
      // this is a package that is provided in the universe but is
      // completely virtual.
      const auto& providers = vp->providers;
      for (CUDFVersionedPackage *provider : providers) {
        cout << "\t" << provider->name << "," << provider->version << endl;
      }
      //cout << "\tComplete virtual??" << endl;
    }
    // there is something I do not understand yet and is what is
    // versioned_providers
    const auto& versProv = vp->versioned_providers;
    for (const auto& e : versProv) {
      cout << "Something here" << endl;
    }
  }
}
*/
int main(int argc, char* argv[]) {
  if (argc != 2) {
    cerr << "Error, no input provided" << endl;
    return 1;
  }
  // the input file is the first argument
  FILE *input_file = fopen(argv[1], "r");
  if (input_file == NULL) {
    cerr << "Error opening file: " << argv[1] << endl;
    return 1;
  }
  // parse the file, using unsa parser
  switch(parse_cudf(input_file)) {
  case 0: break;
  case 1: cerr << "Invalid input" << endl; return 1;
  break;
  case 2: cerr << "Parser memory issue" << endl; return 1;
  break;
  }
  fclose(input_file);
  cout << "CUDF statistics: " << argv[1] << endl;
  cout << "\tPackages in the universe: " << endl
       << "\t\tconcretes: " << all_packages.size() << endl
       << "\t\tvirtuals: " << all_virtual_packages.size() << endl;;
  
  cout << "\tRequest: " << endl;
  if (the_problem->install != NULL)
    cout << "\t\tinstall: " << the_problem->install->size() << endl;
  if (the_problem->upgrade != NULL)
    cout << "\t\tupgrade: " << the_problem->upgrade->size() << endl;
  if (the_problem->remove != NULL)
  cout << "\t\tremove: " << the_problem->remove->size() << endl;

  Model m;
  universeConstraints(m);
  //printVirtuals();
  //printUniverse();
  //handleRequest();
  return 0;
}
