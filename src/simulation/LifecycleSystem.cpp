#include "simulation/LifecycleSystem.hpp"

namespace traffic::simulation::systems {

void finalize_step(domain::SimulationState& state, const SimulationRuntime& runtime) {
    for (auto& vehicle : state.vehicles()) {
        if (vehicle.x() < -runtime.offscreen_margin ||
            vehicle.x() > runtime.sim_width + runtime.offscreen_margin ||
            vehicle.y() < -runtime.offscreen_margin ||
            vehicle.y() > runtime.sim_height + runtime.offscreen_margin) {
            vehicle.set_active(false);
            state.mark_completed(vehicle);
        }
    }

    state.remove_inactive_vehicles();
    state.update_metrics();
}

}  // namespace traffic::simulation::systems
