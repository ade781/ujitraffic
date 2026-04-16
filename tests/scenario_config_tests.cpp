#include "TestSupport.hpp"

#include <filesystem>
#include <fstream>

#include "simulation/ScenarioConfigIO.hpp"

using traffic::tests::require;
using traffic::tests::require_near;

TRAFFIC_TEST(scenario_document_loads_from_cfg_file) {
    const auto document = traffic::simulation::load_scenario_document("scenarios/default_single_intersection.cfg");

    require(document.name == "default_single_intersection", "scenario name should load from cfg");
    require_near(document.config.north_south_green_seconds, 12.0, 1e-9, "NS green mismatch");
    require_near(document.config.spawn_probability, 0.32, 1e-9, "spawn probability mismatch");
    require_near(document.config.geometry.center.x, 480.0, 1e-9, "geometry center x mismatch");
    require(document.config.random_seed == 1337U, "random seed mismatch");
}

TRAFFIC_TEST(scenario_document_round_trips_through_disk) {
    traffic::simulation::ScenarioDocument document;
    document.name = "round_trip";
    document.config.north_south_green_seconds = 15.0;
    document.config.east_west_green_seconds = 18.0;
    document.config.spawn_probability = 0.55;
    document.config.random_seed = 77U;
    document.config.geometry.center = {250.0, 400.0};

    const auto path = std::filesystem::path("build_mingw/test_round_trip_scenario.cfg");
    traffic::simulation::save_scenario_document(path, document);
    const auto loaded = traffic::simulation::load_scenario_document(path);

    require(loaded.name == "round_trip", "round-trip name mismatch");
    require_near(loaded.config.north_south_green_seconds, 15.0, 1e-9, "round-trip NS green mismatch");
    require_near(loaded.config.east_west_green_seconds, 18.0, 1e-9, "round-trip EW green mismatch");
    require_near(loaded.config.spawn_probability, 0.55, 1e-9, "round-trip spawn probability mismatch");
    require_near(loaded.config.geometry.center.y, 400.0, 1e-9, "round-trip geometry mismatch");
    require(loaded.config.random_seed == 77U, "round-trip seed mismatch");
}

TRAFFIC_TEST(scenario_document_ignores_unknown_per_approach_demand_keys) {
    const auto path = std::filesystem::path("build/test_per_approach_demand.cfg");
    std::filesystem::create_directories(path.parent_path());

    std::ofstream output(path);
    output << "name = per_approach_probe\n";
    output << "spawn_probability = 0.41\n";
    output << "spawn_interval_seconds = 0.15\n";
    output << "demand.north.spawn_probability = 0.90\n";
    output << "demand.south.spawn_probability = 0.05\n";
    output << "demand.east.spawn_probability = 0.10\n";
    output << "demand.west.spawn_probability = 0.25\n";
    output.close();

    const auto document = traffic::simulation::load_scenario_document(path);

    require(document.name == "per_approach_probe", "scenario name should still load");
    require_near(document.config.spawn_probability, 0.41, 1e-9, "global spawn probability should remain authoritative");
    require_near(document.config.spawn_interval_seconds, 0.15, 1e-9, "global spawn interval should still load");
}
