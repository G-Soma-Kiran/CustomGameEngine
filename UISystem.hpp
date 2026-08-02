#pragma once
#include"CEntityManager.hpp"
#include"CActionManager.hpp"
#include<functional>
#include"CConstantsAndDefines.hpp"


struct Request;


class RefinedAction : public Action
{
    private:
        size_t m_entityId;
    public:
        RefinedAction(const std::string& name ,const ActionType type , const sf::Vector2i& mousePos , sf::Time triggerTime , size_t entityId) : Action( name , type , mousePos , triggerTime) , m_entityId(entityId){};
        size_t getEntityId() const;

};

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
//UI components End

struct UIState
{
    size_t previousMouseOverEntity = NoEntity; 
    size_t currentMouseOverEntity = NoEntity; 
    size_t beingDraggedEntity = NoEntity;
    // std::unordered_map<size_t , bool> refiningNeededBindingIdsActiveStatus;
    std::vector<BindingInternals> bindingActivationOrder;
    std::unordered_map<size_t , size_t> capturedEntityIdsByBindings;
};

class UISystem
{
    private:
        UIState m_uistate;
        EntityManager& m_entities;
        ActionManager& m_actionManager;
    public:
        UISystem(EntityManager& entityManager , ActionManager& actionManager);
        std::vector<RefinedAction> refineAction(RawActionBundle input);
        std::shared_ptr<Entity> makeButton(const sf::Texture& buttonTexture , ButtonStateRects rects , Vector position , sf::FloatRect localBounds = {});
        bool buttonUpdate(size_t entityId , ButtonState newState); // returns true if really the entity has component and update is done
        size_t updateMouseOver( const sf::Vector2i& mousePos );

};