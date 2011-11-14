#include <iostream>
#include <cudf/cudf.hh>

using namespace std;
void printUniverse(void) {
  cout << "Packages in the universe" << endl;
  for (CUDFVersionedPackage *pkg : all_packages) {
    cout << pkg->name << "," << pkg->version << " rank: " << pkg->rank << endl;
  }
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

  printVirtuals();
  printUniverse();
  return 0;
}
