#pragma once

#include <filesystem>
#include <string>

#include "domain/SimulationState.hpp"

namespace traffic::simulation {

struct ScenarioDocument {
    domain::SimulationConfig config {};
    std::string name {"unnamed"};
};

ScenarioDocument load_scenario_document(const std::filesystem::path& path);
void save_scenario_document(const std::filesystem::path& path, const ScenarioDocument& document);

}  // namespace traffic::simulation
