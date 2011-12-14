#include <iostream>
#include <solver-ap/impact.hh>

using std::cout;
using std::endl;
using namespace CUDFTools;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }
  
  ImpactGraph model(argv[1]);
  //cout << "Relations: " << model.representedRelations() << endl;
  model.generateSubproblems();
  //model.subproblemHierarchy(cout);
 
  return 0;
}
