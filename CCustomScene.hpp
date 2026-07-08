#pragma once
#include"CScene.hpp"

class HomeScreen : public Scene
{
    // private:
    //     std::shared_ptr<Entity> m_spawnDraggableRectangle(sf::Vector2i mousePos);
    public:
        void sDoAction(Action input , std::vector<Request>& requests) override ;
        void sUpdate() override ;
        void sRender(sf::RenderWindow& gameWindow)  override; 
        void onLoad(const std::optional<std::vector<Asset>>& assets) override ;
        void onDelete()  override; 
        bool renderBelow() override ;
        bool updateBelow() override ;
        bool propagateEventBelow() override ;
};