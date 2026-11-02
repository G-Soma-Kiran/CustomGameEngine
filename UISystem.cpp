#include"UISystem.hpp"

UISystem::UISystem(EntityManager& entitymanager , ActionManager& actionManager):m_entities(entitymanager) , m_actionManager(actionManager)
{
    // {
    //     auto& binding =  m_actionManager.addBinding("UI_Click");
    //     binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Left);
    //     binding.addCondition<ModifierCondition>(false, false , false);
    // }
    // {
    //     auto& binding =  m_actionManager.addBinding("UI_Drag");
    //     binding.setGesture<MouseGestureCondition>(mouseGestureType::Drag);
    //     binding.addCondition<ModifierCondition>(false, false , false);
    // }
    // {
    //     auto& binding =  m_actionManager.addBinding("UI_DoubleClick");
    //     binding.setGesture<MouseGestureCondition>(mouseGestureType::DoubleClick );   
    // }
    // {
    //     auto& binding =  m_actionManager.addBinding("UI_MultiDrag");
    //     binding.addCondition<ModifierCondition>(true , false , false);
    //     binding.setGesture<MouseGestureCondition>(mouseGestureType::Drag);
    // }
    // {
    //     auto& binding =  m_actionManager.addBinding("UI_MouseLeftDown");
    //     binding.addCondition<MouseKeyCondition>(sf::Mouse::Button::Left);
    // }
    // {    
    //     auto& binding =  m_actionManager.addBinding("UI_Hover");
    //     binding.setGesture<MouseOverCondition>();
    //     binding.addCondition<NoMouseKeyHeldCondition>();
    // }
};

std::shared_ptr<Entity> UISystem::makeButton(const sf::Texture& buttonTexture , ButtonStateRects rects , Vector position , sf::FloatRect localBounds)
{
    auto p = m_entities.addEntity("UI_Button");
    m_entities.storage<CVisual>().addComponent( p->getId() , buttonTexture);
    m_entities.storage<CRender>().addComponent( p->getId()).zIndex = 2 ;
    m_entities.storage<CButton>().addComponent( p->getId() , rects) ;

    if(localBounds.size.x == 0 && localBounds.size.y == 0)
    {
        m_entities.storage<CMouseInteraction>().addComponent(p->getId() , sf::FloatRect({0 , 0} , m_entities.storage<CVisual>().getComponent( p->getId() ).sprite.getLocalBounds().size ));
    }else
    {
        m_entities.storage<CMouseInteraction>().addComponent(p->getId() , localBounds);
    }
        
    m_entities.storage<CTransform>().addComponent( p->getId() , position);
    return p;
}

void shit(){
true;
// std::vector<RefinedAction> UISystem::refineAction(RawActionBundle input)
// {

//     const ActionBinding& binding = input.getBinding();
//     const Action& action = input.getAction();

//     m_uistate.previousMouseOverEntity = m_uistate.currentMouseOverEntity;
//     m_uistate.currentMouseOverEntity =  updateMouseOver(input.getCurrentMousePos());

//     std::vector<RefinedAction> refinedActions;
//     if(  binding.getGesture().has_value() && binding.getGesture().value()->getConditionName() == conditionName::MouseOverCondition )
//     {

//             if( m_uistate.currentMouseOverEntity != m_uistate.previousMouseOverEntity )
//             {
//                 for ( auto& bindingInternal : m_uistate.bindingActivationOrder )
//                 {
                    
//                     refinedActions.emplace_back( RefinedAction(bindingInternal.getActionName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.previousMouseOverEntity ));
//                     // The input.getCurrentMousePos() should actually be action.getMousePos() but as the bindings with MouseOverCondition will get both of them same, You can leave it like that. Change it later for semantic reasons
                    
//                 }

//                 if( action.getType() == ActionType::Ended )
//                 {
//                     auto it = std::find_if( m_uistate.bindingActivationOrder.begin() , m_uistate.bindingActivationOrder.end() , 
                    
//                     [&](BindingInternals& internal)
//                     {
//                         return internal.getId() == binding.getId();
//                     }
//                 );

//                     if( it != m_uistate.bindingActivationOrder.end())
//                     {
//                         m_uistate.bindingActivationOrder.erase(it);
//                     }
//                 }
//                 if(action.getType() == ActionType::Started )
//                 {
//                     m_uistate.bindingActivationOrder.push_back( {binding.getId() , binding.getActionName()} );
//                 }

//                 for ( auto bindingInternal : m_uistate.bindingActivationOrder )
//                 {
//                     refinedActions.emplace_back( RefinedAction(bindingInternal.getActionName() , ActionType::Started , input.getCurrentMousePos(), action.getTriggerTime() , m_uistate.currentMouseOverEntity ));
//                 }
                

//                 if(action.getType() == ActionType::OnGoing)
//                 {
//                     refinedActions.emplace_back( RefinedAction(binding.getActionName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.currentMouseOverEntity ));
//                 }
//             }
//             else
//             {

//                 if( action.getType() == ActionType::Ended )
//                 {
//                     auto it = std::find_if( m_uistate.bindingActivationOrder.begin() , m_uistate.bindingActivationOrder.end() , 
                    
//                     [&](BindingInternals& internal)
//                     {
//                         return internal.getId() == binding.getId();
//                     }
//                 );

//                     if( it != m_uistate.bindingActivationOrder.end())
//                     {
//                         m_uistate.bindingActivationOrder.erase(it);
//                     }
//                 }
//                 else if(action.getType() == ActionType::Started)
//                 {
//                     m_uistate.bindingActivationOrder.push_back( {binding.getId() , binding.getActionName()} );
//                 }
//                 else if(action.getType() == ActionType::OnGoing )
//                 {
//                     refinedActions.emplace_back( RefinedAction(binding.getActionName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.currentMouseOverEntity ));
//                 }
                
                
//             }
//     }
//     else
//     {

//         if( m_uistate.currentMouseOverEntity != m_uistate.previousMouseOverEntity )
//         {
//             for ( auto bindingInternal : m_uistate.bindingActivationOrder )
//             {
//                 refinedActions.emplace_back( RefinedAction(bindingInternal.getActionName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.previousMouseOverEntity ));
//             }

//             if( action.getType() == ActionType::Ended )
//             {
//                 if( binding.getCaptureEntityFlag() )
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()] ));
//                     m_uistate.capturedEntityIdsByBindings.erase(binding.getId());  
//                 }
//                 else
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime()  ,  NoEntity ));
//                 }
//             }
            
//             for ( auto bindingInternal : m_uistate.bindingActivationOrder )
//             {
//                 refinedActions.emplace_back( RefinedAction(bindingInternal.getActionName() , ActionType::Started , input.getCurrentMousePos(), action.getTriggerTime() , m_uistate.currentMouseOverEntity ));
//             }

//             if( action.getType() == ActionType::Started )
//             {
//                 if( binding.getCaptureEntityFlag() )
//                 {
//                     if( action.getMousePos() != input.getCurrentMousePos() )
//                     {
//                         size_t entityIdAtMousePos = m_entities.findTopMostEntityIdUnderMouse(action.getMousePos());  
//                         m_uistate.capturedEntityIdsByBindings[binding.getId()] = entityIdAtMousePos;        
//                     }
//                     else
//                     {
//                         m_uistate.capturedEntityIdsByBindings[binding.getId()] = m_uistate.currentMouseOverEntity;    
//                     }
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Started , action.getMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()]));

//                 }
//                 else
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Started , action.getMousePos() , action.getTriggerTime() , NoEntity));
//                 }
//             }

//             if( action.getType() == ActionType::OnGoing )
//             {
//                 if( binding.getCaptureEntityFlag() )
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()] ));
//                 }
//                 else
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime()  ,  NoEntity ));
//                 }
//             }
//         }
//         else
//         {
//             if( action.getType() == ActionType::Ended )
//             {
//                 if( binding.getCaptureEntityFlag() )
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()] ));
//                     m_uistate.capturedEntityIdsByBindings.erase(binding.getId());  
//                 }
//                 else
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime()  ,  NoEntity ));
//                 }
//             }
//             if( action.getType() == ActionType::Started )
//             {
//                 if( binding.getCaptureEntityFlag() )
//                 {
//                     if( action.getMousePos() != input.getCurrentMousePos() )
//                     {
//                         size_t entityIdAtMousePos = m_entities.findTopMostEntityIdUnderMouse(action.getMousePos());  
//                         m_uistate.capturedEntityIdsByBindings[binding.getId()] = entityIdAtMousePos;        
//                     }
//                     else
//                     {
//                         m_uistate.capturedEntityIdsByBindings[binding.getId()] = m_uistate.currentMouseOverEntity;    
//                     }
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Started , action.getMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()]));

//                 }
//                 else
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Started , action.getMousePos() , action.getTriggerTime() , NoEntity));
//                 }
//             }
//             if( action.getType() == ActionType::OnGoing )
//             {
//                 if( binding.getCaptureEntityFlag() )
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()] ));
//                 }
//                 else
//                 {
//                     refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime()  ,  NoEntity ));
//                 }
//             }
//         }
//     }
    
    
//     return refinedActions;
// }
}

std::vector<RefinedAction> UISystem::refineAction(RawActionBundle input)
{
    const Action& action = input.getAction();
    const ActionBinding& binding = input.getBinding();
    auto& currMousePos = input.getCurrentMousePos();
    auto& currTime = input.getCurrentTime();
    
    std::vector<RefinedAction> out;

    m_uistate.currentMousePos = currMousePos;
    m_uistate.currentGameTime = currTime;
    m_uistate.previousMouseOverEntity = m_uistate.currentMouseOverEntity;
    m_uistate.currentMouseOverEntity = updateMouseOver(currMousePos);

    const size_t currMouseOverEntity = m_uistate.currentMouseOverEntity;
    const bool mouseOverChanged = currMouseOverEntity != m_uistate.previousMouseOverEntity;
    const bool isMouseOverBinding = binding.getGesture().has_value() && binding.getGesture().value()->getConditionName() == conditionName::MouseOverCondition;

    if (const auto button = input.getStoreInitiateEntitySignal())
        m_uistate.initiateEntityByButton[static_cast<size_t>(*button)] = currMouseOverEntity;
    if (const auto button = input.getStoreDoubleClickFirstClickEntitySignal())
        m_uistate.doubleClickFirstClickEntity[static_cast<size_t>(*button)] = currMouseOverEntity;
    
    if (currMousePos != m_uistate.lastSamplePos)
    {
        m_uistate.prevSampleEntity = m_uistate.sampleEntity;
        m_uistate.sampleEntity     = currMouseOverEntity;
        m_uistate.lastSamplePos    = currMousePos;
    }


    auto& hovers = m_uistate.bindingActivationOrder;
    auto& storedEntities = m_uistate.storedEntities;
    auto recordOfCurrBinding = [&]
    {
        return std::find_if(hovers.begin(), hovers.end(),
                            [&](const HoverRecord& h) { return h.bindingId == binding.getId(); });
    };

    auto recordOfCurrBindingStoredEntity = [&]
    {
        return std::find_if( storedEntities.begin(), storedEntities.end(),
                               [&](const StoredEntities& l) { return l.getBindingId() == binding.getId(); });
    };
    
    if(mouseOverChanged)
    {
        for(const auto& record : hovers)
        {
            out.emplace_back(RefinedAction(record.actionName , ActionType::Ended , record.initiateAndTriggerPos , record.initiateAndTriggerPos , record.initiateAndTriggerTime , record.initiateAndTriggerTime , record.initiateAndTriggerEntityId , record.initiateAndTriggerEntityId));
        }
    }

    if(isMouseOverBinding && action.getType() == ActionType::Ended)
    {
        auto it = recordOfCurrBinding();
        if(it != hovers.end())
        {
            hovers.erase(it);
        }
        else
        {
            throw std::runtime_error("MouseOver gesture containing action ended without being in bindingActivationOrder");
        }
    }

    if(!isMouseOverBinding && action.getType() == ActionType::Ended)
    {
        auto it = recordOfCurrBindingStoredEntity();
        if(it != storedEntities.end())
            out.emplace_back(RefinedAction(binding.getActionName() , ActionType::Ended , action.getInitiateMousePos() , action.getTriggerMousePos()  , action.getInitiateTime() , action.getTriggerTime() , (*it).getInitiateEntityId() , (*it).getTriggerEntityId() ));
        else
            throw std::runtime_error("Not mouseover containing action ended without being in stored entities");
        storedEntities.erase(it);
    }

    if(isMouseOverBinding && action.getType() == ActionType::Started)
    {
        hovers.push_back(HoverRecord(binding.getId() , binding.getActionName() , currMousePos , currTime , currMouseOverEntity));
    }

    if(mouseOverChanged)
    {
        for(auto& record : hovers)
        {
            record.initiateAndTriggerEntityId = currMouseOverEntity;
            record.initiateAndTriggerPos = currMousePos;
            record.initiateAndTriggerTime = currTime;
            out.emplace_back(RefinedAction(record.actionName , ActionType::Started , record.initiateAndTriggerPos , record.initiateAndTriggerPos , record.initiateAndTriggerTime , record.initiateAndTriggerTime , record.initiateAndTriggerEntityId , record.initiateAndTriggerEntityId));
        }
    }

    if(!isMouseOverBinding && (action.getType() == ActionType::Started || action.getType() == ActionType::Pulsed))
    {
        auto getInitiateEntity = [&]() -> size_t
        {
            if (!binding.getGesture().has_value())
                return currMouseOverEntity;              
        
            const GestureCondition& gesture = *binding.getGesture().value();
            const auto button = gesture.getMouseButton();
            const auto kind   = gesture.getMouseGestureType();
            if (!button || !kind)
                return currMouseOverEntity;
        
            const size_t slot = static_cast<size_t>(*button);
            switch (*kind)
            {
                case mouseGestureType::Click:
                case mouseGestureType::Drag:        return m_uistate.initiateEntityByButton[slot];       // under the press
                case mouseGestureType::DoubleClick: return m_uistate.doubleClickFirstClickEntity[slot];  // under the first click's release
                case mouseGestureType::MouseMoved:  return m_uistate.prevSampleEntity;                   // under the previous sample
            }
            return currMouseOverEntity;
        };

        // StoredEntities curr(binding.getId() , getInitiateEntity() , currMouseOverEntity);
        // storedEntities.push_back(curr);
        // out.emplace_back(RefinedAction(binding.getActionName() , action.getType() , action.getInitiateMousePos() , action.getTriggerMousePos()  , action.getInitiateTime() , action.getTriggerTime() , curr.getInitiateEntityId() , curr.getTriggerEntityId() ));
        // in the Started/Pulsed block, replacing the curr / push_back / emplace_back lines
        const size_t initiateEntity = getInitiateEntity();
        out.emplace_back(RefinedAction(binding.getActionName(), action.getType(), action.getInitiateMousePos(), action.getTriggerMousePos(), action.getInitiateTime(), action.getTriggerTime(), initiateEntity, currMouseOverEntity));

        if(action.getType() == ActionType::Started)
        {
            auto it = recordOfCurrBindingStoredEntity();
            if(it != storedEntities.end()) 
                throw std::runtime_error("Not mouseover containing action started while already being in stored entities");
            else
                storedEntities.push_back(StoredEntities(binding.getId(), initiateEntity, currMouseOverEntity));
        }
        
    }

    if( isMouseOverBinding && action.getType() == ActionType::OnGoing)
    {
        auto it = recordOfCurrBinding();
        if(it != hovers.end())
            out.emplace_back(RefinedAction((*it).actionName , ActionType::OnGoing , (*it).initiateAndTriggerPos , (*it).initiateAndTriggerPos , (*it).initiateAndTriggerTime , (*it).initiateAndTriggerTime , (*it).initiateAndTriggerEntityId , (*it).initiateAndTriggerEntityId));
        else
            throw std::runtime_error("MouseOver containing action got OnGoing without being in bindingActivationOrder");
    }

    if(!isMouseOverBinding && action.getType() == ActionType::OnGoing)
    {
        auto it = recordOfCurrBindingStoredEntity();
        if(it != storedEntities.end())
            out.emplace_back(RefinedAction(binding.getActionName() , ActionType::OnGoing , action.getInitiateMousePos() , action.getTriggerMousePos() , action.getInitiateTime() , action.getTriggerTime() , (*it).getInitiateEntityId() , (*it).getTriggerEntityId() ));
        else
            throw std::runtime_error("Not mouseover conatining action got ongoing without being in stored enttities");
    }



    return out;
    
    

}

bool UISystem::buttonUpdate(size_t entityId , ButtonState newState)
{
    if( m_entities.storage<CButton>().hasComponent( entityId ) )
    {
        auto& btnComponent = m_entities.storage<CButton>().getComponent(entityId);
        btnComponent.currentState = newState;
        return true;
    }
    return false;
}

size_t UISystem::updateMouseOver( const sf::Vector2i& mousePos )
{
    m_entities.refreshTopMostEntityIdUnderMouse( mousePos );
    return m_entities.getidOftopMostEntityUnderMouse();
}

size_t RefinedAction::getInitiateEntityId() const
{
    return m_initiateEntityId;
}

size_t RefinedAction::getTriggerEntityId() const
{
    return m_triggeredEntityId;
}