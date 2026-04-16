#include "ui/ControlPanel.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

namespace traffic::ui {

namespace {
std::string yes_no(bool value) {
    return value ? "YES" : "NO";
}

std::string format_decimal(double value, int precision = 1) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}
}  // namespace

ControlPanel::ControlPanel()
    : font_(std::make_unique<sf::Font>()) {
    panelBackground_.setFillColor(sf::Color(24, 24, 28, 235));
    headerStripe_.setFillColor(sf::Color(68, 127, 255));
    statusDot_.setRadius(7.0F);
    statusDot_.setOrigin(7.0F, 7.0F);

    titleText_.setCharacterSize(24);
    titleText_.setFillColor(sf::Color::White);
    statusText_.setCharacterSize(15);
    statusText_.setFillColor(sf::Color(220, 220, 220));
    statsText_.setCharacterSize(16);
    statsText_.setFillColor(sf::Color(230, 230, 230));
    footerText_.setCharacterSize(13);
    footerText_.setFillColor(sf::Color(170, 185, 210));
}

void ControlPanel::initialize(sf::Vector2f position, sf::Vector2f size) {
    position_ = position;
    size_ = size;
    panelBackground_.setPosition(position_);
    panelBackground_.setSize(size_);

    headerStripe_.setPosition(position_);
    headerStripe_.setSize({size_.x, 6.0F});

    statusDot_.setPosition(position_.x + 20.0F, position_.y + 18.0F);
    layoutDirty_ = true;
}

void ControlPanel::set_font_path(std::string fontPath) {
    fontPath_ = std::move(fontPath);
    layoutDirty_ = true;
}

bool ControlPanel::load_font() {
    if (!font_) {
        font_ = std::make_unique<sf::Font>();
    }

    if (!font_->loadFromFile(fontPath_)) {
        return false;
    }

    titleText_.setFont(*font_);
    statusText_.setFont(*font_);
    statsText_.setFont(*font_);
    footerText_.setFont(*font_);
    layoutDirty_ = true;
    update_text_layout();
    return true;
}

void ControlPanel::set_state(const domain::SimulationState* state) noexcept {
    state_ = state;
    layoutDirty_ = true;
}

void ControlPanel::set_running(bool running) noexcept {
    running_ = running;
    layoutDirty_ = true;
}

void ControlPanel::set_paused(bool paused) noexcept {
    paused_ = paused;
    layoutDirty_ = true;
}

void ControlPanel::set_elapsed_seconds(double seconds) noexcept {
    elapsedSeconds_ = std::max(0.0, seconds);
    layoutDirty_ = true;
}

void ControlPanel::update(float dt) {
    blinkTimer_ += dt;
    if (blinkTimer_ >= 1.0F) {
        blinkTimer_ -= 1.0F;
    }

    if (layoutDirty_) {
        update_text_layout();
    }
}

void ControlPanel::draw(sf::RenderTarget& target) const {
    target.draw(panelBackground_);
    target.draw(headerStripe_);
    target.draw(statusDot_);
    target.draw(titleText_);
    target.draw(statusText_);
    target.draw(statsText_);
    target.draw(footerText_);
}

std::string ControlPanel::format_seconds(double seconds) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << std::max(0.0, seconds) << " s";
    return stream.str();
}

std::string ControlPanel::format_count(std::uint64_t value) {
    return std::to_string(value);
}

std::string ControlPanel::light_label(domain::LightState state) {
    switch (state) {
    case domain::LightState::Red:
        return "RED";
    case domain::LightState::Yellow:
        return "YELLOW";
    case domain::LightState::Green:
        return "GREEN";
    }
    return "UNKNOWN";
}

std::string ControlPanel::phase_label(domain::TrafficPhase phase) {
    switch (phase) {
    case domain::TrafficPhase::NorthSouthGreen:
        return "NS GREEN";
    case domain::TrafficPhase::NorthSouthYellow:
        return "NS YELLOW";
    case domain::TrafficPhase::EastWestGreen:
        return "EW GREEN";
    case domain::TrafficPhase::EastWestYellow:
        return "EW YELLOW";
    }
    return "UNKNOWN";
}

std::string ControlPanel::direction_label(domain::Direction direction) {
    switch (direction) {
    case domain::Direction::North:
        return "NORTH";
    case domain::Direction::South:
        return "SOUTH";
    case domain::Direction::East:
        return "EAST";
    case domain::Direction::West:
        return "WEST";
    }
    return "UNKNOWN";
}

std::string ControlPanel::vehicle_type_label(domain::VehicleType type) {
    switch (type) {
    case domain::VehicleType::Car:
        return "CAR";
    case domain::VehicleType::Motorcycle:
        return "MOTORCYCLE";
    case domain::VehicleType::Truck:
        return "TRUCK";
    }
    return "UNKNOWN";
}

void ControlPanel::update_text_layout() const {
    layoutDirty_ = false;

    if (!font_ || font_->getInfo().family.empty()) {
        return;
    }

    titleText_.setString("Traffic Control Panel");
    titleText_.setPosition(position_.x + 18.0F, position_.y + 12.0F);

    const auto* state = state_;
    const auto phaseText = state != nullptr ? phase_label(state->traffic_light().phase()) : std::string {"NO STATE"};
    const auto northLight = state != nullptr
        ? light_label(state->traffic_light().state_for(domain::Direction::North))
        : std::string {"--"};
    const auto southLight = state != nullptr
        ? light_label(state->traffic_light().state_for(domain::Direction::South))
        : std::string {"--"};
    const auto eastLight = state != nullptr
        ? light_label(state->traffic_light().state_for(domain::Direction::East))
        : std::string {"--"};
    const auto westLight = state != nullptr
        ? light_label(state->traffic_light().state_for(domain::Direction::West))
        : std::string {"--"};

    std::ostringstream status;
    status << "Runtime\n";
    status << "Running: " << yes_no(running_) << " | Paused: " << yes_no(paused_) << '\n';
    status << "Elapsed: " << format_seconds(elapsedSeconds_) << '\n';
    status << "Phase: " << phaseText;
    if (state != nullptr) {
        status << " | Remaining: "
               << format_seconds(state->traffic_light().time_remaining_seconds()) << '\n';
        status << "Cycle count: " << format_count(state->traffic_light().cycle_count()) << '\n';
        status << "Signals N/S/E/W: " << northLight << " / " << southLight
               << " / " << eastLight << " / " << westLight;
    } else {
        status << '\n';
        status << "Signals N/S/E/W: -- / -- / -- / --";
    }
    statusText_.setString(status.str());
    statusText_.setPosition(position_.x + 18.0F, position_.y + 54.0F);

    std::ostringstream stats;
    if (state != nullptr) {
        const auto& config = state->config();
        const auto& metrics = state->metrics();
        stats << "Traffic snapshot\n";
        stats << "Active: " << format_count(state->active_vehicle_count())
              << " | Spawned: " << format_count(metrics.total_vehicles_spawned)
              << " | Done: " << format_count(metrics.completed_vehicles) << '\n';
        stats << "Queue NS/EW: " << metrics.north_south_queue
              << " / " << metrics.east_west_queue << '\n';
        stats << "Queue N/S/E/W: "
              << state->queued_vehicle_count(domain::Direction::North) << " / "
              << state->queued_vehicle_count(domain::Direction::South) << " / "
              << state->queued_vehicle_count(domain::Direction::East) << " / "
              << state->queued_vehicle_count(domain::Direction::West) << '\n';
        stats << "Wait NS/EW: " << format_seconds(metrics.north_south_wait_time)
              << " / " << format_seconds(metrics.east_west_wait_time) << '\n';
        stats << "Wait N/S/E/W: " << format_seconds(metrics.north_wait_time)
              << " / " << format_seconds(metrics.south_wait_time)
              << " / " << format_seconds(metrics.east_wait_time)
              << " / " << format_seconds(metrics.west_wait_time) << '\n';
        stats << "Avg done wait: " << format_seconds(state->average_completed_wait_time()) << '\n';
        stats << "Avg queue wait: " << format_seconds(metrics.average_queue_wait_time)
              << " | Avg speed: " << format_decimal(metrics.average_active_speed, 1) << '\n';
        stats << "Approach delay: " << format_seconds(metrics.average_approach_delay)
              << " | Queue dist avg/max: " << format_decimal(metrics.average_queue_distance_to_stop_line, 1)
              << " / " << format_decimal(metrics.max_queue_distance_to_stop_line, 1) << '\n';
        stats << "Moving/Stopped: " << metrics.moving_vehicles
              << " / " << metrics.stopped_vehicles
              << " | Waiting green: " << metrics.vehicles_waiting_for_green << '\n';
        stats << "Crossing/Turning/Exiting: " << metrics.vehicles_in_intersection
              << " / " << metrics.vehicles_turning
              << " / " << metrics.vehicles_exiting << '\n';
        stats << "Cruise/Approach/Decel/Queue/Discharge: "
              << metrics.vehicles_cruising << " / "
              << metrics.vehicles_approaching_signal << " / "
              << metrics.vehicles_decelerating << " / "
              << metrics.vehicles_queued << " / "
              << metrics.vehicles_discharging << '\n';
        stats << "Yellow commit cand/actual: " << metrics.yellow_commit_candidates
              << " / " << metrics.yellow_commit_vehicles
              << " | Near stop line: " << metrics.queueing_vehicles_near_stop_line << '\n';
        stats << "Queue peak NS/EW: " << metrics.peak_north_south_queue
              << " / " << metrics.peak_east_west_queue << '\n';
        stats << '\n';
        stats << "Experiment parameters\n";
        stats << "Green NS/EW: " << format_seconds(config.north_south_green_seconds)
              << " / " << format_seconds(config.east_west_green_seconds) << '\n';
        stats << "Yellow: " << format_seconds(config.yellow_seconds)
              << " | Spawn p: " << format_decimal(config.spawn_probability, 2) << '\n';
        stats << "Spawn interval: " << format_seconds(config.spawn_interval_seconds)
              << " | Seed: " << format_count(config.random_seed) << '\n';
        stats << "Queue speed <= " << format_decimal(config.queue_speed_threshold, 1)
              << " | Wait speed <= " << format_decimal(config.wait_speed_threshold, 1) << '\n';
        stats << "Headway N/S/E/W: "
              << format_decimal(config.approach_demand[0].target_headway_seconds, 1) << " / "
              << format_decimal(config.approach_demand[1].target_headway_seconds, 1) << " / "
              << format_decimal(config.approach_demand[2].target_headway_seconds, 1) << " / "
              << format_decimal(config.approach_demand[3].target_headway_seconds, 1) << '\n';
        stats << "Demand weight N/S/E/W: "
              << format_decimal(config.approach_demand[0].demand_weight, 2) << " / "
              << format_decimal(config.approach_demand[1].demand_weight, 2) << " / "
              << format_decimal(config.approach_demand[2].demand_weight, 2) << " / "
              << format_decimal(config.approach_demand[3].demand_weight, 2) << '\n';
        stats << "Road/Lane: " << format_decimal(config.geometry.road_width, 0)
              << " / " << format_decimal(config.geometry.lane_width, 0)
              << " | Stop line: " << format_decimal(config.geometry.stop_line_distance, 0);
    } else {
        stats << "Traffic snapshot\n";
        stats << "No simulation state attached\n\n";
        stats << "Experiment parameters\n";
        stats << "Unavailable until panel receives SimulationState";
    }
    statsText_.setString(stats.str());
    statsText_.setPosition(position_.x + 18.0F, position_.y + 144.0F);

    std::ostringstream footer;
    footer << "Keys: Space pause, R reset, Q/A NS, W/S EW, Up/Down demand, [/ ] seed";
    footer << " | Esc closes app";
    footer << " | Legend: "
           << vehicle_type_label(domain::VehicleType::Car) << ", "
           << vehicle_type_label(domain::VehicleType::Motorcycle) << ", "
           << vehicle_type_label(domain::VehicleType::Truck);
    footerText_.setString(footer.str());
    footerText_.setPosition(position_.x + 18.0F, position_.y + size_.y - 32.0F);

    headerStripe_.setFillColor(phase_color());
    statusDot_.setFillColor(status_color());
}

sf::Color ControlPanel::phase_color() const noexcept {
    const auto* state = state_;
    if (state == nullptr) {
        return sf::Color(68, 127, 255);
    }

    switch (state->traffic_light().phase()) {
    case domain::TrafficPhase::NorthSouthGreen:
        return sf::Color(80, 220, 120);
    case domain::TrafficPhase::NorthSouthYellow:
        return sf::Color(235, 200, 70);
    case domain::TrafficPhase::EastWestGreen:
        return sf::Color(90, 170, 255);
    case domain::TrafficPhase::EastWestYellow:
        return sf::Color(235, 200, 70);
    }
    return sf::Color(68, 127, 255);
}

sf::Color ControlPanel::status_color() const noexcept {
    if (!running_) {
        return sf::Color(120, 120, 130);
    }

    if (paused_) {
        return sf::Color(235, 200, 70);
    }

    const auto* state = state_;
    if (state == nullptr) {
        return sf::Color(68, 127, 255);
    }

    const auto phase = state->traffic_light().phase();
    if (phase == domain::TrafficPhase::NorthSouthYellow ||
        phase == domain::TrafficPhase::EastWestYellow) {
        const bool bright = blinkTimer_ < 0.5F;
        return bright ? sf::Color(255, 215, 90) : sf::Color(150, 120, 40);
    }

    return state->traffic_light().is_green_for(domain::Direction::North)
        ? sf::Color(80, 220, 120)
        : sf::Color(90, 170, 255);
}

}  // namespace traffic::ui
