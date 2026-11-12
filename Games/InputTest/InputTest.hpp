#pragma once
#include"CGameEngine.hpp"

// Per-entity visual state used only by this test scene
struct CTestVisual
{
    sf::Color baseColor;
    sf::Vector2f baseSize;
    int baseZIndex;

    bool hovered  = false;
    bool dragged  = false;
    bool enlarged = false;

    float flashTimeLeft = 0.f;
    float flashDuration = 0.f;
    int   flashPriority = 0;
    sf::Color flashColor = sf::Color::White;

    CTestVisual(sf::Color color , sf::Vector2f size , int zIndex) : baseColor(color) , baseSize(size) , baseZIndex(zIndex){};
};

// Expanding ring that shows where an action happened (works on empty space too)
struct Ripple
{
    sf::Vector2f position;
    sf::Color color;
    float duration;
    float maxRadius;
    float age = 0.f;
};

class InputTestScene : public Scene
{
    public:
        void sDoAction(RefinedAction& input) override ;
        void sUpdate(sf::Time dt) override ;
        void sRender(sf::RenderWindow& gameWindow) override ;
        void onLoad() override ;
        void onDelete() override ;
        bool renderBelow() override ;
        bool updateBelow() override ;

    private:
        // drag state
        size_t m_draggedEntity = NoEntity;
        sf::Vector2f m_dragGrabOffset = {0.f , 0.f};
        bool m_rubberBandActive = false;
        sf::Vector2f m_rubberBandStart = {0.f , 0.f};

        // MouseMoved dot
        bool m_hasMouseDot = false;
        sf::Vector2f m_mouseDotPosition = {0.f , 0.f};
        sf::Color m_mouseDotColor = sf::Color::Cyan;

        std::vector<Ripple> m_ripples;

        // action handlers
        void handleMouseOver(ActionType type , size_t entityId);
        void handleMouseMoved(size_t initiateEntityId , size_t triggerEntityId , sf::Vector2f position);
        void handleClick(size_t initiateEntityId , size_t triggerEntityId , sf::Vector2f position , const sf::Color& color);
        void handleDoubleClick(size_t initiateEntityId , size_t triggerEntityId , sf::Vector2f position);
        void handleDrag(const RefinedAction& input);

        // helpers
        void spawnBox(sf::Color color , sf::Vector2f position , sf::Vector2f size , int zIndex);
        CTestVisual* visualOf(size_t entityId);
        void flash(size_t entityId , const sf::Color& color , float duration , int priority);
        void toggleSize(size_t entityId);
        void addRipple(sf::Vector2f position , const sf::Color& color , float duration , float maxRadius);
        void applyVisuals(float seconds);
};