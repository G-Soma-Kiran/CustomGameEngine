#pragma once
#include"CEntityManager.hpp"
#include"CActionManager.hpp"
#include<functional>

inline constexpr size_t NoEntity = 0;

struct Request;

//UI components Start
enum class ButtonState
{
    Normal,
    Hovered,
    Pressed,
};

struct ButtonStateRects
{
    sf::IntRect normal;
    sf::IntRect hover;
    sf::IntRect pressed;
    sf::IntRect disabled;
};

class CButton
{
    public:
        ButtonState currentState = ButtonState::Normal;
        ButtonStateRects stateBoundingRects;
        CButton(ButtonStateRects rects) : stateBoundingRects(rects){};
};

class CMouseInteraction
{
public:
    sf::FloatRect localBounds;

    bool enabled = true;

    bool isBeingDragged = false;

    Vector positionWhileBeingDragged ;

    std::function<void()> onHoverEnter;
    std::function<void()> duringHover;
    std::function<void()> onHoverLeave;

    std::function<void()> onMouseLeftPress;
    std::function<void()> onMouseLeftRelease;
    std::function<void()> onLeftClick;

    std::function<void()> onDragStart;
    std::function<void()> duringDrag;
    std::function<void()> onDragEnd;


    CMouseInteraction(sf::FloatRect localHitbox):localBounds(localHitbox){}; 
};
//UI components End

struct UIState
{
    size_t candidateClick = 0;
    size_t previousHoveredEntity = 0; 
    size_t beingDraggedEntity = 0;
};

class UISystem
{
    private:
        UIState m_uistate;
        EntityManager& m_entities;
    public:
        UISystem(EntityManager& entitymanager):m_entities(entitymanager){};
        void update(Action& input , std::vector<Request>& requests);
        std::shared_ptr<Entity> makeButton(const sf::Texture& buttonTexture , ButtonStateRects rects , Vector position , sf::FloatRect localBounds = {});
        bool buttonUpdate(size_t entityId , ButtonState newState); // returns true if really the entity has component and update is done

};