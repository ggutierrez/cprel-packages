#include <map>
#include <iostream>
#include <vector>
#include <string>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>

// graph algorithms
#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>

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
  //typedef subgraph < adjacency_list <vecS, vecS, undirectedS> > Graph;

  typedef adjacency_list_traits<vecS, vecS, directedS> Traits;
  typedef subgraph< 
    adjacency_list<vecS, vecS, directedS,
                   property<vertex_color_t, int>, property<edge_index_t, int> > > Graph;
  
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
    partition(g_,0);
  }
  static Vertex chooseAP(const Graph& g, const vector<Vertex>& aps) {
    return aps.at(0);
  }
  static void removeAP(Graph& g, Vertex ap) {
    clear_vertex(ap,g);
  }
  static void partition(Graph& g, int level) {
    if (num_vertices(g) < 10) {
      //cout << "Small enough" << endl;
      return;
    } else {
      string out(level,'-');
      auto subproblems = components(g);
      if (subproblems.size() == 1) {
        auto ap = articulationPoints(g);
        
        cout << out << "> (" 
             << num_vertices(g) << " * " 
             << num_edges(g) << " :ap: " 
             << ap.size() << ")" << endl;
        auto v = chooseAP(g,ap);
        removeAP(g,v);
        partition(g,level+1);
      } else {
        for (auto s : subproblems) {
          Graph& sg = 
            g.create_subgraph(s.second.begin(),s.second.end());
          if (num_edges(sg) > 2)
            cout << out << "> (" 
                 << num_vertices(sg) << " * " 
                 << num_edges(sg) << ")" << endl;
          
          partition(sg,level+1);
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
    //std::cerr << "Found " << artPoints.size() << " articulation points.\n";
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
  //model.connectedComponents();
  model.generateSubproblems();
  //model.articulationPoints();
  return 0;
}
