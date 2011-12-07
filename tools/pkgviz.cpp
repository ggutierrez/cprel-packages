#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <cudf/model.hh>
#include <libgexf/libgexf.h>

using std::vector;
using std::cout;
using std::endl;
using std::unique_ptr;
using std::string;


class ModelVirtuals : public CUDFTools::Model {
private:
  /// Data structure to associates disjunctions and virtual packages
  std::unordered_map<std::string,int> virtuals_;
protected:
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
  /// Sort a vector of integers
  void makeCanonic(vector<int>& disj) {
    std::sort(std::begin(disj),std::end(disj));
  }
  /// Creates a key for \a disj
  string makeKey(vector<int>& disj) {
    makeCanonic(disj);
    string key;
    std::stringstream ss(key);
    for (int p : disj)
      ss << p;
    return ss.str();
  }
  /// Returns the package representing disjunction \a disj_
  int lookUpOrAdd(const vector<CUDFVersionedPackage*>& disj_) {
    vector<int> disj = toPackageIds(disj_);
    string key(makeKey(disj));
    auto f = virtuals_.find(key);
    if (f != virtuals_.end()) {
      // a disjunction like this already exists.
      return f->second;
    }
    
    int vp = virtuals_.size();
    virtuals_[key] = vp;
    return vp;
  }
public:
  // Objects of this class are non-copyable
  ModelVirtuals() = delete;
  ModelVirtuals(const ModelVirtuals&) = delete;
  ModelVirtuals& operator = (const ModelVirtuals&) = delete;
  /// Constructor from a input specification in \a cudf
  ModelVirtuals(const char* cudf) 
    : CUDFTools::Model(cudf)
  {}
};

//class Visualizer : public CUDFTools::Model {
class Visualizer : public ModelVirtuals {
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
  /// Transform a package into an string (using the rank)
  string label(CUDFVersionedPackage *p) const {
    std::stringstream ss;
    ss << rank(p);
    return ss.str();
  }
  /// Returns the identifier for an edge from \a source to \a target
  static string edgeId(const string& source, const string& target) {
    std::stringstream edgeId;
    edgeId << source << " -- " << target;
    return edgeId.str();
  }
  /// Adds the edge (\a source, \a target) to the graph
  void addEdge(const string& source, const string&  target, const char* relation = "NONE") {
    if (!graph_.containsEdge(source,target)) {
      string edge = edgeId(source,target);
      graph_.addEdge(edge,source,target);
      data_.setEdgeValue(edge, "0", relation);
    }
  }
  /// Adds the edge (\a source, \a target) to the graph
  void addEdge(CUDFVersionedPackage *p, CUDFVersionedPackage *q, const char* relation = "NONE") {
    string source = label(p), target = label(q);
    addEdge(source,target,relation);
  }
  /// Adds the node \a n to the graph
  void addNode(const string&  n, const string& desc, const string& value) {
    graph_.addNode(n);
    data_.setNodeLabel(n,desc);
    data_.setNodeValue(n,"0",value);
  }
  /// Adds the node \a n to the graph
  void addNode(CUDFVersionedPackage *p) {
    string node = label(p);
    string desc = versionedName(p);
    string value = foundInstalled(p) ? "true" : "false";
    addNode(node,desc,value);
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
  //    : CUDFTools::Model(cudf)
    : ModelVirtuals(cudf)
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
    if (disj.size() == 1) {
      addNode(disj.at(0));
      addEdge(p,disj.at(0),"d");
    } else {
      
      int v = lookUpOrAdd(disj);
      addVirtualDependency(p,v);
    }
  }
  void addVirtualDependency(CUDFVersionedPackage *p, int vtual) {
    string target = label(vtual);
    string source = label(p);
    // this should be true if at least one package in the disjunction
    // is installed
    addNode(target,"virtual","false");
    addEdge(source,target, "p");
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
    /*
    for (CUDFVersionedPackage *p : disj)
      requested(p);
    */
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

