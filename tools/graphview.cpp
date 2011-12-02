#include <iostream>
#include <vector>
#include <unordered_map>

#include <cudf/model.hh>
#include <graph-view/graph-view.hh>

using std::vector;
using std::unordered_map;
using std::cout;
using std::endl;

class GraphReader : public CUDFTools::Model {
private:
  CPRelPkg::Repository rep_;
  unordered_map<int,CPRelPkg::RepoVertex> packages_;
public:
  // Objects of this class are non-copyable
  GraphReader() = delete;
  GraphReader(const GraphReader&) = delete;
  GraphReader& operator = (const GraphReader&) = delete;
  /// Constructor from a input specification in \a cudf
  GraphReader(const char* cudf) 
    : CUDFTools::Model(cudf) {
    
    loadUniverse();
    interpretRequest();
    cout << "Created graph with " << boost::num_vertices(rep_) << endl;
  }
  /// Destructor
  virtual ~GraphReader(void) {}
  /// Add vertex
  void addVertex(CUDFVersionedPackage *p) {
    int r = rank(p);
    if (packages_.count(r) > 0) return;
    auto v  = boost::add_vertex(rep_);
    packages_[r] = v;
    rep_[v].rank = r;
  }
  // Inheritance obligations
  virtual void depend(CUDFVersionedPackage *p, const vector<CUDFVersionedPackage*>& disj) {
    addVertex(p);
    auto vp = rep_[rank(p)];
    for (CUDFVersionedPackage *d : disj) {
      addVertex(d);
      auto vd = rep_[rank(d)];
      auto result = boost::add_edge(vp, vd,rep_);
    }
      
  }
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    
  }
  virtual void keep(int , CUDFVersionedPackage *, const vector<CUDFVersionedPackage*>&) {
    
  }
  virtual void install(const vector<CUDFVersionedPackage*>& disj) {
    
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }
  
  GraphReader model(argv[1]);

  
 
  return 0;
}
