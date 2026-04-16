#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "TrafficLight.hpp"
#include "Vehicle.hpp"

namespace traffic::domain {

struct IntersectionGeometry {
    Point2D center {0.0, 0.0};
    double road_width {7.0};
    double lane_width {3.5};
    double stop_line_distance {10.0};
    double queue_distance {20.0};
    double center_size {12.0};
};

struct ApproachDemandProfile {
    double demand_weight {1.0};
    double spawn_probability_multiplier {1.0};
    double min_headway_seconds {1.2};
    double target_headway_seconds {2.0};
    double motorcycle_share {0.20};
    double truck_share {0.10};
};

struct YellowDecisionConfig {
    double commit_min_time_to_stop_line_seconds {1.0};
    double commit_min_distance_after_stop_line {0.0};
    double comfortable_braking_buffer_meters {2.0};
    bool allow_commit_on_yellow {true};
};

struct QueueModelConfig {
    double queue_join_speed_threshold {2.0};
    double queue_release_speed_threshold {3.0};
    double queue_following_gap_meters {14.0};
    double approach_detection_distance {0.0};
};

struct SimulationConfig {
    double north_south_green_seconds {12.0};
    double east_west_green_seconds {12.0};
    double yellow_seconds {3.0};
    double spawn_probability {0.32};
    double spawn_interval_seconds {0.2};
    double queue_speed_threshold {2.0};
    double wait_speed_threshold {1.5};
    double reaction_time_min_seconds {0.2};
    double reaction_time_max_seconds {0.8};
    std::uint32_t random_seed {1337U};
    std::array<ApproachDemandProfile, 4> approach_demand {};
    YellowDecisionConfig yellow_decision {};
    QueueModelConfig queue_model {};
    IntersectionGeometry geometry {};
};

struct SimulationMetrics {
    int north_south_queue {0};
    int east_west_queue {0};
    int north_queue {0};
    int south_queue {0};
    int east_queue {0};
    int west_queue {0};
    double north_south_wait_time {0.0};
    double east_west_wait_time {0.0};
    double north_wait_time {0.0};
    double south_wait_time {0.0};
    double east_wait_time {0.0};
    double west_wait_time {0.0};
    double average_active_speed {0.0};
    double average_queue_wait_time {0.0};
    double average_approach_delay {0.0};
    double average_queue_distance_to_stop_line {0.0};
    double max_queue_distance_to_stop_line {0.0};
    std::size_t moving_vehicles {0};
    std::size_t stopped_vehicles {0};
    std::size_t vehicles_waiting_for_green {0};
    std::size_t vehicles_in_intersection {0};
    std::size_t vehicles_cruising {0};
    std::size_t vehicles_approaching_signal {0};
    std::size_t vehicles_decelerating {0};
    std::size_t vehicles_queued {0};
    std::size_t vehicles_discharging {0};
    std::size_t vehicles_turning {0};
    std::size_t vehicles_exiting {0};
    std::size_t yellow_commit_candidates {0};
    std::size_t yellow_commit_vehicles {0};
    std::size_t queueing_vehicles_near_stop_line {0};
    int peak_north_south_queue {0};
    int peak_east_west_queue {0};
    int peak_north_queue {0};
    int peak_south_queue {0};
    int peak_east_queue {0};
    int peak_west_queue {0};
    std::uint64_t total_vehicles_spawned {0};
    std::uint64_t completed_vehicles {0};
    double total_completed_wait_time {0.0};
    std::vector<double> completed_wait_times {};
};

class SimulationState {
public:
    explicit SimulationState(SimulationConfig config = {});

    const SimulationConfig& config() const noexcept;
    SimulationConfig& config() noexcept;

    const IntersectionGeometry& geometry() const noexcept;
    IntersectionGeometry& geometry() noexcept;

    const TrafficLight& traffic_light() const noexcept;
    TrafficLight& traffic_light() noexcept;

    const std::vector<Vehicle>& vehicles() const noexcept;
    std::vector<Vehicle>& vehicles() noexcept;

    const SimulationMetrics& metrics() const noexcept;
    SimulationMetrics& metrics() noexcept;

    void reset() noexcept;

    Vehicle& add_vehicle(Vehicle vehicle);
    Vehicle& emplace_vehicle(
        VehicleType type,
        Direction direction,
        Point2D position,
        TurnIntent turn_intent = TurnIntent::Straight);
    void remove_inactive_vehicles() noexcept;
    void mark_completed(const Vehicle& vehicle);

    void update_metrics();
    bool is_in_queue_zone(const Vehicle& vehicle) const noexcept;
    bool is_in_approach_queue_zone(const Vehicle& vehicle) const noexcept;
    bool is_vehicle_queued(const Vehicle& vehicle) const noexcept;
    bool is_vehicle_committed_through_yellow(const Vehicle& vehicle) const noexcept;
    double distance_to_stop_line(const Vehicle& vehicle) const noexcept;
    double time_to_stop_line(const Vehicle& vehicle) const noexcept;
    double effective_spawn_probability(Direction direction) const noexcept;
    const ApproachDemandProfile& demand_profile(Direction direction) const noexcept;
    ApproachDemandProfile& demand_profile(Direction direction) noexcept;

    double average_completed_wait_time() const noexcept;
    std::size_t active_vehicle_count() const noexcept;
    std::size_t queued_vehicle_count(Direction direction) const noexcept;

private:
    static std::size_t direction_index(Direction direction) noexcept;
    bool is_vertical(Direction direction) const noexcept;
    double average_wait_time_for(Direction direction) const noexcept;

    SimulationConfig config_ {};
    TrafficLight traffic_light_ {};
    std::vector<Vehicle> vehicles_ {};
    SimulationMetrics metrics_ {};
    std::uint64_t next_vehicle_id_ {1};
};

}  // namespace traffic::domain
