#pragma once


#include<iostream>
#include<memory>
#include"CComponents.hpp"
#include"CVector.hpp"
#include<vector>
#include<map>
#include <typeindex>
#include <typeinfo>




class Entity
{
    private:
        friend class EntityManager;
        bool m_isALive = true;
        size_t m_id = 0 ;
        std::string m_tag = "Default";
        Entity();
        Entity(const size_t id , const std::string& tag);
    public:
        // std::shared_ptr<CTransform> cTransform;
        // std::shared_ptr<CShape> cShape;
        // std::shared_ptr<CCollision> cCollision;
        // std::shared_ptr<CInput> cInput;
        // std::shared_ptr<CScore> cScore;
        // std::shared_ptr<CLifespan> cLifespan;  
        bool isAlive() const;
        const std::string& tag() const;
        void destroy();
        size_t getId() const;


};

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string , std::vector<std::shared_ptr<Entity>>> EntityMap;





class EntityManager
{
    private:
        std::vector<std::shared_ptr<Entity>> m_toAdd;
        std::vector<std::shared_ptr<Entity>> m_allEntities;
        std::map<std::string , std::vector<std::shared_ptr<Entity>>> m_entityGroup;
        size_t m_totalEntities = 0;  
         
        std::unordered_map<std::type_index,std::unique_ptr<IStorage>> m_storages;
        size_t m_idOftopMostEntityUnderMouse = 0;
    public:
        void update();
        std::shared_ptr<Entity> addEntity(const std::string& tag);
        const EntityVec& getEntities();
        const EntityVec& getEntities(const std::string& tag);
        size_t getid(std::shared_ptr<Entity> p);

        template<typename T>
        void registerComponent()
        {
            m_storages[typeid(T)] =
            std::make_unique<ComponentStorage<T>>();
        }

        template<typename T>
        ComponentStorage<T>& storage()
        {
            auto it = m_storages.find(typeid(T));
        
            if (it == m_storages.end())
            {
                registerComponent<T>();
            }
        
            return static_cast<ComponentStorage<T>&>(
                *m_storages[typeid(T)]
            );
        }

        void removeAllComponents(size_t id);


        void refreshTopMostEntityUnderMouse(sf::Vector2i mousePos);
};