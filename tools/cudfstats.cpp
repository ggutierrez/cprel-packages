#include <iostream>
#include <cudf/cudf.hh>

using namespace std;

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
  case 1: break;
  case 2: break;
  }
  fclose(input_file);
  cout << "CUDF statistics" << endl;
  return 0;
}
