#include "TestSupport.hpp"

#include "domain/ScenarioFactory.hpp"
#include "domain/SimulationState.hpp"

using traffic::tests::require;
using traffic::tests::require_near;

TRAFFIC_TEST(simulation_state_reset_clears_runtime_but_keeps_config) {
    traffic::domain::SimulationConfig config;
    config.north_south_green_seconds = 4.0;
    config.east_west_green_seconds = 6.0;
    config.yellow_seconds = 1.5;
    config.geometry.center = {100.0, 120.0};
    config.geometry.queue_distance = 60.0;

    traffic::domain::SimulationState state(config);
    auto& vehicle = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 140.0});
    vehicle.add_wait_time(3.0);
    state.mark_completed(vehicle);
    state.traffic_light().update(4.0);

    require(state.active_vehicle_count() == 1, "precondition failed: state should contain one vehicle");
    require(state.metrics().completed_vehicles == 1, "precondition failed: completed metrics should be populated");

    state.reset();

    require(state.active_vehicle_count() == 0, "reset should remove all vehicles");
    require(state.metrics().completed_vehicles == 0, "reset should clear completed count");
    require(state.metrics().completed_wait_times.empty(), "reset should clear completed wait samples");
    require(state.traffic_light().phase() == traffic::domain::TrafficPhase::NorthSouthGreen,
        "reset should restore initial light phase");
    require_near(state.traffic_light().north_south_green_seconds(), 4.0, 1e-9, "NS green should be preserved");
    require_near(state.traffic_light().east_west_green_seconds(), 6.0, 1e-9, "EW green should be preserved");
    require_near(state.traffic_light().yellow_seconds(), 1.5, 1e-9, "yellow should be preserved");
}

TRAFFIC_TEST(simulation_state_updates_queue_and_wait_metrics) {
    traffic::domain::SimulationConfig config;
    config.geometry.center = {100.0, 100.0};
    config.geometry.queue_distance = 50.0;
    config.queue_speed_threshold = 5.0;

    traffic::domain::SimulationState state(config);

    auto& north = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {102.0, 120.0});
    north.set_speed(0.0);
    north.add_wait_time(10.0);

    auto& south = state.emplace_vehicle(
        traffic::domain::VehicleType::Truck,
        traffic::domain::Direction::South,
        {98.0, 80.0});
    south.set_speed(4.0);
    south.add_wait_time(6.0);

    auto& east = state.emplace_vehicle(
        traffic::domain::VehicleType::Motorcycle,
        traffic::domain::Direction::East,
        {70.0, 102.0});
    east.set_speed(0.0);
    east.add_wait_time(8.0);

    auto& west_fast = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::West,
        {130.0, 98.0});
    west_fast.set_speed(7.0);
    west_fast.add_wait_time(12.0);

    auto& north_outside = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {102.0, 170.0});
    north_outside.set_speed(0.0);
    north_outside.add_wait_time(20.0);

    state.update_metrics();

    require(state.metrics().north_south_queue == 2, "north/south queue should count two waiting vehicles");
    require(state.metrics().east_west_queue == 1, "east/west queue should count one waiting vehicle");
    require_near(state.metrics().north_south_wait_time, 8.0, 1e-9, "north/south average wait mismatch");
    require_near(state.metrics().east_west_wait_time, 8.0, 1e-9, "east/west average wait mismatch");
    require_near(state.metrics().average_queue_wait_time, 8.0, 1e-9, "average queue wait mismatch");
    require(state.queued_vehicle_count(traffic::domain::Direction::North) == 1, "north queue count mismatch");
    require(state.queued_vehicle_count(traffic::domain::Direction::South) == 1, "south queue count mismatch");
    require(state.queued_vehicle_count(traffic::domain::Direction::East) == 1, "east queue count mismatch");
    require(state.queued_vehicle_count(traffic::domain::Direction::West) == 0, "west queue count mismatch");
    require(state.metrics().vehicles_queued >= 1, "vehicle state counts should classify queued vehicles");
    require(state.metrics().peak_north_south_queue == 2, "peak NS queue should update");
    require(state.metrics().peak_east_west_queue == 1, "peak EW queue should update");
}

TRAFFIC_TEST(simulation_state_completion_metrics_track_finished_vehicles) {
    traffic::domain::SimulationState state;

    auto& first = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::East,
        {0.0, 0.0});
    first.add_wait_time(4.0);
    state.mark_completed(first);

    auto& second = state.emplace_vehicle(
        traffic::domain::VehicleType::Truck,
        traffic::domain::Direction::West,
        {0.0, 0.0});
    second.add_wait_time(8.0);
    state.mark_completed(second);

    require(state.metrics().completed_vehicles == 2, "completed vehicle count mismatch");
    require_near(state.metrics().total_completed_wait_time, 12.0, 1e-9, "completed wait total mismatch");
    require(state.metrics().completed_wait_times.size() == 2, "completed wait sample size mismatch");
    require_near(state.average_completed_wait_time(), 6.0, 1e-9, "completed wait average mismatch");
}

TRAFFIC_TEST(simulation_state_remove_inactive_vehicles_prunes_container) {
    traffic::domain::SimulationState state;

    state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {0.0, 0.0});
    auto& inactive = state.emplace_vehicle(
        traffic::domain::VehicleType::Truck,
        traffic::domain::Direction::South,
        {0.0, 0.0});
    inactive.set_active(false);

    state.remove_inactive_vehicles();

    require(state.active_vehicle_count() == 1, "inactive vehicle should be removed");
    require(state.vehicles().front().type() == traffic::domain::VehicleType::Car,
        "active vehicle should remain after pruning");
    require(state.vehicles().front().direction() == traffic::domain::Direction::North,
        "remaining vehicle should preserve its original direction");
}

TRAFFIC_TEST(scenario_factory_presets_expose_expected_demand_shapes) {
    const auto balanced = traffic::domain::ScenarioFactory::make_preset(traffic::domain::DemandPreset::Balanced);
    const auto heavy_ns = traffic::domain::ScenarioFactory::make_preset(traffic::domain::DemandPreset::HeavyNorthSouth);
    const auto heavy_ew = traffic::domain::ScenarioFactory::make_preset(traffic::domain::DemandPreset::HeavyEastWest);
    const auto saturated = traffic::domain::ScenarioFactory::make_preset(traffic::domain::DemandPreset::Saturated);

    require_near(balanced.config.spawn_probability, 0.32, 1e-9, "balanced preset spawn probability mismatch");
    require(heavy_ns.config.north_south_green_seconds > heavy_ns.config.east_west_green_seconds,
        "heavy NS preset should bias NS green");
    require(heavy_ew.config.east_west_green_seconds > heavy_ew.config.north_south_green_seconds,
        "heavy EW preset should bias EW green");
    require(saturated.config.spawn_probability > balanced.config.spawn_probability,
        "saturated preset should raise demand");
    require(saturated.config.spawn_interval_seconds < balanced.config.spawn_interval_seconds,
        "saturated preset should spawn more frequently");
}

TRAFFIC_TEST(simulation_state_observability_metrics_follow_operational_states) {
    traffic::domain::SimulationConfig config;
    config.geometry.center = {100.0, 100.0};
    config.geometry.stop_line_distance = 10.0;
    config.geometry.queue_distance = 80.0;
    config.geometry.center_size = 20.0;
    config.queue_speed_threshold = 5.0;
    config.wait_speed_threshold = 1.0;

    traffic::domain::SimulationState state(config);

    auto& queued = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 130.0});
    queued.set_speed(0.0);
    queued.set_waiting_for_green(true);
    queued.add_wait_time(9.0);

    auto& cruising = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::East,
        {40.0, 80.0});
    cruising.set_speed(8.0);
    cruising.set_target_speed(8.0);

    auto& turning = state.emplace_vehicle(
        traffic::domain::VehicleType::Motorcycle,
        traffic::domain::Direction::West,
        {100.0, 100.0},
        traffic::domain::TurnIntent::Left);
    turning.set_speed(3.0);
    turning.set_turning(true);

    state.update_metrics();

    require(state.metrics().vehicles_queued >= 1, "queued vehicle should populate queued metric");
    require(state.metrics().vehicles_waiting_for_green >= 1, "waiting-for-green metric should update");
    require(state.metrics().vehicles_cruising >= 1, "cruising vehicle should populate cruising metric");
    require(state.metrics().vehicles_turning >= 1, "turning vehicle should populate turning metric");
    require(state.metrics().vehicles_in_intersection >= 1, "turning vehicle should also count in intersection");
    require_near(state.metrics().average_queue_wait_time, 9.0, 1e-9, "average queue wait mismatch");
    require(state.metrics().peak_north_south_queue >= state.metrics().north_south_queue,
        "peak queue should not fall below current queue");
}

TRAFFIC_TEST(simulation_state_queue_zone_uses_front_bumper_based_boundaries) {
    traffic::domain::SimulationConfig config;
    config.geometry.center = {100.0, 100.0};
    config.geometry.stop_line_distance = 20.0;
    config.geometry.queue_distance = 40.0;

    traffic::domain::SimulationState state(config);

    auto& at_entry = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 122.25});
    auto& before_entry = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 121.0});
    auto& beyond_queue = state.emplace_vehicle(
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 165.0});

    require(state.is_in_approach_queue_zone(at_entry), "front bumper at queue entry should count inside approach zone");
    require(!state.is_in_approach_queue_zone(before_entry), "vehicle before queue entry should stay outside approach zone");
    require(!state.is_in_approach_queue_zone(beyond_queue), "vehicle beyond queue tail should stay outside approach zone");
}
