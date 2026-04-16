#pragma once

#include "domain/SimulationState.hpp"
#include "simulation/SimulationRuntime.hpp"

namespace traffic::simulation {

class SimulationEngine {
public:
    void initialize(float simWidth, float simHeight);
    void update(float dt);
    void reset();
    void apply_config(const domain::SimulationConfig& config, bool resetSimulation = false);

    const domain::SimulationState& state() const noexcept;
    domain::SimulationState& state() noexcept;

private:
    domain::SimulationState state_ {};
    SimulationRuntime runtime_ {};
};

}  // namespace traffic::simulation
