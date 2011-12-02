#ifndef __CPREL_PACKAGES_GRAPH_VIEW_GRAPH_VIEW_HH
#define __CPREL_PACKAGES_GRAPH_VIEW_GRAPH_VIEW_HH

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

namespace CPRelPkg {
  /**
   * \brief Package information.
   *
   * The packages are the vertices of the graph. This define the
   * information we keep on them.
   */
  struct Package {
    /// Package identifier
    int rank;
    /// Importance of the package for the current installation
    int relevance;
  };
  enum RelType {
    RelDep,   // a dependency
    RelConf,  // a conflict
    RelProv,  // a provides 
  };
  /**
   * \brief Relation information.
   *
   * The relations are the edges of the graph. This define the
   * information we keep on them.
   */
  struct PkgRelation {
    /// relation type
    RelType rt;
    /// weight of the edge
    int relevance;
  };

  /// The graph type
  typedef boost::adjacency_list<
    boost::vecS, boost::vecS, boost::bidirectionalS,
    Package, PkgRelation>
  Repository;

  /// The type of a vertex in the repository
  typedef typename boost::graph_traits<Repository>::vertex_descriptor
  RepoVertex;
}

#endif

