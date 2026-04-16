#include "TestSupport.hpp"

#include "domain/SimulationState.hpp"

using traffic::tests::require;
using traffic::tests::require_near;

TRAFFIC_TEST(traffic_light_transitions_follow_expected_cycle) {
    traffic::domain::TrafficLight light(5.0, 7.0, 2.0);

    require(light.phase() == traffic::domain::TrafficPhase::NorthSouthGreen, "initial phase should be NS green");
    require(light.state_for(traffic::domain::Direction::North) == traffic::domain::LightState::Green,
        "north light should start green");
    require(light.state_for(traffic::domain::Direction::East) == traffic::domain::LightState::Red,
        "east light should start red");
    require_near(light.time_remaining_seconds(), 5.0, 1e-9, "initial remaining time mismatch");

    light.update(5.0);
    require(light.phase() == traffic::domain::TrafficPhase::NorthSouthYellow, "phase should advance to NS yellow");
    require(light.state_for(traffic::domain::Direction::North) == traffic::domain::LightState::Yellow,
        "north light should be yellow");
    require(light.state_for(traffic::domain::Direction::West) == traffic::domain::LightState::Red,
        "west light should stay red during NS yellow");

    light.update(2.0);
    require(light.phase() == traffic::domain::TrafficPhase::EastWestGreen, "phase should advance to EW green");
    require(light.just_turned_green(traffic::domain::Direction::East), "east/west should report just turned green");
    require(light.state_for(traffic::domain::Direction::East) == traffic::domain::LightState::Green,
        "east light should be green");

    light.update(0.1);
    require(!light.just_turned_green(traffic::domain::Direction::East),
        "just_turned_green should clear on the next update");

    light.update(6.9);
    require(light.phase() == traffic::domain::TrafficPhase::EastWestYellow, "phase should advance to EW yellow");

    light.update(2.0);
    require(light.phase() == traffic::domain::TrafficPhase::NorthSouthGreen,
        "phase should return to NS green after full cycle");
    require(light.just_turned_green(traffic::domain::Direction::North), "north/south should report just turned green");
    require(light.cycle_count() == 1, "cycle count should increment after returning to NS green");
}

TRAFFIC_TEST(traffic_light_reset_restores_initial_state) {
    traffic::domain::TrafficLight light(9.0, 4.0, 1.0);

    light.update(9.0);
    light.update(1.0);
    light.update(4.0);

    require(light.phase() == traffic::domain::TrafficPhase::EastWestYellow,
        "precondition failed: light should have advanced before reset");

    light.reset();

    require(light.phase() == traffic::domain::TrafficPhase::NorthSouthGreen, "reset should restore NS green");
    require_near(light.phase_elapsed_seconds(), 0.0, 1e-9, "phase elapsed should reset to zero");
    require_near(light.time_remaining_seconds(), 9.0, 1e-9, "time remaining should reset to NS green duration");
    require(light.cycle_count() == 0, "cycle count should reset");
    require(!light.just_turned_green(traffic::domain::Direction::North), "just_turned_green should reset");
}
