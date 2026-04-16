#pragma once

#include "domain/SimulationState.hpp"
#include "simulation/SimulationRuntime.hpp"

namespace traffic::simulation::systems {

void update_vehicle_behavior(domain::SimulationState& state, SimulationRuntime& runtime, double dt);

}  // namespace traffic::simulation::systems
