#include <iostream>
#include <string>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/graph/graphviz.hpp>
// graph algorithms
#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <solver-ap/impact.hh>

using namespace std;

namespace CUDFTools {

  void toDot(ImpactGraphType& g, ostream& os) {
    boost::dynamic_properties dp;
    dp.property("node_id", get(&ImpactGraphVertexData::id, g));
    dp.property("label", get(&ImpactGraphVertexData::name, g));
    dp.property("label", get(&ImpactGraphEdgeData::name, g));
    write_graphviz_dp(os, g, dp);
  }

  void ImpactGraph::addEdge(int source, int  target, const char *str) {
    auto src = nodeToVertex.find(source);
    assert(src != end(nodeToVertex));
    
    auto tgt = nodeToVertex.find(target);
    assert(tgt != end(nodeToVertex));

    //g_[target].id = target;
    auto e = add_edge(src->second, tgt->second, g_);
    g_[e.first].name = string(str);
  }

  void ImpactGraph::addNode(int id, const char *name) {
    if (nodeToVertex.find(id) == end(nodeToVertex)) {
      auto n = add_vertex(g_);
      g_[n].id = id;
      g_[n].name = string(name);
      nodeToVertex[id] = n;
    }
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
    cout << "Creating disjunction " << disjName << endl;
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
    if (disj.size() == 1) {
      auto* pkg = disj[0];
      addNode(rank(pkg),versionedName(pkg));
      cout << "Added install of CP " << versionedName(pkg) << endl;
    } else {
      string disjName = name(disj);
      //cout << "Requested: " << disjName << endl;
      int disjId = lookUpOrAdd(disj);
      addNode(disjId,name(disj).c_str());
      
      request_.push_back(disjId);
      
      for (auto *d : disj) {
        addNode(rank(d),versionedName(d));
        addEdge(rank(d),disjId,"IPV");
      }
    }
  }

  int ImpactGraph::representedRelations(void) const {
    return num_edges(g_);
  }

  int ImpactGraph::representedPackages(void) const {
    return num_vertices(g_);
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
    
    /**
     * \brief Select a vertex in the range [\a begin, \a end) that
     * must be removed from graph \a g.
     *
     * \a begin and \a end provide the range of the articulation
     * points that can be removed. This is not the complete set of
     * articulation points, only the one that can be removed.
     */
    template <typename It>
    Vertex select(const ImpactGraphType& g, It begin, It end) {
      (void)g;
      (void)end;

      vector<Vertex> selectedVertices;
      return *begin;
    }

    /**
     * \brief Creates a sub-problem from the data in \a g and the set
     * of vertices \a include.
     *
     * The vertices in \a include belong to \a g.root(). 
     *
     * \todo Both parameters should be constant references.
     */ 
    ImpactGraphType* createSubproblem(ImpactGraphType& g, set<Vertex>& include) {
      set<Vertex> finalNodes;
      auto vg = vertices(g);
      for (; vg.first != vg.second; ++(vg.first)) {
        Vertex v = *(vg.first);
        finalNodes.insert(g.local_to_global(v));
      }
      std::copy(begin(include), end(include), std::inserter(finalNodes,begin(finalNodes)));
      
      auto& fsp = g.root().create_subgraph(begin(finalNodes),end(finalNodes));
      return &fsp;
    }

    /**
     * \brief Creates a sub-graph of \a g that includes all the
     * vertices but \a s.
     *
     * The resulting sub-graph is induced (see boost bgl)
     */
    ImpactGraphType& createSubgraph(ImpactGraphType& g, Vertex s) {
      // functor that returns true for all the vertices but s
      auto filterS = [&s](const Vertex& v) -> bool {
        return v != s;
      };

      auto v = vertices(g);
      boost::filter_iterator<decltype(filterS),VertexIterator>
        b(filterS,v.first), e(filterS,v.second);
      
      return g.create_subgraph(b,e);
    }
    
    template <typename StopFunctor>
    void partition(ImpactGraphType& g, int level, StopFunctor stop,
                   set<Vertex>& include, vector<ImpactGraphType*>& sub) 
    {
      if (stop(g)) {
        // At this point the graph is consider to represent a reasonable
        // sub-problem.
        //subproblems_.push_back(g);
        //cout << "Stop condition " << num_vertices(g) << " + " << include.size() << endl;
        sub.push_back(createSubproblem(g,include));
        return;
      } else {
        auto subproblems = components(g);
        if (subproblems.size() == 1) {
          
          // compute the articulation points of g
          auto ap = articulationPoints(g);

          // Remove from the set of articulation points those that
          // cannot be removed by this algorithm.
          auto newEnd = 
            std::remove_if(begin(ap), end(ap), 
                           [&](const Vertex& v) -> bool {
                             return include.count(g.local_to_global(v)) > 0;
                           });

          if (begin(ap) == newEnd) {
            // It is not possible to split the problem using
            // articulation points or there are but none of them can
            // be removed.
            cout << "Giving up... no articulation points" << endl;
            sub.push_back(createSubproblem(g,include));
            return;
          }
          
          // At this point we are sure that there is at least one
          // articulation point that can be removed.
          Vertex s = select(g,begin(ap),newEnd);
          
          // We create a sub-graph of g that contains all the vertices
          // that exist in g but the selected one.
          ImpactGraphType& leftSubproblem = createSubgraph(g, s);


          //cout << "Will do AP " << ap.size() << endl;
          //cout << "AP splitting " << g[s].id << endl
          //     << "Size(g) " << num_vertices(g) << endl;
          
          // left branch of the tree
          //cout << "Size(sg) " << num_vertices(leftSubproblem) << endl;

          // \todo not happy with this condition!!.
          if (num_vertices(g) != num_vertices(leftSubproblem)) {
            partition(leftSubproblem,level+1,stop,include,sub);
            //assert(num_vertices(g) == (num_vertices(sg) + 1));
           
            // right branch of the tree
            include.insert(g.local_to_global(s));
            auto nodes = vertices(g);
            auto& rightSubproblem = g.create_subgraph(nodes.first, nodes.second);
            partition(rightSubproblem,level+1,stop,include,sub);
          }
        } else {
          //cout << "Naturally" << endl;
          for (auto s : subproblems) {
            auto& sg = 
              g.create_subgraph(s.second.begin(),s.second.end());
            partition(sg,level+1,stop,include,sub);
          }
        }
      }
    }
  }

  void ImpactGraph::generateSubproblems(void) {
    auto edgesStop = [&](const ImpactGraphType& g) -> bool {
      return num_edges(g) < 10;
    };
    set<Vertex> include;
    //include.reserve(num_vertices(g));
    internal::partition(g_,0,edgesStop,include,subproblems_);
  }

  int ImpactGraph::subproblemsCount(void) const {
    return subproblems_.size();
  }

  int ImpactGraph::trivialSubproblemsCount(void) const {
    int trivial = 0;
    for (ImpactGraphType *s : subproblems_) {
      const ImpactGraphType& subproblem = *s;
      if (num_edges(subproblem) < 2) 
        trivial++;
      else {
        //toDot(subproblem,cout);
        cout << " non trivial: N:" << num_vertices(subproblem)
             << " E:" << num_edges(subproblem)
             << endl;
      }
    } 
    return trivial;
  }

  void ImpactGraph::outputSubproblem(int i, ostream& os) {
    auto *p = subproblems_[i];
    toDot(*p,os);
  }

  void ImpactGraph::outputProblem(ostream& os) {
    toDot(g_,os);
  }

  namespace internal {
    int hierarchyHelper(ostream& os, const ImpactGraphType& g, int parent, int& leaves) {
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
  }

  void ImpactGraph::subproblemHierarchy(ostream& os) const {
    assert(g_.is_root());
    int leaves = 0;
    os << "graph G {" << endl;
    internal::hierarchyHelper(os,g_,0,leaves);
    os << "}" << endl;
    cout << "Subproblems: " << leaves << endl;
  }

}
