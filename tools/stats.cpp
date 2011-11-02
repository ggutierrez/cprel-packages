#include <iostream>
#include <fstream>
#include <kcudf/kcudf.hh>
#include <rel/grelation.hh>
/**
 * \brief Stores the information of a kcudf file into a relation.
 */
class RelationWriter : public KCudfWriter {
public:
  /// The relations when the contents are read
  MPG::GRelation packages_;
  MPG::GRelation dependencies_;
  MPG::GRelation conflicts_;
  MPG::GRelation provides_;
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
};

void process(std::istream& kcudf, const char* fname) {
  std::cout << "Doing something" << std::endl; 
  // Read the specification into a relation
  RelationWriter rw;
  read(kcudf,rw);
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
    process(is,kcudf);
  } else {
    std::cerr << "Called with wrong number of arguments" << std::endl; 
    return 1;
  }
  return 0;
}
