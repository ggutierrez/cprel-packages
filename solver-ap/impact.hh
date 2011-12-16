#ifndef __CPREL_PACKAGES_SOLVER_AP_IMPACT_HH
#define __CPREL_PACKAGES_SOLVER_AP_IMPACT_HH

#include <string>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <cudf/virtual_model.hh>

namespace CUDFTools {
  struct ImpactGraphVertexData {
    ///size_t color;
    size_t id;
    std::string name;
  };
  struct ImpactGraphEdgeData {
    //size_t index;
    std::string name;
  };

  struct ImpactGraphData {
    /** 
     * \brief Indicates if the graph is the result of removing an
     * aritculation point
     */
    bool articulated;
    size_t id;
    std::set<size_t> include;
  };

  typedef boost::adjacency_list<
    boost::setS, 
    boost::vecS,
    boost::undirectedS,
    ImpactGraphVertexData,
    boost::property<boost::edge_index_t,int,ImpactGraphEdgeData>,
    ImpactGraphData
    > _Graph_;
 
  /// The graph type
  typedef boost::subgraph<_Graph_> ImpactGraphType;

  /// The type of a vertex in the graph
  typedef boost::graph_traits< ImpactGraphType >::vertex_descriptor Vertex;

  /// The type of a vertex in the graph
  typedef boost::graph_traits< ImpactGraphType >::vertex_iterator VertexIterator;

  /**
   * \brief Class to represent the impact of packages in a model
   */
  class ImpactGraph : public GraphModel {
  private:
    /// The graph
    ImpactGraphType g_;
    /// Map package identifiers to nodes in the grap
    std::map<int,Vertex> nodeToVertex;
    /// All the sub problems
    std::vector<ImpactGraphType*> subproblems_;
    /// Adds the edge (\a source, \a target) to the graph
    void addEdge(int source, int  target, const char *relation = "error");
    /// Adds a node to the graph
    void addNode(int n, const char *name = "none");
    /// The nodes in the graph that are created because of the request
    std::vector<Vertex> request_;
  public:
    // Prevent default construction
    ImpactGraph(void) = delete;
    // Prevent copy
    ImpactGraph(const ImpactGraph&) = delete;
    // Prevent assignment
    ImpactGraph& operator = (const ImpactGraph&) = delete;
    /// Constructor from a cudf specification
    ImpactGraph(const char* cudf);
    /// Destructor
    virtual ~ImpactGraph(void);
    /// Obligations
    virtual void depend(CUDFVersionedPackage *p, const std::vector<CUDFVersionedPackage*>& disj);
    /// Add a conflict between package \a p and package \a q
    virtual void conflict(CUDFVersionedPackage *p, CUDFVersionedPackage *q);
    /// Handle keep constraint \a kcst for package \a p with impact \a pkgs
    virtual void keep(int , CUDFVersionedPackage *, const std::vector<CUDFVersionedPackage*>&);
    /// Handle the installation of one of the packages in \a disj
    virtual void install(const std::vector<CUDFVersionedPackage*>& disj);
    /// Returns the number of represented relations
    int representedRelations(void) const;
    /// Returns the number of represented packages
    int representedPackages(void) const;
    /**
     * \brief Traverse the represented problem and create the subproblems.
     */
    void generateSubproblems(void);
    /// Returns the number of created sub-problems
    int subproblemsCount(void) const;
    /// Returns the number of trivial sub-problems
    int trivialSubproblemsCount(void) const;
    /// Output subproblem \a i to \a os in dot format
    void outputSubproblem(int i, std::ostream& os);
    /// Output the graph of the problem to \a os in dot format
    void outputProblem(std::ostream& os);
    /// Output problem decomposition information
    void outputSubproblemTree(std::ostream& os) const;
    /**
     * \brief Prints the problem hierarchy to \a os as a tree in dot
     * format
     */
    void subproblemHierarchy(std::ostream& os) const;
  };
}

#endif
