#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstdint>

#include "core/AppConfig.hpp"
#include "core/AssetRegistry.hpp"
#include "core/FixedTimestep.hpp"
#include "rendering/SceneRenderer.hpp"
#include "simulation/SimulationEngine.hpp"
#include "ui/ControlPanel.hpp"

namespace traffic::app {

class Application {
public:
    explicit Application(core::AppConfig config);
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void initializeScene();
    void handle_runtime_keypress(sf::Keyboard::Key key);
    void apply_runtime_config(bool resetSimulation);
    void reset_simulation();

    core::AppConfig config_;
    sf::RenderWindow window_;
    core::AssetRegistry assets_;
    sf::Clock deltaClock_;
    core::FixedTimestep fixedTimestep_;
    simulation::SimulationEngine simulationEngine_;
    domain::SimulationConfig runtimeConfig_ {};
    bool paused_ {false};
    std::uint64_t simulationStepCount_ {0};
    float smoothedFrameSeconds_ {0.0F};
    double elapsedSeconds_ {0.0};
    rendering::SceneRenderer sceneRenderer_ {};
    ui::ControlPanel controlPanel_ {};

    sf::RectangleShape roadVertical_;
    sf::RectangleShape roadHorizontal_;
    sf::RectangleShape laneDividerVertical_;
    sf::RectangleShape laneDividerHorizontal_;
    sf::RectangleShape infoPanel_;

    sf::Sprite roadPreview_;
    sf::Sprite vehiclePreviewA_;
    sf::Sprite vehiclePreviewB_;

    sf::Font font_;
    sf::Text titleText_;
    sf::Text bodyText_;
    sf::Text runtimeText_;
};

}  // namespace traffic::app
