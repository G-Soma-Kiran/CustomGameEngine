#pragma once
#include"CScene.hpp"

class HomeScreen : public Scene
{
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