#pragma once

#include "domain/SimulationState.hpp"
#include "simulation/SimulationRuntime.hpp"

namespace traffic::simulation::systems {

void finalize_step(domain::SimulationState& state, const SimulationRuntime& runtime);

}  // namespace traffic::simulation::systems
