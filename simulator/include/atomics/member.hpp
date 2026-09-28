#ifndef MEMBER_HPP
#define MEMBER_HPP

#include <cadmium/core/modeling/atomic.hpp>
#include <limits>
#include <nlohmann/json.hpp>
#include <random>

namespace cadmium::belbin {
using json = nlohmann::json;

//! Token passed between team members
struct Token {
  std::string sender;
  std::string target;
};

inline std::ostream &operator<<(std::ostream &out, const Token &t) {
  out << "[" << t.sender << " -> " << t.target << "]";
  return out;
}

//! Information about the rest of the team
struct TeamRoster {
  json affinity_matrix;
  json members_data;
};

//! Team member's state
struct MemberState {
  bool active;
  std::string previous_speaker;
  std::string next_speaker;
  MemberState() : active(false), previous_speaker(""), next_speaker("") {}
};

inline std::ostream &operator<<(std::ostream &out, const MemberState &s) {
  out << "active: " << s.active << " | rep: " << s.previous_speaker;
  return out;
}

//! Atomic DEVS model for a team member
class Member : public Atomic<MemberState> {
public:
  Port<Token> in;
  Port<Token> out;

  std::string member_id;
  json data;
  std::shared_ptr<TeamRoster> roster;

  mutable std::mt19937 rng;

  explicit Member(const std::string &id, const json &input_data,
                  std::shared_ptr<TeamRoster> shared_roster, size_t global_seed)
      : Atomic<MemberState>(id, MemberState()), member_id(id), data(input_data),
        roster(shared_roster) {
    in = addInPort<Token>("in");
    out = addOutPort<Token>("out");
    rng.seed(std::hash<std::string>{}(member_id) ^ global_seed);
  }

  void internalTransition(MemberState &s) const override { s.active = false; }

  void externalTransition(MemberState &s, double e) const override {
    for (const auto &token : in->getBag()) {
      if (token.target == member_id) {
        s.active = true;
        s.previous_speaker = token.sender;

        std::string p_role = data["primary_role"];
        std::string s_role = data["secondary_role"];
        double p_weight = data["primary_weight"].get<double>();
        double s_weight = data["secondary_weight"].get<double>();
        std::vector<std::string> candidates;
        std::vector<double> probabilities;

        for (const auto &m : roster->members_data) {
          std::string candidate_id = m["member_id"];
          if (member_id == candidate_id)
            continue;

          std::string candidate_p_role = m["primary_role"];
          std::string candidate_s_role = m["secondary_role"];
          double candidate_p_weight = m["primary_weight"].get<double>();
          double candidate_s_weight = m["secondary_weight"].get<double>();

          double score =
              (roster->affinity_matrix[p_role][candidate_p_role].get<double>() *
               p_weight * candidate_p_weight) +
              (roster->affinity_matrix[p_role][candidate_s_role].get<double>() *
               p_weight * candidate_s_weight) +
              (roster->affinity_matrix[s_role][candidate_p_role].get<double>() *
               s_weight * candidate_p_weight) +
              (roster->affinity_matrix[s_role][candidate_s_role].get<double>() *
               s_weight * candidate_s_weight);

          candidates.push_back(candidate_id);
          probabilities.push_back(score);
        }

        if (!candidates.empty()) {
          std::discrete_distribution<> distribution(probabilities.begin(),
                                                    probabilities.end());
          s.next_speaker = candidates[distribution(rng)];
        }
      }
    }
  }

  void output(const MemberState &s) const override {
    if (!s.active || s.next_speaker.empty())
      return;

    Token t{member_id, s.next_speaker};
    out->addMessage(t);
  }

  [[nodiscard]] double timeAdvance(const MemberState &s) const override {
    if (!s.active)
      return std::numeric_limits<double>::infinity();

    if (s.previous_speaker == "Starter") {
      return 2.5;
    }

    for (const auto &m : roster->members_data) {
      if (m["member_id"] == s.previous_speaker) {
        std::string prev_p_role = m["primary_role"];
        std::string prev_s_role = m["secondary_role"];
        double prev_p_weight = m["primary_weight"];
        double prev_s_weight = m["secondary_weight"];

        std::string p_role = data["primary_role"];
        std::string s_role = data["secondary_role"];
        double p_weight = data["primary_weight"].get<double>();
        double s_weight = data["secondary_weight"].get<double>();

        double p_affinity =
            roster->affinity_matrix[prev_p_role][p_role].get<double>() *
                p_weight +
            roster->affinity_matrix[prev_p_role][s_role].get<double>() *
                s_weight;
        double s_affinity =
            roster->affinity_matrix[prev_s_role][p_role].get<double>() *
                p_weight +
            roster->affinity_matrix[prev_s_role][s_role].get<double>() *
                s_weight;

        return ((p_affinity * prev_p_weight) + (s_affinity * prev_s_weight)) *
               10.0; // NOTE: check scale, could be based on the role
      }
    }

    return 2.5;
  }
};

} // namespace cadmium::belbin

#endif
