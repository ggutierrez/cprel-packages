#ifndef __CPREL_PACKAGES_CUDF_VIRTUAL_MODEL_HH__
#define __CPREL_PACKAGES_CUDF_VIRTUAL_MODEL_HH__

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <cudf/model.hh>

namespace CUDFTools {
  
  class ModelVirtuals : public CUDFTools::Model {
  private:
    /// Data structure to associates disjunctions and virtual packages
    std::unordered_map<std::string,int> virtuals_;
  protected:
    /// Return a name for \a disj
    std::string name(const std::vector<CUDFVersionedPackage*>& disj);
    /// Returns the package representing disjunction \a disj_
    int lookUpOrAdd(const std::vector<CUDFVersionedPackage*>& disj);
  public:
    // Objects of this class are non-copyable
    ModelVirtuals() = delete;
    ModelVirtuals(const ModelVirtuals&) = delete;
    ModelVirtuals& operator = (const ModelVirtuals&) = delete;
    /// Constructor from a input specification in \a cudf
    ModelVirtuals(const char* cudf);
    /// Return the number of virtual packages that were processed
    int virtualPackages(void) const;
  };

  /// A class that represents a model as a graph
  class GraphModel : public ModelVirtuals {
  protected:
    /// Returns the identifier for an edge from \a source to \a target
    virtual std::string edgeId(int source, int target);
  public:
    // Objects of this class are non-copyable
    GraphModel() = delete;
    GraphModel(const GraphModel&) = delete;
    GraphModel& operator = (const GraphModel&) = delete;
    /// Constructor
    GraphModel(const char* cudf);
    /// Transform an integer into an string
    virtual std::string label(int p) const;
    /// Adds the edge (\a source, \a target) to the graph
    virtual void addEdge(int source, int  target, const char *relation = "error") = 0;
    /// Adds the node \a n to the graph
    virtual void addNode(int n, const char *name = "none") = 0;
  };
  
}

#endif
