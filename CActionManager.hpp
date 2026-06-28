#pragma once
#include<iostream>
#include<SFML/Graphics.hpp>
#include<functional>
#include<map>
#include<unordered_set>

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



struct DeviceState
{
    std::unordered_map<sf::Keyboard::Key , bool> keys;
    std::unordered_map<sf::Mouse::Button , bool> mouseButtons;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    sf::Vector2i mousePos = {-100 , -100};
    
    bool isDragging = false;          
    sf::Vector2i dragAnchor = {-100,-100};

    std::optional<sf::Mouse::Button> releasedMouseButton;
    bool transientIsDragging = false;
    bool outOfWindowHoverBug = false;

    bool isMouseMoveEvent = false;

    bool isDoubleClick = false;

};

enum class ConditionType
{
    None,
    KeyCondition,
    ModifierCondition,
    MouseClickCondition,
    MouseDragCondition,
    MouseHoverCondition,
    MouseDoubleClickConditon,
};

class InputCondition {
public:
    virtual ~InputCondition() = default;
    virtual bool isSatisfied(const DeviceState& state) const = 0;
    virtual ConditionType type() const = 0;
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
    ConditionType type() const override
    {
        return ConditionType::KeyCondition;
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
    ConditionType type() const override
    {
        return ConditionType::ModifierCondition;
    }
};

class MouseClickCondition : public InputCondition
{
    private:
        sf::Mouse::Button m_mouseButton;
        
    public:
        MouseClickCondition( sf::Mouse::Button mouseButton ) : m_mouseButton(mouseButton){};
        bool isSatisfied(const DeviceState& state) const override
        {
            if(  state.releasedMouseButton.has_value() && (state.releasedMouseButton.value() == m_mouseButton) )
            {
                if(m_mouseButton != sf::Mouse::Button::Left)
                {
                    return true;
                }
                else
                {
                    return !state.transientIsDragging;
                }
            }
            return false;
        }
        
        ConditionType type() const override
        {
            return ConditionType::MouseClickCondition;
        }
};

class MouseDragCondition : public InputCondition
{
    public:
    bool isSatisfied(const DeviceState& state) const override
    {
        return state.isDragging;
    }
    ConditionType type() const override
    {
        return ConditionType::MouseDragCondition;
    }
};

class MouseHoverCondition : public InputCondition
{
    public:
    bool isSatisfied(const DeviceState& state) const override
    {
        return  !state.isDragging && !state.outOfWindowHoverBug && state.isMouseMoveEvent;
    }
    ConditionType type() const override
    {
        return ConditionType::MouseHoverCondition;
    }
};

class MouseDoubleClickCondition : public InputCondition
{
    public:
        bool isSatisfied(const DeviceState& state) const override
        {
            return !state.transientIsDragging && state.isDoubleClick;
        }
        ConditionType type() const override
        {
            return ConditionType::MouseDoubleClickConditon;
        }
};

struct ActionBinding 
{
    std::string actionName;
    std::vector<std::unique_ptr<InputCondition>> conditions;

    bool isTriggered(const DeviceState& state) const 
    {
        if (conditions.empty()) return false;

        for (const auto& condition : conditions) 
        {
            if (!condition->isSatisfied(state)) 
            {
                return false; 
            }
        }
        return true;
    }
    
};


class ActionManager
{
    private:
        DeviceState m_deviceState;
        std::vector<std::string> m_activeActions; //changed from unordered set to vector for queue behaviour of non active actions.
        std::vector<ActionBinding> m_bindings;
        std::vector<Action> m_actions;
        sf::Clock m_clock;
    public:
        void setDeviceState( std::optional<sf::Event> opt );
        void addBinding(ActionBinding bind);
        void processDeviceStateToAction();
        std::vector<Action> getActions();
    
};

