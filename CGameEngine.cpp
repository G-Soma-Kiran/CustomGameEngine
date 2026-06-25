#include"CGameEngine.hpp"
#include"CActionManager.hpp"





Engine::Engine(const Game& game): m_scenes(game)
{
}

void Engine::run()
{
    m_scenes.request({ RequestCommand::Push , "Default"});
    m_scenes.update();
    while(m_window.isOpen())
    {
        while(std::optional<sf::Event> opt = m_window.pollEvent())
        {
            if( opt->is<sf::Event::Closed>())
            {
                m_window.close();
                return;
            }
            m_scenes.pollEvent(opt);
        }
        m_scenes.sceneDoAction();
        m_scenes.update();
        if(m_scenes.isEmpty()){return;}
        m_scenes.sceneUpdate();
        m_scenes.sceneRender();
    }
}

