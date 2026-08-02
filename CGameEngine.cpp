#include"CGameEngine.hpp"
#include"CActionManager.hpp"





Engine::Engine(const Game& game): m_scenes(game , m_window , m_gameClock)
{
    init();
}

void Engine::run()
{
    m_gameClock.start();
    m_scenes.request({ RequestCommand::Push , "Default"});
    m_scenes.update();
    while(m_window.isOpen())
    {
        m_previousFrameStartTime = m_currentFrameStartTime;
        m_currentFrameStartTime = m_gameClock.getElapsedTime();
        sf::Time dt =   m_currentFrameStartTime - m_previousFrameStartTime;
        while(std::optional<sf::Event> opt = m_window.pollEvent())
        {
            if( opt->is<sf::Event::Closed>())
            {
                m_window.close();
                return;
            }
            m_scenes.pollEventToEdgeTriggerActions(opt , m_gameClock.getElapsedTime() );
        }
        m_scenes.pollEventToLevelTriggerActions( m_gameClock.getElapsedTime() );
        m_scenes.sceneDoAction();
        m_scenes.update();
        if(m_scenes.isEmpty()){return;}
        m_scenes.sceneUpdate(dt);
        m_window.clear();
        m_scenes.sceneRender(m_window);
        m_window.display();
    }
}

void Engine::init()
{
    m_window.create(sf::VideoMode({800, 600}) , "TEST" , sf::Style::Default , sf::State::Windowed );
}