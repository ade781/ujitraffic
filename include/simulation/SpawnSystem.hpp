#pragma once

#include "domain/SimulationState.hpp"
#include "simulation/SimulationRuntime.hpp"

namespace traffic::simulation::systems {

void spawn_vehicles(domain::SimulationState& state, SimulationRuntime& runtime, double dt);

}  // namespace traffic::simulation::systems
