#include"CCustomScene.hpp"
#include<iostream>

bool HomeScreen::renderBelow() 
{
    return false;
}

bool HomeScreen::updateBelow()
{
    return false;
}

bool HomeScreen::propagateEventBelow()
{
    return false;
}

void HomeScreen::onDelete()
{
    //Have to implement
}

void HomeScreen::onLoad( const std::optional<std::vector<Asset>>& assets )
{
    m_assetManager.loadAssets(assets);

    ActionBinding& clickBinding = this->registerActionBinding("Click");
    clickBinding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Left);

    ActionBinding& dragBinding = this->registerActionBinding("Drag");
    dragBinding.setGesture<MouseGestureCondition>(mouseGestureType::Drag);
    dragBinding.addCondition<ModifierCondition>(false, false , false);
 
    ActionBinding& doubleClickBinding = this->registerActionBinding("DoubleClick");
    doubleClickBinding.setGesture<MouseGestureCondition>(mouseGestureType::DoubleClick );

    ActionBinding& hoverBinding = this->registerActionBinding("Hover");
    hoverBinding.setGesture<MouseGestureCondition>(mouseGestureType::Hover);

    ActionBinding& walkBinding = this->registerActionBinding("Walk");
    walkBinding.addCondition<KeyCondition>(sf::Keyboard::Key::W);
    walkBinding.addCondition<ModifierCondition>(false , false , false);

    ActionBinding& runBinding = this->registerActionBinding("Run");
    runBinding.addCondition<KeyCondition>(sf::Keyboard::Key::W);
    runBinding.addCondition<ModifierCondition>(false , true , false);

    ActionBinding& multiDragBinding = this->registerActionBinding("MultiDrag");
    multiDragBinding.addCondition<ModifierCondition>(true , false , false);
    multiDragBinding.setGesture<MouseGestureCondition>(mouseGestureType::Drag);

    ActionBinding& mouseLeftDown = this->registerActionBinding("MouseLeftDown");
    mouseLeftDown.addCondition<MouseKeyCondition>(sf::Mouse::Button::Left);

    ActionBinding& closeBinding = this->registerActionBinding("Close");
    closeBinding.addCondition<KeyCondition>(sf::Keyboard::Key::Escape);
    closeBinding.addCondition<ModifierCondition>(false , false , false);


    {
    auto p = m_entities.addEntity("Button");
    CButton& tempCButton = m_entities.storage<CButton>().addComponent(p->getId() ,m_assetManager.getTexture("Button") );
    tempCButton.stateBoundingRects[ButtonState::Normal] = {{0 , 0},{32 , 16}};
    tempCButton.stateBoundingRects[ButtonState::Hovered] = {{32 , 0},{32 , 16}};
    tempCButton.stateBoundingRects[ButtonState::Pressed] = {{64 , 0},{32 , 16}};

    CTransform& tempCTransform = m_entities.storage<CTransform>().addComponent(p->getId() ,Vector(400.0f , 400.0f));

    CVisual& tempCVisual = m_entities.storage<CVisual>().addComponent(p->getId() , tempCButton.buttonTexture);
    tempCVisual.sprite.setOrigin({16 , 8});
    tempCVisual.sprite.setPosition(tempCTransform.position.toVector2f());
    tempCVisual.sprite.setTextureRect(tempCButton.stateBoundingRects[ButtonState::Normal]);
    tempCVisual.sprite.setScale({4.0f , 4.0f});

    CCollision& tempCCollision = m_entities.storage<CCollision>().addComponent(p->getId());
    tempCCollision.localHitbox = {{400.0f - 64.0f , 400.0f - 32.0f} , {128.0f , 64.0f}};
    }
    {
        auto p = m_entities.addEntity("Background");
        CVisual& tempCVisual = m_entities.storage<CVisual>().addComponent(p->getId() , m_assetManager.getTexture("Background"));
        tempCVisual.sprite.setPosition({0 , 0});
        tempCVisual.sprite.setTextureRect({{0 , 0} , {192 , 108}});
        tempCVisual.sprite.setScale({10.0f , 10.0f});

    }
    {
        auto p = m_entities.addEntity("Pea");
        CVisual& tempCVisual = m_entities.storage<CVisual>().addComponent(p->getId() , m_assetManager.getTexture("Pea"));
        tempCVisual.sprite.setPosition({0 , 0});
        tempCVisual.sprite.setTextureRect({{0 , 0} , {64 , 64}});
        tempCVisual.sprite.setScale({6.0f , 6.0f});
    }
}

void HomeScreen::sDoAction(Action& input , std::vector<Request>& requests) 
{
    std::cout << input.getName() << " " << input.getType() << " {" << input.getMousePos().x << "," << input.getMousePos().y << "}\n";
    if(input.getName() == "Close")
    {
        requests.push_back(Request(RequestCommand::Pop , "Default"));
    }
    const auto& vec = m_entities.getEntities("Button");
    if(!vec.empty())
    {
        auto& p = vec[0];
        if(  m_entities.storage<CCollision>().getComponent(p->getId()).localHitbox.contains({input.getMousePos().x , input.getMousePos().y}) )
        {   
            // if( input.getName() == "Hover" && input.getType() == "Started")
            //     m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Hovered;
            // else
            //     m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Normal;
            if( input.getName() == "MouseLeftDown" && input.getType() == "Started" )
                m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Pressed;
            else if((input.getName() == "MouseLeftDown" && input.getType() == "Ended") || input.getName() == "Hover")
                m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Hovered;
        }
    }
    

}

void HomeScreen::sUpdate()
{
    //Spawn entities
    m_entities.update(); 
    

}

void HomeScreen::sRender(sf::RenderWindow& gameWindow)  
{
    //Later
    auto p = m_entities.getEntities("Button")[0];
    if(!m_entities.storage<CCollision>().getComponent(p->getId()).localHitbox.contains(sf::Vector2f(sf::Mouse::getPosition(gameWindow))))
    {
        m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Normal;
    }
    m_entities.storage<CVisual>().getComponent(p->getId()).sprite.setTextureRect(m_entities.storage<CButton>().getComponent(p->getId()).stateBoundingRects[m_entities.storage<CButton>().getComponent(p->getId()).currentState]);
    for( auto& p : m_entities.getEntities() )
    {
        gameWindow.draw( m_entities.storage<CVisual>().getComponent(p->getId()).sprite);
    }

}

// std::shared_ptr<Entity> HomeScreen::m_spawnDraggableRectangle(sf::Vector2i mousePos)
// {

// }
 