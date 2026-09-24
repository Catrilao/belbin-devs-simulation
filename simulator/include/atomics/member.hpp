#ifndef MEMBER_HPP
#define MEMBER_HPP

#include <cadmium/modeling/message_bag.hpp>
#include <cadmium/modeling/ports.hpp>

#include <limits>
#include <sstream>

using namespace cadmium;
using namespace std;

struct Member_defs {
  struct in : public in_port<int> {};
  struct out : public out_port<int> {};
};

template <typename TIME> class Member {
public:
  using input_ports = tuple<typename Member_defs::in>;
  using output_ports = tuple<typename Member_defs::out>;

  struct state_type {
    bool active;
  };
  state_type state;

  Member() noexcept { state.active = false; };

  void internal_transition() { state.active = false; };

  void external_transition(TIME e,
                           typename make_message_bags<input_ports>::type mbs) {
    vector<int> bag_port_in = get_messages<typename Member_defs::in>(mbs);
    state.active = true;
  };

  void
  confluence_transition(TIME e,
                        typename make_message_bags<input_ports>::type mbs) {
    internal_transition();
    external_transition(TIME(), std::move(mbs));
  };

  typename make_message_bags<output_ports>::type output() const {
    typename make_message_bags<output_ports>::type bags;
    return bags;
  };

  TIME time_advance() const {
    TIME next_interval;
    if (state.active) {
      next_interval = TIME("00:00:01:0000");
    } else {
      next_interval = numeric_limits<TIME>::infinity();
    }
    return next_interval;
  };

  friend ostringstream &operator<<(ostringstream &os,
                                   const typename Member<TIME>::state_type &i) {
    os << "active" << i.active;
    return os;
  };
};

#endif // MEMBER_HPP
