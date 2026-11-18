#ifndef BELBIN_CONFIG_HPP
#define BELBIN_CONFIG_HPP

#include "types.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace cadmium::belbin {

struct LoadedConfig {
  SimulationMetadata metadata;
  TeamRoster roster;
  size_t global_seed = 0;
};

inline LoadedConfig parseConfig(const json &raw) {
  LoadedConfig loaded;
  loaded.global_seed = raw.at("global_seed").get<size_t>();
  loaded.metadata = SimulationMetadata{
      raw.value("session_id", "scenario_base"),
      raw.value("simulation_duration_s", 500.0),
      raw.value("context_type", "sequential"),
  };

  loaded.roster.reaction_delay_s = raw.value("reaction_delay_s", 0.2);
  loaded.roster.initial_talk_duration_s =
      raw.value("initial_talk_duration_s", 2.5);
  loaded.roster.duration_scale = raw.value("duration_scale", 10.0);

  for (const auto &m : raw.at("team")) {
    MemberConfig cfg{
        m.at("member_id").get<std::string>(),
        m.at("primary_role").get<std::string>(),
        m.at("secondary_role").get<std::string>(),
        m.at("primary_weight").get<double>(),
        m.at("secondary_weight").get<double>(),
    };
    loaded.roster.member_index.emplace(cfg.member_id, cfg);
    loaded.roster.members.push_back(std::move(cfg));
  }

  for (const auto &[src_role, targets] : raw.at("affinity_matrix").items()) {
    for (const auto &[dst_role, weight] : targets.items()) {
      loaded.roster.affinity_matrix[src_role][dst_role] = weight.get<double>();
    }
  }

  return loaded;
}

inline LoadedConfig loadConfigFile(const std::filesystem::path &config_path) {
  std::ifstream f(config_path);
  if (!f.is_open()) {
    throw std::runtime_error("Error opening config file: " +
                             config_path.string());
  }
  return parseConfig(json::parse(f));
}

} // namespace cadmium::belbin

#endif
