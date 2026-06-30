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
    }
    else if (const auto* mr = opt->getIf<sf::Event::MouseButtonReleased>()) {
        m_deviceState.mouseButtons[mr->button] = false;
        m_deviceState.mousePos = mr->position;
    }else if(const auto* mv = opt->getIf<sf::Event::MouseMoved>()){
        m_deviceState.mousePos = mv->position;
    }
    m_updateMouseGestureState(opt);
}
 
ActionBinding& ActionManager::addBinding(ActionBinding bind)
{
    m_bindings.push_back(std::move(bind));
    return m_bindings.back();
}

void ActionManager::processDeviceStateToAction()
{
    // for (const auto& binding : m_bindings) {
    //         bool isTriggered = binding.isTriggered(m_deviceState);
    //         bool wasActive = m_activeActions.contains(binding.actionName);

    //         ConditionType mouseConditionType = ConditionType::None;
    //         for(auto& condition : binding.conditions)
    //         {
    //             ConditionType t = condition->type();
    //             if (t == ConditionType::MouseDragCondition  || t == ConditionType::MouseClickCondition || t == ConditionType::MouseHoverCondition || t == ConditionType::MouseDoubleClickConditon)
    //             {
    //                 mouseConditionType = t;
    //                 break;
    //             }

    //         }
    //         if( mouseConditionType == ConditionType::MouseClickCondition || mouseConditionType == ConditionType::MouseDoubleClickConditon )
    //         {
    //             if(isTriggered)
    //             {
    //                 m_actions.emplace_back(binding.actionName , "Pulsed" ,m_deviceState.mousePos );
    //             }
    //         }
    //         else
    //         {
    //                 if (isTriggered && !wasActive) {
    //                 m_activeActions.insert(binding.actionName);
    //                 if(mouseConditionType == ConditionType::MouseHoverCondition)
    //                     m_actions.emplace_back(binding.actionName, "Pressed" , m_deviceState.mousePos );
    //                 else if(mouseConditionType == ConditionType::MouseDragCondition)
    //                     m_actions.emplace_back(binding.actionName, "Started" , m_deviceState.dragAnchor );
    //                 else if( mouseConditionType == ConditionType::None )
    //                     m_actions.emplace_back(binding.actionName, "Pressed" ,m_deviceState.mousePos );

    //             }
    //             else if (!isTriggered && wasActive) {
    //                 m_activeActions.erase(binding.actionName);
    //                 if(mouseConditionType == ConditionType::MouseHoverCondition)
    //                     m_actions.emplace_back(binding.actionName, "Released" , m_deviceState.mousePos);
    //                 else if(mouseConditionType == ConditionType::MouseDragCondition)
    //                     m_actions.emplace_back(binding.actionName, "Ended" , m_deviceState.mousePos );
    //                 else if( mouseConditionType == ConditionType::None )
    //                     m_actions.emplace_back(binding.actionName, "Released" , m_deviceState.mousePos );
    //             }
    //         }    
    //     }

    //     if (m_deviceState.isMouseMoveEvent)
    //         m_deviceState.outOfWindowHoverBug = false; 
    //     m_deviceState.isMouseMoveEvent = false;
    //     m_deviceState.transientIsDragging = false;
    //     m_deviceState.releasedMouseButton.reset();
    //     m_deviceState.isDoubleClick =  false;




    //New system of 2 pass for releasing expired events first then starting new actions.
    //Might need to expand to consumed events later for non collision of subset events ex(shift + w , w)

    m_activeActions.erase( 
        std::remove_if( m_activeActions.begin() , m_activeActions.end() , 
        [this](const auto& action)
        {
            for (const auto& binding : m_bindings)
            {
                if (binding.actionName == action && !binding.isTriggered(m_deviceState))
                {
                    m_actions.emplace_back(binding.actionName, "Ended", m_deviceState.mousePos);
                    return true;
                }

            }
            return false;
        }
    
    )
        , m_activeActions.end());
    
    // for ( const auto& binding : m_bindings)
    // {
    //     if( binding.isTriggered(m_deviceState) )
    //     {
    //         for ( const auto& activeAction : m_activeActions)
    //         {
    //             if( binding.actionName == activeAction)
    //             {
    //                 break;
    //             }
    //         }
    //         m_activeActions.push_back(binding.actionName);
    //         m_actions.emplace_back( binding.actionName , "Pressed" , m_deviceState.mousePos );
    //     }
    // }
    for (const auto& binding : m_bindings)
    { 
        
        
        if ( binding.hasPulseGesture() )
        {
            if (binding.isTriggered(m_deviceState))
                m_actions.emplace_back(binding.actionName, "Pulsed", m_deviceState.mousePos);
            continue;
        }

        if (!binding.isTriggered(m_deviceState)) continue;

        bool isActive = std::find(m_activeActions.begin(), m_activeActions.end(),binding.actionName) != m_activeActions.end();

        if (!isActive)
        {
            m_activeActions.push_back(binding.actionName);    
            if(binding.gesture.has_value())
                m_actions.emplace_back(binding.actionName, "Started", binding.gesture.value()->mousePosAtActionStart(m_deviceState));
            else
                m_actions.emplace_back(binding.actionName, "Started", m_deviceState.mousePos);
        }
    }

        m_deviceState.mouseGesture.prepeareForNextEvent();
}

std::vector<Action> ActionManager::getActions()
{
    // return std::move(m_actions);
    auto out = std::move(m_actions);
    m_actions.clear();
    return out;
}


bool MouseGestureCondition::m_isSatisfied(const DeviceState& state) const
{
    switch(m_gesture)
    {
        case mouseGestureType::Click:
            if(  state.mouseGesture.releasedMouseButton.has_value() && (state.mouseGesture.releasedMouseButton.value() == m_mouseButton) )
            {
                if(m_mouseButton != sf::Mouse::Button::Left)
                {
                    return true;
                }
                else
                {
                    return !state.mouseGesture.transientIsDragging;
                }
            }
            return false;
        case mouseGestureType::Drag:
            return state.mouseGesture.isDragging;
        case mouseGestureType::Hover:
            return  !state.mouseGesture.isDragging && !state.mouseGesture.outOfWindowHoverBug && state.mouseGesture.isMouseMoveEvent;
        case mouseGestureType::DoubleClick:
            return !state.mouseGesture.transientIsDragging && state.mouseGesture.isDoubleClick;
        default :
            std::cerr << "No case satisfied in 'MouseGestureCondition::m_isSatisfied'\n";
            return false;
    }

}

void ActionManager::m_updateMouseGestureState( std::optional<sf::Event> opt )
{
    if (const auto* mp = opt->getIf<sf::Event::MouseButtonPressed>()) {
        if( mp->button == sf::Mouse::Button::Left && !m_deviceState.mouseGesture.isDragging && m_deviceState.mouseGesture.dragAnchor == sf::Vector2i(-100,-100) )
        {
            m_deviceState.mouseGesture.dragAnchor = mp->position;
        }
    }
    else if (const auto* mr = opt->getIf<sf::Event::MouseButtonReleased>()) {
        m_deviceState.mouseGesture.releasedMouseButton = mr->button;
        if (mr->button == sf::Mouse::Button::Left) {
            if( !m_deviceState.mouseGesture.isDragging && m_clock.isRunning() )
            {
                float elapsedTime = m_clock.getElapsedTime().asMilliseconds();
                if( elapsedTime <= 300.0f )
                {
                    m_deviceState.mouseGesture.isDoubleClick = true;
                    m_clock.reset();
                }
                else
                {
                    m_deviceState.mouseGesture.isDoubleClick = false;
                    m_clock.restart();
                }
            }
            else if( !m_deviceState.mouseGesture.isDragging )
            {
                m_clock.start();
            }
            if (m_deviceState.mouseGesture.isDragging)
                m_deviceState.mouseGesture.outOfWindowHoverBug = true;
            m_deviceState.mouseGesture.transientIsDragging = m_deviceState.mouseGesture.isDragging;
            m_deviceState.mouseGesture.isDragging = false;
            m_deviceState.mouseGesture.dragAnchor = {-100, -100};
        }
    }else if(const auto* mv = opt->getIf<sf::Event::MouseMoved>()){
        m_deviceState.mouseGesture.isMouseMoveEvent = true;
        if ( m_deviceState.mouseButtons[sf::Mouse::Button::Left] && !m_deviceState.mouseGesture.isDragging ) {
            int deltaX = std::abs(mv->position.x - m_deviceState.mouseGesture.dragAnchor.x);
            int deltaY = std::abs(mv->position.y - m_deviceState.mouseGesture.dragAnchor.y);

            if (deltaX > 5 || deltaY > 5) {
                m_deviceState.mouseGesture.isDragging = true; 
            }
        }
    }
}
