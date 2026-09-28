#include "atomics/member.hpp"
#include "atomics/starter.hpp"
#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <filesystem>
#include <iostream>

using namespace cadmium::belbin;

//! Coupled model for the entire team
struct Team : public cadmium::Coupled {
  explicit Team(const std::string &id, const json &config) : Coupled(id) {
    size_t global_seed = config["global_seed"];

    auto roster = std::make_shared<TeamRoster>();
    roster->members_data = config["team"];
    roster->affinity_matrix = config["affinity_matrix"];

    std::vector<std::shared_ptr<Member>> agents;
    for (const auto &member_data : roster->members_data) {
      std::string m_id = member_data["member_id"];
      std::string p_role = member_data["primary_role"];
      std::string s_role = member_data["secondary_role"];

      auto agent = addComponent<Member>(m_id, member_data, roster, global_seed);
      agents.push_back(agent);
    }

    for (auto &sender : agents) {
      for (auto &receiver : agents) {
        if (sender->member_id != receiver->member_id) {
          addCoupling(sender->out, receiver->in);
        }
      }
    }

    std::string first_member_id = config["team"][0]["member_id"];
    auto starter = addComponent<Starter>("Starter", first_member_id);
    for (auto &receiver : agents) {
      addCoupling(starter->out, receiver->in);
    }
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

  std::ifstream f(config_path);
  if (!f.is_open()) {
    std::cerr << "Error opening config file: " << config_path << "\n";
    return 1;
  }
  json config = json::parse(f);

  auto model = std::make_shared<Team>("team", config);
  auto rootCoordinator = cadmium::RootCoordinator(model);

  auto logger = std::make_shared<cadmium::CSVLogger>(
      (output_dir / "output_log.csv").string(), ";");
  rootCoordinator.setLogger(logger);

  // NOTE: check the time limit
  // it could change based on the scenario
  rootCoordinator.start();
  rootCoordinator.simulate(500.0);
  rootCoordinator.stop();

  return 0;
}
