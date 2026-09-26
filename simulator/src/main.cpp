#include "atomics/member.hpp"
#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <filesystem>
#include <iostream>
#include <limits>

using namespace cadmium::belbin;

//! Coupled model for the entire team
struct Team : public cadmium::Coupled {
  explicit Team(const std::string &id) : Coupled(id) {
    auto member1 = addComponent<Member>("member1");
  }
};

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <config.json> <output_dir>\n";
    return 1;
  }

  std::filesystem::path config_path = argv[1];
  std::filesystem::path output_dir = argv[2];
  std::filesystem::create_directory(output_dir);

  auto model = std::make_shared<Team>("team");
  auto rootCoordinator = cadmium::RootCoordinator(model);

  auto logger = std::make_shared<cadmium::CSVLogger>(
      (output_dir / "output_log.csv").string(), ";");
  rootCoordinator.setLogger(logger);

  rootCoordinator.start();
  rootCoordinator.simulate(std::numeric_limits<double>::infinity());
  rootCoordinator.stop();

  return 0;
}
