#include "domain/ScenarioFactory.hpp"

#include <utility>

namespace traffic::domain {

IntersectionGeometry ScenarioFactory::default_geometry() noexcept {
    return IntersectionGeometry {
        {0.0, 0.0},
        7.0,
        3.5,
        10.0,
        20.0,
        12.0,
    };
}

std::array<VehicleProfile, 3> ScenarioFactory::default_vehicle_profiles() noexcept {
    return {
        Vehicle::default_profile(VehicleType::Car),
        Vehicle::default_profile(VehicleType::Motorcycle),
        Vehicle::default_profile(VehicleType::Truck),
    };
}

VehicleProfile ScenarioFactory::default_vehicle_profile(VehicleType type) noexcept {
    return Vehicle::default_profile(type);
}

SimulationConfig ScenarioFactory::default_timing_and_demand() noexcept {
    auto config = SimulationConfig {};
    config.geometry = default_geometry();
    config.north_south_green_seconds = 12.0;
    config.east_west_green_seconds = 12.0;
    config.yellow_seconds = 3.0;
    config.spawn_probability = 0.32;
    config.spawn_interval_seconds = 0.2;
    config.queue_speed_threshold = 2.0;
    config.wait_speed_threshold = 1.5;
    config.reaction_time_min_seconds = 0.2;
    config.reaction_time_max_seconds = 0.8;
    config.random_seed = 1337U;
    config.approach_demand[0].demand_weight = 1.0;
    config.approach_demand[1].demand_weight = 1.0;
    config.approach_demand[2].demand_weight = 1.0;
    config.approach_demand[3].demand_weight = 1.0;
    config.approach_demand[0].target_headway_seconds = 2.1;
    config.approach_demand[1].target_headway_seconds = 2.1;
    config.approach_demand[2].target_headway_seconds = 2.4;
    config.approach_demand[3].target_headway_seconds = 2.4;
    config.yellow_decision.commit_min_time_to_stop_line_seconds = 1.0;
    config.yellow_decision.comfortable_braking_buffer_meters = 2.0;
    config.queue_model.queue_join_speed_threshold = 2.0;
    config.queue_model.queue_release_speed_threshold = 3.0;
    config.queue_model.queue_following_gap_meters = 14.0;
    config.queue_model.approach_detection_distance = config.geometry.queue_distance;
    return config;
}

SimulationConfig ScenarioFactory::timing_and_demand(DemandPreset preset) noexcept {
    auto config = default_timing_and_demand();
    switch (preset) {
    case DemandPreset::Balanced:
        break;
    case DemandPreset::HeavyNorthSouth:
        config.north_south_green_seconds = 42.0;
        config.east_west_green_seconds = 24.0;
        config.spawn_probability = 0.36;
        config.spawn_interval_seconds = 0.18;
        config.approach_demand[0].demand_weight = 1.2;
        config.approach_demand[1].demand_weight = 1.2;
        config.approach_demand[2].demand_weight = 0.8;
        config.approach_demand[3].demand_weight = 0.8;
        break;
    case DemandPreset::HeavyEastWest:
        config.north_south_green_seconds = 24.0;
        config.east_west_green_seconds = 42.0;
        config.spawn_probability = 0.36;
        config.spawn_interval_seconds = 0.18;
        config.approach_demand[0].demand_weight = 0.8;
        config.approach_demand[1].demand_weight = 0.8;
        config.approach_demand[2].demand_weight = 1.2;
        config.approach_demand[3].demand_weight = 1.2;
        break;
    case DemandPreset::Saturated:
        config.north_south_green_seconds = 12.0;
        config.east_west_green_seconds = 12.0;
        config.spawn_probability = 0.48;
        config.spawn_interval_seconds = 0.12;
        for (auto& demand : config.approach_demand) {
            demand.demand_weight = 1.1;
            demand.target_headway_seconds = 1.5;
            demand.min_headway_seconds = 0.9;
        }
        break;
    }
    return config;
}

ScenarioPreset ScenarioFactory::default_single_intersection_preset() noexcept {
    ScenarioPreset preset {};
    preset.config = default_timing_and_demand();
    preset.vehicle_profiles = default_vehicle_profiles();
    return preset;
}

ScenarioPreset ScenarioFactory::make_preset(DemandPreset preset) noexcept {
    ScenarioPreset scenario = default_single_intersection_preset();
    scenario.config = timing_and_demand(preset);
    return scenario;
}

SimulationState ScenarioFactory::make_default_single_intersection_state() noexcept {
    return make_state(default_single_intersection_preset());
}

SimulationState ScenarioFactory::make_state(const ScenarioPreset& preset) noexcept {
    return SimulationState {preset.config};
}

}  // namespace traffic::domain
