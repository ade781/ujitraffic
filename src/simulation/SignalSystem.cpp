#include "simulation/SignalSystem.hpp"

#include <algorithm>

namespace traffic::simulation::systems {

void update_signal(domain::SimulationState& state, double dt) noexcept {
    state.traffic_light().update(std::max(0.0, dt));
}

}  // namespace traffic::simulation::systems
