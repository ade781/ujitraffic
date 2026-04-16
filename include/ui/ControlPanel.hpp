#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include "domain/SimulationState.hpp"

namespace traffic::ui {

class ControlPanel {
public:
    ControlPanel();

    void initialize(sf::Vector2f position, sf::Vector2f size);
    void set_font_path(std::string fontPath);
    bool load_font();
    void set_state(const domain::SimulationState* state) noexcept;
    void set_running(bool running) noexcept;
    void set_paused(bool paused) noexcept;
    void set_elapsed_seconds(double seconds) noexcept;
    void update(float dt);
    void draw(sf::RenderTarget& target) const;

    [[nodiscard]] static std::string format_seconds(double seconds);
    [[nodiscard]] static std::string format_count(std::uint64_t value);
    [[nodiscard]] static std::string light_label(domain::LightState state);
    [[nodiscard]] static std::string phase_label(domain::TrafficPhase phase);
    [[nodiscard]] static std::string direction_label(domain::Direction direction);
    [[nodiscard]] static std::string vehicle_type_label(domain::VehicleType type);

private:
    void update_text_layout() const;
    sf::Color phase_color() const noexcept;
    sf::Color status_color() const noexcept;

    const domain::SimulationState* state_ {nullptr};
    std::string fontPath_ {"assets/fonts/DejaVuSans.ttf"};
    mutable bool layoutDirty_ {true};

    sf::Vector2f position_ {0.0F, 0.0F};
    sf::Vector2f size_ {320.0F, 720.0F};

    bool running_ {true};
    bool paused_ {false};
    double elapsedSeconds_ {0.0};
    float blinkTimer_ {0.0F};

    std::unique_ptr<sf::Font> font_;
    sf::RectangleShape panelBackground_;
    mutable sf::RectangleShape headerStripe_;
    mutable sf::CircleShape statusDot_;
    mutable sf::Text titleText_;
    mutable sf::Text statusText_;
    mutable sf::Text statsText_;
    mutable sf::Text footerText_;
};

}  // namespace traffic::ui
