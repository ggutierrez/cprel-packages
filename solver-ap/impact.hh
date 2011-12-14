#ifndef __CPREL_PACKAGES_SOLVER_AP_IMPACT_HH
#define __CPREL_PACKAGES_SOLVER_AP_IMPACT_HH

#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <cudf/virtual_model.hh>

namespace CUDFTools {
  /// The graph type
  typedef boost::subgraph< 
    boost::adjacency_list<
      boost::vecS, 
      boost::vecS,
      boost::directedS,
      boost::property<boost::vertex_color_t, int>, 
      boost::property<boost::edge_index_t, int> > 
    > ImpactGraphType;

  /// The type of a vertex in the graph
  typedef boost::graph_traits <ImpactGraphType>::vertex_descriptor Vertex;
  

  class ImpactGraph : public GraphModel {
  private:

    /// The graph
    ImpactGraphType g_;
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
    /**
     * \brief Traverse the represented problem and create the subproblems.
     */
    void generateSubproblems(void);
    /**
     * \brief Prints the problem hierarchy to \a os as a tree in dot
     * format
     */
    void subproblemHierarchy(std::ostream& os) const;
  };
}

#endif
