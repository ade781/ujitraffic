#pragma once

#include <array>

#include "SimulationState.hpp"

namespace traffic::domain {

enum class DemandPreset {
    Balanced,
    HeavyNorthSouth,
    HeavyEastWest,
    Saturated,
};

struct ScenarioPreset {
    SimulationConfig config {};
    std::array<VehicleProfile, 3> vehicle_profiles {};
};

class ScenarioFactory {
public:
    static IntersectionGeometry default_geometry() noexcept;
    static std::array<VehicleProfile, 3> default_vehicle_profiles() noexcept;
    static VehicleProfile default_vehicle_profile(VehicleType type) noexcept;

    static SimulationConfig default_timing_and_demand() noexcept;
    static SimulationConfig timing_and_demand(DemandPreset preset) noexcept;

    static ScenarioPreset default_single_intersection_preset() noexcept;
    static ScenarioPreset make_preset(DemandPreset preset) noexcept;

    static SimulationState make_default_single_intersection_state() noexcept;
    static SimulationState make_state(const ScenarioPreset& preset) noexcept;
};

}  // namespace traffic::domain

