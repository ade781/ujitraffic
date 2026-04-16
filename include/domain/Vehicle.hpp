#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

#include "TrafficTypes.hpp"

namespace traffic::domain {

struct Point2D {
    double x {0.0};
    double y {0.0};
};

struct VehicleProfile {
    double length {4.5};
    double width {1.8};
    double max_speed {13.9};
    double acceleration {2.0};
    double deceleration {4.0};
};

enum class TurnIntent {
    Straight,
    Left,
};

enum class VehicleOperationalState {
    Spawning,
    Cruising,
    ApproachingSignal,
    DeceleratingToStop,
    Queued,
    Discharging,
    CrossingIntersection,
    Turning,
    Exiting,
    Completed,
};

class Vehicle {
public:
    using Id = std::uint64_t;

    Vehicle() = default;

    Vehicle(
        Id id,
        VehicleType type,
        Direction direction,
        Point2D position,
        VehicleProfile profile = {},
        TurnIntent turn_intent = TurnIntent::Straight) noexcept;

    Id id() const noexcept;
    VehicleType type() const noexcept;
    Direction direction() const noexcept;
    void set_direction(Direction direction) noexcept;

    const Point2D& position() const noexcept;
    void set_position(Point2D position) noexcept;
    double x() const noexcept;
    double y() const noexcept;

    const VehicleProfile& profile() const noexcept;
    void set_profile(VehicleProfile profile) noexcept;

    double speed() const noexcept;
    void set_speed(double speed) noexcept;
    double target_speed() const noexcept;
    void set_target_speed(double target_speed) noexcept;

    double wait_time() const noexcept;
    void add_wait_time(double dt) noexcept;
    void reset_wait_time() noexcept;

    bool active() const noexcept;
    void set_active(bool active) noexcept;

    TurnIntent turn_intent() const noexcept;
    void set_turn_intent(TurnIntent turn_intent) noexcept;

    bool turning() const noexcept;
    void set_turning(bool turning) noexcept;

    bool waiting_for_green() const noexcept;
    void set_waiting_for_green(bool waiting_for_green) noexcept;

    bool has_reacted() const noexcept;
    void set_has_reacted(bool has_reacted) noexcept;

    double reaction_timer() const noexcept;
    void reset_reaction_timer() noexcept;
    void add_reaction_time(double dt) noexcept;

    double reaction_delay() const noexcept;
    void set_reaction_delay(double delay) noexcept;

    VehicleOperationalState operational_state() const noexcept;
    void set_operational_state(VehicleOperationalState state) noexcept;
    void refresh_operational_state(
        const Point2D& intersection_center,
        double stop_line_distance,
        double center_size,
        double queue_speed_threshold,
        double wait_speed_threshold) noexcept;

    double longitudinal_position() const noexcept;
    double lateral_position() const noexcept;
    double front_bumper() const noexcept;
    double rear_bumper() const noexcept;
    double stopping_distance(double braking_buffer = 0.0) const noexcept;

    bool same_lane(const Vehicle& other, double lane_tolerance) const noexcept;
    bool is_ahead_of(const Vehicle& other, double lane_tolerance) const noexcept;
    double gap_to(const Vehicle& other) const noexcept;

    bool should_stop_for_light(
        LightState light_state,
        const Point2D& intersection_center,
        double stop_line_distance) const noexcept;

    bool is_at_intersection_center(
        const Point2D& intersection_center,
        double center_size) const noexcept;
    bool should_turn_at_center(
        const Point2D& intersection_center,
        double center_size) const noexcept;
    Direction next_direction_after_turn() const noexcept;

    void advance(double dt) noexcept;
    void accelerate_towards_target(double dt) noexcept;
    void brake_to_stop(double dt) noexcept;
    void clamp_speed() noexcept;

    static VehicleProfile default_profile(VehicleType type) noexcept;

private:
    static double half_length(const VehicleProfile& profile) noexcept;
    bool is_downstream_of_intersection(
        const Point2D& intersection_center,
        double center_size) const noexcept;

    Id id_ {0};
    VehicleType type_ {VehicleType::Car};
    Direction direction_ {Direction::North};
    Point2D position_ {};
    VehicleProfile profile_ {};
    double speed_ {0.0};
    double target_speed_ {0.0};
    double wait_time_ {0.0};
    bool active_ {true};
    TurnIntent turn_intent_ {TurnIntent::Straight};
    bool turning_ {false};
    bool waiting_for_green_ {false};
    bool has_reacted_ {true};
    double reaction_timer_ {0.0};
    double reaction_delay_ {0.0};
    VehicleOperationalState operational_state_ {VehicleOperationalState::Spawning};
};

}  // namespace traffic::domain
