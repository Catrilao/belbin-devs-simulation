#ifndef STARTER_HPP
#define STARTER_HPP

#include "atomics/member.hpp"
#include <cadmium/core/modeling/atomic.hpp>
#include <limits>

struct StarterState {
  bool fired;
  StarterState() : fired(false) {};
};

inline std::ostream &operator<<(std::ostream &out, const StarterState &s) {
  out << "fired: " << s.fired;
  return out;
}

class Starter : public cadmium::Atomic<StarterState> {
public:
  cadmium::Port<cadmium::belbin::Token> out;
  std::string first_speaker;

  Starter(const std::string &id, const std::string &target_member)
      : Atomic<StarterState>(id, StarterState()), first_speaker(target_member) {
    out = addOutPort<cadmium::belbin::Token>("out");
  }

  void internalTransition(StarterState &s) const override { s.fired = true; };

  void externalTransition(StarterState &s, double e) const override {};

  void output(const StarterState &s) const override {
    if (!s.fired)
      out->addMessage(cadmium::belbin::Token{"Starter", first_speaker});
  };

  [[nodiscard]] double timeAdvance(const StarterState &s) const override {
    return s.fired ? std::numeric_limits<double>::infinity() : 0.0;
  }
};

#endif
