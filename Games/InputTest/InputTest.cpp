#include"InputTest.hpp"
#include<algorithm>
#include<cstdint>
#include<iostream>

// -------------------------------------------------------------------
// LEGEND
//   MouseOver        -> white inner outline + lighter fill on the entity under the mouse
//   MouseMoved       -> cyan dot follows the mouse; turns magenta (+ small ring) when the
//                       entity under the mouse changed since the previous sample
//   LeftClick        -> green flash + green ring (grey ring if press and release were on different entities)
//   RightClick       -> red flash + red ring
//   LeftDoubleClick  -> yellow flash + big yellow ring, and the box toggles between normal / 1.4x size
//   LeftDrag         -> on a box: box turns orange, moves with the mouse and is raised above everything
//                       on empty space: a rubber-band rectangle is drawn
//   Escape           -> pops the scene
// -------------------------------------------------------------------

namespace
{
    const sf::Color clickColor      (80  , 220 , 120);
    const sf::Color rightClickColor (230 , 70  , 70);
    const sf::Color doubleClickColor(250 , 220 , 40);
    const sf::Color mismatchColor   (150 , 150 , 150);
    const sf::Color moveCrossColor  (255 , 60  , 220);
    const sf::Color dragColor       (255 , 165 , 0  , 190);

    sf::Color blend(const sf::Color& from , const sf::Color& to , float t)
    {
        t = std::clamp(t , 0.f , 1.f);
        auto mix = [t](std::uint8_t a , std::uint8_t b)
        {
            return static_cast<std::uint8_t>(a + (b - a) * t);
        };
        return sf::Color(mix(from.r , to.r) , mix(from.g , to.g) , mix(from.b , to.b) , mix(from.a , to.a));
    }

    const char* toString(ActionType type)
    {
        switch (type)
        {
            case ActionType::Started: return "Started";
            case ActionType::OnGoing: return "OnGoing";
            case ActionType::Ended:   return "Ended";
            case ActionType::Pulsed:  return "Pulsed";
            case ActionType::Cancelled: return "Cancelled";
        }
        return "Unknown";
    }
}

bool InputTestScene::renderBelow()
{
    return false;
}

bool InputTestScene::updateBelow()
{
    return false;
}

void InputTestScene::onDelete()
{
}

void InputTestScene::onLoad( const std::optional<std::vector<Asset>>& /*assets*/ )
{
    {
        ActionBinding& binding = this->registerActionBinding("MouseOver");
        binding.setGesture<MouseOverCondition>();
    }
    {
        ActionBinding& binding = this->registerActionBinding("MouseMoved");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::MouseMoved);
    }
    {
        ActionBinding& binding = this->registerActionBinding("LeftClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("RightClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Click , sf::Mouse::Button::Right);
    }
    {
        ActionBinding& binding = this->registerActionBinding("LeftDoubleClick");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::DoubleClick , sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("LeftDrag");
        binding.setGesture<MouseGestureCondition>(mouseGestureType::Drag , sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("LeftMouseDown");
        binding.addCondition<MouseKeyCondition>(sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("Close");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::Escape);
        binding.addCondition<ModifierCondition>(false , false , false);
    }
    {
        ActionBinding& binding = this->registerActionBinding("NextScene");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::N);
        binding.addCondition<ModifierCondition>(false , false , false);
    }
    {
        ActionBinding& binding = this->registerActionBinding("PreviousScene");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::P);
        binding.addCondition<ModifierCondition>(false , false , false);
    }
    

    // Overlapping boxes with different z-indices (tests "top-most under mouse")
    spawnBox(sf::Color(200 ,  70 ,  70) , { 60.f ,  80.f} , {200.f , 120.f} , 1);
    spawnBox(sf::Color( 70 , 110 , 210) , {160.f , 140.f} , {200.f , 120.f} , 2);
    spawnBox(sf::Color( 70 , 170 ,  90) , {420.f ,  80.f} , {150.f , 150.f} , 3);
    spawnBox(sf::Color(150 ,  80 , 190) , {450.f , 180.f} , {150.f , 150.f} , 1);
    spawnBox(sf::Color(230 , 190 ,  60) , {640.f , 400.f} , { 90.f ,  90.f} , 5);

    // Two boxes with the SAME z-index (tests the tie-break: the newer one should win)
    spawnBox(sf::Color( 60 , 170 , 170) , {100.f , 380.f} , {160.f , 100.f} , 4);
    spawnBox(sf::Color(120 , 120 , 120) , {180.f , 420.f} , {160.f , 100.f} , 4);

    m_entities.update();
}

void InputTestScene::spawnBox(sf::Color color , sf::Vector2f position , sf::Vector2f size , int zIndex)
{
    auto entity = m_entities.addEntity("Box");
    const size_t id = entity->getId();

    m_entities.storage<CTransform>().addComponent(id , position);
    m_entities.storage<CRender>().addComponent(id).zIndex = zIndex;
    // top-left origin, so the local hitbox starts at (0,0)
    m_entities.storage<CMouseInteraction>().addComponent(id , sf::FloatRect({0.f , 0.f} , size));
    m_entities.storage<CDebugShape>().addComponent(id).shapeRect.setSize(size);
    m_entities.storage<CTestVisual>().addComponent(id , color , size , zIndex);
}

CTestVisual* InputTestScene::visualOf(size_t entityId)
{
    auto& visuals = m_entities.storage<CTestVisual>();
    if(visuals.hasComponent(entityId))
        return &visuals.getComponent(entityId);
    return nullptr;
}

void InputTestScene::flash(size_t entityId , const sf::Color& color , float duration , int priority)
{
    CTestVisual* visual = visualOf(entityId);
    if(!visual) return;

    // a double click also produces a click on the same release; keep the stronger flash
    if(visual->flashTimeLeft > 0.f && visual->flashPriority > priority) return;

    visual->flashColor = color;
    visual->flashDuration = duration;
    visual->flashTimeLeft = duration;
    visual->flashPriority = priority;
}

void InputTestScene::toggleSize(size_t entityId)
{
    CTestVisual* visual = visualOf(entityId);
    if(!visual) return;

    visual->enlarged = !visual->enlarged;
    const sf::Vector2f newSize = visual->baseSize * (visual->enlarged ? 1.4f : 1.f);

    m_entities.storage<CDebugShape>().getComponent(entityId).shapeRect.setSize(newSize);
    m_entities.storage<CMouseInteraction>().getComponent(entityId).localBounds.size = newSize;
}

void InputTestScene::addRipple(sf::Vector2f position , const sf::Color& color , float duration , float maxRadius)
{
    m_ripples.push_back(Ripple{position , color , duration , maxRadius});
}

// -------------------------------------------------------------------
// Actions
// -------------------------------------------------------------------

void InputTestScene::sDoAction(RefinedAction& input)
{
    const std::string& name = input.getName();
    const ActionType type = input.getType();
    const size_t initiateId = input.getInitiateEntityId();
    const size_t triggerId = input.getTriggerEntityId();
    const sf::Vector2f triggerPosition(input.getTriggerMousePos());

    // Console log (skips the noisy ones)
    if(type != ActionType::OnGoing && name != "MouseMoved")
    {
        std::cout << name << " | " << toString(type)
                  << " | initiate entity " << initiateId
                  << " -> trigger entity " << triggerId << '\n';
    }

    if(name == "Close" && type == ActionType::Started)
    {
        m_requests.push_back(Request(RequestCommand::Pop , "InputTest"));
    }
    else if(name == "MouseOver")
    {
        handleMouseOver(type , triggerId);
    }
    else if(name == "MouseMoved")
    {
        handleMouseMoved(initiateId , triggerId , triggerPosition);
    }
    else if(name == "LeftClick" && type == ActionType::Pulsed)
    {
        handleClick(initiateId , triggerId , triggerPosition , clickColor);
    }
    else if(name == "RightClick" && type == ActionType::Pulsed)
    {
        handleClick(initiateId , triggerId , triggerPosition , rightClickColor);
    }
    else if(name == "LeftDoubleClick" && type == ActionType::Pulsed)
    {
        handleDoubleClick(initiateId , triggerId , triggerPosition);
    }
    else if(name == "LeftDrag")
    {
        handleDrag(input);
    }
    else if(input.getName() == "NextScene" && input.getType() == ActionType::Started)
    {
        m_requests.push_back(Request(RequestCommand::Push , "Chess"));
    }
    else if(input.getName() == "PreviousScene" && input.getType() == ActionType::Started)
    {
        m_requests.push_back(Request(RequestCommand::Pop , "InputTest"));
    }
}

void InputTestScene::handleMouseOver(ActionType type , size_t entityId)
{
    // Ended carries the entity that was left, Started the entity that was entered,
    // so the order in which the two arrive doesn't matter.
    CTestVisual* visual = visualOf(entityId);
    if(!visual) return;

    if(type == ActionType::Started)
        visual->hovered = true;
    else if(type == ActionType::Ended)
        visual->hovered = false;
}

void InputTestScene::handleMouseMoved(size_t initiateEntityId , size_t triggerEntityId , sf::Vector2f position)
{
    // For MouseMoved: initiate entity = under the previous sample, trigger entity = under the current one
    m_hasMouseDot = true;
    m_mouseDotPosition = position;

    if(initiateEntityId != triggerEntityId)
    {
        m_mouseDotColor = moveCrossColor;
        addRipple(position , moveCrossColor , 0.3f , 18.f);
    }
    else
    {
        m_mouseDotColor = sf::Color::Cyan;
    }
}

void InputTestScene::handleClick(size_t initiateEntityId , size_t triggerEntityId , sf::Vector2f position , const sf::Color& color)
{
    if(initiateEntityId != triggerEntityId)
    {
        // pressed on one thing, released on another
        addRipple(position , mismatchColor , 0.4f , 30.f);
        return;
    }

    flash(triggerEntityId , color , 0.3f , 1);
    addRipple(position , color , 0.4f , 30.f);
}

void InputTestScene::handleDoubleClick(size_t initiateEntityId , size_t triggerEntityId , sf::Vector2f position)
{
    // initiate entity = under the first click, trigger entity = under the second click
    if(initiateEntityId == triggerEntityId && visualOf(triggerEntityId))
    {
        toggleSize(triggerEntityId);
        flash(triggerEntityId , doubleClickColor , 0.6f , 2);
    }
    addRipple(position , doubleClickColor , 0.7f , 60.f);
}

void InputTestScene::handleDrag(const RefinedAction& input)
{
    // NOTE: the current mouse position is read from the UISystem. Swap it for
    // input.getTriggerMousePos() if you want to test what the drag action reports.
    const sf::Vector2f mousePosition(m_uisystem.getMousePos());

    switch (input.getType())
    {
        case ActionType::Started:
        {
            const size_t initiateId = input.getInitiateEntityId();
            if(CTestVisual* visual = visualOf(initiateId))
            {
                m_draggedEntity = initiateId;
                visual->dragged = true;

                const sf::Vector2f entityPosition = m_entities.storage<CTransform>().getComponent(initiateId).position;
                m_dragGrabOffset = entityPosition - sf::Vector2f(input.getInitiateMousePos());

                m_entities.storage<CRender>().getComponent(initiateId).zIndex = visual->baseZIndex + 100;
            }
            else
            {
                m_rubberBandActive = true;
                m_rubberBandStart = sf::Vector2f(input.getInitiateMousePos());
            }
            break;
        }
        case ActionType::OnGoing:
        {
            if(m_draggedEntity != NoEntity)
                m_entities.storage<CTransform>().getComponent(m_draggedEntity).position = mousePosition + m_dragGrabOffset;
            break;
        }
        case ActionType::Ended:
        {
            if(m_draggedEntity != NoEntity)
            {
                m_entities.storage<CTransform>().getComponent(m_draggedEntity).position = mousePosition + m_dragGrabOffset;

                if(CTestVisual* visual = visualOf(m_draggedEntity))
                {
                    visual->dragged = false;
                    m_entities.storage<CRender>().getComponent(m_draggedEntity).zIndex = visual->baseZIndex;
                }
                m_draggedEntity = NoEntity;
            }
            m_rubberBandActive = false;
            break;
        }
        default:
            break;
    }
}

// -------------------------------------------------------------------
// Update / render
// -------------------------------------------------------------------

void InputTestScene::applyVisuals(float seconds)
{
    auto& visuals = m_entities.storage<CTestVisual>();
    auto& shapes = m_entities.storage<CDebugShape>();

    for(const auto& entity : m_entities.getEntities("Box"))
    {
        if(!entity->isAlive()) continue;
        const size_t id = entity->getId();

        CTestVisual& visual = visuals.getComponent(id);
        sf::RectangleShape& shape = shapes.getComponent(id).shapeRect;

        visual.flashTimeLeft = std::max(0.f , visual.flashTimeLeft - seconds);
        if(visual.flashTimeLeft == 0.f) visual.flashPriority = 0;

        sf::Color fill = visual.baseColor;
        if(visual.hovered)  fill = blend(fill , sf::Color::White , 0.35f);
        if(visual.dragged)  fill = dragColor;
        if(visual.flashTimeLeft > 0.f)
            fill = blend(fill , visual.flashColor , visual.flashTimeLeft / visual.flashDuration);

        shape.setFillColor(fill);
        // negative thickness draws the outline inside the shape, so it never changes the hitbox
        shape.setOutlineThickness(visual.hovered ? -4.f : -1.f);
        shape.setOutlineColor(visual.hovered ? sf::Color::White : sf::Color(0 , 0 , 0 , 150));
    }
}

void InputTestScene::sUpdate(sf::Time dt)
{
    m_entities.update();

    const float seconds = dt.asSeconds();

    for(auto& ripple : m_ripples)
        ripple.age += seconds;
    std::erase_if(m_ripples , [](const Ripple& r){ return r.age >= r.duration; });

    applyVisuals(seconds);
}

void InputTestScene::sRender(sf::RenderWindow& gameWindow)
{
    // The Engine already calls window.clear() / window.display() around sceneRender,
    // so they must not be called here.

    auto& transforms = m_entities.storage<CTransform>();
    auto& renders = m_entities.storage<CRender>();
    auto& shapes = m_entities.storage<CDebugShape>();

    std::vector<std::pair<int , sf::RectangleShape*>> drawList;
    for(const auto& entity : m_entities.getEntities("Box"))
    {
        if(!entity->isAlive()) continue;
        const size_t id = entity->getId();
        if(!renders.hasComponent(id) || !transforms.hasComponent(id) || !shapes.hasComponent(id)) continue;
        if(!renders.getComponent(id).render) continue;

        sf::RectangleShape& shape = shapes.getComponent(id).shapeRect;
        shape.setPosition(transforms.getComponent(id).position);
        drawList.emplace_back(renders.getComponent(id).zIndex , &shape);
    }
    // stable: equal z-indices keep creation order, so the drawing matches the "newest wins" hit-test
    std::stable_sort(drawList.begin() , drawList.end() , [](const auto& a , const auto& b){ return a.first < b.first; });
    for(auto& [zIndex , shape] : drawList)
        gameWindow.draw(*shape);

    if(m_rubberBandActive)
    {
        const sf::Vector2f current(m_uisystem.getMousePos());
        sf::RectangleShape band(current - m_rubberBandStart);   // negative sizes are fine for drawing
        band.setPosition(m_rubberBandStart);
        band.setFillColor(sf::Color(100 , 160 , 255 , 50));
        band.setOutlineThickness(1.f);
        band.setOutlineColor(sf::Color(100 , 160 , 255 , 220));
        gameWindow.draw(band);
    }

    for(const auto& ripple : m_ripples)
    {
        const float t = ripple.age / ripple.duration;
        const float radius = ripple.maxRadius * t;

        sf::Color color = ripple.color;
        color.a = static_cast<std::uint8_t>(255.f * (1.f - t));

        sf::CircleShape circle(radius);
        circle.setOrigin({radius , radius});
        circle.setPosition(ripple.position);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(3.f);
        circle.setOutlineColor(color);
        gameWindow.draw(circle);
    }

    if(m_hasMouseDot)
    {
        sf::CircleShape dot(5.f);
        dot.setOrigin({5.f , 5.f});
        dot.setPosition(m_mouseDotPosition);
        dot.setFillColor(m_mouseDotColor);
        gameWindow.draw(dot);
    }
}