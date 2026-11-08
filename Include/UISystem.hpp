#pragma once
#include"CEntityManager.hpp"
#include"CActionManager.hpp"
#include<functional>
#include"CConstantsAndDefines.hpp"
#include<array>

struct Request;


class RefinedAction : public Action
{
    private:
        size_t m_initiateEntityId;
        size_t m_triggeredEntityId;
    public:
        RefinedAction(const std::string& name ,const ActionType type , const sf::Vector2i& initiateMousePos, const sf::Vector2i& triggerMousePos , sf::Time initiateTime,sf::Time triggerTime , const size_t initiateEntityId , const size_t triggerEntityId) : Action( name , type , initiateMousePos , triggerMousePos ,initiateTime , triggerTime) , m_initiateEntityId(initiateEntityId) ,m_triggeredEntityId(triggerEntityId){};
        size_t getInitiateEntityId() const;
        size_t getTriggerEntityId() const;

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

struct HoverRecord       
{
    size_t bindingId; 
    std::string actionName;
    sf::Vector2i initiateAndTriggerPos;
    sf::Time initiateAndTriggerTime;      
    size_t initiateAndTriggerEntityId;
    HoverRecord(
        size_t bindingId,
        const std::string& actionName,
        const sf::Vector2i& initiateAndTriggerPos,
        sf::Time initiateAndTriggerTime,
        size_t initiateAndTriggerEntityId
    )
        : bindingId(bindingId),
          actionName(actionName),
          initiateAndTriggerPos(initiateAndTriggerPos),
          initiateAndTriggerTime(initiateAndTriggerTime),
          initiateAndTriggerEntityId(initiateAndTriggerEntityId)
    {}
};

class StoredEntities
{
    private:
        size_t m_bindingId;
        size_t m_initiateEntityId;
        size_t m_triggerEntityId;
    public:
        StoredEntities(size_t bindingId , size_t initiateEntityId , size_t triggerEntityId ) : m_bindingId(bindingId) , m_initiateEntityId(initiateEntityId) , m_triggerEntityId(triggerEntityId){}
        size_t getBindingId() const {return m_bindingId;}
        size_t getInitiateEntityId() const {return m_initiateEntityId;}
        size_t getTriggerEntityId() const {return m_triggerEntityId;}
};

struct UIState
{
    size_t previousMouseOverEntity = NoEntity; 
    size_t currentMouseOverEntity = NoEntity; 
    size_t beingDraggedEntity = NoEntity;
    sf::Vector2i currentMousePos = {};
    sf::Time currentGameTime=sf::Time::Zero;
    // std::unordered_map<size_t , bool> refiningNeededBindingIdsActiveStatus;
    std::vector<HoverRecord> bindingActivationOrder;
    std::vector<StoredEntities> storedEntities;
    std::array<size_t, sf::Mouse::ButtonCount> initiateEntityByButton;
    std::array<size_t, sf::Mouse::ButtonCount> doubleClickFirstClickEntity;
    sf::Vector2i lastSamplePos;
    size_t prevSampleEntity;
    size_t sampleEntity;
    UIState()
    {
        initiateEntityByButton.fill(NoEntity);
        doubleClickFirstClickEntity.fill(NoEntity);
        storedEntities.reserve(16);
    }
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
        std::shared_ptr<Entity> makeButton(const sf::Texture& buttonTexture , ButtonStateRects rects , sf::Vector2f position , sf::FloatRect localBounds = {});
        bool buttonUpdate(size_t entityId , ButtonState newState); // returns true if really the entity has component and update is done
        size_t updateMouseOver( const sf::Vector2i& mousePos );
        sf::Vector2i getMousePos() const { return m_uistate.currentMousePos; }
        sf::Time getTime() const { return m_uistate.currentGameTime; }
        size_t getMouseOverEntity() const { return m_uistate.currentMouseOverEntity; }

};