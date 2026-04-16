#include "simulation/VehicleBehaviorSystem.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace traffic::simulation::systems {

namespace {

bool find_leader(
    const domain::SimulationState& state,
    const domain::Vehicle& vehicle,
    const domain::Vehicle** leader) noexcept {
    const domain::Vehicle* nearest = nullptr;
    double nearest_gap = std::numeric_limits<double>::infinity();

    for (const auto& other : state.vehicles()) {
        if (vehicle.id() == other.id()) {
            continue;
        }
        if (!vehicle.is_ahead_of(other, state.geometry().lane_width * 0.45)) {
            continue;
        }

        const double gap = vehicle.gap_to(other);
        if (gap > 0.0 && gap < nearest_gap) {
            nearest_gap = gap;
            nearest = &other;
        }
    }

    if (leader != nullptr) {
        *leader = nearest;
    }
    return nearest != nullptr;
}

bool should_stop_for_signal(
    const domain::SimulationState& state,
    const domain::Vehicle& vehicle,
    domain::LightState light_state) noexcept {
    const double distance_to_stop_line = state.distance_to_stop_line(vehicle);
    if (distance_to_stop_line <= 0.0) {
        return false;
    }

    if (light_state == domain::LightState::Red) {
        return true;
    }

    if (light_state == domain::LightState::Yellow) {
        return !state.is_vehicle_committed_through_yellow(vehicle);
    }

    return false;
}

double desired_stop_approach_speed(
    const domain::SimulationState& state,
    const domain::Vehicle& vehicle) noexcept {
    const double distance_to_stop_line = std::max(0.0, state.distance_to_stop_line(vehicle));
    const double usable_distance = std::max(
        0.0,
        distance_to_stop_line - state.config().yellow_decision.comfortable_braking_buffer_meters);
    const double approach_speed = std::sqrt(
        2.0 * std::max(vehicle.profile().deceleration, 0.1) * usable_distance);
    return std::clamp(approach_speed, 0.0, vehicle.profile().max_speed);
}

}  // namespace

void update_vehicle_behavior(domain::SimulationState& state, SimulationRuntime& runtime, double dt) {
    for (auto& vehicle : state.vehicles()) {
        const auto light_state = state.traffic_light().state_for(vehicle.direction());
        const bool just_green = state.traffic_light().just_turned_green(vehicle.direction());
        const double distance_to_stop_line = state.distance_to_stop_line(vehicle);

        if (just_green && vehicle.waiting_for_green()) {
            vehicle.set_has_reacted(false);
            vehicle.reset_reaction_timer();
            vehicle.set_reaction_delay(runtime.reaction_delay(runtime.rng));
        }

        if (!vehicle.has_reacted()) {
            vehicle.add_reaction_time(dt);
            if (vehicle.reaction_timer() >= vehicle.reaction_delay()) {
                vehicle.set_has_reacted(true);
                vehicle.set_waiting_for_green(false);
            }
        }

        bool should_stop = false;
        if (should_stop_for_signal(state, vehicle, light_state)) {
            should_stop = true;
            vehicle.set_waiting_for_green(true);
        } else if (light_state == domain::LightState::Green ||
                   (light_state == domain::LightState::Yellow &&
                    state.is_vehicle_committed_through_yellow(vehicle))) {
            vehicle.set_waiting_for_green(false);
        }

        const domain::Vehicle* leader = nullptr;
        if (find_leader(state, vehicle, &leader) && leader != nullptr) {
            const double gap = vehicle.gap_to(*leader);
            const double safe_gap = state.config().queue_model.queue_following_gap_meters;
            if (gap < safe_gap) {
                should_stop = true;
            } else if (gap < safe_gap * 3.0) {
                const double factor = std::clamp((gap - safe_gap) / (safe_gap * 2.0), 0.2, 1.0);
                vehicle.set_target_speed(vehicle.profile().max_speed * factor);
            } else {
                vehicle.set_target_speed(vehicle.profile().max_speed);
            }
        } else {
            vehicle.set_target_speed(vehicle.profile().max_speed);
        }

        if (!vehicle.has_reacted() && vehicle.waiting_for_green()) {
            should_stop = true;
        }

        if (should_stop && distance_to_stop_line > 0.0) {
            vehicle.set_target_speed(std::min(vehicle.target_speed(), desired_stop_approach_speed(state, vehicle)));
        } else if (state.traffic_light().is_yellow_for(vehicle.direction()) &&
                   state.is_vehicle_committed_through_yellow(vehicle)) {
            vehicle.set_target_speed(vehicle.profile().max_speed);
        } else if (state.is_vehicle_queued(vehicle) &&
                   vehicle.speed() <= state.config().queue_model.queue_release_speed_threshold) {
            vehicle.set_target_speed(std::min(vehicle.profile().max_speed, vehicle.target_speed() + 1.0));
        }

        if (vehicle.should_turn_at_center(state.geometry().center, state.geometry().center_size)) {
            vehicle.set_turning(true);
            vehicle.set_direction(vehicle.next_direction_after_turn());
        }

        if (should_stop || vehicle.speed() > vehicle.target_speed() + 0.25) {
            vehicle.brake_to_stop(dt);
        } else {
            vehicle.accelerate_towards_target(dt);
        }

        vehicle.advance(dt);

        if (vehicle.speed() <= state.config().wait_speed_threshold) {
            vehicle.add_wait_time(dt);
        }
    }
}

}  // namespace traffic::simulation::systems
