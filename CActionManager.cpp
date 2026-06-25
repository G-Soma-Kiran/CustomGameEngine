#include"CActionManager.hpp"

Action::Action( const std::string& name ,const std::string& type , sf::Vector2i mousePos ) : m_name(name) , m_type(type) , m_mousePos(mousePos)
{

}

const std::string& Action::getName() const
{
    return m_name;
}

const std::string& Action::getType() const
{
    return m_type;
}

const sf::Vector2i& Action::getMousePos() const
{
    return m_mousePos;
}

void ActionManager::setDeviceState(std::optional<sf::Event> opt)
{

    if (const auto* kp = opt->getIf<sf::Event::KeyPressed>()) {
        m_deviceState.keys[kp->code] = true;
        m_deviceState.ctrl = kp->control;
        m_deviceState.shift = kp->shift;
        m_deviceState.alt = kp->alt;
    }
    else if (const auto* kr = opt->getIf<sf::Event::KeyReleased>()) {
        m_deviceState.keys[kr->code] = false;
        m_deviceState.ctrl = kr->control;
        m_deviceState.shift = kr->shift;
        m_deviceState.alt = kr->alt;
    }
    else if (const auto* mp = opt->getIf<sf::Event::MouseButtonPressed>()) {
        m_deviceState.mouseButtons[mp->button] = true;
        m_deviceState.mousePos = mp->position;
        if( mp->button == sf::Mouse::Button::Left && !m_deviceState.isDragging && m_deviceState.dragAnchor == sf::Vector2i(-100,-100) )
        {
            m_deviceState.dragAnchor = mp->position;
        }
    }
    else if (const auto* mr = opt->getIf<sf::Event::MouseButtonReleased>()) {
        m_deviceState.mouseButtons[mr->button] = false;
        m_deviceState.releasedMouseButton = mr->button;
        m_deviceState.mousePos = mr->position;
        if (mr->button == sf::Mouse::Button::Left) {
            if( !m_deviceState.isDragging && m_clock.isRunning() )
            {
                float elapsedTime = m_clock.getElapsedTime().asMilliseconds();
                if( elapsedTime <= 300.0f )
                {
                    m_deviceState.isDoubleClick = true;
                    m_clock.reset();
                }
                else
                {
                    m_deviceState.isDoubleClick = false;
                    m_clock.restart();
                }
            }
            else if( !m_deviceState.isDragging )
            {
                m_clock.start();
            }
            m_deviceState.transientIsDragging = m_deviceState.isDragging;
            m_deviceState.isDragging = false;
            m_deviceState.dragAnchor = {-100, -100};
        }
    }else if(const auto* mv = opt->getIf<sf::Event::MouseMoved>()){
        m_deviceState.isMouseMoveEvent = true;
        m_deviceState.mousePos = mv->position;
        if ( m_deviceState.mouseButtons[sf::Mouse::Button::Left] && !m_deviceState.isDragging ) {
            int deltaX = std::abs(mv->position.x - m_deviceState.dragAnchor.x);
            int deltaY = std::abs(mv->position.y - m_deviceState.dragAnchor.y);

            if (deltaX > 5 || deltaY > 5) {
                m_deviceState.isDragging = true; 
            }
        }
    }
}
 
void ActionManager::addBinding(ActionBinding bind)
{
    m_bindings.push_back(std::move(bind));
}

void ActionManager::processDeviceStateToAction()
{
    for (const auto& binding : m_bindings) {
            bool isTriggered = binding.isTriggered(m_deviceState);
            bool wasActive = m_activeActions.contains(binding.actionName);

            ConditionType mouseConditionType = ConditionType::None;
            for(auto& condition : binding.conditions)
            {
                ConditionType t = condition->type();
                if (t == ConditionType::MouseDragCondition  || t == ConditionType::MouseClickCondition || t == ConditionType::MouseHoverCondition)
                {
                    mouseConditionType = t;
                    break;
                }

            }
            if( mouseConditionType == ConditionType::MouseClickCondition || mouseConditionType == ConditionType::MouseDoubleClickConditon )
            {
                if(isTriggered)
                {
                    m_actions.emplace_back(binding.actionName , "Pulsed" ,m_deviceState.mousePos );
                }
            }
            else
            {
                    if (isTriggered && !wasActive) {
                    m_activeActions.insert(binding.actionName);
                    if(mouseConditionType == ConditionType::MouseHoverCondition)
                        m_actions.emplace_back(binding.actionName, "Pressed" , m_deviceState.mousePos );
                    else if(mouseConditionType == ConditionType::MouseDragCondition)
                        m_actions.emplace_back(binding.actionName, "Started" , m_deviceState.dragAnchor );
                    else if( mouseConditionType == ConditionType::None )
                        m_actions.emplace_back(binding.actionName, "Pressed" ,m_deviceState.mousePos );

                }
                else if (!isTriggered && wasActive) {
                    m_activeActions.erase(binding.actionName);
                    if(mouseConditionType == ConditionType::MouseHoverCondition)
                        m_actions.emplace_back(binding.actionName, "Released" , m_deviceState.mousePos);
                    else if(mouseConditionType == ConditionType::MouseDragCondition)
                        m_actions.emplace_back(binding.actionName, "Ended" , m_deviceState.mousePos );
                    else if( mouseConditionType == ConditionType::None )
                        m_actions.emplace_back(binding.actionName, "Released" , m_deviceState.mousePos );
                }
            }    
        }

            
        m_deviceState.isMouseMoveEvent = false;
        m_deviceState.transientIsDragging = false;
        m_deviceState.releasedMouseButton.reset();
        m_deviceState.isDoubleClick =  false;
}

std::vector<Action> ActionManager::getActions()
{
    return std::move(m_actions);
}