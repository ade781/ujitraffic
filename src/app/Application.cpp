#include "app/Application.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "core/Logger.hpp"

namespace traffic::app {

namespace {
constexpr float kRoadWidth = 180.0F;
constexpr float kPanelWidth = 320.0F;
constexpr float kPreviewScale = 4.0F;
constexpr const char* kAssetFontPath = "assets/fonts/DejaVuSans.ttf";
constexpr const char* kSystemFontPath = "C:/Windows/Fonts/segoeui.ttf";
constexpr double kGreenStepSeconds = 2.0;
constexpr double kDemandStep = 0.02;
constexpr std::uint32_t kSeedStep = 101U;
}  // namespace

Application::Application(core::AppConfig config)
    : config_(std::move(config)),
      window_(sf::VideoMode(config_.windowWidth, config_.windowHeight), config_.windowTitle),
      assets_("assets"),
      fixedTimestep_(1.0F / 60.0F) {
    window_.setFramerateLimit(config_.frameLimit);
    core::default_logger().info("Application started");
    simulationEngine_.initialize(
        static_cast<float>(config_.windowWidth) - kPanelWidth,
        static_cast<float>(config_.windowHeight));
    sceneRenderer_.initialize();
    controlPanel_.initialize(
        {static_cast<float>(config_.windowWidth) - kPanelWidth, 0.0F},
        {kPanelWidth, static_cast<float>(config_.windowHeight)});
    controlPanel_.set_font_path(kAssetFontPath);
    if (!controlPanel_.load_font()) {
        controlPanel_.set_font_path(kSystemFontPath);
        controlPanel_.load_font();
    }
    runtimeConfig_ = simulationEngine_.state().config();
    initializeScene();
}

void Application::run() {
    while (window_.isOpen()) {
        processEvents();
        const float frameSeconds = deltaClock_.restart().asSeconds();
        smoothedFrameSeconds_ = smoothedFrameSeconds_ == 0.0F
            ? frameSeconds
            : (smoothedFrameSeconds_ * 0.9F) + (frameSeconds * 0.1F);

        fixedTimestep_.add_frame_time(std::min(frameSeconds, 0.25F));
        while (fixedTimestep_.consume_step()) {
            update(fixedTimestep_.step_seconds());
        }
        render();
    }
}

void Application::processEvents() {
    sf::Event event {};
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            core::default_logger().info("Window close requested");
            window_.close();
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                window_.close();
            } else {
                handle_runtime_keypress(event.key.code);
            }
        }
    }
}

void Application::update(float dt) {
    if (!paused_) {
        simulationStepCount_++;
        elapsedSeconds_ += dt;
        simulationEngine_.update(dt);
    }
    controlPanel_.set_state(&simulationEngine_.state());
    controlPanel_.set_running(window_.isOpen());
    controlPanel_.set_paused(paused_);
    controlPanel_.set_elapsed_seconds(elapsedSeconds_);
    controlPanel_.update(dt);

    const float pulse = 0.5F + (static_cast<float>(simulationStepCount_ % 120) / 240.0F);
    vehiclePreviewA_.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(180 + pulse * 60.0F)));
    vehiclePreviewB_.move(8.0F * dt, 0.0F);

    const float panelLimit = static_cast<float>(config_.windowWidth) - kPanelWidth - 30.0F;
    if (vehiclePreviewB_.getPosition().x > panelLimit) {
        vehiclePreviewB_.setPosition(120.0F, vehiclePreviewB_.getPosition().y);
    }

    if (font_.getInfo().family != "") {
        std::ostringstream runtime;
        runtime << std::fixed << std::setprecision(3);
        const auto& state = simulationEngine_.state();
        const auto& metrics = state.metrics();
        runtime << "Runtime:\n";
        runtime << "- fixed step: " << fixedTimestep_.step_seconds() << " s\n";
        runtime << "- sim steps: " << simulationStepCount_ << '\n';
        runtime << "- frame dt: " << smoothedFrameSeconds_ << " s\n";
        runtime << "- alpha: " << fixedTimestep_.alpha() << '\n';
        runtime << "- vehicles active: " << state.active_vehicle_count() << '\n';
        runtime << "- vehicles done: " << metrics.completed_vehicles << '\n';
        runtime << "- queue NS: " << metrics.north_south_queue << '\n';
        runtime << "- queue EW: " << metrics.east_west_queue << '\n';
        runtime << "- avg wait done: " << state.average_completed_wait_time() << " s\n";
        runtime << "- paused: " << (paused_ ? "yes" : "no") << '\n';
        runtime << "- demand p: " << runtimeConfig_.spawn_probability << '\n';
        runtime << "- green NS/EW: " << runtimeConfig_.north_south_green_seconds << "/"
                << runtimeConfig_.east_west_green_seconds << " s\n";
        runtimeText_.setString(runtime.str());
    }
}

void Application::render() {
    sceneRenderer_.render(window_, simulationEngine_.state());
    controlPanel_.draw(window_);
    window_.display();
}

void Application::initializeScene() {
    const float centerX = (static_cast<float>(config_.windowWidth) - kPanelWidth) * 0.5F;
    const float centerY = static_cast<float>(config_.windowHeight) * 0.5F;

    roadVertical_.setSize({kRoadWidth, static_cast<float>(config_.windowHeight)});
    roadVertical_.setFillColor(sf::Color(60, 60, 60));
    roadVertical_.setPosition(centerX - kRoadWidth * 0.5F, 0.0F);

    roadHorizontal_.setSize({static_cast<float>(config_.windowWidth) - kPanelWidth, kRoadWidth});
    roadHorizontal_.setFillColor(sf::Color(60, 60, 60));
    roadHorizontal_.setPosition(0.0F, centerY - kRoadWidth * 0.5F);

    laneDividerVertical_.setSize({4.0F, static_cast<float>(config_.windowHeight)});
    laneDividerVertical_.setFillColor(sf::Color(230, 230, 230));
    laneDividerVertical_.setPosition(centerX - 2.0F, 0.0F);

    laneDividerHorizontal_.setSize({static_cast<float>(config_.windowWidth) - kPanelWidth, 4.0F});
    laneDividerHorizontal_.setFillColor(sf::Color(230, 230, 230));
    laneDividerHorizontal_.setPosition(0.0F, centerY - 2.0F);

    infoPanel_.setSize({kPanelWidth, static_cast<float>(config_.windowHeight)});
    infoPanel_.setFillColor(sf::Color(28, 28, 32));
    infoPanel_.setPosition(static_cast<float>(config_.windowWidth) - kPanelWidth, 0.0F);

    assets_.loadTexture("road_tiles", "roads/road_tiles_cc0.png");
    assets_.loadTexture("vehicle_a", "vehicles/car_topdown_cc0.png");
    assets_.loadTexture("vehicle_b", "vehicles/car_compact_cc0.png");

    if (const auto* texture = assets_.getTexture("road_tiles")) {
        roadPreview_.setTexture(*texture);
        roadPreview_.setScale(kPreviewScale, kPreviewScale);
        roadPreview_.setPosition(static_cast<float>(config_.windowWidth) - 290.0F, 70.0F);
    }

    if (const auto* texture = assets_.getTexture("vehicle_a")) {
        vehiclePreviewA_.setTexture(*texture);
        vehiclePreviewA_.setScale(3.0F, 3.0F);
        vehiclePreviewA_.setPosition(centerX - 22.0F, 90.0F);
    }

    if (const auto* texture = assets_.getTexture("vehicle_b")) {
        vehiclePreviewB_.setTexture(*texture);
        vehiclePreviewB_.setScale(3.0F, 3.0F);
        vehiclePreviewB_.setPosition(centerX + 40.0F, centerY + 20.0F);
    }

    font_.loadFromFile(kAssetFontPath);
    if (font_.getInfo().family == "") {
        font_.loadFromFile(kSystemFontPath);
    }
    if (font_.getInfo().family != "") {
        titleText_.setFont(font_);
        titleText_.setCharacterSize(26);
        titleText_.setFillColor(sf::Color::White);
        titleText_.setString("Stage 3 Domain Model");
        titleText_.setPosition(static_cast<float>(config_.windowWidth) - 295.0F, 20.0F);

        std::ostringstream body;
        body << "Stack:\n";
        body << "- C++20\n";
        body << "- CMake\n";
        body << "- SFML 2D runtime\n\n";
        body << "Status:\n";
        body << "- project skeleton ready\n";
        body << "- asset registry ready\n";
        body << "- road/vehicle CC0 assets imported\n";
        body << "- fixed timestep integrated\n";
        body << "- logger integrated\n";
        body << "- traffic domain integrated\n";
        body << "- simulation state visible\n";
        body << "- runtime tuning keys enabled\n";

        bodyText_.setFont(font_);
        bodyText_.setCharacterSize(18);
        bodyText_.setFillColor(sf::Color(220, 220, 220));
        bodyText_.setString(body.str());
        bodyText_.setPosition(static_cast<float>(config_.windowWidth) - 295.0F, 320.0F);

        runtimeText_.setFont(font_);
        runtimeText_.setCharacterSize(16);
        runtimeText_.setFillColor(sf::Color(180, 220, 255));
        runtimeText_.setPosition(static_cast<float>(config_.windowWidth) - 295.0F, 470.0F);
    } else {
        core::default_logger().warn("No usable font found for legacy text overlays");
    }
}

void Application::handle_runtime_keypress(sf::Keyboard::Key key) {
    auto updated = runtimeConfig_;
    bool applyChange = false;
    bool resetSimulation = false;

    switch (key) {
    case sf::Keyboard::Space:
        paused_ = !paused_;
        break;
    case sf::Keyboard::R:
        reset_simulation();
        break;
    case sf::Keyboard::Q:
        updated.north_south_green_seconds += kGreenStepSeconds;
        applyChange = true;
        break;
    case sf::Keyboard::A:
        updated.north_south_green_seconds =
            std::max(2.0, updated.north_south_green_seconds - kGreenStepSeconds);
        applyChange = true;
        break;
    case sf::Keyboard::W:
        updated.east_west_green_seconds += kGreenStepSeconds;
        applyChange = true;
        break;
    case sf::Keyboard::S:
        updated.east_west_green_seconds =
            std::max(2.0, updated.east_west_green_seconds - kGreenStepSeconds);
        applyChange = true;
        break;
    case sf::Keyboard::Up:
        updated.spawn_probability = std::min(1.0, updated.spawn_probability + kDemandStep);
        applyChange = true;
        break;
    case sf::Keyboard::Down:
        updated.spawn_probability = std::max(0.0, updated.spawn_probability - kDemandStep);
        applyChange = true;
        break;
    case sf::Keyboard::LBracket:
        updated.random_seed = updated.random_seed > kSeedStep
            ? updated.random_seed - kSeedStep
            : 1U;
        applyChange = true;
        resetSimulation = true;
        break;
    case sf::Keyboard::RBracket:
        updated.random_seed += kSeedStep;
        applyChange = true;
        resetSimulation = true;
        break;
    default:
        break;
    }

    if (applyChange) {
        runtimeConfig_ = updated;
        apply_runtime_config(resetSimulation);
    }
}

void Application::apply_runtime_config(bool resetSimulation) {
    simulationEngine_.apply_config(runtimeConfig_, resetSimulation);
    if (resetSimulation) {
        simulationStepCount_ = 0;
        elapsedSeconds_ = 0.0;
        smoothedFrameSeconds_ = 0.0F;
    }
}

void Application::reset_simulation() {
    simulationEngine_.reset();
    simulationStepCount_ = 0;
    elapsedSeconds_ = 0.0;
    smoothedFrameSeconds_ = 0.0F;
}

}  // namespace traffic::app
