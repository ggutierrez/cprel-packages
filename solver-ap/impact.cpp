#include <iostream>
// graph algorithms
#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <solver-ap/impact.hh>

using namespace std;

namespace CUDFTools {

  void ImpactGraph::addEdge(int source, int  target, const char *) {
    add_edge(source, target, g_);
  }

  void ImpactGraph::addNode(int, const char *) {
    
  }

  ImpactGraph::ImpactGraph(const char* cudf) 
    : GraphModel(cudf)
      //    , g_(packages().size())
  {
    loadUniverse();
    interpretRequest();
  }

  ImpactGraph::~ImpactGraph(void) {}

  void ImpactGraph::depend(CUDFVersionedPackage *p, const vector<CUDFVersionedPackage*>& disj) {
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
      //provides(d,disjId);
    }
  }

  void ImpactGraph::conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q) {
    addNode(rank(p),versionedName(p));
    addNode(rank(q),versionedName(q));
    addEdge(rank(p),rank(q),"C");
  }
 
  void ImpactGraph::keep(int , CUDFVersionedPackage *, const vector<CUDFVersionedPackage*>&) {
    
  }
  
  void ImpactGraph::install(const vector<CUDFVersionedPackage*>& disj) {

    string disjName = name(disj);
    //cout << "Requested: " << disjName << endl;
    int disjId = lookUpOrAdd(disj);
    addNode(disjId,name(disj).c_str());
    
    request_.push_back(disjId);
    
    for (auto *d : disj) {
      addNode(rank(d),versionedName(d));
      addEdge(rank(d),disjId,"PV");
    }
  }

  int ImpactGraph::representedRelations(void) const {
    return num_edges(g_);
  }

  
  
  // helper functions
  namespace internal {
    map<int,vector<int>> components(ImpactGraphType& g) {
      vector<int> component(num_vertices(g));
      connected_components(g, &component[0]);
      
      // process the information obtained by the algorithm.
      map<int,vector<int>> subproblems; // map<component,vector<nodes_in_component>
      for (unsigned int i = 0; i != component.size(); ++i)
        subproblems[component.at(i)].push_back(i);
      return subproblems;
    }
    
    /// Articulation points
    vector<Vertex> articulationPoints(ImpactGraphType& g) {    
      std::vector<Vertex> artPoints;
      articulation_points(g, std::back_inserter(artPoints));
      return artPoints;
    }

    vector<Vertex> select(const ImpactGraphType& g, const vector<Vertex>& aps) {
      vector<Vertex> selectedVertices;
      (void)aps;
      selectedVertices.reserve(num_vertices(g));
      auto v = vertices(g);
      std::copy(v.first,v.second,std::back_inserter(selectedVertices));
      selectedVertices.pop_back();
      return selectedVertices;
    }

    template <typename StopFunctor>
    void partition(ImpactGraphType& g, int level, StopFunctor stop) {
      if (stop(g)) {
        // At this point the graph is consider to represent a reasonable
        // sub-problem.
        return;
      } else {
        auto subproblems = components(g);
        if (subproblems.size() == 1) {
          auto ap = articulationPoints(g);
          auto s = select(g,ap);
          auto& sg = g.create_subgraph(s.begin(), s.end());
          partition(sg,level+1,stop);
        } else {
          for (auto s : subproblems) {
            auto& sg = 
              g.create_subgraph(s.second.begin(),s.second.end());
            partition(sg,level+1,stop);
          }
        }
      }
    }
  }

  void ImpactGraph::generateSubproblems(void) {
    int interesting = 0;
    auto edgesStop = [&](const ImpactGraphType& g) -> bool {
      int edges =  num_edges(g);
      if (num_vertices(g) == 1)
        interesting++;
      return edges < 10;
    };
    internal::partition(g_,0,edgesStop);
    cout << "Interesting count " << interesting << endl;

  }
}
