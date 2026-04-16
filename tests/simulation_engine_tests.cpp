#include "TestSupport.hpp"

#include "simulation/SimulationEngine.hpp"

using traffic::tests::require;
using traffic::tests::require_near;

TRAFFIC_TEST(simulation_engine_apply_config_clamps_values_and_can_reset_runtime) {
    traffic::simulation::SimulationEngine engine;
    engine.initialize(800.0F, 600.0F);

    engine.state().emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {430.0, 420.0});
    engine.state().traffic_light().update(12.0);

    auto config = engine.state().config();
    config.spawn_probability = 4.0;
    config.spawn_interval_seconds = 0.0;
    config.north_south_green_seconds = 0.0;
    config.east_west_green_seconds = -5.0;
    config.yellow_seconds = 0.0;
    config.queue_speed_threshold = -1.0;
    config.wait_speed_threshold = -3.0;
    config.reaction_time_min_seconds = 1.5;
    config.reaction_time_max_seconds = 0.5;
    config.random_seed = 99U;

    engine.apply_config(config, true);

    const auto& updated = engine.state().config();
    require_near(updated.spawn_probability, 1.0, 1e-9, "spawn probability should clamp to 1");
    require_near(updated.spawn_interval_seconds, 0.2, 1e-9, "spawn interval should fall back to default");
    require_near(updated.north_south_green_seconds, 1.0, 1e-9, "NS green should clamp to minimum");
    require_near(updated.east_west_green_seconds, 1.0, 1e-9, "EW green should clamp to minimum");
    require_near(updated.yellow_seconds, 1.0, 1e-9, "yellow should clamp to minimum");
    require_near(updated.queue_speed_threshold, 0.0, 1e-9, "queue threshold should clamp to zero");
    require_near(updated.wait_speed_threshold, 0.0, 1e-9, "wait threshold should clamp to zero");
    require_near(updated.reaction_time_min_seconds, 1.5, 1e-9, "reaction min mismatch");
    require_near(updated.reaction_time_max_seconds, 1.5, 1e-9, "reaction max should not fall below min");
    require(engine.state().active_vehicle_count() == 0, "resetSimulation should clear runtime vehicles");
    require(engine.state().traffic_light().phase() == traffic::domain::TrafficPhase::NorthSouthGreen,
        "resetSimulation should restore initial phase");
}

TRAFFIC_TEST(simulation_engine_reset_clears_runtime_state) {
    traffic::simulation::SimulationEngine engine;
    engine.initialize(800.0F, 600.0F);

    engine.state().emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {430.0, 420.0});
    engine.state().update_metrics();
    require(engine.state().active_vehicle_count() == 1, "precondition failed: manual runtime vehicle missing");

    engine.state().traffic_light().update(engine.state().config().north_south_green_seconds);
    engine.reset();

    require(engine.state().active_vehicle_count() == 0, "reset should clear active vehicles");
    require(engine.state().metrics().total_vehicles_spawned == 0, "reset should clear spawn metrics");
    require(engine.state().traffic_light().phase() == traffic::domain::TrafficPhase::NorthSouthGreen,
        "reset should restore initial traffic phase");
}

TRAFFIC_TEST(simulation_engine_updates_signal_spawn_and_metrics_across_steps) {
    traffic::simulation::SimulationEngine engine;
    engine.initialize(800.0F, 600.0F);

    auto config = engine.state().config();
    config.spawn_probability = 1.0;
    config.spawn_interval_seconds = 0.05;
    config.random_seed = 42U;
    for (auto& demand : config.approach_demand) {
        demand.min_headway_seconds = 0.05;
        demand.target_headway_seconds = 0.05;
    }
    engine.apply_config(config, true);

    for (int i = 0; i < 10; ++i) {
        engine.update(0.05F);
    }

    require(engine.state().metrics().total_vehicles_spawned > 0, "engine should spawn vehicles over repeated steps");
    require(engine.state().active_vehicle_count() > 0, "spawned vehicles should remain active for a while");
    require(engine.state().metrics().average_active_speed >= 0.0, "metrics system should update average speed");
    require(engine.state().traffic_light().phase_elapsed_seconds() > 0.0, "signal system should advance over time");
}

TRAFFIC_TEST(simulation_engine_vehicle_commits_through_yellow_when_past_decision_point) {
    traffic::simulation::SimulationEngine engine;
    engine.initialize(800.0F, 600.0F);

    auto config = engine.state().config();
    config.spawn_probability = 0.0;
    config.reaction_time_min_seconds = 0.0;
    config.reaction_time_max_seconds = 0.0;
    engine.apply_config(config, true);

    engine.state().traffic_light().update(config.north_south_green_seconds);
    require(engine.state().traffic_light().phase() == traffic::domain::TrafficPhase::NorthSouthYellow,
        "precondition failed: light should be NS yellow");

    auto& vehicle = engine.state().emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {430.0, 214.0});
    vehicle.set_speed(10.0);
    vehicle.set_target_speed(10.0);

    const double beforeY = vehicle.y();
    engine.update(0.2F);

    require(vehicle.y() < beforeY, "committed vehicle should continue moving through yellow");
    require(!vehicle.waiting_for_green(), "committed vehicle should not enter waiting-for-green state");
    require(vehicle.operational_state() != traffic::domain::VehicleOperationalState::Queued,
        "committed vehicle should not be classified as queued");
}

TRAFFIC_TEST(simulation_engine_vehicle_brakes_for_yellow_when_before_commit_point) {
    traffic::simulation::SimulationEngine engine;
    engine.initialize(800.0F, 600.0F);

    auto config = engine.state().config();
    config.spawn_probability = 0.0;
    config.reaction_time_min_seconds = 0.0;
    config.reaction_time_max_seconds = 0.0;
    engine.apply_config(config, true);

    engine.state().traffic_light().update(config.north_south_green_seconds);
    require(engine.state().traffic_light().phase() == traffic::domain::TrafficPhase::NorthSouthYellow,
        "precondition failed: light should be NS yellow");

    auto& vehicle = engine.state().emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {430.0, 420.0});
    vehicle.set_speed(10.0);
    vehicle.set_target_speed(10.0);

    engine.update(0.2F);

    require(vehicle.waiting_for_green(), "vehicle before decision point should start waiting at yellow");
    require(vehicle.speed() < 10.0, "vehicle before decision point should brake for yellow");
    require(
        vehicle.operational_state() == traffic::domain::VehicleOperationalState::DeceleratingToStop ||
        vehicle.operational_state() == traffic::domain::VehicleOperationalState::Queued,
        "vehicle braking for yellow should classify as decelerating or queued");
}
