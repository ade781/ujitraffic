#pragma once

#include <array>
#include <random>

namespace traffic::simulation {

struct SimulationRuntime {
    std::mt19937 rng {1337U};
    std::uniform_real_distribution<double> probability {0.0, 1.0};
    std::uniform_real_distribution<double> reaction_delay {0.2, 0.8};
    double spawn_accumulator_seconds {0.0};
    double spawn_interval_seconds {0.2};
    std::array<double, 4> approach_spawn_accumulator_seconds {};
    double sim_width {960.0};
    double sim_height {720.0};
    double offscreen_margin {80.0};
};

}  // namespace traffic::simulation
