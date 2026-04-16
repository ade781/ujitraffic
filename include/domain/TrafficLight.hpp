#pragma once

#include <cstdint>

#include "TrafficTypes.hpp"

namespace traffic::domain {

enum class TrafficPhase {
    NorthSouthGreen,
    NorthSouthYellow,
    EastWestGreen,
    EastWestYellow,
};

class TrafficLight {
public:
    TrafficLight(
        double north_south_green_seconds = 30.0,
        double east_west_green_seconds = 30.0,
        double yellow_seconds = 3.0) noexcept;

    double north_south_green_seconds() const noexcept;
    double east_west_green_seconds() const noexcept;
    double yellow_seconds() const noexcept;

    void set_north_south_green_seconds(double seconds) noexcept;
    void set_east_west_green_seconds(double seconds) noexcept;
    void set_yellow_seconds(double seconds) noexcept;

    TrafficPhase phase() const noexcept;
    double phase_elapsed_seconds() const noexcept;
    double time_remaining_seconds() const noexcept;
    std::uint64_t cycle_count() const noexcept;

    LightState state_for(Direction direction) const noexcept;
    bool is_green_for(Direction direction) const noexcept;
    bool is_yellow_for(Direction direction) const noexcept;
    bool is_red_for(Direction direction) const noexcept;
    bool just_turned_green(Direction direction) const noexcept;

    void update(double dt) noexcept;
    void reset() noexcept;

private:
    void transition_to(TrafficPhase phase) noexcept;

    double north_south_green_seconds_ {30.0};
    double east_west_green_seconds_ {30.0};
    double yellow_seconds_ {3.0};
    TrafficPhase phase_ {TrafficPhase::NorthSouthGreen};
    double phase_elapsed_seconds_ {0.0};
    std::uint64_t cycle_count_ {0};
    bool ns_just_turned_green_ {false};
    bool ew_just_turned_green_ {false};
};

}  // namespace traffic::domain
