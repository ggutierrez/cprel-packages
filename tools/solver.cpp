#include <iostream>
#include <fstream>
#include <gecode/gist.hh>
#include <solver/solver.hh>
#include <solversupport/reader.hh>
#include <boost/chrono.hpp>
#include <cmath>

namespace bc = boost::chrono;

void search(CPRelPkg::Solver *root) {
  Gecode::DFS<CPRelPkg::Solver> e(root);
  delete root;
  
  int solutionsFound = 0;
  std::cout << "### Search will start" << std::endl;
  while (Gecode::Space* s = e.next()) {
    static_cast<CPRelPkg::Solver*>(s)->print(std::cout);
    solutionsFound++;
    std::cout << "**** End solution ****" << std::endl; 
    delete s;
  }
  std::cout << "### search ends, Total solutions: " << solutionsFound << std::endl; 

}


void searchGist(CPRelPkg::Solver *root) {
  using namespace Gecode;
  Gist::Print<CPRelPkg::Solver> p("Solver");
  Gist::Options o;
  o.inspect.click(&p);
  Gist::dfs(root,o);
  delete root;
}

int main(int argc, char* argv[]) {
  if (argc > 1 && argc < 3 ) {
    const char* kcudf = argv[1];
    std::cout << "Gathering some statistics from: " << kcudf << std::endl;
    std::ifstream is(kcudf);
    if (!is.good()) {
      std::cout << "Error opening the file: " << kcudf << std::endl; 
      return 1;
    }
    bc::system_clock::time_point start = bc::system_clock::now();
    auto problem = CPRelPkg::process(is);
    bc::duration<double> sec = bc::system_clock::now() - start;
    std::cout << "Reading time " << sec.count() << " seconds\n";
    CPRelPkg::Solver* root = new CPRelPkg::Solver();
    //search(root);
    searchGist(root);
  } else {
    std::cerr << "Called with wrong number of arguments" << std::endl; 
    return 1;
  }
  return 0;
}
