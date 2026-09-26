#ifndef MEMBER_HPP
#define MEMBER_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <limits>

namespace cadmium::belbin {

//! Team member's state
struct MemberState {
  bool active;
  MemberState() : active(false) {}
};

//! Insertion operator, used automatically by Atomic<S>::logState()
inline std::ostream &operator<<(std::ostream &out, const MemberState &s) {
  out << "active: " << s.active;
  return out;
}

//! Atomic DEVS model for a team member
class Member : public Atomic<MemberState> {
public:
  Port<int> in;
  Port<int> out;

  explicit Member(const std::string &id)
      : Atomic<MemberState>(id, MemberState()) {
    in = addInPort<int>("in");
    out = addOutPort<int>("out");
  }

  void internalTransition(MemberState &s) const override { s.active = false; }

  void externalTransition(MemberState &s, double e) const override {
    s.active = true;
  }

  void output(const MemberState &s) const override {
    // TODO: add behavior
  }

  [[nodiscard]] double timeAdvance(const MemberState &s) const override {
    return s.active ? 1.0 : std::numeric_limits<double>::infinity();
  }
};

} // namespace cadmium::belbin

#endif
