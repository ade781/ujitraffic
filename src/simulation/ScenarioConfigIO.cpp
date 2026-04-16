#include "simulation/ScenarioConfigIO.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace traffic::simulation {

namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

double read_double(
    const std::unordered_map<std::string, std::string>& values,
    const std::string& key,
    double fallback) {
    if (const auto it = values.find(key); it != values.end()) {
        return std::stod(it->second);
    }
    return fallback;
}

std::uint32_t read_uint(
    const std::unordered_map<std::string, std::string>& values,
    const std::string& key,
    std::uint32_t fallback) {
    if (const auto it = values.find(key); it != values.end()) {
        return static_cast<std::uint32_t>(std::stoul(it->second));
    }
    return fallback;
}

}  // namespace

ScenarioDocument load_scenario_document(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Failed to open scenario file: " + path.string());
    }

    std::unordered_map<std::string, std::string> values;
    std::string line;
    while (std::getline(input, line)) {
        const auto comment = line.find('#');
        if (comment != std::string::npos) {
            line.erase(comment);
        }

        line = trim(line);
        if (line.empty()) {
            continue;
        }

        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            throw std::runtime_error("Invalid scenario line: " + line);
        }

        values.emplace(trim(line.substr(0, separator)), trim(line.substr(separator + 1)));
    }

    ScenarioDocument document;
    if (const auto it = values.find("name"); it != values.end()) {
        document.name = it->second;
    }

    auto& config = document.config;
    config.north_south_green_seconds = read_double(values, "north_south_green_seconds", config.north_south_green_seconds);
    config.east_west_green_seconds = read_double(values, "east_west_green_seconds", config.east_west_green_seconds);
    config.yellow_seconds = read_double(values, "yellow_seconds", config.yellow_seconds);
    config.spawn_probability = read_double(values, "spawn_probability", config.spawn_probability);
    config.spawn_interval_seconds = read_double(values, "spawn_interval_seconds", config.spawn_interval_seconds);
    config.queue_speed_threshold = read_double(values, "queue_speed_threshold", config.queue_speed_threshold);
    config.wait_speed_threshold = read_double(values, "wait_speed_threshold", config.wait_speed_threshold);
    config.reaction_time_min_seconds = read_double(values, "reaction_time_min_seconds", config.reaction_time_min_seconds);
    config.reaction_time_max_seconds = read_double(values, "reaction_time_max_seconds", config.reaction_time_max_seconds);
    config.random_seed = read_uint(values, "random_seed", config.random_seed);
    config.yellow_decision.commit_min_time_to_stop_line_seconds = read_double(
        values,
        "yellow.commit_min_time_to_stop_line_seconds",
        config.yellow_decision.commit_min_time_to_stop_line_seconds);
    config.yellow_decision.commit_min_distance_after_stop_line = read_double(
        values,
        "yellow.commit_min_distance_after_stop_line",
        config.yellow_decision.commit_min_distance_after_stop_line);
    config.yellow_decision.comfortable_braking_buffer_meters = read_double(
        values,
        "yellow.comfortable_braking_buffer_meters",
        config.yellow_decision.comfortable_braking_buffer_meters);
    config.queue_model.queue_join_speed_threshold = read_double(
        values,
        "queue.queue_join_speed_threshold",
        config.queue_model.queue_join_speed_threshold);
    config.queue_model.queue_release_speed_threshold = read_double(
        values,
        "queue.queue_release_speed_threshold",
        config.queue_model.queue_release_speed_threshold);
    config.queue_model.queue_following_gap_meters = read_double(
        values,
        "queue.queue_following_gap_meters",
        config.queue_model.queue_following_gap_meters);
    config.queue_model.approach_detection_distance = read_double(
        values,
        "queue.approach_detection_distance",
        config.queue_model.approach_detection_distance);
    config.approach_demand[0].demand_weight = read_double(values, "demand.north.demand_weight", config.approach_demand[0].demand_weight);
    config.approach_demand[1].demand_weight = read_double(values, "demand.south.demand_weight", config.approach_demand[1].demand_weight);
    config.approach_demand[2].demand_weight = read_double(values, "demand.east.demand_weight", config.approach_demand[2].demand_weight);
    config.approach_demand[3].demand_weight = read_double(values, "demand.west.demand_weight", config.approach_demand[3].demand_weight);
    config.approach_demand[0].spawn_probability_multiplier = read_double(values, "demand.north.spawn_probability_multiplier", config.approach_demand[0].spawn_probability_multiplier);
    config.approach_demand[1].spawn_probability_multiplier = read_double(values, "demand.south.spawn_probability_multiplier", config.approach_demand[1].spawn_probability_multiplier);
    config.approach_demand[2].spawn_probability_multiplier = read_double(values, "demand.east.spawn_probability_multiplier", config.approach_demand[2].spawn_probability_multiplier);
    config.approach_demand[3].spawn_probability_multiplier = read_double(values, "demand.west.spawn_probability_multiplier", config.approach_demand[3].spawn_probability_multiplier);
    config.approach_demand[0].min_headway_seconds = read_double(values, "demand.north.min_headway_seconds", config.approach_demand[0].min_headway_seconds);
    config.approach_demand[1].min_headway_seconds = read_double(values, "demand.south.min_headway_seconds", config.approach_demand[1].min_headway_seconds);
    config.approach_demand[2].min_headway_seconds = read_double(values, "demand.east.min_headway_seconds", config.approach_demand[2].min_headway_seconds);
    config.approach_demand[3].min_headway_seconds = read_double(values, "demand.west.min_headway_seconds", config.approach_demand[3].min_headway_seconds);
    config.approach_demand[0].target_headway_seconds = read_double(values, "demand.north.target_headway_seconds", config.approach_demand[0].target_headway_seconds);
    config.approach_demand[1].target_headway_seconds = read_double(values, "demand.south.target_headway_seconds", config.approach_demand[1].target_headway_seconds);
    config.approach_demand[2].target_headway_seconds = read_double(values, "demand.east.target_headway_seconds", config.approach_demand[2].target_headway_seconds);
    config.approach_demand[3].target_headway_seconds = read_double(values, "demand.west.target_headway_seconds", config.approach_demand[3].target_headway_seconds);
    config.approach_demand[0].motorcycle_share = read_double(values, "demand.north.motorcycle_share", config.approach_demand[0].motorcycle_share);
    config.approach_demand[1].motorcycle_share = read_double(values, "demand.south.motorcycle_share", config.approach_demand[1].motorcycle_share);
    config.approach_demand[2].motorcycle_share = read_double(values, "demand.east.motorcycle_share", config.approach_demand[2].motorcycle_share);
    config.approach_demand[3].motorcycle_share = read_double(values, "demand.west.motorcycle_share", config.approach_demand[3].motorcycle_share);
    config.approach_demand[0].truck_share = read_double(values, "demand.north.truck_share", config.approach_demand[0].truck_share);
    config.approach_demand[1].truck_share = read_double(values, "demand.south.truck_share", config.approach_demand[1].truck_share);
    config.approach_demand[2].truck_share = read_double(values, "demand.east.truck_share", config.approach_demand[2].truck_share);
    config.approach_demand[3].truck_share = read_double(values, "demand.west.truck_share", config.approach_demand[3].truck_share);
    config.geometry.center.x = read_double(values, "geometry.center.x", config.geometry.center.x);
    config.geometry.center.y = read_double(values, "geometry.center.y", config.geometry.center.y);
    config.geometry.road_width = read_double(values, "geometry.road_width", config.geometry.road_width);
    config.geometry.lane_width = read_double(values, "geometry.lane_width", config.geometry.lane_width);
    config.geometry.stop_line_distance = read_double(values, "geometry.stop_line_distance", config.geometry.stop_line_distance);
    config.geometry.queue_distance = read_double(values, "geometry.queue_distance", config.geometry.queue_distance);
    config.geometry.center_size = read_double(values, "geometry.center_size", config.geometry.center_size);

    return document;
}

void save_scenario_document(const std::filesystem::path& path, const ScenarioDocument& document) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Failed to write scenario file: " + path.string());
    }

    const auto& config = document.config;
    output << "# Traffic microsimulation scenario\n";
    output << "name = " << document.name << '\n';
    output << "north_south_green_seconds = " << config.north_south_green_seconds << '\n';
    output << "east_west_green_seconds = " << config.east_west_green_seconds << '\n';
    output << "yellow_seconds = " << config.yellow_seconds << '\n';
    output << "spawn_probability = " << config.spawn_probability << '\n';
    output << "spawn_interval_seconds = " << config.spawn_interval_seconds << '\n';
    output << "queue_speed_threshold = " << config.queue_speed_threshold << '\n';
    output << "wait_speed_threshold = " << config.wait_speed_threshold << '\n';
    output << "reaction_time_min_seconds = " << config.reaction_time_min_seconds << '\n';
    output << "reaction_time_max_seconds = " << config.reaction_time_max_seconds << '\n';
    output << "random_seed = " << config.random_seed << '\n';
    output << "yellow.commit_min_time_to_stop_line_seconds = "
           << config.yellow_decision.commit_min_time_to_stop_line_seconds << '\n';
    output << "yellow.commit_min_distance_after_stop_line = "
           << config.yellow_decision.commit_min_distance_after_stop_line << '\n';
    output << "yellow.comfortable_braking_buffer_meters = "
           << config.yellow_decision.comfortable_braking_buffer_meters << '\n';
    output << "queue.queue_join_speed_threshold = " << config.queue_model.queue_join_speed_threshold << '\n';
    output << "queue.queue_release_speed_threshold = " << config.queue_model.queue_release_speed_threshold << '\n';
    output << "queue.queue_following_gap_meters = " << config.queue_model.queue_following_gap_meters << '\n';
    output << "queue.approach_detection_distance = " << config.queue_model.approach_detection_distance << '\n';
    output << "demand.north.demand_weight = " << config.approach_demand[0].demand_weight << '\n';
    output << "demand.south.demand_weight = " << config.approach_demand[1].demand_weight << '\n';
    output << "demand.east.demand_weight = " << config.approach_demand[2].demand_weight << '\n';
    output << "demand.west.demand_weight = " << config.approach_demand[3].demand_weight << '\n';
    output << "demand.north.spawn_probability_multiplier = " << config.approach_demand[0].spawn_probability_multiplier << '\n';
    output << "demand.south.spawn_probability_multiplier = " << config.approach_demand[1].spawn_probability_multiplier << '\n';
    output << "demand.east.spawn_probability_multiplier = " << config.approach_demand[2].spawn_probability_multiplier << '\n';
    output << "demand.west.spawn_probability_multiplier = " << config.approach_demand[3].spawn_probability_multiplier << '\n';
    output << "demand.north.min_headway_seconds = " << config.approach_demand[0].min_headway_seconds << '\n';
    output << "demand.south.min_headway_seconds = " << config.approach_demand[1].min_headway_seconds << '\n';
    output << "demand.east.min_headway_seconds = " << config.approach_demand[2].min_headway_seconds << '\n';
    output << "demand.west.min_headway_seconds = " << config.approach_demand[3].min_headway_seconds << '\n';
    output << "demand.north.target_headway_seconds = " << config.approach_demand[0].target_headway_seconds << '\n';
    output << "demand.south.target_headway_seconds = " << config.approach_demand[1].target_headway_seconds << '\n';
    output << "demand.east.target_headway_seconds = " << config.approach_demand[2].target_headway_seconds << '\n';
    output << "demand.west.target_headway_seconds = " << config.approach_demand[3].target_headway_seconds << '\n';
    output << "demand.north.motorcycle_share = " << config.approach_demand[0].motorcycle_share << '\n';
    output << "demand.south.motorcycle_share = " << config.approach_demand[1].motorcycle_share << '\n';
    output << "demand.east.motorcycle_share = " << config.approach_demand[2].motorcycle_share << '\n';
    output << "demand.west.motorcycle_share = " << config.approach_demand[3].motorcycle_share << '\n';
    output << "demand.north.truck_share = " << config.approach_demand[0].truck_share << '\n';
    output << "demand.south.truck_share = " << config.approach_demand[1].truck_share << '\n';
    output << "demand.east.truck_share = " << config.approach_demand[2].truck_share << '\n';
    output << "demand.west.truck_share = " << config.approach_demand[3].truck_share << '\n';
    output << "geometry.center.x = " << config.geometry.center.x << '\n';
    output << "geometry.center.y = " << config.geometry.center.y << '\n';
    output << "geometry.road_width = " << config.geometry.road_width << '\n';
    output << "geometry.lane_width = " << config.geometry.lane_width << '\n';
    output << "geometry.stop_line_distance = " << config.geometry.stop_line_distance << '\n';
    output << "geometry.queue_distance = " << config.geometry.queue_distance << '\n';
    output << "geometry.center_size = " << config.geometry.center_size << '\n';
}

}  // namespace traffic::simulation
