#include<iostream>
#include<memory>
#include"CComponents.hpp"
#include"CVector.hpp"
#include"CEntityManager.hpp"
#include<vector>
#include<map>
#include<algorithm>


Entity::Entity(){};
Entity::Entity(const size_t id , const std::string& tag): m_id(id) , m_tag(tag){};

bool Entity::isAlive() const 
{
    return m_isALive;
};
const std::string& Entity::tag() const
{
    return m_tag;
};
void Entity::destroy()
{
    m_isALive = false;
};

size_t Entity::getId() const
{
    return m_id;
}
// typedef std::vector<std::shared_ptr<Entity>> EntityVec;
// typedef std::map<std::string , std::vector<std::shared_ptr<Entity>>> EntityMap;



void EntityManager::update()
{
    for(auto p : m_toAdd)
    {
        m_allEntities.push_back(p);
        m_entityGroup[p->tag()].push_back(p);
    }
    m_toAdd.clear();

    for(auto& pair : m_entityGroup)
    {
        pair.second.erase( std::remove_if(pair.second.begin() , pair.second.end() , [](const auto& p){ 
            return !p->isAlive();
        } ), pair.second.end());
    }
    m_allEntities.erase( std::remove_if(m_allEntities.begin() , m_allEntities.end() , [this](const auto& p){
        if(!p->isAlive())
           {
               removeAllComponents(p->getId());
               return true;
           }
        return false;
        } ), m_allEntities.end());
};


std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
    std::shared_ptr<Entity> p(new Entity(++m_totalEntities , tag));
    m_toAdd.push_back(p);
    return p;
};


const EntityVec& EntityManager::getEntities()
{
    return m_allEntities;
};


const EntityVec& EntityManager::getEntities(const std::string& tag)
{
    return m_entityGroup[tag];
};


size_t EntityManager::getid(std::shared_ptr<Entity> p)
{
    return p->m_id;
}

void EntityManager::removeAllComponents(size_t id)
{
    for (auto &[key , value] : m_storages)
    {
        value->remove(id);
    }
}

void EntityManager::refreshTopMostEntityUnderMouse(sf::Vector2i mousePos)
{
    size_t topMostEntityId = 0;
    int highestZIndex = INT_MIN;
    auto& RenderComponents = this->storage<CRender>();
    auto& CollisionComponents = this->storage<CCollision>();
    for ( auto& entity : this->getEntities())
    {
        if( RenderComponents.hasComponent(entity->getId()) && CollisionComponents.hasComponent(entity->getId()) )
        {
            if(CollisionComponents.getComponent(entity->getId()).localHitbox.contains( sf::Vector2f(mousePos) ) && (RenderComponents.getComponent(entity->getId()).zIndex > highestZIndex) )
            {
                highestZIndex = RenderComponents.getComponent(entity->getId()).zIndex;
                topMostEntityId = entity->getId();
            }
        }
    }
    m_idOftopMostEntityUnderMouse = topMostEntityId;
   
}