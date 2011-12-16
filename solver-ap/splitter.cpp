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
  cout << "Relations: " << model.representedRelations() << endl;
  cout << "Packages: " << model.representedPackages() << endl;
  model.outputProblem(cout);
  
  model.generateSubproblems();
  /* 
  for (int i = 0; i < model.subproblemsCount(); i++) {
    cout << "New subproblem" << endl;
    model.outputSubproblem(i,cout);
  }
  */
  model.outputSubproblemTree(cout);
  cout << "Subproblems: " << model.subproblemsCount() << endl;

 
  return 0;
}
