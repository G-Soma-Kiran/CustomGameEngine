#pragma once
#include<iostream>
#include<SFML/Graphics.hpp>
#include<functional>
#include<map>
#include<unordered_set>
#include <typeindex>
#include <typeinfo>
#include"CConstantsAndDefines.hpp"
#include <typeinfo>


enum class ActionType
{
    Started,
    OnGoing,
    Ended,
    Pulsed,
};

class Action
{
    private:
        std::string m_name;
        ActionType m_type; 
        sf::Vector2i m_initiatemousePos;
        sf::Vector2i m_triggerMousePos;
        sf::Time m_initiateTime;
        sf::Time m_triggerTime;
    public:
        Action(const std::string& name ,const ActionType type , const sf::Vector2i& initiateMousePos, const sf::Vector2i& triggerMousePos , sf::Time initiateTime,sf::Time triggerTime);
        const std::string& getName() const;
        const ActionType getType() const;
        const sf::Vector2i& getTriggerMousePos() const;
        const sf::Vector2i& getInitiateMousePos() const;
        const sf::Time& getInitiateTime() const;
        const sf::Time& getTriggerTime() const;

};

class ActionPositionAndTime
{
    public:
        sf::Vector2i m_position;
        sf::Time m_time;
        ActionPositionAndTime(const sf::Vector2i position , const sf::Time time) : m_position(position) , m_time(time)
        {

        }
};

class ActiveActionData
{
    public:
        size_t m_actionId;
        sf::Vector2i m_initiateMousePos;
        sf::Vector2i m_triggerMousePos;
        sf::Time m_initiateTime;
        sf::Time m_triggerTime;
        ActiveActionData(size_t actionId,sf::Vector2i initiateMousePos,sf::Vector2i triggerMousePos,sf::Time initiateTime,sf::Time triggerTime):m_actionId(actionId) , m_initiateMousePos(initiateMousePos) , m_triggerMousePos(triggerMousePos) , m_initiateTime(initiateTime) , m_triggerTime(triggerTime)
        {

        }

};

struct mouseGestureState
{
    // bool isDragging = false;          
    // sf::Vector2i dragAnchor = {-100,-100};

    std::optional<sf::Mouse::Button> releasedMouseButton;
    // bool transientIsDragging = false;
    // bool outOfWindowHoverBug = false;

    bool isMouseMoveEvent = false;

    // bool isDoubleClick = false;
    // sf::Vector2i doubleClickStartPosition = {-100,-100};


    size_t numberOfMouseButtonsHeld = 0;

    std::unordered_map<sf::Mouse::Button , ActionPositionAndTime > clickOrDragAnchors;
    std::unordered_map<sf::Mouse::Button , bool > draggingStatus;
    std::optional<sf::Mouse::Button> transientDraggingMouseButton;

    std::optional<ActionPositionAndTime> clickAnchorSave;

    struct DoubleClickData
    {
        sf::Time firstClickTriggerTime = sf::Time::Zero;
        sf::Time firstClickGameTime = sf::Time::Zero;
        std::optional<sf::Vector2i> firstClickPosition = {};
    };
    std::unordered_map<sf::Mouse::Button , DoubleClickData> doubleClickFirstClickTimeStamps;

    struct DoubleClickTriggered
    {
        std::optional<sf::Mouse::Button> doubleClickButton;
        std::optional<sf::Vector2i> anchorPosition;
        std::optional<sf::Time> inititateTime;

        void reset()
        {
            doubleClickButton.reset();
            anchorPosition.reset();
            inititateTime.reset();
        }
    }isDoubleClick;

    std::optional<sf::Mouse::Button> storeInitiateEntity;
    std::optional<sf::Mouse::Button> storeDoubleClickFirstClickEntity;

    void prepareForNextEvent()
    {
        // if (isMouseMoveEvent)
        //     outOfWindowHoverBug = false;
        isMouseMoveEvent = false;
        // transientIsDragging = false;
        releasedMouseButton.reset();
        // isDoubleClick =  false;
        transientDraggingMouseButton.reset();
        isDoubleClick.reset();

        clickAnchorSave.reset();

        storeInitiateEntity.reset();

        storeDoubleClickFirstClickEntity.reset();
    }
};

struct DeviceState
{
    std::unordered_map<sf::Keyboard::Key , bool> keys;
    std::unordered_map<sf::Mouse::Button , bool> mouseButtons;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    sf::Vector2i mousePos = {-100 , -100};
    sf::Vector2i previousMousePos = mousePos;
    sf::Time previousMousePositionLockTime = sf::Time::Zero;
    sf::Time previousMousePositionLockTimeSave = sf::Time::Zero;

    size_t numberOfKeyboardKeysHeld = 0;
    
    mouseGestureState mouseGesture;


};

enum class mouseGestureType
{
    Click,
    Drag,
    DoubleClick,
    MouseMoved,
};

enum class gestureType
{
    Pulse,
    NonPulse,
};


enum class conditionName
{
    KeyCondition,
    NoKeyboardKeyHeldCondition,
    ModifierCondition,
    MouseKeyCondition,
    NoMouseKeyHeldCondition,
    MouseGestureCondition,
    MouseOverCondition,
};

class InputCondition 
{
public:
    virtual ~InputCondition() = default;
    virtual bool isSatisfied(const DeviceState& state) const = 0;
    virtual conditionName getConditionName() const = 0;
};

class GestureCondition
{
    
    public:
        virtual ~GestureCondition() = default;
        virtual bool isSatisfied(const DeviceState& state) const = 0;
        virtual gestureType type() const = 0;
        virtual ActionPositionAndTime initiationPositionAndTime(const DeviceState& state) const = 0;
        virtual conditionName getConditionName() const = 0; 
        virtual std::optional<mouseGestureType> getMouseGestureType() const{return std::nullopt;}
        virtual std::optional<sf::Mouse::Button> getMouseButton() const {return std::nullopt;}
};

class KeyCondition : public InputCondition {
private:
    sf::Keyboard::Key m_key;
public:
    KeyCondition(sf::Keyboard::Key key) : m_key(key) {}
    bool isSatisfied(const DeviceState& state) const override {
        auto it = state.keys.find(m_key);
        return (it != state.keys.end()) ? it->second : false;
    }
    conditionName getConditionName() const override{
        return conditionName::KeyCondition;
    }
};

class NoKeyboardKeyHeldCondition : public InputCondition
{
    public:
        bool isSatisfied(const DeviceState& state) const override{
            return (state.numberOfKeyboardKeysHeld == 0);
        }
        conditionName getConditionName() const override{
            return conditionName::NoKeyboardKeyHeldCondition;
        }


};

class ModifierCondition : public InputCondition {
private:
    bool m_ctrl, m_shift, m_alt;
public:
    ModifierCondition(bool c, bool s, bool a) : m_ctrl(c), m_shift(s), m_alt(a) {}
    bool isSatisfied(const DeviceState& state) const override {
        return (m_ctrl == state.ctrl) && (m_shift == state.shift) && (m_alt == state.alt);
    }
    conditionName getConditionName() const override{
        return conditionName::ModifierCondition;
    }
};

class MouseKeyCondition : public InputCondition
{
    private:
        sf::Mouse::Button m_mouseButton;
    public:
        MouseKeyCondition(sf::Mouse::Button mouseButton) : m_mouseButton(mouseButton) {};
        bool isSatisfied(const DeviceState& state) const override {
            auto it = state.mouseButtons.find(m_mouseButton);
            return (it != state.mouseButtons.end()) ? it->second : false;
        }
        conditionName getConditionName() const override{
            return conditionName::MouseKeyCondition;
        }

};
class NoMouseKeyHeldCondition : public InputCondition
{
    public:
        bool isSatisfied(const DeviceState& state) const override{
            return (state.mouseGesture.numberOfMouseButtonsHeld == 0);
        }
        conditionName getConditionName() const override{
            return conditionName::NoMouseKeyHeldCondition;
        }
};

class MouseGestureCondition : public GestureCondition
{
    private:
        mouseGestureType m_gesture;
        sf::Mouse::Button m_mouseButton = sf::Mouse::Button::Left;
        bool m_isSatisfied(const DeviceState& state) const;
    public:
        MouseGestureCondition(mouseGestureType gesture , sf::Mouse::Button mouseButton):m_gesture(gesture) , m_mouseButton(mouseButton){};
        MouseGestureCondition(mouseGestureType gesture ):m_gesture(gesture){};

        bool isSatisfied(const DeviceState& state) const override
        {
            return m_isSatisfied(state);
        }

        conditionName getConditionName() const override{
           return conditionName::MouseGestureCondition;
        }

        gestureType type() const override
        {
            if( m_gesture == mouseGestureType::Click || m_gesture == mouseGestureType::DoubleClick || m_gesture == mouseGestureType::MouseMoved )
            {
                return gestureType::Pulse;
            }
            else
            {
                return gestureType::NonPulse;
            }
        }
        ActionPositionAndTime initiationPositionAndTime(const DeviceState& state) const override
        {
            if( m_gesture == mouseGestureType::Drag)
            {
                return state.mouseGesture.clickOrDragAnchors.at(m_mouseButton);
            }
            else if(m_gesture == mouseGestureType::Click)
            {
                return state.mouseGesture.clickAnchorSave.value();
            }
            else if(  m_gesture == mouseGestureType::DoubleClick )
            {
                return {state.mouseGesture.isDoubleClick.anchorPosition.value() , state.mouseGesture.isDoubleClick.inititateTime.value()};
            }
            else if(m_gesture == mouseGestureType::MouseMoved)
            {
                return {state.previousMousePos , state.previousMousePositionLockTimeSave};
            }
        }

        std::optional<mouseGestureType> getMouseGestureType() const override
        {
            return m_gesture;
        }
        std::optional<sf::Mouse::Button> getMouseButton() const override
        {
            return m_mouseButton;
        }


};

class MouseOverCondition : public GestureCondition
{
    public:
        bool isSatisfied(const DeviceState& state) const override
        {
            return true;
        }
        gestureType type() const override
        {
            return gestureType::NonPulse;
        }
        conditionName getConditionName() const override{
            return conditionName::MouseOverCondition;
        }
        
        ActionPositionAndTime initiationPositionAndTime(const DeviceState& state) const override
        {
            return {{} , {}};
        }
};

// class ScrollCondition
// class OrCondition // and its siblings

class BindingInternals
{
    private:
        size_t m_id;
        std::string m_actionName;
    public:
        BindingInternals(const size_t id , const std::string& actionName): m_id(id) , m_actionName(actionName){};

        size_t getId() const
        {
            return m_id;
        }

        const std::string& getActionName() const
        {
            return m_actionName;
        }
};

class ActionBinding 
{
    private:
        BindingInternals m_internals;
        bool m_isEnabled = true;
        std::vector<std::unique_ptr<InputCondition>> m_conditions;
        std::optional<std::unique_ptr<GestureCondition>> m_gesture;
    public:

    ActionBinding(const size_t id , const std::string& actionName ) : m_internals(id , actionName) {};

    bool hasPulseGesture() const
    {
        if(!m_gesture) return false;

        if(m_gesture.value() && (m_gesture.value()->type() == gestureType::Pulse) ) return true;
        
        return false;
    }

    bool isTriggered(const DeviceState& state) const 
    {
        if (m_conditions.empty() && !m_gesture.has_value()) return false;
        if (!m_isEnabled) return false;

        for (const auto& condition : m_conditions) 
        {
            if (!condition->isSatisfied(state)) 
            {
                return false; 
            }
        }

        if(m_gesture.has_value())
        {
            if(!(m_gesture.value()->isSatisfied(state)))
            {
                return false;
            }
        }
        
        

        return true;
    }

    size_t getId() const
    {
        return m_internals.getId();
    }

    const std::string& getActionName() const
    {
        return m_internals.getActionName();
    } 

    template<typename T , typename... Args>
    void addCondition(Args&&... args)
    {
        m_conditions.push_back( std::make_unique<T>(std::forward<Args>(args)...));
    };

    template<typename T , typename... Args>
    void setGesture(Args&&... args)
    {
        m_gesture = std::make_unique<T>(std::forward<Args>(args)...);
    };

    const std::optional<std::unique_ptr<GestureCondition>>& getGesture() const
    {
        return m_gesture;
    }

    const std::vector<std::unique_ptr<InputCondition>>& getConditions() const
    {
        return m_conditions;
    }

    void enable()
    {
        m_isEnabled = true;
    }

    void disable()
    {
        m_isEnabled = false;
    }

    void clearConditions()
    {
        m_conditions.clear();
    }

    void clearGesture()
    {
        m_gesture.reset();
    }

    
};

class RawActionBundle
{
    private:
        const Action m_action ;
        const ActionBinding& m_actionBinding;
        const sf::Vector2i m_currentMousePos;
        const sf::Time m_currentTime;
        const std::optional<sf::Mouse::Button> m_storeInitiateEntitySignal;
        const std::optional<sf::Mouse::Button> m_storeDoubleClickFirstClickEntitySignal;
    public:
        RawActionBundle(const Action& action , 
            const ActionBinding& actionBinding ,
            const sf::Vector2i currentMousePos , 
            const sf::Time currentTime,
            const std::optional<sf::Mouse::Button> storeInitiateEntitySignal,
            const std::optional<sf::Mouse::Button> storeDoubleClickFirstClickEntitySignal
        ) : m_action(action) ,  m_actionBinding(actionBinding) , m_currentMousePos(currentMousePos) ,m_currentTime(currentTime) , m_storeInitiateEntitySignal(storeInitiateEntitySignal) , m_storeDoubleClickFirstClickEntitySignal(storeDoubleClickFirstClickEntitySignal){};

        const ActionBinding& getBinding() const
        {
            return m_actionBinding;
        }
        const Action& getAction() const
        {
            return m_action;
        }
        const sf::Vector2i& getCurrentMousePos() const
        {
            return m_currentMousePos;
        }
        const sf::Time& getCurrentTime() const
        {
            return m_currentTime;
        }
        const std::optional<sf::Mouse::Button> getStoreInitiateEntitySignal() const
        {
            return m_storeInitiateEntitySignal;
        }
        const std::optional<sf::Mouse::Button> getStoreDoubleClickFirstClickEntitySignal() const
        {
            return m_storeDoubleClickFirstClickEntitySignal;
        }
};




class ActionManager
{
    private:

        struct GameTimeData
        {
            //game time and its book keeping for calculation
            sf::Time m_gameTime = sf::Time::Zero;
            sf::Time m_previousEventTime = sf::Time::Zero;
            sf::Time m_currentEventTime = sf::Time::Zero;
        } m_gameTimeData;
        void m_updateGameTimeData(sf::Time eventTriggerTime , bool isScenePaused);


        DeviceState m_deviceState;
        std::vector<ActiveActionData> m_activeActions; //changed from unordered set to vector for queue behaviour of non active actions.
        std::vector<ActionBinding> m_bindings;
        std::vector<RawActionBundle> m_actions;
        std::unordered_map<std::string , size_t> m_actionNameToBindingIdLookupTable; 
        std::optional<size_t> m_binarySearchForBindingThroughId(size_t bindingId);
        std::unordered_map<size_t , ActionBinding> m_deletedBindings;

        // sf::Clock m_clock;
        void m_updateMouseGestureState( std::optional<sf::Event> opt );
        size_t m_totalActionBindings = 0;
        void setDeviceState( std::optional<sf::Event> opt);
        void processDeviceStateToAction();
        std::vector<RawActionBundle> getActions();
        void processOngoingActions();

        friend class SceneManager;
    public:
        ActionBinding& addBinding(const std::string& actionName);
        ActionBinding& getBinding(size_t bindingId);
        ActionBinding& getBinding(std::string& actionName);
        void deleteBinding(size_t bindingId);
        void deleteBinding(std::string& actionName);
        bool isBindingPresent(size_t bindingId);
        bool isBindingPresent(std::string& actionName);

        void forceFireAction(const Action& action);
    
};

