#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <cudf/virtual_model.hh>
#include <libgexf/libgexf.h>

using std::vector;
using std::cout;
using std::endl;
using std::unique_ptr;
using std::string;



//class Visualizer : public CUDFTools::Model {
class Visualizer : public CUDFTools::GraphModel {
private:
  /// Pointer to the gexf object
  unique_ptr<libgexf::GEXF> gexf_;
  /// The graph
  libgexf::DirectedGraph &graph_;
  /// The data
  libgexf::Data &data_;
  /// Transform an integer into an string
  string label(int p) const {
    std::stringstream ss;
    ss << p;
    return ss.str();
  }
  /// Adds the edge (\a source, \a target) to the graph
  void addEdge(int source, int  target, const char *relation = "error") {
    string sid = label(source);
    string tid = label(target);
    if (!graph_.containsEdge(sid,tid)) {
      string edge = edgeId(source,target);
      graph_.addEdge(edge,sid,tid);
      data_.setEdgeValue(edge,"0",relation);
    }
    
  }
  void addNode(int n, const char *name = "none") {
    string id  = label(n);
    graph_.addNode(id);
    data_.setNodeLabel(id,name);
  }
public:
  // Objects of this class are non-copyable
  Visualizer() = delete;
  Visualizer(const Visualizer&) = delete;
  Visualizer& operator = (const Visualizer&) = delete;
  /// Constructor from a input specification in \a cudf
  Visualizer(const char* cudf) 
    : GraphModel(cudf)
    , gexf_(new libgexf::GEXF())
    , graph_(gexf_->getDirectedGraph())
    , data_(gexf_->getData())
  {
    // set up some of the attributes we will store on nodes and edges
    /*
      data_.addNodeAttributeColumn("0", "Reported Installed?", "boolean");
      data_.setNodeAttributeDefault("0", "false");
      data_.addNodeAttributeColumn("1", "Involved in Request?", "boolean");
      data_.setNodeAttributeDefault("1", "false");
    */
    data_.addEdgeAttributeColumn("0", "Relation", "string");
    data_.setEdgeAttributeDefault("0", "None");
    // The call to these methods will make the interpretation of the
    // constraints. This will indirectly make the methods that we
    // override to be called.
    loadUniverse();
    interpretRequest();
  }
  /// Destructor
  virtual ~Visualizer(void) {}
  ///  Write graph to \a fname
  void writeGraph() const {
    libgexf::FileWriter writer;
    writer.init("life.gexf", gexf_.get());
    writer.write();
  }
  /** 
   * \brief Add a dependency between package \a p on one of the
   * packages in \a disj
   */
  virtual void depend(CUDFVersionedPackage *p, const vector<CUDFVersionedPackage*>& disj) {
    addNode(rank(p),versionedName(p));
    if (disj.size() == 1) {
      CUDFVersionedPackage *d = disj.at(0);
      addNode(rank(d),versionedName(d));
      addEdge(rank(p),rank(d),"D");
      return;
    }

    string disjName = name(disj);
    int disjId = lookUpOrAdd(disj);
    addNode(disjId,name(disj).c_str());
    addEdge(rank(p),disjId, "DV");
    for (auto *d : disj) {
      addNode(rank(d),versionedName(d));
      addEdge(rank(d),disjId,"PV");
      provides(d,disjId);
    }
  }
  /**
   * \brief Called when a package \a p is found to be a provider of
   * virtual package \a v.
   *
   */
  void provides(CUDFVersionedPackage *p, int v) {
    
  }
  
  /// Add a conflict between package \a p and package \a q
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    addNode(rank(p),versionedName(p));
    addNode(rank(q),versionedName(q));
    addEdge(rank(p),rank(q),"C");
  }
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  virtual void keep(int , CUDFVersionedPackage *, const vector<CUDFVersionedPackage*>&) {
    
  }
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const vector<CUDFVersionedPackage*>& disj) {

  }
};

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
  model.writeGraph();
  return 0;
}

