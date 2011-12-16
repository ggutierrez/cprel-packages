#include <map>
#include <iostream>
#include <vector>
#include <iterator>
#include <string>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/iterator/filter_iterator.hpp>

// graph algorithms
#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>

#include <cudf/virtual_model.hh>

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;
using std::ostream;

using namespace boost;

class ImpactGraph : public CUDFTools::GraphModel {
private:
  /// The graph type
  //typedef subgraph < adjacency_list <vecS, vecS, undirectedS> > Graph;

  typedef adjacency_list_traits<vecS, vecS, directedS> Traits;
  typedef subgraph< 
    adjacency_list<vecS, vecS, directedS,
                   property<vertex_color_t, int>, 
                   property<edge_index_t, int> > 
                   > Graph;
  
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
  /// Type for a vertex in the graph
  typedef graph_traits <Graph>::vertex_descriptor Vertex;
public:
  // Prevent default construction
  ImpactGraph(void) = delete;
  // Prevent copy
  ImpactGraph(const ImpactGraph&) = delete;
  // Prevent assignment
  ImpactGraph& operator = (const ImpactGraph&) = delete;
  /// Constructor from a cudf specification
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
  int representedRelations(void) const {
    return num_edges(g_);
  }
  /**
   * \brief Create a graph for a sub-problem \a s
   *
   * A sub-problem is represented by the set of vertices in the
   * original problem graph.
   */
  void generateSubproblems(void) {
    int interesting = 0;
    auto edgesStop = [&](const Graph& g) -> bool {
      int edges =  num_edges(g);
      if (num_vertices(g) == 1)
        interesting++;
      return edges < 10;
    };
    partition(g_,0,edgesStop);
    cout << "Interesting count " << interesting << endl;
  }
  static int hierarchyHelper(ostream& os, const Graph& g, int parent, int& leaves) {
    assert(!g.is_root());
    int children = parent + 1;
    auto c = g.children();
    int numChildren = distance(c.first,c.second);
    if (numChildren == 0) {
      leaves++;
    }
    if (numChildren == 1) {
      os << "\t" << parent << " -- " 
         << children << "; - ap break" << endl;
      children = hierarchyHelper(os, *(c.first), children, leaves);
      return children;
    }
    for (; c.first != c.second; ++c.first) {
      os << "\t" << parent << " -- " << children << ";" << endl;
      children = hierarchyHelper(os, *(c.first), children, leaves);
    }
    return children;
  }
  /**
   * \brief Prints the problem hierarchy to \a os as a tree in dot
   * format
   */
  void subproblemHierarchy(ostream& os) const {
    assert(g_.is_root());
    int leaves = 0;
    os << "graph G {" << endl;
    hierarchyHelper(os,g_,0,leaves);
    os << "}" << endl;
    cout << "Subproblems: " << leaves << endl;
  }
  static vector<Vertex> select(const Graph& g, const vector<Vertex>& aps) {
    vector<Vertex> selectedVertices;
    (void)aps;
    selectedVertices.reserve(num_vertices(g));
    auto v = vertices(g);
    std::copy(v.first,v.second,std::back_inserter(selectedVertices));
    selectedVertices.pop_back();
    return selectedVertices;
  }

  template <typename StopFunctor>
  static void partition(Graph& g, int level, StopFunctor stop) {
    if (stop(g)) {
      // At this point the graph is consider to represent a reasonable
      // sub-problem.
   
      return;
    } else {
      auto subproblems = components(g);
      if (subproblems.size() == 1) {
        auto ap = articulationPoints(g);
        auto s = select(g,ap);
        Graph& sg = g.create_subgraph(s.begin(), s.end());
        partition(sg,level+1,stop);
      } else {
        for (auto s : subproblems) {
          Graph& sg = 
            g.create_subgraph(s.second.begin(),s.second.end());
          partition(sg,level+1,stop);
        }
      }
    }
  }
  
  static map<int,vector<int>> components(Graph& g) {
    vector<int> component(num_vertices(g));
    connected_components(g, &component[0]);

    // process the information obtained by the algorithm.
    map<int,vector<int>> subproblems; // map<component,vector<nodes_in_component>
    for (unsigned int i = 0; i != component.size(); ++i)
      subproblems[component.at(i)].push_back(i);
    return subproblems;
  }

  /// Articulation points
  static vector<Vertex> articulationPoints(Graph& g) {    
    std::vector<Vertex> artPoints;
    articulation_points(g, std::back_inserter(artPoints));
    return artPoints;
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Invalid number of arguments" << endl;
    exit(1);
  }
  
  ImpactGraph model(argv[1]);
  cout << "Relations: " << model.representedRelations() << endl;
  model.generateSubproblems();
  //model.subproblemHierarchy(cout);
  return 0;
}
