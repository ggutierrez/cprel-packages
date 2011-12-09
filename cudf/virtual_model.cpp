#include <cudf/virtual_model.hh>

namespace CUDFTools {
 
  ModelVirtuals::ModelVirtuals(const char* cudf) 
    : CUDFTools::Model(cudf)
  {}

  int ModelVirtuals::virtualPackages(void) const {
    return virtuals_.size();
  }
   
  std::string ModelVirtuals::name(const std::vector<CUDFVersionedPackage*>& disj) {
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

  int ModelVirtuals::lookUpOrAdd(const std::vector<CUDFVersionedPackage*>& disj) {
    std::string key = name(disj);
    auto e = virtuals_.find(key);
    if (e == virtuals_.end()) {
      int newVirtual = packages().size() + virtuals_.size();
      virtuals_[key] = newVirtual;
      return newVirtual;
    }
    return e->second;
  }

  GraphModel::GraphModel(const char* cudf) 
    : ModelVirtuals(cudf)
  {}
    
  std::string GraphModel::label(int p) const {
    std::stringstream ss;
    ss << p;
    return ss.str();
  }

  std::string GraphModel::edgeId(int source, int target) {
      std::stringstream edgeId;
      edgeId << label(source) << " -- " << label(target);
      return edgeId.str();
    }

}
