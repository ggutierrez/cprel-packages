#include <vector>
#include <fstream>
#include <cudf/model.hh>
#include <libgexf/libgexf.h>

using std::vector;
using std::cout;
using std::endl;

class Visualizer : public CUDFTools::Model {
private:

public:
    // Objects of this class are non-copyable
  Visualizer() = delete;
  Visualizer(const Visualizer&) = delete;
  Visualizer& operator = (const Visualizer&) = delete;
  /// Constructor from a input specification in \a cudf
  Visualizer(const char* cudf) 
  : CUDFTools::Model(cudf) {}
  /// Destructor
  virtual ~Visualizer(void) {}
  /// Add a dependency between package \a p on one of the packages in
  /// \a disj
  virtual void depend(CUDFVersionedPackage *p, const vector<CUDFVersionedPackage*>& disj) {
    
  }
  /// Add a conflict between package \a p and package \a q
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    
  }
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  virtual void keep(int , CUDFVersionedPackage *, const vector<CUDFVersionedPackage*>&) {
    
  }
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const vector<CUDFVersionedPackage*>& disj){
    
  }
};


void create() {
  
  libgexf::GEXF *gexf = new libgexf::GEXF();

  libgexf::DirectedGraph& graph = gexf->getDirectedGraph();

  // nodes
  graph.addNode("0");
  graph.addNode("1");

  // edges
  graph.addEdge("0", "0", "1");

  // node labels
  libgexf::Data& data = gexf->getData();
  data.setNodeLabel("0", "Hello");
  data.setNodeLabel("1", "world");

  // attributes
  data.addNodeAttributeColumn("0", "foo", "boolean");
  data.setNodeAttributeDefault("0", "false");
  data.setNodeValue("1", "0", "true");

  // meta data
  libgexf::MetaData& meta = gexf->getMetaData();
  meta.setCreator("The Hitchhiker's Guide to the Galaxy");
  meta.setDescription("The answer is 42.");

  // write gexf file
  libgexf::FileWriter *writer = new libgexf::FileWriter();
  writer->init("life.gexf", gexf);
  writer->write();
}


int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }
  
  // load the solution file
  std::fstream sol(argv[2]);
  if (!sol.good()) {
    cout << "Unable to open solution file" << endl;
    exit(1);
  }
 
  Visualizer model(argv[1]);
 /*  
 model.debugPackageRanks(cout);

  vector<int> s = Visualizer::readSolution(sol);

  // cout << "Solution: " << endl;
  // for (int p : s) {
  //   cout << "Package installed: " << p << endl;
  // }

  //model.postSolution(s);
  //model.problemInfo();
  //  model.solverInit();
  model.solverInit(s);
  model.solve(1);
  */
  return 0;
}

