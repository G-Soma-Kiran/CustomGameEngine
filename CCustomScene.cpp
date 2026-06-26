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
    std::vector<std::unique_ptr<InputCondition>> conditionsClick;
    conditionsClick.push_back(std::make_unique<MouseClickCondition>(sf::Mouse::Button::Left));
    this->registerActionBinding("Click" , std::move(conditionsClick));

    std::vector<std::unique_ptr<InputCondition>> conditionsDrag;
    conditionsDrag.push_back(std::make_unique<MouseDragCondition>());
    this->registerActionBinding("Drag" , std::move(conditionsDrag));

    std::vector<std::unique_ptr<InputCondition>> conditionsDoubleClick;
    conditionsDoubleClick.push_back(std::make_unique<MouseDoubleClickCondition>());
    this->registerActionBinding("DoubleClick" , std::move(conditionsDoubleClick));

    std::vector<std::unique_ptr<InputCondition>> conditionsHover;
    conditionsHover.push_back(std::make_unique<MouseHoverCondition>());
    this->registerActionBinding("Hover" , std::move(conditionsHover));

    std::vector<std::unique_ptr<InputCondition>> conditionsWalk;
    conditionsWalk.push_back(std::make_unique<KeyCondition>(sf::Keyboard::Key::W));
    this->registerActionBinding("Walk" , std::move(conditionsWalk));

    std::vector<std::unique_ptr<InputCondition>> conditionsRun;
    conditionsRun.push_back(std::make_unique<KeyCondition>(sf::Keyboard::Key::W));
    conditionsRun.push_back(std::make_unique<ModifierCondition>(false , true , false));
    this->registerActionBinding("Run" , std::move(conditionsRun));
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