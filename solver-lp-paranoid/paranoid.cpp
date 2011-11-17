#include <cudf/model.hh>
#include <iostream>

using std::cout;
using std::endl;

class CUDFVersionedPackage;

class Paranoid : public CUDFTools::Model {
private:
  /// LP model
  
public:
  // Objects of this class are non-copyable
  Paranoid() = delete;
  Paranoid(const Paranoid&) = delete;
  Paranoid& operator = (const Paranoid&) = delete;
  /// Constructor from a input specification in \a cudf
  Paranoid(const char* cudf)
    : CUDFTools::Model(cudf) {    
    loadUniverse();
    interpretRequest();
  }
  /// Destructor
  virtual ~Paranoid(void) {}
  /// Add a dependency between package \a p on one of the packages in
  /// \a disj
  virtual void depend(CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& disj) {
    cout << name(p) << "*" << version(p)  << endl;
  }
  /// Add a conflict between package \a p and package \a q
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {

  }
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  virtual void keep(int kcst, CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& pkgs) {

  }
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const std::vector<CUDFVersionedPackage*>& disj) {

  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }

  Paranoid model(argv[1]);
  cout << "Constructed model" << endl;
  return 0;
}
