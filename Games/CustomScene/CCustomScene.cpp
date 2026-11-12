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

void HomeScreen::onDelete()
{
    //Have to implement
}

void HomeScreen::onLoad()
{

    {
        ActionBinding& binding = this->registerActionBinding("LeftClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("RightClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Right);
    }
    {
        ActionBinding& binding = this->registerActionBinding("LeftDrag");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Drag , sf::Mouse::Button::Left);
    }
    {   
        ActionBinding& binding = this->registerActionBinding("MouseMoved");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::MouseMoved);
    }
    {
        ActionBinding& binding = this->registerActionBinding("Close");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::Escape);
        binding.addCondition<ModifierCondition>(false , false , false);
    }


}

void HomeScreen::sDoAction(RefinedAction& input) 
{

    // if(input.getName() == "Close" && input.getType() == ActionType::Started)
    // {
    //     requests.push_back(Request(RequestCommand::Pop , "Default"));
    // }
    
}

void HomeScreen::sUpdate(sf::Time dt)
{
    m_entities.update();

    if(!m_inputTestLaunched)
    {
        m_inputTestLaunched = true;
        m_requests.push_back(Request(RequestCommand::Push , "InputTest"));
    }
    else
    {
        // only reached again after InputTest has popped itself
        m_requests.push_back(Request(RequestCommand::Pop , "Default"));
    }
    

}

void HomeScreen::sRender(sf::RenderWindow& gameWindow)  
{
    
    gameWindow.clear();
    
    gameWindow.display();
}