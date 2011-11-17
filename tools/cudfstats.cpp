#include <iostream>
#include <cudf/model.hh>

using namespace std;
/*
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
*/
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
  CUDFTools::Model m(argv[1]);
  return 0;
}
