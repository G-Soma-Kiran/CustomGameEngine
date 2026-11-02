#pragma once
#include"CEntityManager.hpp"
#include<map>
#include"CActionManager.hpp"
#include"CAssetManager.hpp"
#include<functional>
#include"UISystem.hpp"

class Action;
class InputCondition;
class ActionManager;

enum class RequestCommand
{
    Push,
    Pop,
    Clear,
    Replace,
    ClearAndPush,
};

struct Request
{
    RequestCommand type;
    std::string sceneName;
};



class Scene
{
    private:
        void setSceneName(const std::string& sceneName);
        void setSceneId(size_t sceneId);
        void setGameClockSource(const sf::Clock& gameClock);
        std::string m_sceneName;
        size_t m_sceneId;
        const sf::Clock* m_gameClockPtr = nullptr;
    protected:
        EntityManager  m_entities;
        size_t m_currentFrame =0;
        bool m_isPaused = false;
        AssetManager m_assetManager;
        ActionManager m_actionManager;
        UISystem m_uisystem; 
        friend class SceneManager;
    public:
        Scene():m_uisystem(m_entities , m_actionManager){};

        ActionBinding& registerActionBinding( const std::string& actionName);
        virtual void sDoAction(RefinedAction& input , std::vector<Request>& requests) = 0;
        virtual void sUpdate(sf::Time dt) = 0;
        virtual void sRender(sf::RenderWindow& gameWindow) = 0;
        
        virtual void onLoad(const std::optional<std::vector<Asset>>& assets) = 0;
        virtual void onDelete() = 0;
        
        virtual bool renderBelow() = 0;
        virtual bool updateBelow() = 0;
        virtual bool propagateEventBelow() = 0;
        virtual ~Scene() = default;

        const std::string& getSceneName() const;
        size_t getSceneId() const;

};


class Game
{
    private:
        std::unordered_map<std::string , std::function<std::unique_ptr<Scene>()>> m_sceneMakers;
        std::unordered_map<std::string , std::vector<Asset>> m_gameAssets;
    public:
        template<typename sceneDerivative>
        void addSceneMaker(const std::string& sceneName)
        {
            m_sceneMakers[sceneName] = []()
            {
                return std::make_unique<sceneDerivative>();
            };
        };
        void addGameAsset( const Asset& asset , const std::vector<std::string> ownerScenes );
        bool hasAssets( const std::string& sceneName ) const;
        std::unique_ptr<Scene> makeScene(const std::string& sceneName) const;
        const std::optional<std::vector<Asset>> getAssetsForScene( const std::string& sceneName ) const;
        // Game();
};

class SceneManager
{
    private:
        std::vector< std::unique_ptr<Scene>> m_sceneStack;   
        std::vector<Request> m_toUpdate;
        const Game& m_game;
        void m_pushScene(const std::string& sceneName);
        void m_popScene();
        void m_clearScenes();
        void m_clearAndPushScene(const std::string& sceneName);
        void m_popAndPushScene(const std::string& sceneName);
        sf::RenderWindow& m_gameWindow;
        size_t m_totalScenes = 0;
        const sf::Clock& m_gameClock;
    public:
        void update();
        void sceneDoAction();
        void sceneUpdate(sf::Time dt);
        void sceneRender(sf::RenderWindow& gameWindow);
        bool isEmpty();
        void pollEventToEdgeTriggerActions(std::optional<sf::Event> opt, sf::Time eventTriggerTime);
        void pollEventToLevelTriggerActions(sf::Time eventTriggerTime);
        SceneManager(const Game& game , sf::RenderWindow& gameWindow , sf::Clock& gameClock );
        void request(Request req);
        
};

