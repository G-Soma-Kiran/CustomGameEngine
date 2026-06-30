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
    if(assets)
    {
        //Load assets to asset manager
    }

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


}

void HomeScreen::sDoAction(Action input , std::vector<Request>& requests) 
{
    std::cout << input.getName() << " " << input.getType() << " {" << input.getMousePos().x << "," << input.getMousePos().y << "}\n";
}

void HomeScreen::sUpdate()
{
    //Spawn entities
    m_entities.update();
}

void HomeScreen::sRender(sf::RenderWindow& gameWindow)  
{
    //Later
}