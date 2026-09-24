#include "atomics/member.hpp"
#include <NDTime.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/modeling/ports.hpp>
#include <fstream>
#include <iostream>

using namespace std;
using namespace cadmium;
using TIME = NDTime;

int main() {
  shared_ptr<dynamic::modeling::model> member1 =
      dynamic::translate::make_dynamic_atomic_model<Member, TIME>("member1");

  dynamic::modeling::Ports iports_TOP = {};
  dynamic::modeling::Ports oports_TOP = {};
  dynamic::modeling::Models submodels_TOP = {member1};
  dynamic::modeling::EICs eics_TOP = {};
  dynamic::modeling::EOCs eocs_TOP = {};
  dynamic::modeling::ICs ics_TOP = {};

  shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
  TOP = make_shared<dynamic::modeling::coupled<TIME>>(
      "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP,
      ics_TOP);

  static ofstream out_messages("simulation_results/output_messages.txt");
  struct oss_sink_messages {
    static ostream &sink() { return out_messages; };
  };

  static ofstream out_state("simulation_results/output_state.txt");
  struct oss_sink_state {
    static ostream &sink() { return out_state; };
  };

  using state =
      logger::logger<logger::logger_state, dynamic::logger::formatter<TIME>,
                     oss_sink_state>;
  using log_messages =
      logger::logger<logger::logger_messages, dynamic::logger::formatter<TIME>,
                     oss_sink_messages>;
  using global_time_mes =
      logger::logger<logger::logger_global_time,
                     dynamic::logger::formatter<TIME>, oss_sink_messages>;
  using global_time_sta =
      logger::logger<logger::logger_global_time,
                     dynamic::logger::formatter<TIME>, oss_sink_state>;
  using logger_top = logger::multilogger<state, log_messages, global_time_mes,
                                         global_time_sta>;

  dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
  r.run_until_passivate();
}
