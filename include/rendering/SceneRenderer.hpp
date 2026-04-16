#pragma once

#include <SFML/Graphics/RenderTarget.hpp>

#include "domain/SimulationState.hpp"

namespace traffic::rendering {

class SceneRenderer {
public:
    void initialize();
    void render(sf::RenderTarget& target, const domain::SimulationState& state);

private:
    void draw_background(sf::RenderTarget& target, const domain::SimulationState& state);
    void draw_roads(sf::RenderTarget& target, const domain::SimulationState& state);
    void draw_markings(sf::RenderTarget& target, const domain::SimulationState& state);
    void draw_vehicles(sf::RenderTarget& target, const domain::SimulationState& state);
};

}  // namespace traffic::rendering
