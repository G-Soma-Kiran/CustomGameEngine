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
        bool isYsorted = false;
};

class CCollision
{   public:
        sf::FloatRect localHitbox;
};

class CVisual
{
    public:
        sf::Sprite sprite;
        CVisual(sf::Texture texture) : sprite(texture){};
};