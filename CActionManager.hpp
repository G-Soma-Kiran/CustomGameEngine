#pragma once
#include<iostream>
#include<SFML/Graphics.hpp>
#include<functional>
#include<map>
#include<unordered_set>
#include <typeindex>
#include <typeinfo>

class Action
{
    private:
        std::string m_name;
        std::string m_type; 
        sf::Vector2i m_mousePos;
    public:
        Action(const std::string& name ,const std::string& type , sf::Vector2i mousePos);
        const std::string& getName() const;
        const std::string& getType() const;
        const sf::Vector2i& getMousePos() const;

};

struct mouseGestureState
{
    bool isDragging = false;          
    sf::Vector2i dragAnchor = {-100,-100};

    std::optional<sf::Mouse::Button> releasedMouseButton;
    bool transientIsDragging = false;
    bool outOfWindowHoverBug = false;

    bool isMouseMoveEvent = false;

    bool isDoubleClick = false;

    void prepeareForNextEvent()
    {
        if (isMouseMoveEvent)
            outOfWindowHoverBug = false;
        isMouseMoveEvent = false;
        transientIsDragging = false;
        releasedMouseButton.reset();
        isDoubleClick =  false;
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
    
    mouseGestureState mouseGesture;

};

enum class mouseGestureType
{
    Click,
    Drag,
    Hover,
    DoubleClick,
};

enum class gestureType
{
    Pulse,
    NonPulse,
};


class InputCondition 
{
public:
    virtual ~InputCondition() = default;
    virtual bool isSatisfied(const DeviceState& state) const = 0;
};

class GestureCondition
{
    public:
        virtual ~GestureCondition() = default;
        virtual bool isSatisfied(const DeviceState& state) const = 0;
        virtual gestureType type() const = 0;
        virtual sf::Vector2i mousePosAtActionStart(const DeviceState& state) const = 0;
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
};

class ModifierCondition : public InputCondition {
private:
    bool m_ctrl, m_shift, m_alt;
public:
    ModifierCondition(bool c, bool s, bool a) : m_ctrl(c), m_shift(s), m_alt(a) {}
    bool isSatisfied(const DeviceState& state) const override {
        return (m_ctrl == state.ctrl) && (m_shift == state.shift) && (m_alt == state.alt);
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

        gestureType type() const override
        {
            if( m_gesture == mouseGestureType::Click || m_gesture == mouseGestureType::DoubleClick )
            {
                return gestureType::Pulse;
            }
            else
            {
                return gestureType::NonPulse;
            }
        }
        sf::Vector2i mousePosAtActionStart(const DeviceState& state) const
        {
            if( m_gesture == mouseGestureType::Drag )
            {
                return state.mouseGesture.dragAnchor;
            }
            else
            {
                return state.mousePos;
            }
        }
};

struct ActionBinding 
{
    public:
    std::string actionName;
    std::vector<std::unique_ptr<InputCondition>> conditions;
    std::optional<std::unique_ptr<GestureCondition>> gesture;
    bool hasPulseGesture() const
    {
        if(!gesture) return false;

        if(gesture.value() && (gesture.value()->type() == gestureType::Pulse) ) return true;
        
        return false;
    }

    bool isTriggered(const DeviceState& state) const 
    {
        if (conditions.empty() && !gesture.has_value()) return false;

        for (const auto& condition : conditions) 
        {
            if (!condition->isSatisfied(state)) 
            {
                return false; 
            }
        }

        if(gesture.has_value())
        {
            if(!(gesture.value()->isSatisfied(state)))
            {
                return false;
            }
        }
        
        

        return true;
    }

    template<typename T , typename... Args>
    void addCondition(Args&&... args)
    {
        conditions.push_back( std::make_unique<T>(std::forward<Args>(args)...));
    };

    template<typename T , typename... Args>
    void setGesture(Args&&... args)
    {
        gesture = std::make_unique<T>(std::forward<Args>(args)...);
    };
    
};


class ActionManager
{
    private:
        DeviceState m_deviceState;
        std::vector<std::string> m_activeActions; //changed from unordered set to vector for queue behaviour of non active actions.
        std::vector<ActionBinding> m_bindings;
        std::vector<Action> m_actions;
        sf::Clock m_clock;
        void m_updateMouseGestureState( std::optional<sf::Event> opt );
    public:
        void setDeviceState( std::optional<sf::Event> opt );
        ActionBinding& addBinding(ActionBinding bind);
        void processDeviceStateToAction();
        std::vector<Action> getActions();
    
};

