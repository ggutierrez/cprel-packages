#include <iostream>
#include <cudf/cudf.hh>

using namespace std;

/// Prints the dependencies of \a ipkg
void dependencies(CUDFVersionedPackage *pkg) {
  const CUDFVersionedPackage& ipkg = *pkg;
  if (ipkg.depends == NULL) {
    return;
  }
  bool has_coeff = false;
  for (const CUDFVpkgList *anddep : *(ipkg.depends)) {
    // every _anddep_ is a conjuntion of disjuntions. We will traverse
    // now the disjunctions.
    
    // store if there is a self-dependency
    bool self_depend = false;
    for (const CUDFVpkg *ordep : *anddep) {
      const CUDFVirtualPackage& vpackage = *(ordep->virtual_package);
      a_compptr comp = get_comparator(ordep->op);

      cout << "\t\t" << vpackage.name << " {";
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
              cout << jpkg->name << "," << jpkg->version << " ";
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
            cout << jpkg->name << "," << jpkg->version << " ";
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
                cout << kpkg->name << "," << kpkg->version << " ";
              }
      }
      cout << "}" << endl;
    }
    if (has_coeff) {
      cout << "\t\tA constraint was posted" << endl;
    } else if (!self_depend) {
      cout << "\t\tNo coeff and no self depend " << ipkg.name << "," << ipkg.version << endl;
    }
    cout << endl;
  }
}

void printUniverse(void) {
  // in the future this will offer a package traversal that will post
  // the constraints.
  for (CUDFVersionedPackage *pkg : all_packages) {
    cout << pkg->name << "," << pkg->version << " rank: " << pkg->rank << endl;
    dependencies(pkg);
    // conflicts(pkg);
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

  //printVirtuals();
  //printUniverse();
  handleRequest();
  return 0;
}
