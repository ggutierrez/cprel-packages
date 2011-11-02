#include <iostream>
#include <fstream>
#include <kcudf/kcudf.hh>
#include <rel/grelation.hh>
/**
 * \brief Stores the information of a kcudf file into a relation.
 */
class RelationWriter : public KCudfWriter {
public:
  ///Constructor
  RelationWriter(void) {}
  /// Destructor
  virtual ~RelationWriter(void) {}
  /// Callback for a package
  virtual void package(unsigned int id, bool keep, bool install, const char*) {
    //std::cout << "Package: " << id << std::endl; 
  }
  /// Callback for a dependency
  virtual void dependency(unsigned int package, unsigned int depends, const char*) {
    //std::cout << "Dependency " << package << " on " << depends << std::endl; 
  }
  /// Callback for a dependency
  virtual void conflict(unsigned int package, unsigned int depends, const char*) {
    //std::cout << "Conflict " << package << " on " << depends << std::endl; 
  }
  /// Callback for a dependency
  virtual void provides(unsigned int package, unsigned int depends, const char*) {
    //std::cout << "Provides " << package << " on " << depends << std::endl; 
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
