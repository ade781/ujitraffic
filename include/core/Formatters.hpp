#pragma once

#include <string_view>

#include "domain/TrafficLight.hpp"
#include "domain/TrafficTypes.hpp"

namespace traffic::core {

[[nodiscard]] constexpr std::string_view direction_label(domain::Direction direction) noexcept {
    switch (direction) {
    case domain::Direction::North:
        return "North";
    case domain::Direction::South:
        return "South";
    case domain::Direction::East:
        return "East";
    case domain::Direction::West:
        return "West";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::string_view light_label(domain::LightState light_state) noexcept {
    switch (light_state) {
    case domain::LightState::Red:
        return "Red";
    case domain::LightState::Yellow:
        return "Yellow";
    case domain::LightState::Green:
        return "Green";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::string_view phase_label(domain::TrafficPhase phase) noexcept {
    switch (phase) {
    case domain::TrafficPhase::NorthSouthGreen:
        return "NS Green";
    case domain::TrafficPhase::NorthSouthYellow:
        return "NS Yellow";
    case domain::TrafficPhase::EastWestGreen:
        return "EW Green";
    case domain::TrafficPhase::EastWestYellow:
        return "EW Yellow";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::string_view vehicle_type_label(domain::VehicleType type) noexcept {
    switch (type) {
    case domain::VehicleType::Car:
        return "Car";
    case domain::VehicleType::Motorcycle:
        return "Motorcycle";
    case domain::VehicleType::Truck:
        return "Truck";
    }
    return "Unknown";
}

}  // namespace traffic::core

