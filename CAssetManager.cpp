#include"CAssetManager.hpp"
#include<iostream>

void AssetManager::addTexture( const std::string& textureName , const std::string& texturePath )
{
    sf::Texture texture;

    if (!texture.loadFromFile(texturePath))
    {
        throw std::runtime_error(
            "Failed to load texture from file: " + texturePath);
    }

    auto [it, inserted] =
        m_textures.try_emplace(textureName, std::move(texture));

    if (!inserted)
    {
        throw std::runtime_error(
            "Texture with name '" + textureName + "' already exists in m_textures of AssetManager.\n");
    }
}

const sf::Texture& AssetManager::getTexture(const std::string& textureName) const
{
    return m_textures.at(textureName);
}