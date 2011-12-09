#include <map>
#include <iostream>
#include <vector>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include <cudf/virtual_model.hh>

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;

using namespace boost;

class ImpactGraph : public CUDFTools::GraphModel {
private:
  /// The graph type
  typedef adjacency_list <vecS, vecS, undirectedS> Graph;
  /// The graph
  Graph g_;
  /// The nodes in the graph that are created because of the request
  vector<int> request_;
  /// Adds the edge (\a source, \a target) to the graph
  void addEdge(int source, int  target, const char *relation = "error") {
    add_edge(source, target, g_);
  }
  void addNode(int n, const char *name = "none") {
    
  }
public:
  ImpactGraph(void) = delete;
  ImpactGraph(const ImpactGraph&) = delete;
  ImpactGraph& operator = (const ImpactGraph&) = delete;
  /// constructor from a cudf specification
  ImpactGraph(const char* cudf) 
    : GraphModel(cudf)
      //    , g_(packages().size())
  {
    loadUniverse();
    interpretRequest();
  }
  /// Destructor
  virtual ~ImpactGraph(void) {}
  /// Obligations
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

    string disjName = name(disj);
    cout << "Requested: " << disjName << endl;
    int disjId = lookUpOrAdd(disj);
    addNode(disjId,name(disj).c_str());

    request_.push_back(disjId);

    for (auto *d : disj) {
      addNode(rank(d),versionedName(d));
      addEdge(rank(d),disjId,"PV");
    }
  }

  /// Connected components
  void connectedComponents(void) const {
    vector<int> component(num_vertices(g_));
    int num = connected_components(g_, &component[0]);

    // In which components are the nodes in the request?
    for (int n : request_)
      cout << "Node: " << n << " in " << component.at(n) << endl;
    // process the information obtained by the algorithm.
    map<int,vector<int>> subproblems; // map<component,vector<nodes_in_component>
    for (unsigned int i = 0; i != component.size(); ++i)
      subproblems[component.at(i)].push_back(i);
    /*
    cout << "Component " << ", " << "Size" << endl; 
    for (auto& r : subproblems) {
      cout << r.first << ", " << r.second.size() << endl; 
    }
    */
    //vector<int>::size_type i;
    cout << "Total number of components: " << num << endl;
    /*
    for (i = 0; i != component.size(); ++i)
      cout << "Vertex " << i <<" is in component " << component[i] << endl;
    cout << endl;
    */
    cout << "Number packages: " << packages().size() << endl;
    cout << "Number of virtual packages: " << virtualPackages() << endl;
    cout << "Number of edges in the graph: " << num_edges(g_) << endl;
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }
  
  ImpactGraph model(argv[1]);
  model.connectedComponents();
  return 0;
}
