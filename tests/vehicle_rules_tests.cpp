#include "TestSupport.hpp"

#include "domain/Vehicle.hpp"

using traffic::tests::require;
using traffic::tests::require_near;

TRAFFIC_TEST(vehicle_obeys_red_and_yellow_stop_rules) {
    traffic::domain::Vehicle northbound {
        1,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 140.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
        traffic::domain::TurnIntent::Straight,
    };

    require(
        northbound.should_stop_for_light(traffic::domain::LightState::Red, {100.0, 100.0}, 20.0),
        "northbound straight vehicle should stop for red before stop line");
    require(
        northbound.should_stop_for_light(traffic::domain::LightState::Yellow, {100.0, 100.0}, 20.0),
        "northbound straight vehicle should stop for yellow before stop line");
    require(
        !northbound.should_stop_for_light(traffic::domain::LightState::Green, {100.0, 100.0}, 20.0),
        "northbound vehicle should not stop for green");
}

TRAFFIC_TEST(vehicle_yellow_commit_threshold_distinguishes_near_and_far_approach) {
    traffic::domain::Vehicle near_stop_line {
        3,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 84.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
        traffic::domain::TurnIntent::Straight,
    };
    traffic::domain::Vehicle far_from_stop_line {
        4,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 140.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
        traffic::domain::TurnIntent::Straight,
    };

    require(
        !near_stop_line.should_stop_for_light(traffic::domain::LightState::Yellow, {100.0, 100.0}, 20.0),
        "vehicle close to stop line should commit through yellow");
    require(
        far_from_stop_line.should_stop_for_light(traffic::domain::LightState::Yellow, {100.0, 100.0}, 20.0),
        "vehicle with enough distance should still stop for yellow");
}

TRAFFIC_TEST(vehicle_left_turn_still_stops_for_red_in_current_rule) {
    traffic::domain::Vehicle leftTurner {
        2,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 140.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
        traffic::domain::TurnIntent::Left,
    };

    require(
        leftTurner.should_stop_for_light(traffic::domain::LightState::Red, {100.0, 100.0}, 20.0),
        "left-turn vehicle should still stop for red under current rules");
}

TRAFFIC_TEST(vehicle_lane_and_gap_rules_match_direction_geometry) {
    traffic::domain::Vehicle follower {
        10,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {130.0, 200.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
    };
    traffic::domain::Vehicle leader_same_lane {
        11,
        traffic::domain::VehicleType::Truck,
        traffic::domain::Direction::North,
        {130.0, 180.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Truck),
    };
    traffic::domain::Vehicle other_lane {
        12,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {170.0, 180.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
    };

    require(follower.same_lane(leader_same_lane, 1.0), "vehicles aligned on x should be in same northbound lane");
    require(!follower.same_lane(other_lane, 1.0), "vehicles in different x position should not be in same lane");
    require(follower.is_ahead_of(leader_same_lane, 1.0), "leader should be detected ahead for northbound flow");
    require_near(follower.gap_to(leader_same_lane), 26.25, 1e-9, "northbound bumper gap mismatch");
}

TRAFFIC_TEST(vehicle_turning_and_motion_rules_follow_current_model) {
    traffic::domain::Vehicle leftTurner {
        30,
        traffic::domain::VehicleType::Motorcycle,
        traffic::domain::Direction::East,
        {100.0, 100.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Motorcycle),
        traffic::domain::TurnIntent::Left,
    };

    require(leftTurner.is_at_intersection_center({100.0, 100.0}, 20.0), "vehicle should be inside center box");
    require(leftTurner.should_turn_at_center({100.0, 100.0}, 20.0), "left turn should trigger at center");
    require(leftTurner.next_direction_after_turn() == traffic::domain::Direction::North,
        "eastbound left turn should become northbound");

    leftTurner.set_direction(leftTurner.next_direction_after_turn());
    leftTurner.set_speed(10.0);
    leftTurner.advance(0.5);

    require_near(leftTurner.x(), 100.0, 1e-9, "northbound advance should keep x fixed");
    require_near(leftTurner.y(), 95.0, 1e-9, "northbound advance should reduce y");
}

TRAFFIC_TEST(vehicle_speed_control_respects_profile_limits) {
    traffic::domain::Vehicle vehicle {
        50,
        traffic::domain::VehicleType::Truck,
        traffic::domain::Direction::West,
        {200.0, 50.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Truck),
    };

    vehicle.set_target_speed(100.0);
    vehicle.accelerate_towards_target(10.0);
    require_near(vehicle.speed(), vehicle.profile().max_speed, 1e-9, "speed should clamp to max speed");

    vehicle.brake_to_stop(10.0);
    require_near(vehicle.speed(), 0.0, 1e-9, "brake_to_stop should clamp at zero");
}

TRAFFIC_TEST(vehicle_operational_state_refreshes_from_runtime_conditions) {
    traffic::domain::Vehicle queued {
        60,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 130.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
    };
    queued.set_waiting_for_green(true);
    queued.set_speed(0.0);
    queued.refresh_operational_state({100.0, 100.0}, 20.0, 20.0, 2.0, 0.5);
    require(
        queued.operational_state() == traffic::domain::VehicleOperationalState::Queued,
        "waiting vehicle near stop line should be classified as queued");

    traffic::domain::Vehicle crossing {
        61,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::East,
        {100.0, 100.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
    };
    crossing.set_speed(5.0);
    crossing.refresh_operational_state({100.0, 100.0}, 20.0, 20.0, 2.0, 0.5);
    require(
        crossing.operational_state() == traffic::domain::VehicleOperationalState::CrossingIntersection,
        "vehicle inside center box should be classified as crossing");
}

TRAFFIC_TEST(vehicle_operational_state_refresh_tracks_queue_and_turning_states) {
    traffic::domain::Vehicle queued {
        60,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {130.0, 500.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
    };
    queued.set_speed(0.0);
    queued.set_waiting_for_green(true);
    queued.refresh_operational_state({100.0, 300.0}, 90.0, 120.0, 12.0, 1.5);
    require(queued.operational_state() == traffic::domain::VehicleOperationalState::Queued,
        "queued vehicle should map to queued operational state");

    traffic::domain::Vehicle turning {
        61,
        traffic::domain::VehicleType::Motorcycle,
        traffic::domain::Direction::East,
        {100.0, 300.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Motorcycle),
        traffic::domain::TurnIntent::Left,
    };
    turning.set_turning(true);
    turning.refresh_operational_state({100.0, 300.0}, 90.0, 120.0, 12.0, 1.5);
    require(turning.operational_state() == traffic::domain::VehicleOperationalState::Turning,
        "turning vehicle inside center should map to turning state");
}

TRAFFIC_TEST(vehicle_operational_state_can_transition_from_queue_to_discharging) {
    traffic::domain::Vehicle vehicle {
        62,
        traffic::domain::VehicleType::Car,
        traffic::domain::Direction::North,
        {100.0, 130.0},
        traffic::domain::Vehicle::default_profile(traffic::domain::VehicleType::Car),
    };

    vehicle.set_waiting_for_green(true);
    vehicle.set_speed(0.0);
    vehicle.refresh_operational_state({100.0, 100.0}, 20.0, 20.0, 2.0, 0.5);
    require(vehicle.operational_state() == traffic::domain::VehicleOperationalState::Queued,
        "precondition failed: vehicle should first classify as queued");

    vehicle.set_waiting_for_green(false);
    vehicle.set_speed(4.0);
    vehicle.set_target_speed(8.0);
    vehicle.refresh_operational_state({100.0, 100.0}, 20.0, 20.0, 2.0, 0.5);
    require(vehicle.operational_state() == traffic::domain::VehicleOperationalState::Discharging,
        "vehicle leaving a queue before the stop line should classify as discharging");
}
