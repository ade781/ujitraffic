#include "simulation/SimulationEngine.hpp"

#include <algorithm>

#include "domain/ScenarioFactory.hpp"
#include "simulation/LifecycleSystem.hpp"
#include "simulation/SignalSystem.hpp"
#include "simulation/SpawnSystem.hpp"
#include "simulation/VehicleBehaviorSystem.hpp"

namespace traffic::simulation {

namespace {
constexpr double kSafeGap = 14.0;

double clamp_positive(double value, double fallback) {
    return value > 0.0 ? value : fallback;
}
}

void SimulationEngine::initialize(float simWidth, float simHeight) {
    runtime_.sim_width = simWidth;
    runtime_.sim_height = simHeight;

    state_ = domain::ScenarioFactory::make_default_single_intersection_state();

    auto& config = state_.config();
    config.geometry.center = {runtime_.sim_width * 0.5, runtime_.sim_height * 0.5};
    config.geometry.road_width = 180.0;
    config.geometry.lane_width = 60.0;
    config.geometry.stop_line_distance = 90.0;
    config.geometry.queue_distance = 220.0;
    config.geometry.center_size = 120.0;

    apply_config(config, true);
}

void SimulationEngine::update(float dt) {
    const double step = std::max(0.0F, dt);
    systems::update_signal(state_, step);
    systems::spawn_vehicles(state_, runtime_, step);
    systems::update_vehicle_behavior(state_, runtime_, step);
    systems::finalize_step(state_, runtime_);
}

void SimulationEngine::reset() {
    state_.reset();
    runtime_.spawn_accumulator_seconds = 0.0;
    runtime_.approach_spawn_accumulator_seconds.fill(0.0);
    runtime_.rng.seed(state_.config().random_seed);
}

void SimulationEngine::apply_config(const domain::SimulationConfig& config, bool resetSimulation) {
    auto updated = config;
    updated.spawn_probability = std::clamp(updated.spawn_probability, 0.0, 1.0);
    updated.spawn_interval_seconds = clamp_positive(updated.spawn_interval_seconds, 0.2);
    updated.north_south_green_seconds = clamp_positive(updated.north_south_green_seconds, 1.0);
    updated.east_west_green_seconds = clamp_positive(updated.east_west_green_seconds, 1.0);
    updated.yellow_seconds = clamp_positive(updated.yellow_seconds, 1.0);
    updated.queue_speed_threshold = std::max(0.0, updated.queue_speed_threshold);
    updated.wait_speed_threshold = std::max(0.0, updated.wait_speed_threshold);
    updated.reaction_time_min_seconds = std::max(0.0, updated.reaction_time_min_seconds);
    updated.reaction_time_max_seconds =
        std::max(updated.reaction_time_min_seconds, updated.reaction_time_max_seconds);

    state_.config() = updated;
    state_.traffic_light().set_north_south_green_seconds(updated.north_south_green_seconds);
    state_.traffic_light().set_east_west_green_seconds(updated.east_west_green_seconds);
    state_.traffic_light().set_yellow_seconds(updated.yellow_seconds);
    runtime_.spawn_interval_seconds = updated.spawn_interval_seconds;
    runtime_.reaction_delay = std::uniform_real_distribution<double>(
        updated.reaction_time_min_seconds,
        updated.reaction_time_max_seconds);
    runtime_.rng.seed(updated.random_seed);

    if (resetSimulation) {
        reset();
    }
}

const domain::SimulationState& SimulationEngine::state() const noexcept {
    return state_;
}

domain::SimulationState& SimulationEngine::state() noexcept {
    return state_;
}

}  // namespace traffic::simulation
