#include"CActionManager.hpp"

Action::Action( const std::string& name ,const ActionType type ,const sf::Vector2i& mousePos , sf::Time triggerTime ) : m_name(name) , m_type(type) , m_mousePos(mousePos) , m_triggerTime(triggerTime)
{

}

const std::string& Action::getName() const
{
    return m_name;
}

const ActionType Action::getType() const
{
    return m_type;
}

const sf::Vector2i& Action::getMousePos() const
{
    return m_mousePos;
}
const sf::Time& Action::getTriggerTime() const
{
    return m_triggerTime;
}

void ActionManager::setDeviceState(std::optional<sf::Event> opt)
{

    if (const auto* kp = opt->getIf<sf::Event::KeyPressed>()) {
        if( !m_deviceState.keys[kp->code] )
        {
            m_deviceState.numberOfKeyboardKeysHeld++;
        }

        m_deviceState.keys[kp->code] = true;
        m_deviceState.ctrl = kp->control;
        m_deviceState.shift = kp->shift;
        m_deviceState.alt = kp->alt;

    }
    else if (const auto* kr = opt->getIf<sf::Event::KeyReleased>()) {
        m_deviceState.numberOfKeyboardKeysHeld--;

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
 
ActionBinding& ActionManager::addBinding(const std::string& actionName , bool capturesEntity)
{
    if( m_actionNameToBindingIdLookupTable.contains(actionName))
    {
        throw std::runtime_error("Tried to overwrite binding : " + actionName);
    }
    m_bindings.push_back(std::move(ActionBinding(++m_totalActionBindings , actionName , capturesEntity)));
    m_actionNameToBindingIdLookupTable[actionName] = m_totalActionBindings;
    return m_bindings.back();
}

void ActionManager::processDeviceStateToAction()
{
   
    //New system of 2 pass for releasing expired events first then starting new actions.
    //Might need to expand to consumed events later for non collision of subset events ex(shift + w , w)

    

    m_activeActions.erase( 
        std::remove_if( m_activeActions.begin() , m_activeActions.end() , 
        [this](const auto actionId)
        {
            // for (const auto& binding : m_bindings)
            // {
            //     if (binding.getId() == actionId && !binding.isTriggered(m_deviceState))
            //     {
            //         m_actions.emplace_back(RawActionBundle(Action(binding.getActionName(), "Ended", m_deviceState.mousePos , m_gameTimeData.m_gameTime ) , binding , m_deviceState.mousePos));
            //         return true;
            //     }

            // }
            // return false;
            std::optional<size_t> indexOfBindingId = m_binarySearchForBindingThroughId(actionId);
            if(!indexOfBindingId.has_value()  )
            {
                m_actions.emplace_back(RawActionBundle(Action(  m_deletedBindings.at(actionId).getActionName(), ActionType::Ended , m_deviceState.mousePos , m_gameTimeData.m_gameTime ) , m_deletedBindings.at(actionId) , m_deviceState.mousePos));
                return true;
            }
            else if(  !m_bindings[indexOfBindingId.value()].isTriggered(m_deviceState)  )
            {
                m_actions.emplace_back(RawActionBundle(Action(m_bindings[indexOfBindingId.value()].getActionName(), ActionType::Ended, m_deviceState.mousePos , m_gameTimeData.m_gameTime ) , m_bindings[indexOfBindingId.value()] , m_deviceState.mousePos));
                return true;
            }
            return false;
        }
    
    )
        , m_activeActions.end());


    for (const auto& binding : m_bindings)
    { 
        
        
        if ( binding.hasPulseGesture() )
        {
            if (binding.isTriggered(m_deviceState))
            {
                m_actions.emplace_back(RawActionBundle(Action(binding.getActionName(), ActionType::Pulsed, binding.getGesture().value()->mousePosAtGestureStart(m_deviceState) , m_gameTimeData.m_gameTime) , binding , m_deviceState.mousePos));
            }
            continue;
        }

        if (!binding.isTriggered(m_deviceState)) continue;

        bool isActive = std::find(m_activeActions.begin(), m_activeActions.end(),binding.getId()) != m_activeActions.end();

        if (!isActive)
        {
            m_activeActions.push_back(binding.getId());    
            if(binding.getGesture().has_value())
                m_actions.emplace_back(RawActionBundle(Action(binding.getActionName(), ActionType::Started, binding.getGesture().value()->mousePosAtGestureStart(m_deviceState) , m_gameTimeData.m_gameTime) , binding , m_deviceState.mousePos));
            else
                m_actions.emplace_back(RawActionBundle(Action(binding.getActionName(), ActionType::Started, m_deviceState.mousePos , m_gameTimeData.m_gameTime) , binding , m_deviceState.mousePos));
                
        }
    }

        m_deviceState.mouseGesture.prepeareForNextEvent();
}

std::vector<RawActionBundle> ActionManager::getActions()
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
            // if(  state.mouseGesture.releasedMouseButton.has_value() && (state.mouseGesture.releasedMouseButton.value() == m_mouseButton) )
            // {
            //     if(m_mouseButton != sf::Mouse::Button::Left)
            //     {
            //         return true;
            //     }
            //     else
            //     {
            //         return !state.mouseGesture.transientIsDragging;
            //     }
            // }
            // return false;
            if(state.mouseGesture.transientDraggingMouseButton.has_value())
            {
                return (state.mouseGesture.releasedMouseButton.has_value()) && (state.mouseGesture.releasedMouseButton.value() == m_mouseButton) && (state.mouseGesture.transientDraggingMouseButton.value() != m_mouseButton);
            }
            else
            {
                return (state.mouseGesture.releasedMouseButton.has_value()) && (state.mouseGesture.releasedMouseButton.value() == m_mouseButton);
            }

        case mouseGestureType::Drag:
            // return state.mouseGesture.isDragging;
            {auto it = state.mouseGesture.draggingStatus.find(m_mouseButton);
            return (it != state.mouseGesture.draggingStatus.end()) ? it->second : false;}
            // return state.mouseGesture.draggingStatus[m_mouseButton];
        case mouseGestureType::MouseMoved:
            return state.mouseGesture.isMouseMoveEvent;
        case mouseGestureType::DoubleClick:
            // return !state.mouseGesture.transientIsDragging && state.mouseGesture.isDoubleClick;
            return state.mouseGesture.isDoubleClick.doubleClickButton.has_value() && state.mouseGesture.isDoubleClick.doubleClickButton.value() == m_mouseButton;
        default :
            std::cerr << "No case satisfied in 'MouseGestureCondition::m_isSatisfied'\n";
            return false; 
    }

}

void ActionManager::m_updateMouseGestureState( std::optional<sf::Event> opt ) 
{
    if (const auto* mp = opt->getIf<sf::Event::MouseButtonPressed>()) 
    {
        // if( mp->button == sf::Mouse::Button::Left && !m_deviceState.mouseGesture.isDragging && m_deviceState.mouseGesture.dragAnchor == sf::Vector2i(-100,-100) )
        // {
        //     m_deviceState.mouseGesture.dragAnchor = mp->position;
        // }
        m_deviceState.mouseGesture.clickOrDragAnchors[mp->button] = mp->position;
        m_deviceState.mouseGesture.numberOfMouseButtonsHeld++;
    }
    else if (const auto* mr = opt->getIf<sf::Event::MouseButtonReleased>()) 
    {
        m_deviceState.mouseGesture.releasedMouseButton = mr->button;

        
        if( m_deviceState.mouseGesture.draggingStatus[mr->button] )
        {
            m_deviceState.mouseGesture.doubleClickFirstClickTimeStamps[mr->button] = {};
            m_deviceState.mouseGesture.isDoubleClick.reset() ;
        }
        else
        {
            auto distanceGreaterThanThreshold = [](const sf::Vector2i& pos1 ,const sf::Vector2i& pos2) -> bool
            {
                int deltaX = std::abs( pos1.x - pos2.x );
                int deltaY = std::abs( pos1.y - pos2.y );

                return deltaX*deltaX + deltaY*deltaY > 25;
            };


            if(m_deviceState.mouseGesture.doubleClickFirstClickTimeStamps[mr->button].firstClickTriggerTime == sf::Time::Zero || (m_gameTimeData.m_currentEventTime - m_deviceState.mouseGesture.doubleClickFirstClickTimeStamps[mr->button].firstClickTriggerTime).asMilliseconds() > 300.0f 
                || distanceGreaterThanThreshold(mr->position , m_deviceState.mouseGesture.doubleClickFirstClickTimeStamps[mr->button].firstClickPosition.value()))
            {
                m_deviceState.mouseGesture.doubleClickFirstClickTimeStamps[mr->button].firstClickPosition = m_deviceState.mouseGesture.clickOrDragAnchors[mr->button];
                m_deviceState.mouseGesture.doubleClickFirstClickTimeStamps[mr->button].firstClickTriggerTime = m_gameTimeData.m_currentEventTime;
                m_deviceState.mouseGesture.isDoubleClick.reset();
            }
            else
            {
                m_deviceState.mouseGesture.isDoubleClick.doubleClickButton = mr->button;
                m_deviceState.mouseGesture.isDoubleClick.anchorPosition = m_deviceState.mouseGesture.clickOrDragAnchors[mr->button];
                m_deviceState.mouseGesture.doubleClickFirstClickTimeStamps[mr->button] = {};
            }
        }

        // if (mr->button == sf::Mouse::Button::Left) {
        //     if( !m_deviceState.mouseGesture.isDragging && m_clock.isRunning() )
        //     {
        //         float elapsedTime = m_clock.getElapsedTime().asMilliseconds();
        //         if( elapsedTime <= 300.0f )
        //         {
        //             int deltaX = std::abs(mr->position.x - m_deviceState.mouseGesture.doubleClickStartPosition.x);
        //             int deltaY = std::abs(mr->position.y - m_deviceState.mouseGesture.doubleClickStartPosition.y);

        //             if (deltaX*deltaX + deltaY*deltaY <= 25) {
        //                 m_deviceState.mouseGesture.isDoubleClick = true;
        //             }else{
        //                 m_deviceState.mouseGesture.isDoubleClick = false;
        //             }
        //             m_clock.reset();
        //         }
        //         else
        //         {
        //             m_deviceState.mouseGesture.isDoubleClick = false;
        //             m_deviceState.mouseGesture.doubleClickStartPosition = mr->position;
        //             m_clock.restart();
        //         }
        //     }
        //     else if( !m_deviceState.mouseGesture.isDragging )
        //     {
        //         m_clock.start();
        //         m_deviceState.mouseGesture.doubleClickStartPosition = mr->position;
        //     }
        //     if (m_deviceState.mouseGesture.isDragging)
        //         m_deviceState.mouseGesture.outOfWindowHoverBug = true;
        //     m_deviceState.mouseGesture.transientIsDragging = m_deviceState.mouseGesture.isDragging;
        //     m_deviceState.mouseGesture.isDragging = false;
        //     m_deviceState.mouseGesture.dragAnchor = {-100, -100};
            
            
        // }
        if( m_deviceState.mouseGesture.draggingStatus[mr->button] )
        {
            m_deviceState.mouseGesture.transientDraggingMouseButton = mr->button;
        }
        m_deviceState.mouseGesture.draggingStatus[mr->button] = false;
        m_deviceState.mouseGesture.clickAnchorSave = m_deviceState.mouseGesture.clickOrDragAnchors[mr->button];
        m_deviceState.mouseGesture.clickOrDragAnchors.erase(mr->button);



        m_deviceState.mouseGesture.numberOfMouseButtonsHeld--;
    }
    else if(const auto* mv = opt->getIf<sf::Event::MouseMoved>())
    {
        m_deviceState.mouseGesture.isMouseMoveEvent = true;
        // if ( m_deviceState.mouseButtons[sf::Mouse::Button::Left] && !m_deviceState.mouseGesture.isDragging ) {
        //     int deltaX = std::abs(mv->position.x - m_deviceState.mouseGesture.dragAnchor.x);
        //     int deltaY = std::abs(mv->position.y - m_deviceState.mouseGesture.dragAnchor.y);

        //     if (deltaX*deltaX + deltaY*deltaY > 25) {
        //         m_deviceState.mouseGesture.isDragging = true; 
        //     }
        // }
        for( const auto& [MouseButton , AnchorPosition] : m_deviceState.mouseGesture.clickOrDragAnchors )
        {
            if( m_deviceState.mouseGesture.draggingStatus[MouseButton] ) continue;

            int deltaX = std::abs( mv->position.x - AnchorPosition.x );
            int deltaY = std::abs( mv->position.y - AnchorPosition.y );

            if( deltaX*deltaX + deltaY*deltaY > 25 )
            {
                m_deviceState.mouseGesture.draggingStatus[MouseButton] = true;
            }
        }
    }
}

//Implement later .....
// void ActionManager::forceFireAction(const Action& action)
// {
//     m_actions.push_back(action);
// }
//&& !state.mouseGesture.outOfWindowHoverBug

void ActionManager::processOngoingActions()
{
    for( auto activeActionId : m_activeActions )
    {
        // for ( auto& binding : m_bindings)
        // {
        //     if( binding.getId() == activeActionId )
        //     {
        //         // m_actions.emplace_back(binding.getActionName(), "OnGoing", m_deviceState.mousePos);
        //         m_actions.emplace_back(RawActionBundle(Action(binding.getActionName(), "OnGoing", m_deviceState.mousePos , m_gameTimeData.m_gameTime) , binding , m_deviceState.mousePos));
        //     }   
        // }

        std::optional<size_t> indexOfBindingId = m_binarySearchForBindingThroughId(activeActionId);
        m_actions.emplace_back(RawActionBundle(Action(m_bindings[*indexOfBindingId].getActionName(), ActionType::OnGoing, m_deviceState.mousePos , m_gameTimeData.m_gameTime) , m_bindings[*indexOfBindingId] , m_deviceState.mousePos));
        
    }
    
} 


void ActionManager::m_updateGameTimeData(sf::Time eventTriggerTime , bool isScenePaused)
{
    m_gameTimeData.m_previousEventTime = m_gameTimeData.m_currentEventTime;
    m_gameTimeData.m_currentEventTime = eventTriggerTime;
    sf::Time de = m_gameTimeData.m_currentEventTime - m_gameTimeData.m_previousEventTime;
    if(!isScenePaused)
    {
        m_gameTimeData.m_gameTime+=de;
    }

}

std::optional<size_t> ActionManager::m_binarySearchForBindingThroughId(size_t bindingId)
{
    size_t low = 0 , high = m_bindings.size() -  1;
    while(low <= high)
    {
        size_t mid = low + (high - low)/2;
        if(m_bindings[mid].getId() == bindingId )
        {
            return mid;
        }
        else if( m_bindings[mid].getId() < bindingId )
        {
            low = mid+1;
        }
        else
        {
            high = mid-1;
        }
    }
    return {};
}

ActionBinding& ActionManager::getBinding(size_t bindingId)
{
    std::optional<size_t> bindingIdIndexIn_m_bindings = m_binarySearchForBindingThroughId(bindingId);
    if(bindingIdIndexIn_m_bindings.has_value() )
    {
        return m_bindings[ bindingIdIndexIn_m_bindings.value() ];
    }
    else
    {
        throw std::runtime_error(std::format("No binding found with id : {}" , bindingId ));
    }
}

ActionBinding& ActionManager::getBinding(std::string& actionName)
{
    if( m_actionNameToBindingIdLookupTable.contains(actionName) )
    {
        return getBinding( m_actionNameToBindingIdLookupTable[actionName]);
    }
    else
    {
        throw std::runtime_error(std::format("No binding found for action with name : {}" , actionName));
    }
}

void ActionManager::deleteBinding(size_t bindingId)
{
    std::optional<size_t> bindingIdIndexIn_m_bindings = m_binarySearchForBindingThroughId(bindingId);
    if(bindingIdIndexIn_m_bindings.has_value() )
    {
        m_actionNameToBindingIdLookupTable.erase(m_bindings[ bindingIdIndexIn_m_bindings.value() ].getActionName());
        m_deletedBindings.emplace(bindingId , std::move(m_bindings[ bindingIdIndexIn_m_bindings.value() ]));
        m_bindings.erase(m_bindings.begin() + bindingIdIndexIn_m_bindings.value() );

    }
    else
    {
        throw std::runtime_error(std::format("No binding found to delete with id : {}" , bindingId ));
    }
}

void ActionManager::deleteBinding(std::string& actionName)
{
    if( m_actionNameToBindingIdLookupTable.contains(actionName) )
    {
        deleteBinding( m_actionNameToBindingIdLookupTable[actionName]);
    }
    else
    {
        throw std::runtime_error(std::format("No binding found to delete for action with name : {}" , actionName));
    }
}