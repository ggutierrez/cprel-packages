#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <sstream>
#include <cudf/model.hh>
#include <libgexf/libgexf.h>

using std::vector;
using std::cout;
using std::endl;
using std::unique_ptr;
using std::string;

class Visualizer : public CUDFTools::Model {
private:
  /// Pointer to the gexf object
  unique_ptr<libgexf::GEXF> gexf_;
  /// The graph
  libgexf::DirectedGraph &graph_;
  /// The data
  libgexf::Data &data_;
  /// Transform an integer into an string
  string label(CUDFVersionedPackage *p) const {
    std::stringstream ss;
    ss << rank(p);
    return ss.str();
  }
  /// Returns the identifier for package \a p
  int toPackageId(CUDFVersionedPackage *p) const  {
    return rank(p);
  }
  /// Converts a disjunction of packages into packages ids using the rank
  vector<int> toPackageIds(const vector<CUDFVersionedPackage*>& disj) {
    vector<int> r;
    r.reserve(disj.size()); 
    for (CUDFVersionedPackage *p : disj)
      r.push_back(toPackageId(p));
    return r;
  }
  /// Returns the identifier for an edge from \a source to \a target
  static string edgeId(const string& source, const string& target) {
    std::stringstream edgeId;
    edgeId << source << " -- " << target;
    return edgeId.str();
  }
  /// Adds the edge (\a source, \a target) to the graph
  void addEdge(CUDFVersionedPackage *p, CUDFVersionedPackage *q, const char* relation = "NONE") {
    string source = label(p), target = label(q);
    if (!graph_.containsEdge(source,target)) {
      string edge = edgeId(source,target);
      graph_.addEdge(edge,source,target);
      data_.setEdgeValue(edge, "0", relation);
    }
  }
  /// Adds the node \a n to the graph
  void addNode(CUDFVersionedPackage *p) {
    string node = label(p);
    graph_.addNode(node);
    data_.setNodeLabel(node,versionedName(p));
    data_.setNodeValue(node,"0", foundInstalled(p) ? "true" : "false");
  }
  /// Adds information on the graph that the package is part of the request
  void requested(CUDFVersionedPackage *p) {
    string node = label(p);
    data_.setNodeValue(node, "1", "true");
  }
public:
    // Objects of this class are non-copyable
  Visualizer() = delete;
  Visualizer(const Visualizer&) = delete;
  Visualizer& operator = (const Visualizer&) = delete;
  /// Constructor from a input specification in \a cudf
  Visualizer(const char* cudf) 
    : CUDFTools::Model(cudf)
    , gexf_(new libgexf::GEXF())
    , graph_(gexf_->getDirectedGraph())
    , data_(gexf_->getData())
  {
    // set up some of the attributes we will store on nodes and edges
    data_.addNodeAttributeColumn("0", "Reported Installed?", "boolean");
    data_.setNodeAttributeDefault("0", "false");
    data_.addNodeAttributeColumn("1", "Involved in Request?", "boolean");
    data_.setNodeAttributeDefault("1", "false");
    data_.addEdgeAttributeColumn("0", "Relation", "string");
    data_.setEdgeAttributeDefault("0", "N");

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
    addNode(p);
    for (CUDFVersionedPackage *d : disj) {
      addNode(d);
      addEdge(p,d,"d");
    }
  }
  /// Add a conflict between package \a p and package \a q
  virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    addNode(p);
    addNode(q);
    addEdge(p,q, "c");
  }
  /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
  virtual void keep(int , CUDFVersionedPackage *, const vector<CUDFVersionedPackage*>&) {
    
  }
  /// Handle the installation of one of the packages in \a disj
  virtual void install(const vector<CUDFVersionedPackage*>& disj) {
    for (CUDFVersionedPackage *p : disj)
      requested(p);
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
  model.writeGraph();
  return 0;
}

