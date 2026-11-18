#include "config.hpp"
#include "coupled/team.hpp"
#include "loggers/json_trace_logger.hpp"
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <filesystem>
#include <iostream>
#include <memory>

using namespace cadmium::belbin;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <config.json> <output_dir>\n";
    return 1;
  }

  std::filesystem::path config_path = argv[1];
  std::filesystem::path output_dir = argv[2];
  std::filesystem::create_directories(output_dir);

  LoadedConfig config = loadConfigFile(config_path);
  auto roster = std::make_shared<TeamRoster>(std::move(config.roster));

  auto model = std::make_shared<Team>("team", roster, config.global_seed);
  auto rootCoordinator = cadmium::RootCoordinator(model);

  auto logger = std::make_shared<JsonTraceLogger>(
      output_dir / "trace.json", config.metadata, roster->members);
  rootCoordinator.setLogger(logger);

  rootCoordinator.start();
  rootCoordinator.simulate(config.metadata.simulation_duration_s);
  rootCoordinator.stop();

  return 0;
}
