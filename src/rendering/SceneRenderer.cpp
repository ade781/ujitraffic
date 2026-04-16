#include "rendering/SceneRenderer.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace traffic::rendering {

namespace {
sf::Color vehicle_color(traffic::domain::VehicleType type) {
    switch (type) {
    case traffic::domain::VehicleType::Car:
        return sf::Color(70, 150, 255);
    case traffic::domain::VehicleType::Motorcycle:
        return sf::Color(255, 180, 40);
    case traffic::domain::VehicleType::Truck:
        return sf::Color(155, 95, 60);
    }
    return sf::Color::White;
}

}  // namespace

void SceneRenderer::initialize() {}

void SceneRenderer::render(sf::RenderTarget& target, const domain::SimulationState& state) {
    draw_background(target, state);
    draw_roads(target, state);
    draw_markings(target, state);
    draw_vehicles(target, state);
}

void SceneRenderer::draw_background(sf::RenderTarget& target, const domain::SimulationState& state) {
    target.clear(sf::Color(48, 130, 64));
}

void SceneRenderer::draw_roads(sf::RenderTarget& target, const domain::SimulationState& state) {
    const auto& geometry = state.geometry();
    const float roadWidth = static_cast<float>(geometry.road_width);
    const float centerX = static_cast<float>(geometry.center.x);
    const float centerY = static_cast<float>(geometry.center.y);
    const float sceneWidth = centerX * 2.0F;
    const float sceneHeight = centerY * 2.0F;

    sf::RectangleShape verticalRoad;
    verticalRoad.setSize({roadWidth, sceneHeight});
    verticalRoad.setFillColor(sf::Color(60, 60, 60));
    verticalRoad.setPosition(centerX - roadWidth * 0.5F, 0.0F);
    target.draw(verticalRoad);

    sf::RectangleShape horizontalRoad;
    horizontalRoad.setSize({sceneWidth, roadWidth});
    horizontalRoad.setFillColor(sf::Color(60, 60, 60));
    horizontalRoad.setPosition(0.0F, centerY - roadWidth * 0.5F);
    target.draw(horizontalRoad);
}

void SceneRenderer::draw_markings(sf::RenderTarget& target, const domain::SimulationState& state) {
    const auto& geometry = state.geometry();
    const float roadWidth = static_cast<float>(geometry.road_width);
    const float centerX = static_cast<float>(geometry.center.x);
    const float centerY = static_cast<float>(geometry.center.y);
    const float stopLineOffset = static_cast<float>(geometry.stop_line_distance);
    const float centerSize = static_cast<float>(geometry.center_size);
    const float sceneWidth = centerX * 2.0F;
    const float sceneHeight = centerY * 2.0F;

    sf::RectangleShape laneDividerVertical;
    laneDividerVertical.setSize({4.0F, sceneHeight});
    laneDividerVertical.setFillColor(sf::Color(240, 240, 240));
    laneDividerVertical.setPosition(centerX - 2.0F, 0.0F);
    target.draw(laneDividerVertical);

    sf::RectangleShape laneDividerHorizontal;
    laneDividerHorizontal.setSize({sceneWidth, 4.0F});
    laneDividerHorizontal.setFillColor(sf::Color(240, 240, 240));
    laneDividerHorizontal.setPosition(0.0F, centerY - 2.0F);
    target.draw(laneDividerHorizontal);

    sf::RectangleShape stopLine;
    stopLine.setFillColor(sf::Color::White);

    stopLine.setSize({roadWidth * 0.5F, 4.0F});
    stopLine.setPosition(centerX - roadWidth * 0.5F, centerY + stopLineOffset);
    target.draw(stopLine);
    stopLine.setPosition(centerX, centerY - stopLineOffset);
    target.draw(stopLine);

    stopLine.setSize({4.0F, roadWidth * 0.5F});
    stopLine.setPosition(centerX + stopLineOffset, centerY - roadWidth * 0.5F);
    target.draw(stopLine);
    stopLine.setPosition(centerX - stopLineOffset, centerY);
    target.draw(stopLine);

    sf::RectangleShape crosswalkStripe;
    crosswalkStripe.setFillColor(sf::Color(235, 235, 235));
    crosswalkStripe.setSize({10.0F, 16.0F});
    for (int i = 0; i < 6; ++i) {
        const float offset = static_cast<float>(i * 18 - 45);
        crosswalkStripe.setPosition(centerX + offset, centerY + roadWidth * 0.5F + 10.0F);
        target.draw(crosswalkStripe);
        crosswalkStripe.setPosition(centerX + offset, centerY - roadWidth * 0.5F - 26.0F);
        target.draw(crosswalkStripe);
    }

    sf::RectangleShape intersectionBox;
    intersectionBox.setSize({centerSize, centerSize});
    intersectionBox.setOrigin(centerSize * 0.5F, centerSize * 0.5F);
    intersectionBox.setPosition(centerX, centerY);
    intersectionBox.setFillColor(sf::Color(70, 70, 70));
    target.draw(intersectionBox);
}

void SceneRenderer::draw_vehicles(sf::RenderTarget& target, const domain::SimulationState& state) {
    for (const auto& vehicle : state.vehicles()) {
        if (!vehicle.active()) {
            continue;
        }

        sf::RectangleShape shape;
        const bool vertical = vehicle.direction() == domain::Direction::North ||
                              vehicle.direction() == domain::Direction::South;
        if (vertical) {
            shape.setSize({14.0F, 30.0F});
        } else {
            shape.setSize({30.0F, 14.0F});
        }

        shape.setOrigin(shape.getSize().x * 0.5F, shape.getSize().y * 0.5F);
        shape.setPosition(static_cast<float>(vehicle.x()), static_cast<float>(vehicle.y()));
        shape.setFillColor(vehicle_color(vehicle.type()));
        shape.setOutlineColor(sf::Color::Black);
        shape.setOutlineThickness(1.0F);
        target.draw(shape);
    }
}

}  // namespace traffic::rendering
