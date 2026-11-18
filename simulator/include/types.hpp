#ifndef BELBIN_TYPES_HPP
#define BELBIN_TYPES_HPP

#include <nlohmann/json.hpp>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace cadmium::belbin {
using json = nlohmann::json;

enum class BelbinProfile { Action, Social, Mental };

inline std::string toString(BelbinProfile profile) {
  switch (profile) {
  case BelbinProfile::Action:
    return "Action";
  case BelbinProfile::Social:
    return "Social";
  case BelbinProfile::Mental:
    return "Mental";
  }
  throw std::invalid_argument("Unknown BelbinProfile");
}

inline BelbinProfile roleToProfile(const std::string &role) {
  if (role == "shaper" || role == "implementer" ||
      role == "completer_finisher") {
    return BelbinProfile::Action;
  }
  if (role == "coordinator" || role == "resource_investigator" ||
      role == "teamworker") {
    return BelbinProfile::Social;
  }
  if (role == "plant" || role == "monitor_evaluator" || role == "specialist") {
    return BelbinProfile::Mental;
  }
  throw std::invalid_argument("Unknown Belbin role: " + role);
}

enum class Phase { Silent, Talking, Overlapped };

inline std::string toString(Phase phase) {
  switch (phase) {
  case Phase::Silent:
    return "Silent";
  case Phase::Talking:
    return "Talking";
  case Phase::Overlapped:
    return "Overlapped";
  }
  throw std::invalid_argument("Unknown Phase");
}

enum class EventType { StartTalking, StopTalking, Interruption };

inline std::string toString(EventType type) {
  switch (type) {
  case EventType::StartTalking:
    return "StartTalking";
  case EventType::StopTalking:
    return "StopTalking";
  case EventType::Interruption:
    return "Interruption";
  }
  throw std::invalid_argument("Unknown EventType");
}

//! Communication event exchanged between team members
struct Message {
  EventType event_type;
  std::string sender_id;
  std::string receiver_id;
  Phase sender_state_after;
};

inline json toJson(const Message &msg) {
  return json{
      {"event_type", toString(msg.event_type)},
      {"sender_id", msg.sender_id},
      {"receiver_id", msg.receiver_id},
      {"sender_state_after", toString(msg.sender_state_after)},
  };
}

inline std::ostream &operator<<(std::ostream &out, const Message &msg) {
  out << toJson(msg).dump();
  return out;
}

struct MemberConfig {
  std::string member_id;
  std::string primary_role;
  std::string secondary_role;
  double primary_weight;
  double secondary_weight;
};

struct SimulationMetadata {
  std::string session_id;
  double simulation_duration_s;
  std::string context_type;
};

// NOTE: revisar si debe existir, probablemente vaya a eliminar la matriz de
// afinidad
//! Shared team data
struct TeamRoster {
  std::vector<MemberConfig> members;
  std::unordered_map<std::string, MemberConfig> member_index;
  std::unordered_map<std::string, std::unordered_map<std::string, double>>
      affinity_matrix;
  double reaction_delay_s = 0.2;
  double initial_talk_duration_s = 2.5;
  double duration_scale = 10.0;
};

// NOTE: lo mismo de arriba
inline double computeBilinearAffinity(const MemberConfig &from,
                                      const MemberConfig &to,
                                      const TeamRoster &roster) {
  const auto &p_row = roster.affinity_matrix.at(from.primary_role);
  const auto &s_row = roster.affinity_matrix.at(from.secondary_role);

  return (p_row.at(to.primary_role) * from.primary_weight * to.primary_weight) +
         (p_row.at(to.secondary_role) * from.primary_weight *
          to.secondary_weight) +
         (s_row.at(to.primary_role) * from.secondary_weight *
          to.primary_weight) +
         (s_row.at(to.secondary_role) * from.secondary_weight *
          to.secondary_weight);
}

} // namespace cadmium::belbin

#endif
