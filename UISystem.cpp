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

std::vector<RefinedAction> UISystem::refineAction(RawActionBundle input)
{

    const ActionBinding& binding = input.getBinding();
    const Action& action = input.getAction();

    m_uistate.previousMouseOverEntity = m_uistate.currentMouseOverEntity;
    m_uistate.currentMouseOverEntity =  updateMouseOver(input.getCurrentMousePos());

    std::vector<RefinedAction> refinedActions;
    if(  binding.getGesture().has_value() && binding.getGesture().value()->getConditionName() == conditionName::MouseOverCondition )
    {

            if( m_uistate.currentMouseOverEntity != m_uistate.previousMouseOverEntity )
            {
                for ( auto& bindingInternal : m_uistate.bindingActivationOrder )
                {
                    
                    refinedActions.emplace_back( RefinedAction(bindingInternal.getActionName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.previousMouseOverEntity ));
                    // The input.getCurrentMousePos() should actually be action.getMousePos() but as the bindings with MouseOverCondition will get both of them same, You can leave it like that. Change it later for semantic reasons
                    
                }

                if( action.getType() == ActionType::Ended )
                {
                    auto it = std::find_if( m_uistate.bindingActivationOrder.begin() , m_uistate.bindingActivationOrder.end() , 
                    
                    [&](BindingInternals& internal)
                    {
                        return internal.getId() == binding.getId();
                    }
                );

                    if( it != m_uistate.bindingActivationOrder.end())
                    {
                        m_uistate.bindingActivationOrder.erase(it);
                    }
                }
                if(action.getType() == ActionType::Started )
                {
                    m_uistate.bindingActivationOrder.push_back( {binding.getId() , binding.getActionName()} );
                }

                for ( auto bindingInternal : m_uistate.bindingActivationOrder )
                {
                    refinedActions.emplace_back( RefinedAction(bindingInternal.getActionName() , ActionType::Started , input.getCurrentMousePos(), action.getTriggerTime() , m_uistate.currentMouseOverEntity ));
                }
                

                if(action.getType() == ActionType::OnGoing)
                {
                    refinedActions.emplace_back( RefinedAction(binding.getActionName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.currentMouseOverEntity ));
                }
            }
            else
            {

                if( action.getType() == ActionType::Ended )
                {
                    auto it = std::find_if( m_uistate.bindingActivationOrder.begin() , m_uistate.bindingActivationOrder.end() , 
                    
                    [&](BindingInternals& internal)
                    {
                        return internal.getId() == binding.getId();
                    }
                );

                    if( it != m_uistate.bindingActivationOrder.end())
                    {
                        m_uistate.bindingActivationOrder.erase(it);
                    }
                }
                else if(action.getType() == ActionType::Started)
                {
                    m_uistate.bindingActivationOrder.push_back( {binding.getId() , binding.getActionName()} );
                }
                else if(action.getType() == ActionType::OnGoing )
                {
                    refinedActions.emplace_back( RefinedAction(binding.getActionName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.currentMouseOverEntity ));
                }
                
                
            }
    }
    else
    {

        if( m_uistate.currentMouseOverEntity != m_uistate.previousMouseOverEntity )
        {
            for ( auto bindingInternal : m_uistate.bindingActivationOrder )
            {
                refinedActions.emplace_back( RefinedAction(bindingInternal.getActionName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.previousMouseOverEntity ));
            }

            if( action.getType() == ActionType::Ended )
            {
                if( binding.getCaptureEntityFlag() )
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()] ));
                    m_uistate.capturedEntityIdsByBindings.erase(binding.getId());  
                }
                else
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime()  ,  NoEntity ));
                }
            }
            
            for ( auto bindingInternal : m_uistate.bindingActivationOrder )
            {
                refinedActions.emplace_back( RefinedAction(bindingInternal.getActionName() , ActionType::Started , input.getCurrentMousePos(), action.getTriggerTime() , m_uistate.currentMouseOverEntity ));
            }

            if( action.getType() == ActionType::Started )
            {
                if( binding.getCaptureEntityFlag() )
                {
                    if( action.getMousePos() != input.getCurrentMousePos() )
                    {
                        size_t entityIdAtMousePos = m_entities.findTopMostEntityIdUnderMouse(action.getMousePos());  
                        m_uistate.capturedEntityIdsByBindings[binding.getId()] = entityIdAtMousePos;        
                    }
                    else
                    {
                        m_uistate.capturedEntityIdsByBindings[binding.getId()] = m_uistate.currentMouseOverEntity;    
                    }
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Started , action.getMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()]));

                }
                else
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Started , action.getMousePos() , action.getTriggerTime() , NoEntity));
                }
            }

            if( action.getType() == ActionType::OnGoing )
            {
                if( binding.getCaptureEntityFlag() )
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()] ));
                }
                else
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime()  ,  NoEntity ));
                }
            }
        }
        else
        {
            if( action.getType() == ActionType::Ended )
            {
                if( binding.getCaptureEntityFlag() )
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()] ));
                    m_uistate.capturedEntityIdsByBindings.erase(binding.getId());  
                }
                else
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Ended , input.getCurrentMousePos() , action.getTriggerTime()  ,  NoEntity ));
                }
            }
            if( action.getType() == ActionType::Started )
            {
                if( binding.getCaptureEntityFlag() )
                {
                    if( action.getMousePos() != input.getCurrentMousePos() )
                    {
                        size_t entityIdAtMousePos = m_entities.findTopMostEntityIdUnderMouse(action.getMousePos());  
                        m_uistate.capturedEntityIdsByBindings[binding.getId()] = entityIdAtMousePos;        
                    }
                    else
                    {
                        m_uistate.capturedEntityIdsByBindings[binding.getId()] = m_uistate.currentMouseOverEntity;    
                    }
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Started , action.getMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()]));

                }
                else
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::Started , action.getMousePos() , action.getTriggerTime() , NoEntity));
                }
            }
            if( action.getType() == ActionType::OnGoing )
            {
                if( binding.getCaptureEntityFlag() )
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime() , m_uistate.capturedEntityIdsByBindings[binding.getId()] ));
                }
                else
                {
                    refinedActions.emplace_back(RefinedAction(action.getName() , ActionType::OnGoing , input.getCurrentMousePos() , action.getTriggerTime()  ,  NoEntity ));
                }
            }
        }
    }
    
    
    return refinedActions;
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

size_t RefinedAction::getEntityId() const
{
    return m_entityId;
}