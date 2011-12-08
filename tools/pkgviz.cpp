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
  /// Return a name for \a disj
  std::string name(const vector<CUDFVersionedPackage*>& disj) {
    // the name to be returned is based in the names of the packages
    // involved in the disjunction, so we first extract the names
    std::vector<std::string> names;
    names.reserve(disj.size());
    for (auto *p : disj)
      names.push_back(versionedName(p));
    // to make the disjunction unique we sort
    std::sort(begin(names), end(names));
    
    std::stringstream ss;
    for (auto &p : names)
      ss << p << "|";
    return ss.str();
  }
  /// Returns the package representing disjunction \a disj_
  int lookUpOrAdd(const vector<CUDFVersionedPackage*>& disj) {
    string key = name(disj);
    auto e = virtuals_.find(key);
    if (e == virtuals_.end()) {
      int newVirtual = packages().size() + virtuals_.size();
      virtuals_[key] = newVirtual;
      return newVirtual;
    }
    return e->second;
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
  /// Return the number of virtual packages that were processed
  int virtualPackages(void) const {
    return virtuals_.size();
  }
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
  /// Returns the identifier for an edge from \a source to \a target
  static string edgeId(int source, int target) {
    std::stringstream edgeId;
    edgeId << source << " -- " << target;
    return edgeId.str();
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
  //    : CUDFTools::Model(cudf)
    : ModelVirtuals(cudf)
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
    }
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

