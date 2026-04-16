#include "simulation/SpawnSystem.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace traffic::simulation::systems {

namespace {

std::size_t direction_index(domain::Direction direction) noexcept {
    switch (direction) {
    case domain::Direction::North:
        return 0;
    case domain::Direction::South:
        return 1;
    case domain::Direction::East:
        return 2;
    case domain::Direction::West:
        return 3;
    }
    return 0;
}

domain::Point2D spawn_point_for(
    const domain::SimulationState& state,
    const SimulationRuntime& runtime,
    domain::Direction direction) noexcept {
    const auto& center = state.geometry().center;
    const double half_lane = state.geometry().lane_width * 0.5;

    switch (direction) {
    case domain::Direction::North:
        return {center.x + half_lane, runtime.sim_height + runtime.offscreen_margin * 0.5};
    case domain::Direction::South:
        return {center.x - half_lane, -runtime.offscreen_margin * 0.5};
    case domain::Direction::East:
        return {-runtime.offscreen_margin * 0.5, center.y + half_lane};
    case domain::Direction::West:
        return {runtime.sim_width + runtime.offscreen_margin * 0.5, center.y - half_lane};
    }

    return {};
}

bool spawn_blocked(
    const domain::SimulationState& state,
    domain::Direction direction,
    const domain::Point2D& spawn_point) noexcept {
    for (const auto& vehicle : state.vehicles()) {
        if (vehicle.direction() != direction) {
            continue;
        }

        const double dx = vehicle.x() - spawn_point.x;
        const double dy = vehicle.y() - spawn_point.y;
        if (std::sqrt(dx * dx + dy * dy) < 85.0) {
            return true;
        }
    }

    return false;
}

domain::VehicleType pick_vehicle_type(
    const domain::ApproachDemandProfile& profile,
    SimulationRuntime& runtime) {
    const auto roll = runtime.probability(runtime.rng);
    const double motorcycle_share = std::clamp(profile.motorcycle_share, 0.0, 1.0);
    const double truck_share = std::clamp(profile.truck_share, 0.0, 1.0 - motorcycle_share);

    if (roll < motorcycle_share) {
        return domain::VehicleType::Motorcycle;
    }
    if (roll < motorcycle_share + truck_share) {
        return domain::VehicleType::Truck;
    }
    return domain::VehicleType::Car;
}

}  // namespace

void spawn_vehicles(domain::SimulationState& state, SimulationRuntime& runtime, double dt) {
    runtime.spawn_accumulator_seconds += dt;

    const std::array directions {
        domain::Direction::North,
        domain::Direction::South,
        domain::Direction::East,
        domain::Direction::West,
    };

    for (const auto direction : directions) {
        const auto& profile = state.demand_profile(direction);
        auto& approach_accumulator = runtime.approach_spawn_accumulator_seconds[direction_index(direction)];
        approach_accumulator += dt;

        const double min_headway = std::max(0.1, profile.min_headway_seconds);
        const double target_headway = std::max(min_headway, profile.target_headway_seconds);
        if (approach_accumulator < min_headway) {
            continue;
        }

        const double readiness = target_headway <= min_headway
            ? 1.0
            : std::clamp((approach_accumulator - min_headway) / (target_headway - min_headway), 0.0, 1.0);
        const double effective_probability =
            state.effective_spawn_probability(direction) * (0.35 + 0.65 * readiness);
        const bool forced_attempt = approach_accumulator >= target_headway;
        if (!forced_attempt && runtime.probability(runtime.rng) > effective_probability) {
            continue;
        }

        const auto spawn_point = spawn_point_for(state, runtime, direction);
        if (spawn_blocked(state, direction, spawn_point)) {
            continue;
        }

        auto& vehicle = state.emplace_vehicle(
            pick_vehicle_type(profile, runtime),
            direction,
            spawn_point,
            domain::TurnIntent::Straight);
        vehicle.set_reaction_delay(runtime.reaction_delay(runtime.rng));
        approach_accumulator = 0.0;
    }
}

}  // namespace traffic::simulation::systems
