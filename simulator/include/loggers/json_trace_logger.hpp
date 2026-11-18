#ifndef JSON_TRACE_LOGGER_HPP
#define JSON_TRACE_LOGGER_HPP

#include "types.hpp"
#include <cadmium/core/logger/logger.hpp>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cadmium::belbin {

//! Custom Cadmium logger that exports simulation traces in JSON format
class JsonTraceLogger : public cadmium::Logger {
  std::filesystem::path output_path;
  SimulationMetadata metadata;
  std::vector<MemberConfig> members;
  json trace_events;

public:
  JsonTraceLogger(std::filesystem::path path, SimulationMetadata meta,
                  std::vector<MemberConfig> team_members)
      : output_path(std::move(path)), metadata(std::move(meta)),
        members(std::move(team_members)), trace_events(json::array()) {}

  void start() override { trace_events = json::array(); }

  void stop() override {
    json members_json = json::array();
    for (const auto &m : members) {
      members_json.push_back(json{
          {"member_id", m.member_id},
          {"belbin_profile", toString(roleToProfile(m.primary_role))},
          {"dominant_roles", json::array({m.primary_role, m.secondary_role})},
      });
    }

    json root = {
        {"simulation_metadata",
         {
             {"session_id", metadata.session_id},
             {"simulation_duration_s", metadata.simulation_duration_s},
             {"context_type", metadata.context_type},
             {"members", members_json},
         }},
        {"trace_events", trace_events},
    };

    std::ofstream out(output_path);
    if (!out.is_open()) {
      throw std::runtime_error("Failed to open trace output file: " +
                               output_path.string());
    }
    out << root.dump(2) << '\n';
  }

  void logOutput(double time, long /*modelId*/,
                 const std::string & /*modelName*/, const std::string &portName,
                 const std::string &output) override {
    if (portName != "out" || output.empty()) {
      return;
    }

    json event = json::parse(output);
    event["time_ms"] = std::round(time * 1e16) / 1e3;
    trace_events.push_back(std::move(event));
  }

  void logState(double /*time*/, long /*modelId*/,
                const std::string & /*modelName*/,
                const std::string & /*state*/) override {
    // State transitions are embedded via sender_state_after in output events
  }
};

} // namespace cadmium::belbin

#endif
