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
            return m_data.at(id);
        }
        const T& getComponent(size_t id) const
        {
            return m_data.at(id);
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

// template<typename component>
// class ComponentStorage
// {
//     private:
//         std::unordered_map<size_t id , component> data;
//     public:
//         void add(size_t id)
//         {
//             data[id] = std::make_shared<component>( )
//         }
// };


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
        Vector position = {0 , 0};
        Vector velocity = {0 , 0};
        CTransform(const Vector& p , const Vector& v = {0 , 0} ) : position(p) , velocity(v) {};
};

class CDraggable
{
    public:
        bool isBeingDragged = false;
        Vector dragPosition = {0 , 0};
        sf::FloatRect localDragHitbox;
        CDraggable(sf::FloatRect rect) :localDragHitbox(rect){};
};

class CMouseInteraction
{
public:
    sf::FloatRect localBounds;

    bool enabled = true;

    bool isBeingDragged = false;
    bool isBeingHovered = false;

    Vector positionWhileBeingDragged ;

    CMouseInteraction(sf::FloatRect localHitbox):localBounds(localHitbox){}; 
    
};