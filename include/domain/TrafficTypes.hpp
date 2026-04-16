#pragma once

namespace traffic::domain {

enum class Direction {
    North,
    South,
    East,
    West,
};

enum class LightState {
    Red,
    Yellow,
    Green,
};

enum class VehicleType {
    Car,
    Motorcycle,
    Truck,
};

}  // namespace traffic::domain

