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

    {
        ActionBinding& binding = this->registerActionBinding("LeftClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("RightClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Right);
    }
    {
        ActionBinding& binding = this->registerActionBinding("MiddleClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Middle);
    }
    {
        ActionBinding& binding = this->registerActionBinding("LeftDrag");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Drag , sf::Mouse::Button::Left);
        binding.addCondition<ModifierCondition>(false, false , false);
    }
    {
        ActionBinding& binding = this->registerActionBinding("RightDrag");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Drag , sf::Mouse::Button::Right);
        binding.addCondition<ModifierCondition>(false, false , false);
    }
    {
        ActionBinding& binding = this->registerActionBinding("MiddleDrag");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Drag , sf::Mouse::Button::Middle);
        binding.addCondition<ModifierCondition>(false, false , false);
    }
    {
        ActionBinding& binding = this->registerActionBinding("LeftDoubleClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::DoubleClick  , sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("RightDoubleClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::DoubleClick  , sf::Mouse::Button::Right);
    }
    {
        ActionBinding& binding = this->registerActionBinding("MiddleDoubleClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::DoubleClick  , sf::Mouse::Button::Middle);
    }
    {   
        ActionBinding& binding = this->registerActionBinding("MouseMoved");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::MouseMoved);
    }

    // ActionBinding& hoverBinding = this->registerActionBinding("Hover");
    // hoverBinding.setGesture<MouseGestureCondition>(mouseGestureType::Hover);

    // ActionBinding& walkBinding = this->registerActionBinding("Walk");
    // walkBinding.addCondition<KeyCondition>(sf::Keyboard::Key::W);
    // walkBinding.addCondition<ModifierCondition>(false , false , false);

    // ActionBinding& runBinding = this->registerActionBinding("Run");
    // runBinding.addCondition<KeyCondition>(sf::Keyboard::Key::W);
    // runBinding.addCondition<ModifierCondition>(false , true , false);

    // ActionBinding& multiDragBinding = this->registerActionBinding("MultiDrag");
    // multiDragBinding.addCondition<ModifierCondition>(true , false , false);
    // multiDragBinding.setGesture<MouseGestureCondition>(mouseGestureType::Drag);

    // ActionBinding& mouseLeftDown = this->registerActionBinding("MouseLeftDown");
    // mouseLeftDown.addCondition<MouseKeyCondition>(sf::Mouse::Button::Left);

    ActionBinding& closeBinding = this->registerActionBinding("Close");
    closeBinding.addCondition<KeyCondition>(sf::Keyboard::Key::Escape);
    closeBinding.addCondition<ModifierCondition>(false , false , false);


    // {
    // auto p = m_entities.addEntity("Button");
    // CButton& tempCButton = m_entities.storage<CButton>().addComponent(p->getId() ,m_assetManager.getTexture("Button") );
    // tempCButton.stateBoundingRects[ButtonState::Normal] = {{0 , 0},{32 , 16}};
    // tempCButton.stateBoundingRects[ButtonState::Hovered] = {{32 , 0},{32 , 16}};
    // tempCButton.stateBoundingRects[ButtonState::Pressed] = {{64 , 0},{32 , 16}};

    // CTransform& tempCTransform = m_entities.storage<CTransform>().addComponent(p->getId() ,Vector(400.0f , 400.0f));

    // CVisual& tempCVisual = m_entities.storage<CVisual>().addComponent(p->getId() , tempCButton.buttonTexture);
    // tempCVisual.sprite.setOrigin({16 , 8});
    // tempCVisual.sprite.setPosition(tempCTransform.position.toVector2f());
    // tempCVisual.sprite.setTextureRect(tempCButton.stateBoundingRects[ButtonState::Normal]);
    // tempCVisual.sprite.setScale({4.0f , 4.0f});

    // CCollision& tempCCollision = m_entities.storage<CCollision>().addComponent(p->getId());
    // tempCCollision.localHitbox = {{400.0f - 64.0f , 400.0f - 32.0f} , {128.0f , 64.0f}};
    // }
    // {
    //     auto p = m_entities.addEntity("Background");
    //     CVisual& tempCVisual = m_entities.storage<CVisual>().addComponent(p->getId() , m_assetManager.getTexture("Background"));
    //     tempCVisual.sprite.setPosition({0 , 0});
    //     tempCVisual.sprite.setTextureRect({{0 , 0} , {192 , 108}});
    //     tempCVisual.sprite.setScale({10.0f , 10.0f});

    // }
    // {
    //     auto p = m_entities.addEntity("Pea");
    //     CVisual& tempCVisual = m_entities.storage<CVisual>().addComponent(p->getId() , m_assetManager.getTexture("Pea"));
    //     tempCVisual.sprite.setPosition({0 , 0});
    //     tempCVisual.sprite.setTextureRect({{0 , 0} , {64 , 64}});
    //     tempCVisual.sprite.setScale({6.0f , 6.0f});
    // }
}

void HomeScreen::sDoAction(RefinedAction& input , std::vector<Request>& requests) 
{
    auto actionTypeToString = [](ActionType actionType)
    {
        switch (actionType)
        {
            case ActionType::Started:
                return "Started";
            case ActionType::OnGoing:
                return "OnGoing";
            case ActionType::Ended:
                return "Ended";
            case ActionType::Pulsed:
                return "Pulsed";  
        }
    };
    // if(input.getType() != ActionType::OnGoing)
    //     std::cout << std::format("{} , {} , {} , {} , {}\n" ,input.getName() ,actionTypeToString(input.getType()) , Vector(input.getMousePos().x , input.getMousePos().y).toString() , input.getTriggerTime().asSeconds() , input.getEntityId() );
    
//     std::cout << std::format(
//     "Name : {}\n"
//     "Type : {}\n"
//     "Initiate Mouse Pos : ({}, {})\n"
//     "Trigger Mouse Pos : ({}, {})\n"
//     "Initiate Time : {}\n"
//     "Trigger Time : {}\n"
//     "Initiate Entity ID : {}\n"
//     "Trigger Entity ID : {}\n",
//     input.getName(),
//     input.getType(),
//     input.getInitiateMousePos().x,
//     input.getInitiateMousePos().y,
//     input.getTriggerMousePos().x,
//     input.getTriggerMousePos().y,
//     input.getInitiateTime().asSeconds(),
//     input.getTriggerTime().asSeconds(),
//     input.getInitiateEntityId(),
//     input.getTriggerEntityId()
// );
// std::cout << std::format("currPos : {}\ncurrTime : {}\n\n\n\n" ,  m_uisystem.getMousePos() , m_uisystem.getTime().asSeconds());

std::cout << std::format(
    "Name : {}\n"
    "Type : {}\n"
    "Initiate Mouse Pos : ({}, {})\n"
    "Trigger Mouse Pos : ({}, {})\n"
    "Initiate Time : {}\n"
    "Trigger Time : {}\n"
    "Initiate Entity ID : {}\n"
    "Trigger Entity ID : {}\n",
    input.getName(),
    static_cast<int>(input.getType()), // Cast enum to int (or use a string conversion)
    input.getInitiateMousePos().x,
    input.getInitiateMousePos().y,
    input.getTriggerMousePos().x,
    input.getTriggerMousePos().y,
    input.getInitiateTime().asSeconds(),
    input.getTriggerTime().asSeconds(),
    input.getInitiateEntityId(),
    input.getTriggerEntityId()
);

// And for your mouse position log:
auto mousePos = m_uisystem.getMousePos();
std::cout << std::format("currPos : ({}, {})\ncurrTime : {}\n\n\n\n", 
    mousePos.x, mousePos.y, m_uisystem.getTime().asSeconds());


    if(input.getName() == "Close" && input.getType() == ActionType::Started)
    {
        requests.push_back(Request(RequestCommand::Pop , "Default"));
    }
    // const auto& vec = m_entities.getEntities("Button");
    // if(!vec.empty())
    // {
    //     auto& p = vec[0];
    //     if(  m_entities.storage<CCollision>().getComponent(p->getId()).localHitbox.contains({input.getMousePos().x , input.getMousePos().y}) )
    //     {   
    //         // if( input.getName() == "Hover" && input.getType() == "Started")
    //         //     m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Hovered;
    //         // else
    //         //     m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Normal;
    //         if( input.getName() == "MouseLeftDown" && input.getType() == "Started" )
    //             m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Pressed;
    //         else if((input.getName() == "MouseLeftDown" && input.getType() == "Ended") || input.getName() == "Hover")
    //             m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Hovered;
    //     }
    // }
    
}

void HomeScreen::sUpdate(sf::Time dt)
{
    //Spawn entities
    m_entities.update(); 
    

}

void HomeScreen::sRender(sf::RenderWindow& gameWindow)  
{
    //Later
    // auto p = m_entities.getEntities("Button")[0];
    // if(!m_entities.storage<CCollision>().getComponent(p->getId()).localHitbox.contains(sf::Vector2f(sf::Mouse::getPosition(gameWindow))))
    // {
    //     m_entities.storage<CButton>().getComponent(p->getId()).currentState = ButtonState::Normal;
    // }
    // m_entities.storage<CVisual>().getComponent(p->getId()).sprite.setTextureRect(m_entities.storage<CButton>().getComponent(p->getId()).stateBoundingRects[m_entities.storage<CButton>().getComponent(p->getId()).currentState]);
    // for( auto& p : m_entities.getEntities() )
    // {
    //     gameWindow.draw( m_entities.storage<CVisual>().getComponent(p->getId()).sprite);
    // }

}

// std::shared_ptr<Entity> HomeScreen::m_spawnDraggableRectangle(sf::Vector2i mousePos)
// {

// }
 