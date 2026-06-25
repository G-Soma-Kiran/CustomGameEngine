#pragma once
#include"CEntityManager.hpp"
#include<map>
#include"CActionManager.hpp"
#include"CAssetManager.hpp"
#include<functional>

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
    protected:
        const std::string m_id = "Default";
        EntityManager  m_entities;
        size_t m_currentFrame =0;
        bool m_isPaused = false;
        AssetManager m_assetManager;
        ActionManager m_actionManager;
        friend class SceneManager;
    public:
        void registerActionBinding( const std::string& actionName , std::vector<std::unique_ptr<InputCondition>> conditions);
        virtual void sDoAction(Action input , std::vector<Request>& requests) = 0;
        virtual void sUpdate() = 0;
        virtual void sRender() = 0;
        
        virtual void onLoad(std::vector<Asset> assets) = 0;
        virtual void onDelete() = 0;
        
        virtual bool renderBelow() = 0;
        virtual bool updateBelow() = 0;
        virtual bool propagateEventBelow() = 0;
        virtual ~Scene() = default;

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

        std::unique_ptr<Scene> makeScene(const std::string& sceneName) const;
        const std::vector<Asset> getAssetsForScene( const std::string& sceneName ) const;
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
    public:
        void update();
        void sceneDoAction();
        void sceneUpdate();
        void sceneRender();
        bool isEmpty();
        void pollEvent(std::optional<sf::Event> opt);
        SceneManager(const Game& game);
        void request(Request req);
        
};

// class HomeScreen : public Scene
// {

// };