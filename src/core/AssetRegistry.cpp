#include "core/AssetRegistry.hpp"

namespace traffic::core {

AssetRegistry::AssetRegistry(std::filesystem::path assetRoot)
    : assetRoot_(std::move(assetRoot)) {}

bool AssetRegistry::loadTexture(const std::string& id, const std::filesystem::path& relativePath) {
    sf::Texture texture;
    if (!texture.loadFromFile((assetRoot_ / relativePath).string())) {
        return false;
    }

    texture.setSmooth(false);
    textures_[id] = std::move(texture);
    return true;
}

const sf::Texture* AssetRegistry::getTexture(const std::string& id) const {
    const auto it = textures_.find(id);
    if (it == textures_.end()) {
        return nullptr;
    }
    return &it->second;
}

const std::filesystem::path& AssetRegistry::assetRoot() const noexcept {
    return assetRoot_;
}

}  // namespace traffic::core

