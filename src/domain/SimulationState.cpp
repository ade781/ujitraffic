#include "domain/SimulationState.hpp"

#include <algorithm>
#include <numeric>
#include <utility>

namespace traffic::domain {

namespace {
bool is_zero_or_negative(double value) noexcept {
    return value <= 0.0;
}
}

Vehicle::Vehicle(
    Id id,
    VehicleType type,
    Direction direction,
    Point2D position,
    VehicleProfile profile,
    TurnIntent turn_intent) noexcept
    : id_(id),
      type_(type),
      direction_(direction),
      position_(position),
      profile_(profile),
      target_speed_(profile.max_speed),
      turn_intent_(turn_intent),
      reaction_delay_(0.0) {
    if (profile_.max_speed <= 0.0) {
        profile_ = default_profile(type_);
        target_speed_ = profile_.max_speed;
    }
}

Vehicle::Id Vehicle::id() const noexcept { return id_; }
VehicleType Vehicle::type() const noexcept { return type_; }
Direction Vehicle::direction() const noexcept { return direction_; }
void Vehicle::set_direction(Direction direction) noexcept { direction_ = direction; }

const Point2D& Vehicle::position() const noexcept { return position_; }
void Vehicle::set_position(Point2D position) noexcept { position_ = position; }
double Vehicle::x() const noexcept { return position_.x; }
double Vehicle::y() const noexcept { return position_.y; }

const VehicleProfile& Vehicle::profile() const noexcept { return profile_; }
void Vehicle::set_profile(VehicleProfile profile) noexcept {
    profile_ = profile;
    target_speed_ = std::clamp(target_speed_, 0.0, profile_.max_speed);
    speed_ = std::clamp(speed_, 0.0, profile_.max_speed);
}

double Vehicle::speed() const noexcept { return speed_; }
void Vehicle::set_speed(double speed) noexcept { speed_ = std::max(0.0, speed); clamp_speed(); }
double Vehicle::target_speed() const noexcept { return target_speed_; }
void Vehicle::set_target_speed(double target_speed) noexcept { target_speed_ = std::max(0.0, target_speed); }

double Vehicle::wait_time() const noexcept { return wait_time_; }
void Vehicle::add_wait_time(double dt) noexcept { wait_time_ = std::max(0.0, wait_time_ + dt); }
void Vehicle::reset_wait_time() noexcept { wait_time_ = 0.0; }

bool Vehicle::active() const noexcept { return active_; }
void Vehicle::set_active(bool active) noexcept { active_ = active; }

TurnIntent Vehicle::turn_intent() const noexcept { return turn_intent_; }
void Vehicle::set_turn_intent(TurnIntent turn_intent) noexcept { turn_intent_ = turn_intent; }

bool Vehicle::turning() const noexcept { return turning_; }
void Vehicle::set_turning(bool turning) noexcept { turning_ = turning; }

bool Vehicle::waiting_for_green() const noexcept { return waiting_for_green_; }
void Vehicle::set_waiting_for_green(bool waiting_for_green) noexcept { waiting_for_green_ = waiting_for_green; }

bool Vehicle::has_reacted() const noexcept { return has_reacted_; }
void Vehicle::set_has_reacted(bool has_reacted) noexcept { has_reacted_ = has_reacted; }

double Vehicle::reaction_timer() const noexcept { return reaction_timer_; }
void Vehicle::reset_reaction_timer() noexcept { reaction_timer_ = 0.0; }
void Vehicle::add_reaction_time(double dt) noexcept { reaction_timer_ = std::max(0.0, reaction_timer_ + dt); }

double Vehicle::reaction_delay() const noexcept { return reaction_delay_; }
void Vehicle::set_reaction_delay(double delay) noexcept { reaction_delay_ = std::max(0.0, delay); }

VehicleOperationalState Vehicle::operational_state() const noexcept { return operational_state_; }

void Vehicle::set_operational_state(VehicleOperationalState state) noexcept {
    operational_state_ = state;
}

void Vehicle::refresh_operational_state(
    const Point2D& intersection_center,
    double stop_line_distance,
    double center_size,
    double queue_speed_threshold,
    double wait_speed_threshold) noexcept {
    const auto previous_state = operational_state_;
    const bool in_intersection = is_at_intersection_center(intersection_center, center_size);
    const bool downstream = is_downstream_of_intersection(intersection_center, center_size);

    double distance_to_stop = 0.0;
    switch (direction_) {
    case Direction::North:
        distance_to_stop = front_bumper() - (intersection_center.y - stop_line_distance);
        break;
    case Direction::South:
        distance_to_stop = (intersection_center.y + stop_line_distance) - front_bumper();
        break;
    case Direction::East:
        distance_to_stop = (intersection_center.x + stop_line_distance) - front_bumper();
        break;
    case Direction::West:
        distance_to_stop = front_bumper() - (intersection_center.x - stop_line_distance);
        break;
    }

    if (!active_) {
        operational_state_ = VehicleOperationalState::Completed;
    } else if (in_intersection && turning_) {
        operational_state_ = VehicleOperationalState::Turning;
    } else if (in_intersection) {
        operational_state_ = VehicleOperationalState::CrossingIntersection;
    } else if (downstream) {
        operational_state_ = VehicleOperationalState::Exiting;
    } else if (waiting_for_green_ && speed_ <= wait_speed_threshold) {
        operational_state_ = VehicleOperationalState::Queued;
    } else if (waiting_for_green_) {
        operational_state_ = VehicleOperationalState::DeceleratingToStop;
    } else if (previous_state == VehicleOperationalState::Queued &&
               speed_ > wait_speed_threshold &&
               distance_to_stop > 0.0) {
        operational_state_ = VehicleOperationalState::Discharging;
    } else if (distance_to_stop > 0.0 && speed_ <= queue_speed_threshold) {
        operational_state_ = VehicleOperationalState::Queued;
    } else if (distance_to_stop > 0.0 && speed_ < target_speed_) {
        operational_state_ = VehicleOperationalState::ApproachingSignal;
    } else if (speed_ > 0.0) {
        operational_state_ = VehicleOperationalState::Cruising;
    } else {
        operational_state_ = VehicleOperationalState::Spawning;
    }
}

double Vehicle::longitudinal_position() const noexcept {
    switch (direction_) {
    case Direction::North:
    case Direction::South:
        return position_.y;
    case Direction::East:
    case Direction::West:
        return position_.x;
    }
    return position_.x;
}

double Vehicle::lateral_position() const noexcept {
    switch (direction_) {
    case Direction::North:
    case Direction::South:
        return position_.x;
    case Direction::East:
    case Direction::West:
        return position_.y;
    }
    return position_.y;
}

double Vehicle::front_bumper() const noexcept {
    const double offset = half_length(profile_);
    switch (direction_) {
    case Direction::North:
        return position_.y - offset;
    case Direction::South:
        return position_.y + offset;
    case Direction::East:
        return position_.x + offset;
    case Direction::West:
        return position_.x - offset;
    }
    return position_.y;
}

double Vehicle::rear_bumper() const noexcept {
    const double offset = half_length(profile_);
    switch (direction_) {
    case Direction::North:
        return position_.y + offset;
    case Direction::South:
        return position_.y - offset;
    case Direction::East:
        return position_.x - offset;
    case Direction::West:
        return position_.x + offset;
    }
    return position_.y;
}

double Vehicle::stopping_distance(double braking_buffer) const noexcept {
    const double deceleration = std::max(profile_.deceleration, 0.1);
    return (speed_ * speed_) / (2.0 * deceleration) + std::max(0.0, braking_buffer);
}

bool Vehicle::same_lane(const Vehicle& other, double lane_tolerance) const noexcept {
    if (direction_ != other.direction_) {
        return false;
    }
    return std::abs(lateral_position() - other.lateral_position()) <= lane_tolerance;
}

bool Vehicle::is_ahead_of(const Vehicle& other, double lane_tolerance) const noexcept {
    if (!same_lane(other, lane_tolerance)) {
        return false;
    }

    switch (direction_) {
    case Direction::North:
        return other.longitudinal_position() < longitudinal_position();
    case Direction::South:
        return other.longitudinal_position() > longitudinal_position();
    case Direction::East:
        return other.longitudinal_position() > longitudinal_position();
    case Direction::West:
        return other.longitudinal_position() < longitudinal_position();
    }
    return false;
}

double Vehicle::gap_to(const Vehicle& other) const noexcept {
    if (direction_ != other.direction_) {
        return std::numeric_limits<double>::infinity();
    }

    switch (direction_) {
    case Direction::North:
        return rear_bumper() - other.front_bumper();
    case Direction::South:
        return other.front_bumper() - rear_bumper();
    case Direction::East:
        return other.front_bumper() - rear_bumper();
    case Direction::West:
        return rear_bumper() - other.front_bumper();
    }
    return std::numeric_limits<double>::infinity();
}

bool Vehicle::should_stop_for_light(
    LightState light_state,
    const Point2D& intersection_center,
    double stop_line_distance) const noexcept {
    if (light_state == LightState::Green) {
        return false;
    }

    const double front = front_bumper();
    const double threshold = std::max(half_length(profile_), 1.0);

    double distance_to_stop = 0.0;
    switch (direction_) {
    case Direction::North:
        distance_to_stop = front - (intersection_center.y - stop_line_distance);
        break;
    case Direction::South:
        distance_to_stop = (intersection_center.y + stop_line_distance) - front;
        break;
    case Direction::East:
        distance_to_stop = (intersection_center.x + stop_line_distance) - front;
        break;
    case Direction::West:
        distance_to_stop = front - (intersection_center.x - stop_line_distance);
        break;
    }

    if (light_state == LightState::Red) {
        return distance_to_stop > 0.0;
    }

    return distance_to_stop > threshold;
}

bool Vehicle::is_at_intersection_center(
    const Point2D& intersection_center,
    double center_size) const noexcept {
    const double half_extent = center_size * 0.5;
    return std::abs(position_.x - intersection_center.x) <= half_extent &&
           std::abs(position_.y - intersection_center.y) <= half_extent;
}

bool Vehicle::should_turn_at_center(
    const Point2D& intersection_center,
    double center_size) const noexcept {
    return turn_intent_ != TurnIntent::Straight &&
           !turning_ &&
           is_at_intersection_center(intersection_center, center_size);
}

Direction Vehicle::next_direction_after_turn() const noexcept {
    if (turn_intent_ == TurnIntent::Straight) {
        return direction_;
    }

    switch (direction_) {
    case Direction::North:
        return Direction::West;
    case Direction::South:
        return Direction::East;
    case Direction::East:
        return Direction::North;
    case Direction::West:
        return Direction::South;
    }
    return direction_;
}

void Vehicle::advance(double dt) noexcept {
    const double distance = std::max(0.0, speed_) * dt;
    switch (direction_) {
    case Direction::North:
        position_.y -= distance;
        break;
    case Direction::South:
        position_.y += distance;
        break;
    case Direction::East:
        position_.x += distance;
        break;
    case Direction::West:
        position_.x -= distance;
        break;
    }
}

void Vehicle::accelerate_towards_target(double dt) noexcept {
    if (speed_ < target_speed_) {
        speed_ = std::min(target_speed_, speed_ + profile_.acceleration * dt);
    }
    clamp_speed();
}

void Vehicle::brake_to_stop(double dt) noexcept {
    speed_ = std::max(0.0, speed_ - profile_.deceleration * dt);
}

void Vehicle::clamp_speed() noexcept {
    speed_ = std::clamp(speed_, 0.0, profile_.max_speed);
    target_speed_ = std::clamp(target_speed_, 0.0, profile_.max_speed);
}

VehicleProfile Vehicle::default_profile(VehicleType type) noexcept {
    switch (type) {
    case VehicleType::Car:
        return {4.5, 1.8, 13.9, 2.5, 4.5};
    case VehicleType::Motorcycle:
        return {2.2, 0.8, 16.7, 3.0, 5.0};
    case VehicleType::Truck:
        return {8.0, 2.5, 11.1, 1.5, 3.5};
    }
    return {};
}

double Vehicle::half_length(const VehicleProfile& profile) noexcept {
    return profile.length * 0.5;
}

bool Vehicle::is_downstream_of_intersection(
    const Point2D& intersection_center,
    double center_size) const noexcept {
    const double half_extent = center_size * 0.5;
    switch (direction_) {
    case Direction::North:
        return rear_bumper() < intersection_center.y - half_extent;
    case Direction::South:
        return rear_bumper() > intersection_center.y + half_extent;
    case Direction::East:
        return rear_bumper() > intersection_center.x + half_extent;
    case Direction::West:
        return rear_bumper() < intersection_center.x - half_extent;
    }
    return false;
}

TrafficLight::TrafficLight(
    double north_south_green_seconds,
    double east_west_green_seconds,
    double yellow_seconds) noexcept
    : north_south_green_seconds_(std::max(0.0, north_south_green_seconds)),
      east_west_green_seconds_(std::max(0.0, east_west_green_seconds)),
      yellow_seconds_(std::max(0.0, yellow_seconds)) {}

double TrafficLight::north_south_green_seconds() const noexcept { return north_south_green_seconds_; }
double TrafficLight::east_west_green_seconds() const noexcept { return east_west_green_seconds_; }
double TrafficLight::yellow_seconds() const noexcept { return yellow_seconds_; }

void TrafficLight::set_north_south_green_seconds(double seconds) noexcept {
    north_south_green_seconds_ = std::max(0.0, seconds);
}

void TrafficLight::set_east_west_green_seconds(double seconds) noexcept {
    east_west_green_seconds_ = std::max(0.0, seconds);
}

void TrafficLight::set_yellow_seconds(double seconds) noexcept {
    yellow_seconds_ = std::max(0.0, seconds);
}

TrafficPhase TrafficLight::phase() const noexcept { return phase_; }
double TrafficLight::phase_elapsed_seconds() const noexcept { return phase_elapsed_seconds_; }
double TrafficLight::time_remaining_seconds() const noexcept {
    switch (phase_) {
    case TrafficPhase::NorthSouthGreen:
        return std::max(0.0, north_south_green_seconds_ - phase_elapsed_seconds_);
    case TrafficPhase::NorthSouthYellow:
    case TrafficPhase::EastWestYellow:
        return std::max(0.0, yellow_seconds_ - phase_elapsed_seconds_);
    case TrafficPhase::EastWestGreen:
        return std::max(0.0, east_west_green_seconds_ - phase_elapsed_seconds_);
    }
    return 0.0;
}

std::uint64_t TrafficLight::cycle_count() const noexcept { return cycle_count_; }

LightState TrafficLight::state_for(Direction direction) const noexcept {
    switch (phase_) {
    case TrafficPhase::NorthSouthGreen:
        return (direction == Direction::North || direction == Direction::South) ? LightState::Green : LightState::Red;
    case TrafficPhase::NorthSouthYellow:
        return (direction == Direction::North || direction == Direction::South) ? LightState::Yellow : LightState::Red;
    case TrafficPhase::EastWestGreen:
        return (direction == Direction::East || direction == Direction::West) ? LightState::Green : LightState::Red;
    case TrafficPhase::EastWestYellow:
        return (direction == Direction::East || direction == Direction::West) ? LightState::Yellow : LightState::Red;
    }
    return LightState::Red;
}

bool TrafficLight::is_green_for(Direction direction) const noexcept {
    return state_for(direction) == LightState::Green;
}

bool TrafficLight::is_yellow_for(Direction direction) const noexcept {
    return state_for(direction) == LightState::Yellow;
}

bool TrafficLight::is_red_for(Direction direction) const noexcept {
    return state_for(direction) == LightState::Red;
}

bool TrafficLight::just_turned_green(Direction direction) const noexcept {
    switch (direction) {
    case Direction::North:
    case Direction::South:
        return ns_just_turned_green_;
    case Direction::East:
    case Direction::West:
        return ew_just_turned_green_;
    }
    return false;
}

void TrafficLight::update(double dt) noexcept {
    ns_just_turned_green_ = false;
    ew_just_turned_green_ = false;
    phase_elapsed_seconds_ += std::max(0.0, dt);

    switch (phase_) {
    case TrafficPhase::NorthSouthGreen:
        if (phase_elapsed_seconds_ >= north_south_green_seconds_) {
            transition_to(TrafficPhase::NorthSouthYellow);
        }
        break;
    case TrafficPhase::NorthSouthYellow:
        if (phase_elapsed_seconds_ >= yellow_seconds_) {
            transition_to(TrafficPhase::EastWestGreen);
            ew_just_turned_green_ = true;
        }
        break;
    case TrafficPhase::EastWestGreen:
        if (phase_elapsed_seconds_ >= east_west_green_seconds_) {
            transition_to(TrafficPhase::EastWestYellow);
        }
        break;
    case TrafficPhase::EastWestYellow:
        if (phase_elapsed_seconds_ >= yellow_seconds_) {
            transition_to(TrafficPhase::NorthSouthGreen);
            ns_just_turned_green_ = true;
            ++cycle_count_;
        }
        break;
    }
}

void TrafficLight::reset() noexcept {
    phase_ = TrafficPhase::NorthSouthGreen;
    phase_elapsed_seconds_ = 0.0;
    cycle_count_ = 0;
    ns_just_turned_green_ = false;
    ew_just_turned_green_ = false;
}

void TrafficLight::transition_to(TrafficPhase phase) noexcept {
    phase_ = phase;
    phase_elapsed_seconds_ = 0.0;
}

SimulationState::SimulationState(SimulationConfig config)
    : config_(std::move(config)),
      traffic_light_(
          config_.north_south_green_seconds,
          config_.east_west_green_seconds,
          config_.yellow_seconds) {}

const SimulationConfig& SimulationState::config() const noexcept { return config_; }
SimulationConfig& SimulationState::config() noexcept { return config_; }

const IntersectionGeometry& SimulationState::geometry() const noexcept { return config_.geometry; }
IntersectionGeometry& SimulationState::geometry() noexcept { return config_.geometry; }

const TrafficLight& SimulationState::traffic_light() const noexcept { return traffic_light_; }
TrafficLight& SimulationState::traffic_light() noexcept { return traffic_light_; }

const std::vector<Vehicle>& SimulationState::vehicles() const noexcept { return vehicles_; }
std::vector<Vehicle>& SimulationState::vehicles() noexcept { return vehicles_; }

const SimulationMetrics& SimulationState::metrics() const noexcept { return metrics_; }
SimulationMetrics& SimulationState::metrics() noexcept { return metrics_; }

void SimulationState::reset() noexcept {
    vehicles_.clear();
    traffic_light_.reset();
    traffic_light_.set_north_south_green_seconds(config_.north_south_green_seconds);
    traffic_light_.set_east_west_green_seconds(config_.east_west_green_seconds);
    traffic_light_.set_yellow_seconds(config_.yellow_seconds);
    metrics_ = {};
    next_vehicle_id_ = 1;
}

Vehicle& SimulationState::add_vehicle(Vehicle vehicle) {
    if (vehicle.id() == 0) {
        vehicle = Vehicle(
            next_vehicle_id_++,
            vehicle.type(),
            vehicle.direction(),
            vehicle.position(),
            vehicle.profile(),
            vehicle.turn_intent());
    }

    vehicles_.push_back(std::move(vehicle));
    ++metrics_.total_vehicles_spawned;
    return vehicles_.back();
}

Vehicle& SimulationState::emplace_vehicle(
    VehicleType type,
    Direction direction,
    Point2D position,
    TurnIntent turn_intent) {
    Vehicle vehicle {
        next_vehicle_id_++,
        type,
        direction,
        position,
        Vehicle::default_profile(type),
        turn_intent,
    };
    vehicles_.push_back(std::move(vehicle));
    ++metrics_.total_vehicles_spawned;
    return vehicles_.back();
}

void SimulationState::remove_inactive_vehicles() noexcept {
    vehicles_.erase(
        std::remove_if(
            vehicles_.begin(),
            vehicles_.end(),
            [](const Vehicle& vehicle) { return !vehicle.active(); }),
        vehicles_.end());
}

void SimulationState::mark_completed(const Vehicle& vehicle) {
    ++metrics_.completed_vehicles;
    metrics_.total_completed_wait_time += vehicle.wait_time();
    metrics_.completed_wait_times.push_back(vehicle.wait_time());
}

void SimulationState::update_metrics() {
    metrics_.north_south_queue = 0;
    metrics_.east_west_queue = 0;
    metrics_.north_queue = 0;
    metrics_.south_queue = 0;
    metrics_.east_queue = 0;
    metrics_.west_queue = 0;
    metrics_.average_active_speed = 0.0;
    metrics_.average_queue_wait_time = 0.0;
    metrics_.average_approach_delay = 0.0;
    metrics_.average_queue_distance_to_stop_line = 0.0;
    metrics_.max_queue_distance_to_stop_line = 0.0;
    metrics_.moving_vehicles = 0;
    metrics_.stopped_vehicles = 0;
    metrics_.vehicles_waiting_for_green = 0;
    metrics_.vehicles_in_intersection = 0;
    metrics_.vehicles_cruising = 0;
    metrics_.vehicles_approaching_signal = 0;
    metrics_.vehicles_decelerating = 0;
    metrics_.vehicles_queued = 0;
    metrics_.vehicles_discharging = 0;
    metrics_.vehicles_turning = 0;
    metrics_.vehicles_exiting = 0;
    metrics_.yellow_commit_candidates = 0;
    metrics_.yellow_commit_vehicles = 0;
    metrics_.queueing_vehicles_near_stop_line = 0;

    double north_south_total_wait = 0.0;
    double east_west_total_wait = 0.0;
    double north_total_wait = 0.0;
    double south_total_wait = 0.0;
    double east_total_wait = 0.0;
    double west_total_wait = 0.0;
    double total_active_speed = 0.0;
    double total_queue_wait = 0.0;
    double total_approach_delay = 0.0;
    double total_queue_distance = 0.0;
    std::size_t north_south_waiting = 0;
    std::size_t east_west_waiting = 0;
    std::size_t north_waiting = 0;
    std::size_t south_waiting = 0;
    std::size_t east_waiting = 0;
    std::size_t west_waiting = 0;
    std::size_t queue_vehicle_count = 0;

    for (auto& vehicle : vehicles_) {
        vehicle.refresh_operational_state(
            config_.geometry.center,
            config_.geometry.stop_line_distance,
            config_.geometry.center_size,
            config_.queue_speed_threshold,
            config_.wait_speed_threshold);

        total_active_speed += vehicle.speed();
        if (vehicle.speed() <= config_.wait_speed_threshold) {
            ++metrics_.stopped_vehicles;
        } else {
            ++metrics_.moving_vehicles;
        }

        if (vehicle.waiting_for_green()) {
            ++metrics_.vehicles_waiting_for_green;
        }

        if (traffic_light_.is_yellow_for(vehicle.direction())) {
            ++metrics_.yellow_commit_candidates;
            if (is_vehicle_committed_through_yellow(vehicle)) {
                ++metrics_.yellow_commit_vehicles;
            }
        }

        switch (vehicle.operational_state()) {
        case VehicleOperationalState::Cruising:
            ++metrics_.vehicles_cruising;
            break;
        case VehicleOperationalState::ApproachingSignal:
            ++metrics_.vehicles_approaching_signal;
            break;
        case VehicleOperationalState::DeceleratingToStop:
            ++metrics_.vehicles_decelerating;
            break;
        case VehicleOperationalState::Queued:
            ++metrics_.vehicles_queued;
            break;
        case VehicleOperationalState::Discharging:
            ++metrics_.vehicles_discharging;
            break;
        case VehicleOperationalState::CrossingIntersection:
            ++metrics_.vehicles_in_intersection;
            break;
        case VehicleOperationalState::Turning:
            ++metrics_.vehicles_turning;
            ++metrics_.vehicles_in_intersection;
            break;
        case VehicleOperationalState::Exiting:
            ++metrics_.vehicles_exiting;
            break;
        case VehicleOperationalState::Spawning:
        case VehicleOperationalState::Completed:
            break;
        }

        if (!is_vehicle_queued(vehicle)) {
            continue;
        }

        const double distance_to_stop = std::max(0.0, distance_to_stop_line(vehicle));
        total_queue_wait += vehicle.wait_time();
        total_queue_distance += distance_to_stop;
        ++queue_vehicle_count;
        if (distance_to_stop <= config_.geometry.stop_line_distance * 0.5) {
            ++metrics_.queueing_vehicles_near_stop_line;
        }
        metrics_.max_queue_distance_to_stop_line =
            std::max(metrics_.max_queue_distance_to_stop_line, distance_to_stop);

        if (is_vertical(vehicle.direction())) {
            ++metrics_.north_south_queue;
            north_south_total_wait += vehicle.wait_time();
            ++north_south_waiting;
        } else {
            ++metrics_.east_west_queue;
            east_west_total_wait += vehicle.wait_time();
            ++east_west_waiting;
        }

        total_approach_delay += vehicle.wait_time();
        switch (vehicle.direction()) {
        case Direction::North:
            ++metrics_.north_queue;
            north_total_wait += vehicle.wait_time();
            ++north_waiting;
            break;
        case Direction::South:
            ++metrics_.south_queue;
            south_total_wait += vehicle.wait_time();
            ++south_waiting;
            break;
        case Direction::East:
            ++metrics_.east_queue;
            east_total_wait += vehicle.wait_time();
            ++east_waiting;
            break;
        case Direction::West:
            ++metrics_.west_queue;
            west_total_wait += vehicle.wait_time();
            ++west_waiting;
            break;
        }
    }

    metrics_.north_south_wait_time = north_south_waiting == 0
        ? 0.0
        : north_south_total_wait / static_cast<double>(north_south_waiting);
    metrics_.east_west_wait_time = east_west_waiting == 0
        ? 0.0
        : east_west_total_wait / static_cast<double>(east_west_waiting);
    metrics_.north_wait_time = north_waiting == 0 ? 0.0 : north_total_wait / static_cast<double>(north_waiting);
    metrics_.south_wait_time = south_waiting == 0 ? 0.0 : south_total_wait / static_cast<double>(south_waiting);
    metrics_.east_wait_time = east_waiting == 0 ? 0.0 : east_total_wait / static_cast<double>(east_waiting);
    metrics_.west_wait_time = west_waiting == 0 ? 0.0 : west_total_wait / static_cast<double>(west_waiting);
    metrics_.average_active_speed = vehicles_.empty()
        ? 0.0
        : total_active_speed / static_cast<double>(vehicles_.size());
    metrics_.average_queue_wait_time = queue_vehicle_count == 0
        ? 0.0
        : total_queue_wait / static_cast<double>(queue_vehicle_count);
    metrics_.average_approach_delay = queue_vehicle_count == 0
        ? 0.0
        : total_approach_delay / static_cast<double>(queue_vehicle_count);
    metrics_.average_queue_distance_to_stop_line = queue_vehicle_count == 0
        ? 0.0
        : total_queue_distance / static_cast<double>(queue_vehicle_count);
    metrics_.peak_north_south_queue = std::max(metrics_.peak_north_south_queue, metrics_.north_south_queue);
    metrics_.peak_east_west_queue = std::max(metrics_.peak_east_west_queue, metrics_.east_west_queue);
    metrics_.peak_north_queue = std::max(metrics_.peak_north_queue, metrics_.north_queue);
    metrics_.peak_south_queue = std::max(metrics_.peak_south_queue, metrics_.south_queue);
    metrics_.peak_east_queue = std::max(metrics_.peak_east_queue, metrics_.east_queue);
    metrics_.peak_west_queue = std::max(metrics_.peak_west_queue, metrics_.west_queue);
}

bool SimulationState::is_in_queue_zone(const Vehicle& vehicle) const noexcept {
    return is_in_approach_queue_zone(vehicle);
}

bool SimulationState::is_in_approach_queue_zone(const Vehicle& vehicle) const noexcept {
    const auto& center = config_.geometry.center;
    const double queue_start = config_.geometry.stop_line_distance;
    const double approach_distance = config_.queue_model.approach_detection_distance > 0.0
        ? config_.queue_model.approach_detection_distance
        : config_.geometry.queue_distance;
    const double queue_end = queue_start + approach_distance;

    switch (vehicle.direction()) {
    case Direction::North:
        return vehicle.front_bumper() >= center.y + queue_start &&
               vehicle.front_bumper() <= center.y + queue_end;
    case Direction::South:
        return vehicle.front_bumper() <= center.y - queue_start &&
               vehicle.front_bumper() >= center.y - queue_end;
    case Direction::East:
        return vehicle.front_bumper() <= center.x - queue_start &&
               vehicle.front_bumper() >= center.x - queue_end;
    case Direction::West:
        return vehicle.front_bumper() >= center.x + queue_start &&
               vehicle.front_bumper() <= center.x + queue_end;
    }
    return false;
}

double SimulationState::average_completed_wait_time() const noexcept {
    if (metrics_.completed_vehicles == 0) {
        return 0.0;
    }

    return metrics_.total_completed_wait_time / static_cast<double>(metrics_.completed_vehicles);
}

std::size_t SimulationState::active_vehicle_count() const noexcept {
    return vehicles_.size();
}

std::size_t SimulationState::queued_vehicle_count(Direction direction) const noexcept {
    std::size_t count = 0;
    for (const auto& vehicle : vehicles_) {
        if (vehicle.direction() != direction) {
            continue;
        }
        if (is_vehicle_queued(vehicle)) {
            ++count;
        }
    }
    return count;
}

bool SimulationState::is_vehicle_queued(const Vehicle& vehicle) const noexcept {
    if (!is_in_approach_queue_zone(vehicle)) {
        return false;
    }

    const double speed_threshold = std::max(config_.queue_speed_threshold, config_.queue_model.queue_join_speed_threshold);
    if (vehicle.speed() > speed_threshold) {
        return false;
    }

    const double distance_to_stop = distance_to_stop_line(vehicle);
    return distance_to_stop >= 0.0 && distance_to_stop <= config_.geometry.queue_distance;
}

bool SimulationState::is_vehicle_committed_through_yellow(const Vehicle& vehicle) const noexcept {
    if (!config_.yellow_decision.allow_commit_on_yellow) {
        return false;
    }

    const double distance_to_stop = distance_to_stop_line(vehicle);
    if (distance_to_stop <= config_.yellow_decision.commit_min_distance_after_stop_line) {
        return true;
    }

    const double stopping_distance =
        vehicle.stopping_distance(config_.yellow_decision.comfortable_braking_buffer_meters);
    if (distance_to_stop <= stopping_distance) {
        return true;
    }

    const double time_to_line = time_to_stop_line(vehicle);
    return !is_zero_or_negative(time_to_line) &&
           time_to_line <= config_.yellow_decision.commit_min_time_to_stop_line_seconds;
}

double SimulationState::distance_to_stop_line(const Vehicle& vehicle) const noexcept {
    const auto& center = config_.geometry.center;
    const double stop_line_distance = config_.geometry.stop_line_distance;

    switch (vehicle.direction()) {
    case Direction::North:
        return vehicle.front_bumper() - (center.y - stop_line_distance);
    case Direction::South:
        return (center.y + stop_line_distance) - vehicle.front_bumper();
    case Direction::East:
        return (center.x + stop_line_distance) - vehicle.front_bumper();
    case Direction::West:
        return vehicle.front_bumper() - (center.x - stop_line_distance);
    }
    return 0.0;
}

double SimulationState::time_to_stop_line(const Vehicle& vehicle) const noexcept {
    const double speed = vehicle.speed();
    if (speed <= 0.0) {
        return std::numeric_limits<double>::infinity();
    }
    const double distance = distance_to_stop_line(vehicle);
    if (distance <= 0.0) {
        return 0.0;
    }
    return distance / speed;
}

double SimulationState::effective_spawn_probability(Direction direction) const noexcept {
    const auto& profile = demand_profile(direction);
    return std::clamp(
        config_.spawn_probability * profile.demand_weight * profile.spawn_probability_multiplier,
        0.0,
        1.0);
}

const ApproachDemandProfile& SimulationState::demand_profile(Direction direction) const noexcept {
    return config_.approach_demand[direction_index(direction)];
}

ApproachDemandProfile& SimulationState::demand_profile(Direction direction) noexcept {
    return config_.approach_demand[direction_index(direction)];
}

std::size_t SimulationState::direction_index(Direction direction) noexcept {
    switch (direction) {
    case Direction::North:
        return 0;
    case Direction::South:
        return 1;
    case Direction::East:
        return 2;
    case Direction::West:
        return 3;
    }
    return 0;
}

bool SimulationState::is_vertical(Direction direction) const noexcept{
    return direction == Direction::North || direction == Direction::South;
}

double SimulationState::average_wait_time_for(Direction direction) const noexcept {
    double total = 0.0;
    std::size_t count = 0;
    for (const auto& vehicle : vehicles_) {
        if (vehicle.direction() != direction) {
            continue;
        }
        if (!is_vehicle_queued(vehicle)) {
            continue;
        }
        total += vehicle.wait_time();
        ++count;
    }

    return count == 0 ? 0.0 : total / static_cast<double>(count);
}

}  // namespace traffic::domain
