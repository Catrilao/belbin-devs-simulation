#ifndef BELBIN_TEAM_HPP
#define BELBIN_TEAM_HPP

#include "atomics/member.hpp"
#include <cadmium/core/modeling/coupled.hpp>
#include <memory>
#include <string>
#include <vector>

namespace cadmium::belbin {

//! Coupled model for the entire team
struct Team : public cadmium::Coupled {
  explicit Team(const std::string &id, std::shared_ptr<const TeamRoster> roster,
                size_t global_seed)
      : Coupled(id) {
    std::vector<std::shared_ptr<Member>> members;
    members.reserve(roster->members.size());

    for (size_t i = 0; i < roster->members.size(); i++) {
      bool starts_first = (i == 0);
      auto member =
          addComponent<Member>(roster->members[i].member_id, roster->members[i],
                               roster, global_seed, starts_first);
      members.push_back(member);
    }

    for (auto &sender : members) {
      for (auto &receiver : members) {
        if (sender->config.member_id != receiver->config.member_id) {
          addCoupling(sender->out, receiver->in);
        }
      }
    }
  }
};

} // namespace cadmium::belbin

#endif
