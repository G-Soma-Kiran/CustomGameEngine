#pragma once
#include"CScene.hpp"
#include"CAssetManager.hpp"
#include<functional>
#include<map>
#include"CActionManager.hpp"






class Engine
{ 
    sf::RenderWindow m_window;
    SceneManager m_scenes;    
    void init();
    public:
        Engine(const Game& game);
        void run();  

};


























