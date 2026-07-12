#include"UISystem.hpp"

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

void UISystem::update(Action& input , std::vector<Request>& requests)
{
    // if( m_entities.storage<CMouseInteraction>().hasComponent(m_entities.getidOftopMostEntityUnderMouse() ))
    // {
    //     auto& currentClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_entities.getidOftopMostEntityUnderMouse() );
    //     if(input.getName() == "Hover")
    //     {
    //         buttonUpdate(m_entities.getidOftopMostEntityUnderMouse() , ButtonState::Hovered);

    //         if( m_uistate.previousHoveredEntity == m_entities.getidOftopMostEntityUnderMouse() )
    //         {
    //             currentClickableComponent.duringHover();
    //         }
    //         else if( 0 == m_uistate.previousHoveredEntity  )
    //         {
    //             currentClickableComponent.onHoverEnter();
    //         }
    //         else if( m_uistate.previousHoveredEntity != m_entities.getidOftopMostEntityUnderMouse() )
    //         {
    //             auto& previousHoveredClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_uistate.previousHoveredEntity );
    //             previousHoveredClickableComponent.onHoverLeave();
    //             buttonUpdate(m_uistate.previousHoveredEntity , ButtonState::Normal);

    //             currentClickableComponent.onHoverEnter();
    //         }
    //         m_uistate.previousHoveredEntity = m_entities.getidOftopMostEntityUnderMouse();
    //     }
    //     else if( input.getName() == "MouseLeftDown" )
    //     {
    //         if(input.getType() == "Started")
    //         {
    //             currentClickableComponent.onMousePress();
    //             buttonUpdate(m_entities.getidOftopMostEntityUnderMouse() , ButtonState::Pressed);
    //             m_uistate.candidateClick = m_entities.getidOftopMostEntityUnderMouse();
    //         }
    //         else
    //         {
    //             currentClickableComponent.onMouseRelease();
    //         }
    //     }
    //     else if( input.getName() == "Click" )
    //     {
    //         if( m_uistate.candidateClick == m_entities.getidOftopMostEntityUnderMouse() )
    //         {
    //             currentClickableComponent.onClick();
    //             buttonUpdate(m_entities.getidOftopMostEntityUnderMouse() , ButtonState::Normal);
    //         }
    //         m_uistate.candidateClick = 0;
    //     }
    //     else if( input.getName() == "Drag" )
    //     {
    //         if( input.getType() == "Started" )
    //         {
    //             buttonUpdate(m_entities.getidOftopMostEntityUnderMouse() , ButtonState::Normal);
    //             currentClickableComponent.onDragStart();
    //             m_uistate.candidateClick = 0;
    //             m_uistate.beingDraggedEntity = m_entities.getidOftopMostEntityUnderMouse();
    //         }
    //         else
    //         {
    //             buttonUpdate(m_entities.getidOftopMostEntityUnderMouse() , ButtonState::Hovered);
    //             currentClickableComponent.onDragEnd();   
    //             m_uistate.beingDraggedEntity=0;
    //         }
    //     }

    // }
    size_t topMost = m_entities.getidOftopMostEntityUnderMouse();
    if( input.getName() == "Hover" )
    {
        if( NoEntity == m_uistate.previousHoveredEntity)
        {
            if( NoEntity != topMost )
            {
                auto& currentClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( topMost );
                currentClickableComponent.onHoverEnter();
                buttonUpdate(topMost , ButtonState::Hovered);

                m_uistate.previousHoveredEntity = topMost;
            }
        }
        else
        {
            if( NoEntity != topMost)
            {
                if( m_uistate.previousHoveredEntity != topMost )
                {
                    auto& currentClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( topMost );
                    auto& previousClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_uistate.previousHoveredEntity);

                    previousClickableComponent.onHoverLeave();
                    buttonUpdate(m_uistate.previousHoveredEntity , ButtonState::Normal);

                    currentClickableComponent.onHoverEnter();
                    buttonUpdate(topMost , ButtonState::Hovered);

                    m_uistate.previousHoveredEntity = topMost;
                }
                
            }
            else
            {
                auto& previousClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_uistate.previousHoveredEntity);

                previousClickableComponent.onHoverLeave();
                buttonUpdate(m_uistate.previousHoveredEntity , ButtonState::Normal);

                m_uistate.previousHoveredEntity = NoEntity;
            }
        }
    }
    else if( input.getName() == "MouseLeftDown" )
    {
        if( input.getType() == "Started" )
        {
            if( NoEntity != topMost )
            {
                if( NoEntity != m_uistate.previousHoveredEntity )
                {
                    auto& previousClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_uistate.previousHoveredEntity);

                    previousClickableComponent.onHoverLeave();
                    buttonUpdate(m_uistate.previousHoveredEntity , ButtonState::Normal);
                    
                    m_uistate.previousHoveredEntity = NoEntity;

                }

                auto& currentClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( topMost );

                currentClickableComponent.onMouseLeftPress();
                buttonUpdate(topMost , ButtonState::Pressed);

                m_uistate.candidateClick = topMost;

            }
            else
            {
                if( NoEntity != m_uistate.previousHoveredEntity )
                {
                    auto& previousClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_uistate.previousHoveredEntity);

                    previousClickableComponent.onHoverLeave();
                    buttonUpdate(m_uistate.previousHoveredEntity , ButtonState::Normal);
                    
                    m_uistate.previousHoveredEntity = NoEntity;
                }
                m_uistate.candidateClick = NoEntity;
            }
        }
        else
        {
            if( NoEntity != topMost)
            {
                auto& currentClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( topMost );

                currentClickableComponent.onMouseLeftRelease();
                buttonUpdate(topMost , ButtonState::Normal);

                m_uistate.candidateClick = NoEntity;


                if( NoEntity != m_uistate.previousHoveredEntity )
                {
                    auto& previousClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_uistate.previousHoveredEntity);

                    previousClickableComponent.onHoverLeave();
                    buttonUpdate(m_uistate.previousHoveredEntity , ButtonState::Normal);

                    m_uistate.previousHoveredEntity = NoEntity;
                }

                currentClickableComponent.onHoverEnter();
                buttonUpdate(topMost , ButtonState::Hovered);

                m_uistate.previousHoveredEntity = topMost;


            }
            else
            {
                if( NoEntity != m_uistate.previousHoveredEntity )
                {
                    auto& previousClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_uistate.previousHoveredEntity);

                    previousClickableComponent.onHoverLeave();
                    buttonUpdate(m_uistate.previousHoveredEntity , ButtonState::Normal);

                    m_uistate.previousHoveredEntity = NoEntity;
                }
                m_uistate.candidateClick = NoEntity;
            }
        }
    }
    else if( input.getName() == "Drag" )
    {
        if( input.getType() == "Started" )
        {
            if( NoEntity != topMost )
            {
                if( NoEntity != m_uistate.previousHoveredEntity )
                {
                    auto& previousClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_uistate.previousHoveredEntity);

                    previousClickableComponent.onHoverLeave();
                    buttonUpdate(m_uistate.previousHoveredEntity , ButtonState::Normal);

                    m_uistate.previousHoveredEntity = NoEntity;
                }


                auto& currentClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( topMost );
                
                currentClickableComponent.onDragStart();
                buttonUpdate(topMost , ButtonState::Normal);

                m_uistate.candidateClick = NoEntity;
                m_uistate.beingDraggedEntity = topMost;

            }
            else
            {
                if( NoEntity != m_uistate.previousHoveredEntity )
                {
                    auto& previousClickableComponent = m_entities.storage<CMouseInteraction>().getComponent( m_uistate.previousHoveredEntity);

                    previousClickableComponent.onHoverLeave();
                    buttonUpdate(m_uistate.previousHoveredEntity , ButtonState::Normal);

                    m_uistate.previousHoveredEntity = NoEntity;
                }
                m_uistate.candidateClick = NoEntity;
            }
        }
        else
        {
            if( NoEntity != topMost )
            {

            }
        }
    }

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