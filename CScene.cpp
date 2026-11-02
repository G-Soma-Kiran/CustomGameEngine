#include"CScene.hpp"

SceneManager::SceneManager(const Game& game , sf::RenderWindow& gameWindow , sf::Clock& gameClock ):m_game(game) , m_gameWindow(gameWindow) , m_gameClock(gameClock)
{

}

void SceneManager::m_pushScene(const std::string& sceneName)
{
    auto scene = m_game.makeScene(sceneName);
    scene->setSceneName(sceneName);
    scene->setSceneId(++m_totalScenes);
    scene->setGameClockSource(m_gameClock);
    const auto& assets = m_game.getAssetsForScene(sceneName); 
    scene->onLoad(assets);
    m_sceneStack.push_back(std::move(scene));
}

void SceneManager::m_popScene()
{
    m_sceneStack.pop_back();
}

void SceneManager::m_clearScenes()
{
    m_sceneStack.clear();
}

void SceneManager::m_clearAndPushScene(const std::string& sceneName)
{
    m_sceneStack.clear();
    SceneManager::m_pushScene(sceneName);

}

void SceneManager::m_popAndPushScene(const std::string& sceneName)
{
    m_sceneStack.pop_back();
    SceneManager::m_pushScene(sceneName);
}

void SceneManager::update()
{
    for(auto& req : m_toUpdate)
    {
        switch(req.type)
        {
            case RequestCommand::Push:
                m_pushScene(req.sceneName);
                break;
            case RequestCommand::Pop:
                m_popScene();
                break;
            case RequestCommand::Clear:
                m_clearScenes();
                break;
            case RequestCommand::ClearAndPush:
                m_clearAndPushScene(req.sceneName);
                break;
            case RequestCommand::Replace:
                m_popAndPushScene(req.sceneName);
                break;
            default:
                std::cerr << "No Matching RequestCommand\n";
                break;
        }
    }
    m_toUpdate.clear();
}

void SceneManager::sceneDoAction()
{
    size_t index=0;
    for( size_t i = m_sceneStack.size()-1 ;i>0 ; i--)
    {
        if(m_sceneStack[i]->propagateEventBelow() == false)
        {
            index = i;
            break;
        }
    }
    for( size_t i=index ; i < m_sceneStack.size(); i++)
    {
        auto& ptr = m_sceneStack[i];
        for( auto& rawActionBundle : ptr->m_actionManager.getActions())
        {
            auto refinedActions = ptr->m_uisystem.refineAction(rawActionBundle);
            for( auto& refinedAction : refinedActions )
            {
                ptr->sDoAction(refinedAction , m_toUpdate); 
            }
        }
    }
}

ActionBinding& Scene::registerActionBinding( const std::string& actionName)
{
    return m_actionManager.addBinding(actionName);
}

void SceneManager::pollEventToEdgeTriggerActions(std::optional<sf::Event> opt , sf::Time eventTriggerTime)
{
    size_t index=0;
    for( size_t i = m_sceneStack.size()-1 ;i>0 ; i--)
    {
        if(m_sceneStack[i]->propagateEventBelow() == false)
        {
            index = i;
            break;
        }
    }

    for( size_t i=index ; i < m_sceneStack.size(); i++)
    {
        
        m_sceneStack[i]->m_actionManager.m_updateGameTimeData(eventTriggerTime , m_sceneStack[i]->m_isPaused);
        m_sceneStack[i]->m_actionManager.setDeviceState(opt);
        m_sceneStack[i]->m_actionManager.processDeviceStateToAction();
    }
}

void SceneManager::sceneUpdate(sf::Time dt)
{
    size_t index=0;
    for( size_t i = m_sceneStack.size()-1 ;i>0 ; i--)
    {
        if(m_sceneStack[i]->updateBelow() == false)
        {
            index = i;
            break;
        }
    }
    for( size_t i = index ; i < m_sceneStack.size() ; i++)
    {
        m_sceneStack[i]->sUpdate(dt);
    }

}

void SceneManager::sceneRender(sf::RenderWindow& gameWindow)
{
    size_t index=0;
    for( size_t i = m_sceneStack.size()-1 ;i>0 ; i--)
    {
        if(m_sceneStack[i]->renderBelow() == false)
        {
            index = i;
            break;
        }
    }
    for( size_t i = index ; i < m_sceneStack.size() ; i++)
    {
        m_sceneStack[i]->sRender(gameWindow);
    }

}

bool SceneManager::isEmpty()
{
    return m_sceneStack.empty();
}

void SceneManager::request(Request req)
{
    m_toUpdate.push_back(req);
}

const std::optional<std::vector<Asset>> Game::getAssetsForScene( const std::string& sceneName ) const
{
    if(hasAssets(sceneName))
        return m_gameAssets.at(sceneName) ;
    return std::nullopt;
}

std::unique_ptr<Scene> Game::makeScene(const std::string& sceneName ) const
{   
    return m_sceneMakers.at(sceneName)();
}

void Game::addGameAsset( const Asset& asset , const std::vector<std::string> ownerScenes )
{
    for(const std::string& ownerScene : ownerScenes)
    {
        m_gameAssets[ownerScene].push_back(asset);
    }
}

bool Game::hasAssets(const std::string& sceneName ) const
{
    return (m_gameAssets.count(sceneName) > 0);
}

void SceneManager::pollEventToLevelTriggerActions(sf::Time eventTriggerTime)
{
    size_t index=0;
    for( size_t i = m_sceneStack.size()-1 ;i>0 ; i--)
    {
        if(m_sceneStack[i]->propagateEventBelow() == false)
        {
            index = i;
            break;
        }
    }

    for( size_t i=index ; i < m_sceneStack.size(); i++)
    {
        m_sceneStack[i]->m_actionManager.m_updateGameTimeData(eventTriggerTime , m_sceneStack[i]->m_isPaused);
        m_sceneStack[i]->m_actionManager.processOngoingActions();
    }
}

void Scene::setSceneName(const std::string& sceneName)
{
    m_sceneName = sceneName;
}
void Scene::setSceneId(size_t sceneId)
{
    m_sceneId = sceneId;
}

const std::string& Scene::getSceneName() const
{
    return m_sceneName;
}
size_t Scene::getSceneId() const
{
    return m_sceneId;
}

void Scene::setGameClockSource(const sf::Clock& gameClock)
{
    m_gameClockPtr = &gameClock;
}