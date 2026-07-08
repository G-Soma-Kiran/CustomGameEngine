#pragma once
#include<map>
#include<SFML/Graphics.hpp>
#include<iostream>

struct Asset
{
    std::string name;
    std::string type;
    std::string filePath;
};

class AssetManager
{
    private:
        std::unordered_map<std::string , sf::Texture> m_textures;
    public:
        void addTexture(const std::string& textureName , const std::string& texturePath);
        const sf::Texture& getTexture(const std::string& textureName) const;
        void loadAssets( const std::optional<std::vector<Asset>>& assets );
};