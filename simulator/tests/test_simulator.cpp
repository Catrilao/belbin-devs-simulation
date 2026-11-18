#include "atomics/member.hpp"
#include "coupled/team.hpp"
#include "loggers/json_trace_logger.hpp"
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <set>
#include <string>

using namespace cadmium::belbin;

namespace {

std::shared_ptr<TeamRoster> makeTestRoster() {
  auto roster = std::make_shared<TeamRoster>();
  roster->reaction_delay_s = 0.2;
  roster->initial_talk_duration_s = 2.5;
  roster->duration_scale = 10.0;

  MemberConfig alice{"Alice", "specialist", "teamworker", 0.6, 0.4};
  MemberConfig bob{"Bob", "coordinator", "plant", 0.8, 0.2};
  MemberConfig carol{"Carol", "shaper", "implementer", 0.7, 0.3};

  roster->members = {alice, bob, carol};
  for (const auto &m : roster->members) {
    roster->member_index.emplace(m.member_id, m);
  }

  const std::vector<std::string> roles = {"specialist",  "teamworker",
                                          "coordinator", "plant",
                                          "shaper",      "implementer"};
  for (const auto &r1 : roles) {
    for (const auto &r2 : roles) {
      roster->affinity_matrix[r1][r2] = 2.0;
    }
  }

  roster->affinity_matrix["specialist"]["coordinator"] = 4.0;
  roster->affinity_matrix["specialist"]["plant"] = 2.0;
  roster->affinity_matrix["teamworker"]["coordinator"] = 1.0;
  roster->affinity_matrix["teamworker"]["plant"] = 5.0;

  return roster;
}

} // namespace

TEST(TypesTest, RoleToProfileMapsAllBelbinRoles) {
  EXPECT_EQ(roleToProfile("shaper"), BelbinProfile::Action);
  EXPECT_EQ(roleToProfile("implementer"), BelbinProfile::Action);
  EXPECT_EQ(roleToProfile("completer_finisher"), BelbinProfile::Action);

  EXPECT_EQ(roleToProfile("coordinator"), BelbinProfile::Social);
  EXPECT_EQ(roleToProfile("resource_investigator"), BelbinProfile::Social);
  EXPECT_EQ(roleToProfile("teamworker"), BelbinProfile::Social);

  EXPECT_EQ(roleToProfile("plant"), BelbinProfile::Mental);
  EXPECT_EQ(roleToProfile("monitor_evaluator"), BelbinProfile::Mental);
  EXPECT_EQ(roleToProfile("specialist"), BelbinProfile::Mental);

  EXPECT_THROW(roleToProfile("unknown_role"), std::invalid_argument);
}

TEST(TypesTest, BilinearAffinityComputesExactCrossProduct) {
  auto roster = makeTestRoster();
  const auto &alice = roster->member_index.at("Alice");
  const auto &bob = roster->member_index.at("Bob");

  // 4.0*(0.6*0.8) + 2.0*(0.6*0.2) + 1.0*(0.4*0.8) + 5.0*(0.4*0.2) = 2.88
  EXPECT_NEAR(computeBilinearAffinity(alice, bob, *roster), 2.88, 1e-9);
}

TEST(MemberAtomicTest, InitialStateRespectsZenoBound) {
  auto roster = makeTestRoster();

  Member passive_member(roster->members[1].member_id, roster->members[1],
                        roster, 42, false);
  EXPECT_EQ(passive_member.currentState().phase, Phase::Silent);
  EXPECT_TRUE(
      std::isinf(passive_member.timeAdvance(passive_member.currentState())));

  Member starter_member(roster->members[0].member_id, roster->members[0],
                        roster, 42, true);
  EXPECT_EQ(starter_member.currentState().phase, Phase::Silent);
  EXPECT_GT(starter_member.timeAdvance(starter_member.currentState()), 0.0);
  EXPECT_DOUBLE_EQ(starter_member.timeAdvance(starter_member.currentState()),
                   roster->reaction_delay_s);
}

TEST(MemberAtomicTest, FullTurnLifecycleUpdatesStateAndAccumulatedTime) {
  auto roster = makeTestRoster();
  Member alice(roster->members[0].member_id, roster->members[0], roster, 42,
               true);
  MemberState s = alice.currentState();

  alice.output(s);
  ASSERT_EQ(alice.out->getBag().size(), 1u);
  EXPECT_EQ(alice.out->getBag()[0].event_type, EventType::StartTalking);
  EXPECT_EQ(alice.out->getBag()[0].sender_id, "Alice");
  EXPECT_EQ(alice.out->getBag()[0].sender_state_after, Phase::Talking);
  alice.out->clear();

  alice.internalTransition(s);
  EXPECT_EQ(s.phase, Phase::Talking);
  EXPECT_DOUBLE_EQ(alice.timeAdvance(s), roster->initial_talk_duration_s);

  alice.output(s);
  ASSERT_EQ(alice.out->getBag().size(), 1u);
  EXPECT_EQ(alice.out->getBag()[0].event_type, EventType::StopTalking);
  EXPECT_EQ(alice.out->getBag()[0].sender_state_after, Phase::Silent);
  alice.out->clear();

  alice.internalTransition(s);
  EXPECT_EQ(s.phase, Phase::Silent);
  EXPECT_TRUE(std::isinf(alice.timeAdvance(s)));
  EXPECT_DOUBLE_EQ(s.accumulated_talk_time, roster->initial_talk_duration_s);
}

TEST(MemberAtomicTest, ExternalTransitionDiscountsSigmaAndHandlesHandoff) {
  auto roster = makeTestRoster();
  Member bob(roster->members[1].member_id, roster->members[1], roster, 42,
             false);
  MemberState s = bob.currentState();

  // Unrelated start event updates room context without activating Bob
  bob.in->addMessage(
      Message{EventType::StartTalking, "Alice", "Carol", Phase::Talking});
  bob.externalTransition(s, 1.0);
  bob.in->clear();
  EXPECT_EQ(s.phase, Phase::Silent);
  EXPECT_EQ(s.active_speakers.count("Alice"), 1u);
  EXPECT_TRUE(std::isinf(s.sigma));

  // Directed stop event schedules Bob's turn after reaction_delay_s
  bob.in->addMessage(
      Message{EventType::StopTalking, "Alice", "Bob", Phase::Silent});
  bob.externalTransition(s, 2.5);
  bob.in->clear();
  EXPECT_EQ(s.active_speakers.count("Alice"), 0u);
  EXPECT_EQ(s.previous_speaker, "Alice");
  EXPECT_TRUE(s.planned_action.active);
  EXPECT_DOUBLE_EQ(s.sigma, roster->reaction_delay_s);

  // Elapsed time discounts remaining sigma when another unrelated event arrives
  bob.in->addMessage(
      Message{EventType::StartTalking, "Carol", "Alice", Phase::Talking});
  bob.externalTransition(s, 0.05);
  bob.in->clear();
  EXPECT_NEAR(s.sigma, 0.15, 1e-9);
}

TEST(MemberAtomicTest, ConfluentTransitionRunsInternalBeforeExternal) {
  auto roster = makeTestRoster();
  Member alice(roster->members[0].member_id, roster->members[0], roster, 42,
               true);
  MemberState s = alice.currentState();

  // Alice finishes speaking at the exact instant Carol starts talking
  alice.internalTransition(s);
  ASSERT_EQ(s.phase, Phase::Talking);

  alice.in->addMessage(
      Message{EventType::StartTalking, "Carol", "Bob", Phase::Talking});
  alice.confluentTransition(s, s.sigma);
  alice.in->clear();

  EXPECT_EQ(s.phase, Phase::Silent);
  EXPECT_EQ(s.active_speakers.count("Carol"), 1u);
}

TEST(TeamIntegrationTest, SequentialRunProducesValidDeterministicJsonTrace) {
  auto roster = makeTestRoster();
  SimulationMetadata meta{"test_session", 120.0, "sequential"};

  auto runOnce = [&](const std::filesystem::path &path) {
    auto model = std::make_shared<Team>("team", roster, 99);
    auto coordinator = cadmium::RootCoordinator(model);
    auto logger =
        std::make_shared<JsonTraceLogger>(path, meta, roster->members);
    coordinator.setLogger(logger);
    coordinator.start();
    coordinator.simulate(meta.simulation_duration_s);
    coordinator.stop();

    std::ifstream in(path);
    return json::parse(in);
  };

  auto path1 = std::filesystem::temp_directory_path() / "belbin_trace_1.json";
  auto path2 = std::filesystem::temp_directory_path() / "belbin_trace_2.json";

  json trace1 = runOnce(path1);
  json trace2 = runOnce(path2);
  std::filesystem::remove(path1);
  std::filesystem::remove(path2);

  // Determinism under identical seed
  EXPECT_EQ(trace1, trace2);

  // Metadata schema
  EXPECT_EQ(trace1.at("simulation_metadata").at("session_id"), "test_session");
  EXPECT_EQ(trace1.at("simulation_metadata").at("members").size(), 3u);

  const auto &events = trace1.at("trace_events");
  ASSERT_GT(events.size(), 4u);

  // Sequential invariants: non-decreasing time, strict alternation, no
  // self-handoff
  double prev_time = -1.0;
  std::string current_speaker;

  for (const auto &ev : events) {
    double t = ev.at("time_ms").get<double>();
    std::string type = ev.at("event_type").get<std::string>();
    std::string sender = ev.at("sender_id").get<std::string>();
    std::string receiver = ev.at("receiver_id").get<std::string>();

    EXPECT_GE(t, prev_time);
    EXPECT_NE(sender, receiver);
    prev_time = t;

    if (type == "StartTalking") {
      EXPECT_TRUE(current_speaker.empty())
          << "Overlap detected in sequential MVP";
      current_speaker = sender;
    } else if (type == "StopTalking") {
      EXPECT_EQ(current_speaker, sender);
      current_speaker.clear();
    }
  }
}
