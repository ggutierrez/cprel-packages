#ifndef __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_BRANCH_CHOICE_HH
#define __CPREL_PACKAGES_SOLVER_CPREL_PARANOID_BRANCH_CHOICE_HH

#include <gecode/search.hh>
#include <bdddomain/tuple.hh>

namespace CUDFTools {
  /**
   * \brief Choice class that stores a tuple.
   *
   */
  class RelChoice : public Gecode::Choice {
  public:
    /// Tuple to branch on
    MPG::Tuple t_;
    /// Constructor
    RelChoice(const Gecode::Brancher& b, const MPG::Tuple& t)
      : Choice(b,2), t_(t) {}
    /// Returns the size of the object
    virtual size_t size(void) const {
      return sizeof(*this);
    }
    virtual void archive(Gecode::Archive& e) const {
      Choice::archive(e);
      std::vector<int> t(t_.value());
      // first the arity of the tuple and then the tuple itself
      e << t_.arity();
      for (int i = 0; i < t_.arity(); i++) {
        e << t[i];
      }
    }
  };
}

#endif
