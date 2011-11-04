#include <iostream>
#include <fstream>
#include <tuple>
#include <boost/chrono.hpp>
#include <cmath>
#include <solversupport/reader.hh>
#include <solversupport/pkg-builtins.hh>
#include <bdddomain/manager.hh>

namespace bc = boost::chrono;

MPG::GRelation oneProvider(const CPRelPkg::ProblemDesc& problem) {
  // providers is the 4-th term of the problem description
  const MPG::GRelation& providers = std::get<3>(problem);
  MPG::GRelation oneProv(providers);
  auto u = oneProv.unique(0).shiftRight(1);
  /*
  std::cout // << "One provider " << u << std::endl
	    << "\tnumber: " << u.cardinality() << std::endl
	    << "\tout of " << providers.cardinality() << std::endl;  
  */
  return u;
}

MPG::GRelation concretes(const CPRelPkg::ProblemDesc& problem) {
  return std::get<4>(problem);
}


void problemStats(const CPRelPkg::ProblemDesc& problem) {
  std::cout << "Problem statistics: " << std::endl
	    << "\tPackages: " << std::get<0>(problem).cardinality() << std::endl
	    << "\tDependencies: " << std::get<1>(problem).cardinality() << std::endl
	    << "\tconflicts: " << std::get<2>(problem).cardinality() / 2 << std::endl
	    << "\tProvides: " << std::get<3>(problem).cardinality() << std::endl;
  //oneProvider(problem);
  //filterPackages(problem,0,1);
  //std::cout << "Concrete packages: " << concretes(problem).cardinality() << std::endl; 
  //auto sProvides = CPRelPkg::isStableProvides(problem);
  //std::cout << "Stable: " << sProvides << std::endl; 
}

void bddStats(const CPRelPkg::ProblemDesc& problem) {
  
  auto& factory = MPG::VarImpl::factory();
 
  std::cout << "BDD statistics:" << std::endl
	    << "\tNodes used: " << factory.ReadNodeCount()
	    << "\tReordering time: " << factory.ReadReorderingTime() << std::endl;

  //std::cout << "Will start reordering: " << std::endl; 
  //factory.ReduceHeap(CUDD_REORDER_ANNEALING,1000);
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
   
    bddStats(problem);
    problemStats(problem);
  } else {
    std::cerr << "Called with wrong number of arguments" << std::endl; 
    return 1;
  }
  return 0;
}
