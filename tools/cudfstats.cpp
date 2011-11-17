#include <iostream>
#include <cudf/model.hh>

using namespace std;
int main(int argc, char* argv[]) {
  if (argc != 2) {
    cerr << "Error, no input provided" << endl;
    return 1;
  }
  CUDFTools::Model m(argv[1]);
  return 0;
}
