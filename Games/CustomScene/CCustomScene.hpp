#pragma once
#include"CGameEngine.hpp"

class HomeScreen : public Scene
{
    private:
        // std::shared_ptr<Entity> m_spawnDraggableRectangle(sf::Vector2i mousePos);
        bool m_inputTestLaunched = false;
    public:
        void sDoAction(RefinedAction& input) override ;
        void sUpdate(sf::Time dt) override ;
        void sRender(sf::RenderWindow& gameWindow)  override; 
        void onLoad(const std::optional<std::vector<Asset>>& assets) override ;
        void onDelete()  override; 
        bool renderBelow() override ;
        bool updateBelow() override ;
};