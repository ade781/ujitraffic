#pragma once

#include "domain/SimulationState.hpp"

namespace traffic::simulation::systems {

void update_signal(domain::SimulationState& state, double dt) noexcept;

}  // namespace traffic::simulation::systems
