#pragma once
#include"CGameEngine.hpp"
#include<optional>
#include<random>
#include<string>
#include<vector>

// -------------------------------------------------------------------
// ORB POP
//   Orbs rise from behind the hills. Click them before they escape.
//     normal orb : +10 points (times the combo multiplier)
//     golden orb : +50 points and +3 seconds
//     bomb       : -4 seconds, breaks the combo, shakes the screen
//   R restarts, Escape leaves the scene.
// -------------------------------------------------------------------

enum class OrbKind { Normal , Golden , Bomb };

// Component stored per orb entity (the engine's CTransform / CMouseInteraction
// hold the top-left corner and the hit box, this holds everything else).
struct COrb
{
    OrbKind kind = OrbKind::Normal;
    float radius = 40.f;
    sf::Color color = sf::Color::White;
    sf::Vector2f center = {0.f , 0.f};
    float baseX = 0.f;
    float speed = 100.f;
    float swayAmp = 0.f;
    float swayFreq = 1.f;
    float phase = 0.f;
    float age = 0.f;
    float hover = 0.f;        // smoothed 0..1, drives the glow / pop-up scale
    bool hovered = false;     // set by the MouseOver action
    bool popped = false;
};

struct Star
{
    sf::Vector2f pos;         // normalised 0..1
    float size;
    float phase;
    float speed;
    float depth;              // 0 far .. 1 near (parallax + brightness)
};

struct ShootingStar
{
    sf::Vector2f pos;
    sf::Vector2f dir;
    float age = 0.f;
    float life = 1.f;
    float length = 200.f;
};

struct Particle
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color color;
    float life = 1.f;
    float maxLife = 1.f;
    float size = 4.f;
    float gravity = 0.f;
};

struct Ring
{
    sf::Vector2f pos;
    sf::Color color;
    float age;
    float duration;
    float maxRadius;
    float thickness;
};

struct Popup
{
    sf::Vector2f pos;
    std::string text;
    sf::Color color;
    float age;
    float duration;
    float size;
};

class OrbPop : public Scene
{
    public:
        void sDoAction(RefinedAction& input) override ;
        void sUpdate(sf::Time dt) override ;
        void sRender(sf::RenderWindow& gameWindow) override ;
        void onLoad(const std::optional<std::vector<Asset>>& assets) override ;
        void onDelete() override ;
        bool renderBelow() override ;
        bool updateBelow() override ;

    private:
        enum class State { Playing , GameOver };
        enum class TextAlign { Left , Center , Right };

        // ---- game state ----
        State m_state = State::Playing;
        int m_score = 0;
        int m_best = 0;
        int m_combo = 0;
        bool m_newBest = false;
        float m_timeLeft = 45.f;
        float m_elapsed = 0.f;       // seconds played this round
        float m_overTime = 0.f;      // seconds since the round ended
        float m_spawnTimer = 0.6f;
        float m_time = 0.f;          // scene clock, drives all ambient animation

        // ---- juice ----
        float m_shake = 0.f;
        float m_flash = 0.f;
        sf::Color m_flashColor = sf::Color::White;
        float m_comboPulse = 0.f;
        float m_shootTimer = 2.f;
        sf::Vector2f m_mouse = {0.f , 0.f};
        bool m_hasMouse = false;

        std::vector<Star> m_stars;
        std::vector<ShootingStar> m_shootingStars;
        std::vector<Particle> m_particles;
        std::vector<Ring> m_rings;
        std::vector<Popup> m_popups;

        // ---- window info (cached from sRender) ----
        sf::Vector2f m_size = {1920.f , 1080.f};
        float m_scale = 1.f;

        // ---- rendering helpers ----
        std::optional<sf::Font> m_font;
        sf::CircleShape m_circle;
        sf::RectangleShape m_rect;
        std::vector<sf::Vertex> m_vertices;
        sf::RenderStates m_normal;
        sf::RenderStates m_additive;

        std::mt19937 m_rng{std::random_device{}()};
        float rnd(float lo , float hi);

        // ---- logic ----
        void resetRound();
        void endRound();
        void spawnOrb();
        void popOrb(size_t entityId);
        int multiplier() const;
        void handleClick(sf::Vector2f position);
        void handleMouseOver(ActionType type , size_t entityId);
        void handleMouseMoved(sf::Vector2f position);

        // ---- effects ----
        void burst(sf::Vector2f pos , const sf::Color& color , int count , float speedMin , float speedMax , float size);
        void addRing(sf::Vector2f pos , const sf::Color& color , float duration , float maxRadius , float thickness);
        void addPopup(sf::Vector2f pos , const std::string& text , const sf::Color& color , float size);

        // ---- update / draw pieces ----
        void updateOrbs(float dt);
        void updateEffects(float dt);
        void loadFont();
        void drawBackground(sf::RenderWindow& w);
        void drawHills(sf::RenderWindow& w , float baseY , float amp , float freq , float phase , const sf::Color& top , const sf::Color& bottom);
        void drawOrb(sf::RenderWindow& w , const COrb& orb);
        void drawEffects(sf::RenderWindow& w);
        void drawHud(sf::RenderWindow& w);
        void drawGameOver(sf::RenderWindow& w);

        void drawCircle(sf::RenderWindow& w , sf::Vector2f center , float radius , const sf::Color& fill , bool additive = false , float outline = 0.f , const sf::Color& outlineColor = sf::Color::Transparent);
        void drawRect(sf::RenderWindow& w , sf::Vector2f center , sf::Vector2f size , float rotationDeg , const sf::Color& color , bool additive = false);
        void drawText(sf::RenderWindow& w , const std::string& str , float size , sf::Vector2f pos , const sf::Color& color , TextAlign align = TextAlign::Center , float scale = 1.f);
};