#ifndef __CPREL_PACKAGES_SOLVERSUPPORT_READER_HH__
#define __CPREL_PACKAGES_SOLVERSUPPORT_READER_HH__

#include <kcudf/kcudf.hh>
#include <rel/grelation.hh>

namespace CPRelPkg {
  /// A problem is a tuple of ground relations: <Packages,Dependencies,Conflicts,Provides>
  typedef std::tuple<MPG::GRelation,MPG::GRelation,MPG::GRelation,MPG::GRelation,MPG::GRelation>
  ProblemDesc;

  /**
   * \brief Stores the information of a kcudf file into a relation.
   */
  class RelationWriter : public KCudfWriter {
  private:
    /// The relations when the contents are read
    MPG::GRelation packages_;
    MPG::GRelation dependencies_;
    MPG::GRelation conflicts_;
    MPG::GRelation provides_;
    MPG::GRelation concretes_;
  public:
    ///Constructor
    RelationWriter(void)
      : packages_(3), dependencies_(2), conflicts_(2), provides_(2),
	concretes_(1) {}
    /// Destructor
    virtual ~RelationWriter(void) {}
    /// Callback for a package
    virtual void package(unsigned int id, bool keep, bool install, const char*) {
      int k = keep ? 1 : 0;
      int i = install ? 1 : 0;
      MPG::Tuple t({id,k,i});
      packages_.add(t);
    }
    /// Callback for a dependency
    virtual void dependency(unsigned int package, unsigned int depends, const char*) {
      MPG::Tuple t({(int)package,(int)depends});
      dependencies_.add(t);
    }
    /// Callback for a dependency
    virtual void conflict(unsigned int package, unsigned int conflict, const char*) {
      MPG::Tuple t({(int)package, (int)conflict});
      MPG::Tuple t2({(int)conflict, (int)package});
      conflicts_.add(t);
      conflicts_.add(t2);
    }
    /// Callback for a dependency
    virtual void provides(unsigned int package, unsigned int provides, const char*) {
      if (package == provides) {
	MPG::Tuple c({(int)package});
	concretes_.add(c);
      }

      MPG::Tuple t({(int)package,(int)provides});
      provides_.add(t);
    }
    /// Returns a tuple with the relations
    inline ProblemDesc problem(void) {
      return ProblemDesc(packages_,dependencies_,conflicts_,provides_,concretes_);
    }
    /// Returns the number of read concrete packages
    int concretes(void) const {
      return concretes_.cardinality();
    }
  };

  inline ProblemDesc process(std::istream& kcudf) {
    //std::cout << "Transforming problem into relations" << std::endl; 
    RelationWriter rw;
    read(kcudf,rw);
    return rw.problem();
  }

}

#endif
