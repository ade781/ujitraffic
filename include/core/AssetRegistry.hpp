#pragma once

#include <SFML/Graphics/Texture.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace traffic::core {

class AssetRegistry {
public:
    explicit AssetRegistry(std::filesystem::path assetRoot);

    bool loadTexture(const std::string& id, const std::filesystem::path& relativePath);
    const sf::Texture* getTexture(const std::string& id) const;
    const std::filesystem::path& assetRoot() const noexcept;

private:
    std::filesystem::path assetRoot_;
    std::unordered_map<std::string, sf::Texture> textures_;
};

}  // namespace traffic::core

