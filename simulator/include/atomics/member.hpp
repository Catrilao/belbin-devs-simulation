#ifndef MEMBER_HPP
#define MEMBER_HPP

#include "types.hpp"
#include <algorithm>
#include <cadmium/core/modeling/atomic.hpp>
#include <cmath>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <string>

namespace cadmium::belbin {

struct PlannedAction {
  bool active = false;
  EventType type = EventType::StartTalking;
  std::string target = "Group";
};

//! Team member's structured state
struct MemberState {
  Phase phase = Phase::Silent;
  double sigma = std::numeric_limits<double>::infinity();
  std::set<std::string> active_speakers;
  std::string previous_speaker;
  double accumulated_talk_time = 0.0;
  PlannedAction planned_action;
};

inline std::ostream &operator<<(std::ostream &out, const MemberState &s) {
  out << "phase: " << toString(s.phase) << " | sigma: " << s.sigma
      << " | prev: " << s.previous_speaker;
  return out;
}

//! Atomic DEVS model for a team member
class Member : public Atomic<MemberState> {
public:
  Port<Message> in;
  Port<Message> out;

  MemberConfig config;
  std::shared_ptr<const TeamRoster> roster;
  mutable std::mt19937 rng;

  explicit Member(const std::string &id, const MemberConfig &member_config,
                  std::shared_ptr<const TeamRoster> shared_roster,
                  size_t global_seed, bool starts_first = false)
      : Atomic<MemberState>(id, MemberState()), config(member_config),
        roster(std::move(shared_roster)) {
    in = addInPort<Message>("in");
    out = addOutPort<Message>("out");
    rng.seed(std::hash<std::string>{}(member_config.member_id) ^ global_seed);

    if (starts_first) {
      state.planned_action = {true, EventType::StartTalking,
                              selectNextSpeaker()};
      state.sigma = roster->reaction_delay_s;
    }
  }

  void internalTransition(MemberState &s) const override {
    if (!s.planned_action.active) {
      s.sigma = std::numeric_limits<double>::infinity();
      return;
    }

    if (s.planned_action.type == EventType::StartTalking) {
      s.phase = Phase::Talking;
      s.sigma = computeSpeakingDuration(s.previous_speaker);
      s.planned_action.type = EventType::StopTalking;
    } else if (s.planned_action.type == EventType::StopTalking) {
      s.accumulated_talk_time += s.sigma;
      s.phase = Phase::Silent;
      s.sigma = std::numeric_limits<double>::infinity();
      s.planned_action.active = false;
    }
  }

  void externalTransition(MemberState &s, double e) const override {
    if (std::isfinite(s.sigma)) {
      s.sigma = std::max(0.0, s.sigma - e);
    }

    for (const auto &msg : in->getBag()) {
      if (msg.event_type == EventType::StartTalking) {
        s.active_speakers.insert(msg.sender_id);
      } else if (msg.event_type == EventType::StopTalking) {
        s.active_speakers.erase(msg.sender_id);

        // TODO: agregar interrupciones, por ahora se respetan los turnos
        if (msg.receiver_id == config.member_id && s.phase == Phase::Silent) {
          s.previous_speaker = msg.sender_id;
          s.planned_action = {true, EventType::StartTalking,
                              selectNextSpeaker()};
          s.sigma = roster->reaction_delay_s;
        }
      }
    }
  }

  void confluentTransition(MemberState &s, double /*e*/) const override {
    internalTransition(s);
    externalTransition(s, 0.0);
  }

  void output(const MemberState &s) const override {
    if (!s.planned_action.active) {
      return;
    }

    Phase state_after = (s.planned_action.type == EventType::StopTalking)
                            ? Phase::Silent
                            : Phase::Talking;

    out->addMessage(Message{s.planned_action.type, config.member_id,
                            s.planned_action.target, state_after});
  }

  [[nodiscard]] double timeAdvance(const MemberState &s) const override {
    return s.sigma;
  }

  [[nodiscard]] const MemberState &currentState() const { return state; }

private:
  [[nodiscard]] std::string selectNextSpeaker() const {
    std::vector<std::string> candidates;
    std::vector<double> weights;

    for (const auto &peer : roster->members) {
      if (peer.member_id == config.member_id) {
        continue;
      }
      double score = computeBilinearAffinity(config, peer, *roster);
      candidates.push_back(peer.member_id);
      weights.push_back(std::max(0.0, score));
    }

    if (candidates.empty()) {
      return "Group";
    }

    double total = std::accumulate(weights.begin(), weights.end(), 0.0);
    if (total <= 0.0) {
      std::uniform_int_distribution<size_t> uniform(0, candidates.size() - 1);
      return candidates[uniform(rng)];
    }

    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    return candidates[dist(rng)];
  }

  [[nodiscard]] double
  computeSpeakingDuration(const std::string &prev_speaker) const {
    if (prev_speaker.empty()) {
      return roster->initial_talk_duration_s;
    }

    auto it = roster->member_index.find(prev_speaker);
    if (it == roster->member_index.end()) {
      return roster->initial_talk_duration_s;
    }

    double affinity = computeBilinearAffinity(it->second, config, *roster);
    double duration = affinity * roster->duration_scale;
    return std::max(roster->reaction_delay_s, duration);
  }
};

} // namespace cadmium::belbin

#endif
