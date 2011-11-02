#include <iostream>
#include <fstream>
#include <tuple>
#include <kcudf/kcudf.hh>
#include <rel/grelation.hh>
#include <bdddomain/manager.hh>

/// A problem is a tuple of ground relations: <Packages,Dependencies,Conflicts,Provides>
typedef std::tuple<MPG::GRelation,MPG::GRelation,MPG::GRelation,MPG::GRelation>
ProblemDesc;

/**
 * \brief Stores the information of a kcudf file into a relation.
 */
class RelationWriter : public KCudfWriter {
private:
  /// The relations when the contents are read
  MPG::GRelation packages_;
  MPG::GRelation dependencies_;
  MPG::GRelation conflicts_;
  MPG::GRelation provides_;
public:
  ///Constructor
  RelationWriter(void)
    : packages_(3), dependencies_(2), conflicts_(2), provides_(2) {}
  /// Destructor
  virtual ~RelationWriter(void) {}
  /// Callback for a package
  virtual void package(unsigned int id, bool keep, bool install, const char*) {
    int k = keep ? 1 : 0;
    int i = install ? 1 : 0;
    MPG::Tuple t({id,k,i});
    packages_.add(t);
  }
  /// Callback for a dependency
  virtual void dependency(unsigned int package, unsigned int depends, const char*) {
    MPG::Tuple t({(int)package,(int)depends});
    dependencies_.add(t);
  }
  /// Callback for a dependency
  virtual void conflict(unsigned int package, unsigned int conflict, const char*) {
    MPG::Tuple t({(int)package, (int)conflict});
    MPG::Tuple t2({(int)conflict, (int)package});
    conflicts_.add(t);
    conflicts_.add(t2);
  }
  /// Callback for a dependency
  virtual void provides(unsigned int package, unsigned int provides, const char*) {
    MPG::Tuple t({(int)package,(int)provides});
    provides_.add(t);
  }
  /// Returns a tuple with the relations
  ProblemDesc problem(void) {
    return ProblemDesc(packages_,dependencies_,conflicts_,provides_);
  }
};

ProblemDesc process(std::istream& kcudf) {
  std::cout << "Transforming problem into relations" << std::endl; 
  RelationWriter rw;
  read(kcudf,rw);
  return rw.problem();
}

void bddStats(const ProblemDesc& problem) {
  std::cout << "Problem statistics: " << std::endl
	    << "\tPackages: " << std::get<0>(problem).cardinality() << std::endl
	    << "\tDependencies: " << std::get<1>(problem).cardinality() << std::endl
	    << "\tconflicts: " << std::get<2>(problem).cardinality() / 2 << std::endl
	    << "\tProvides: " << std::get<3>(problem).cardinality() << std::endl;
  
  auto& factory = MPG::VarImpl::factory();
 
  std::cout << "BDD statistics:" << std::endl
	    << "\tNodes used: " << factory.ReadNodeCount()
	    << "\tReordering time: " << factory.ReadReorderingTime() << std::endl;

  std::cout << "Will start reordering: " << std::endl; 
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
    auto problem = process(is);
    bddStats(problem);
    
  } else {
    std::cerr << "Called with wrong number of arguments" << std::endl; 
    return 1;
  }
  return 0;
}
