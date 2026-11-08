#pragma once
#include"CScene.hpp"
#include"CAssetManager.hpp"
#include<functional>
#include<map>
#include"CActionManager.hpp"






class Engine
{ 
    private:
        sf::RenderWindow m_window;
        sf::Clock m_gameClock; 
        SceneManager m_scenes; 
        sf::Time m_currentFrameStartTime = sf::Time::Zero;
        sf::Time m_previousFrameStartTime = sf::Time::Zero;
        void init();
        std::string m_defaultScene;
    public:
        Engine(const Game& game);
        void run();  

};


























