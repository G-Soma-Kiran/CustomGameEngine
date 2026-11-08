#pragma once


#include"CVector.hpp"
#include<iostream>
#include<memory>
#include<SFML/Graphics.hpp>
#include<map>


struct IStorage
{
    virtual ~IStorage() = default;
    virtual void remove(size_t id) = 0;
};

template<typename T>
struct ComponentStorage : IStorage
{
    private:
        std::unordered_map<size_t , T> m_data;
        friend class EntityManager;
    public:
        void remove(size_t id) override
        {
            m_data.erase(id);
        }

        template<typename... Args>
        T& addComponent(size_t id, Args&&... args)
        {
            return m_data.emplace(
                id,
                T(std::forward<Args>(args)...)
            ).first->second;
        }

        T& getComponent(size_t id)
        {
            if(m_data.contains(id))
                return m_data.at(id);
            else
                throw std::runtime_error(std::format("There is no component of type : {} and id : {}" , typeid(T).name() , id));
        }

        const T& getComponent(size_t id) const
        {
            if(m_data.contains(id))
                return m_data.at(id);
            else
                throw std::runtime_error(std::format("There is no component of type : {} and id : {}" , typeid(T).name() , id));
        }

        
        bool hasComponent(size_t id) const
        {
            return m_data.contains(id);
        }

        void removeComponent(size_t id) 
        {
            m_data.erase(id);
        }
};

class CRender
{
    public:
        int zIndex = 0;
        bool render = true;
          
};

class CCollision
{   public:
        sf::FloatRect localHitbox;
};

class CVisual
{
    public:
        sf::Sprite sprite;
        CVisual(const sf::Texture& texture) : sprite(texture){};
};

class CDebugShape
{
    public:
        sf::RectangleShape shapeRect = sf::RectangleShape({200.0f, 100.0f});
};

class CTransform
{
    public:
        sf::Vector2f position = {0 , 0};
        sf::Vector2f velocity = {0 , 0};
        CTransform(const sf::Vector2f& p , const sf::Vector2f& v = {0 , 0} ) : position(p) , velocity(v) {};
};

class CDraggable
{
    public:
        bool isBeingDragged = false;
        sf::Vector2f dragPosition = {0 , 0};
        sf::FloatRect localDragHitbox;
        CDraggable(sf::FloatRect rect) :localDragHitbox(rect){};
};

class CMouseInteraction
{
public:
    sf::FloatRect localBounds;

    bool enabled = true;

    CMouseInteraction(sf::FloatRect localHitbox):localBounds(localHitbox){}; 
    
};