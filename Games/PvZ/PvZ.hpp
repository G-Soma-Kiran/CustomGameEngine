#pragma once
#include "CGameEngine.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <random>
#include <utility>
#include <vector>

// ============================================================================
//  Rain   (unchanged)
// ============================================================================

struct RainConfig
{
    std::size_t count        = 260;
    float baseSlope          = 0.15f;   // horizontal px per vertical px
    float gustStrength       = 1.f;     // 0 = steady wind
    float fallSpeedFar       = 500.f;
    float fallSpeedNear      = 1100.f;
    float lengthFactor       = 0.04f;
    float widthFar           = 1.2f;
    float widthNear          = 2.8f;
    float alphaFar           = 70.f;
    float alphaNear          = 200.f;
    float groundDepthRange   = 150.f;
    float splashChance       = 0.45f;
    sf::Color color          { 200, 215, 240 };
};

class Rain : public sf::Drawable
{
public:
    Rain(sf::Vector2f screenSize, RainConfig config = {})
        : m_cfg(config),
          m_screenSize(screenSize),
          m_groundY(screenSize.y),
          m_particles(config.count),
          m_vertices(sf::PrimitiveType::Triangles, config.count * 3),
          m_splashVertices(sf::PrimitiveType::Lines),
          m_rng(std::random_device{}())
    {
        for (auto& p : m_particles)
            respawn(p, true);
    }

    void update(sf::Time dt)
    {
        const float d = std::min(dt.asSeconds(), 1.f / 20.f);
        m_time += d;

        updateRain(d, currentSlope());
        updateSplashes(d);
    }

private:
    struct Particle
    {
        sf::Vector2f position;
        float depth       = 0.f;
        float fallSpeed   = 0.f;
        float length      = 0.f;
        float halfWidth   = 0.f;
        float opacity     = 0.f;
        float slopeJitter = 0.f;
        float groundY     = 0.f;
    };

    struct Splash
    {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float age   = 0.f;
        float life  = 0.f;
        float alpha = 0.f;
    };

    RainConfig m_cfg;
    sf::Vector2f m_screenSize;
    float m_groundY;
    float m_time = 0.f;

    std::vector<Particle> m_particles;
    std::vector<Splash> m_splashes;
    sf::VertexArray m_vertices;
    sf::VertexArray m_splashVertices;

    std::mt19937 m_rng;

    float randomFloat(float min, float max)
    {
        return std::uniform_real_distribution<float>(min, max)(m_rng);
    }

    static float lerp(float a, float b, float t) { return a + (b - a) * t; }

    float currentSlope() const
    {
        const float gust =
            0.07f * std::sin(m_time * 0.45f) +
            0.04f * std::sin(m_time * 1.30f + 2.0f) +
            0.02f * std::sin(m_time * 3.10f + 0.7f);

        return m_cfg.baseSlope + gust * m_cfg.gustStrength;
    }

    float maxDrift() const
    {
        const float maxSlope =
            std::abs(m_cfg.baseSlope) + 0.13f * m_cfg.gustStrength + 0.03f;
        return maxSlope * m_groundY;
    }

    void respawn(Particle& p, bool initial)
    {
        const float r = randomFloat(0.f, 1.f);
        p.depth = r * r;

        p.fallSpeed = lerp(m_cfg.fallSpeedFar, m_cfg.fallSpeedNear, p.depth)
                    * randomFloat(0.92f, 1.08f);

        p.length    = p.fallSpeed * m_cfg.lengthFactor * randomFloat(0.8f, 1.25f);
        p.halfWidth = lerp(m_cfg.widthFar, m_cfg.widthNear, p.depth) * 0.5f;
        p.opacity   = lerp(m_cfg.alphaFar, m_cfg.alphaNear, p.depth)
                    * randomFloat(0.75f, 1.f);

        p.slopeJitter = randomFloat(-0.03f, 0.03f);
        p.groundY     = m_groundY - (1.f - p.depth) * randomFloat(0.f, m_cfg.groundDepthRange);

        const float drift  = maxDrift();
        const float margin = 40.f;
        const bool windRight = m_cfg.baseSlope >= 0.f;

        const float minX = windRight ? -drift - margin : -margin;
        const float maxX = windRight ? m_screenSize.x + margin
                                     : m_screenSize.x + drift + margin;

        p.position.x = randomFloat(minX, maxX);
        p.position.y = -p.length;

        if (initial)
        {
            p.position.y = randomFloat(-p.length, p.groundY);
            p.position.x += (currentSlope() + p.slopeJitter) * p.position.y;
        }
    }

    void updateRain(float d, float slope)
    {
        const float margin = 60.f;

        for (std::size_t i = 0; i < m_particles.size(); ++i)
        {
            Particle& p = m_particles[i];

            sf::Vector2f velocity{ p.fallSpeed * (slope + p.slopeJitter), p.fallSpeed };
            p.position += velocity * d;

            if (p.position.y >= p.groundY)
            {
                if (p.depth > 0.4f && randomFloat(0.f, 1.f) < m_cfg.splashChance)
                    spawnSplash(p);

                respawn(p, false);
                velocity = { p.fallSpeed * (slope + p.slopeJitter), p.fallSpeed };
            }
            else if (p.position.x > m_screenSize.x + margin ||
                     p.position.x < -margin - maxDrift())
            {
                respawn(p, false);
                velocity = { p.fallSpeed * (slope + p.slopeJitter), p.fallSpeed };
            }

            const sf::Vector2f dir  = velocity.normalized();
            const sf::Vector2f side = dir.perpendicular() * p.halfWidth;
            const sf::Vector2f head = p.position;
            const sf::Vector2f tail = head - dir * p.length;

            const auto headAlpha = static_cast<std::uint8_t>(p.opacity);
            const auto tailAlpha = static_cast<std::uint8_t>(p.opacity * 0.3f);
            const sf::Color headColor(m_cfg.color.r, m_cfg.color.g, m_cfg.color.b, headAlpha);
            const sf::Color tailColor(m_cfg.color.r, m_cfg.color.g, m_cfg.color.b, tailAlpha);

            sf::Vertex* v = &m_vertices[i * 3];
            v[0] = { head + side, headColor };
            v[1] = { head - side, headColor };
            v[2] = { tail,        tailColor };
        }
    }

    void spawnSplash(const Particle& p)
    {
        if (m_splashes.size() > 600)
            return;

        const int n = 2 + static_cast<int>(randomFloat(0.f, 2.99f));
        for (int k = 0; k < n; ++k)
        {
            const float angle = randomFloat(-2.5f, -0.64f);
            const float speed = randomFloat(35.f, 95.f) * (0.5f + p.depth);

            Splash s;
            s.position = { p.position.x, p.groundY };
            s.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };
            s.life     = randomFloat(0.12f, 0.30f);
            s.alpha    = p.opacity * 0.9f;
            m_splashes.push_back(s);
        }
    }

    void updateSplashes(float d)
    {
        constexpr float gravity = 450.f;

        m_splashVertices.clear();

        for (std::size_t i = 0; i < m_splashes.size();)
        {
            Splash& s = m_splashes[i];

            s.age += d;
            if (s.age >= s.life)
            {
                s = m_splashes.back();
                m_splashes.pop_back();
                continue;
            }

            s.velocity.y += gravity * d;
            s.position   += s.velocity * d;

            const float t = 1.f - s.age / s.life;
            const auto a  = static_cast<std::uint8_t>(s.alpha * t);
            const sf::Color c(m_cfg.color.r, m_cfg.color.g, m_cfg.color.b, a);

            m_splashVertices.append({ s.position, c });
            m_splashVertices.append({ s.position - s.velocity * 0.025f, c });

            ++i;
        }
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(m_vertices, states);
        target.draw(m_splashVertices, states);
    }
};

// ============================================================================
//  Lightning   (unchanged: timer only, the shader does the visuals)
// ============================================================================

class Lightning
{
public:
    Lightning() : m_rng(std::random_device{}()) { m_interval = rnd(4.f, 8.f); }

    void update(float dt)
    {
        m_clock += dt;
        if (m_clock >= m_interval)
        {
            m_clock    = 0.f;
            m_interval = rnd(7.f, 18.f);
            m_pulses.clear();

            float start = 0.f;
            const int n = 2 + static_cast<int>(rnd(0.f, 2.99f));
            for (int i = 0; i < n; ++i)
            {
                m_pulses.push_back({ start, i == 0 ? 1.f : rnd(0.4f, 0.9f) });
                start += rnd(0.06f, 0.18f);
            }
            m_struck = true;   // hook thunder sound here (play after a 0.5-2s delay)
        }
    }

    float flash() const
    {
        float f = 0.f;
        for (const auto& p : m_pulses)
        {
            const float x = m_clock - p.start;
            if (x >= 0.f)
                f = std::max(f, p.peak * std::exp(-x * 9.f));
        }
        return f;
    }

    bool takeStrike() { return std::exchange(m_struck, false); }

private:
    struct Pulse { float start, peak; };

    std::vector<Pulse> m_pulses;
    float m_clock = 0.f, m_interval = 5.f;
    bool  m_struck = false;
    std::mt19937 m_rng;

    float rnd(float a, float b) { return std::uniform_real_distribution<float>(a, b)(m_rng); }
};

// ============================================================================
//  Leaves   (unchanged; image-space coordinates, draw with image->scene transform)
// ============================================================================

class Leaves : public sf::Drawable
{
public:
    Leaves(sf::FloatRect spawnArea, sf::Vector2f sceneSize, std::size_t count = 26)
        : m_area(spawnArea), m_size(sceneSize), m_leaves(count), m_rng(std::random_device{}())
    {
        m_shape.setPointCount(6);
        m_shape.setPoint(0, { 0.f, -7.f});
        m_shape.setPoint(1, { 3.5f, -3.f});
        m_shape.setPoint(2, { 3.f,  3.f});
        m_shape.setPoint(3, { 0.f,  7.f});
        m_shape.setPoint(4, {-3.f,  3.f});
        m_shape.setPoint(5, {-3.5f,-3.f});

        for (auto& l : m_leaves) respawn(l, true);
    }

    void update(float dt, float wind = 1.f)
    {
        for (auto& l : m_leaves)
        {
            l.phase += l.swaySpeed * dt;
            l.flip  += l.flipSpeed * dt;
            l.rot   += l.spin * dt;

            l.pos.x += (std::sin(l.phase) * l.swayAmp + wind * 22.f * l.scale) * dt;
            l.pos.y += l.fall * dt;

            if (l.pos.y > l.landY || l.pos.x > m_size.x + 20.f)
                respawn(l, false);
        }
    }

private:
    struct Leaf
    {
        sf::Vector2f pos;
        float phase = 0.f, swaySpeed = 0.f, swayAmp = 0.f;
        float fall = 0.f, rot = 0.f, spin = 0.f, flip = 0.f, flipSpeed = 0.f;
        float scale = 1.f, landY = 0.f;
        sf::Color color;
    };

    sf::FloatRect m_area;
    sf::Vector2f  m_size;
    std::vector<Leaf> m_leaves;
    mutable sf::ConvexShape m_shape;
    std::mt19937 m_rng;

    float rnd(float a, float b) { return std::uniform_real_distribution<float>(a, b)(m_rng); }

    void respawn(Leaf& l, bool initial)
    {
        l.scale     = rnd(0.7f, 1.5f);
        l.fall      = rnd(25.f, 55.f) * (0.6f + 0.4f * l.scale);
        l.swayAmp   = rnd(15.f, 40.f);
        l.swaySpeed = rnd(1.f, 2.2f);
        l.phase     = rnd(0.f, 6.28f);
        l.rot       = rnd(0.f, 360.f);
        l.spin      = rnd(-70.f, 70.f);
        l.flip      = rnd(0.f, 6.28f);
        l.flipSpeed = rnd(1.5f, 4.f);
        l.landY     = rnd(m_size.y * 0.78f, m_size.y * 0.97f);

        l.pos.x = m_area.position.x + rnd(0.f, m_area.size.x);
        l.pos.y = initial ? rnd(m_area.position.y, l.landY)
                          : m_area.position.y + rnd(0.f, m_area.size.y);

        l.color = rnd(0.f, 1.f) < 0.8f ? sf::Color(45, 80, 50) : sf::Color(105, 80, 45);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        for (const auto& l : m_leaves)
        {
            const float fade = std::clamp((l.landY - l.pos.y) / 40.f, 0.f, 1.f);
            sf::Color c = l.color;
            c.a = static_cast<std::uint8_t>(230.f * fade);

            m_shape.setFillColor(c);
            m_shape.setPosition(l.pos);
            m_shape.setRotation(sf::degrees(l.rot));
            m_shape.setScale({ l.scale * (0.25f + 0.75f * std::abs(std::cos(l.flip))), l.scale });
            target.draw(m_shape, states);
        }
    }
};

// ============================================================================
//  Actor
//  A cut-out sprite drawn as a deformable mesh. The pivot (feet / base) stays
//  planted; everything above it can lean, squash, tilt and ripple. All values
//  are in texture pixels, so draw it with the image->scene transform.
// ============================================================================

class Actor : public sf::Drawable
{
public:
    struct Pose
    {
        sf::Vector2f offset { 0.f, 0.f };
        float shear      = 0.f;   // px the top of the actor is pushed sideways (+ = right)
        float scaleX     = 1.f;   // scale about the pivot
        float scaleY     = 1.f;
        float rotation   = 0.f;   // degrees about the pivot
        float ripple     = 0.f;   // px of soft cloth / leaf wobble
        float rippleBase = 0.f;   // 0 = ripple only on the upper body, 1 = everywhere
        float phase      = 0.f;   // time feeding the ripple
        float flash      = 0.f;   // 0..1 additive white hit-flash
    };

    // bounds: rectangle of the texture that contains the actor (with a few px margin)
    // pivot : point on the ground the actor stands on, in texture pixels
    Actor(const sf::Texture& texture, sf::IntRect bounds, sf::Vector2f pivot,
          sf::Color tint = sf::Color::White)
        : m_texture(&texture),
          m_bounds(bounds),
          m_pivot(pivot),
          m_tint(tint),
          m_vertices(sf::PrimitiveType::Triangles),
          m_flashVertices(sf::PrimitiveType::Triangles)
    {
        m_height = std::max(1.f, pivot.y - static_cast<float>(bounds.position.y));
        setPose(Pose{});
    }

    void setPose(const Pose& pose)
    {
        m_pose = pose;
        m_vertices.clear();
        m_flashVertices.clear();

        const float x0 = static_cast<float>(m_bounds.position.x);
        const float y0 = static_cast<float>(m_bounds.position.y);
        const float w  = static_cast<float>(m_bounds.size.x);
        const float h  = static_cast<float>(m_bounds.size.y);

        const bool flashing = pose.flash > 0.002f;
        const sf::Color flashColor(255, 255, 255,
            static_cast<std::uint8_t>(std::clamp(pose.flash, 0.f, 1.f) * 255.f));

        auto add = [&](sf::Vector2f p)
        {
            m_vertices.append(sf::Vertex{ deform(p), m_tint, p });
            if (flashing)
                m_flashVertices.append(sf::Vertex{ deform(p), flashColor, p });
        };

        for (int r = 0; r < kRows; ++r)
        {
            const float va = y0 + h * static_cast<float>(r)     / kRows;
            const float vb = y0 + h * static_cast<float>(r + 1) / kRows;

            for (int c = 0; c < kCols; ++c)
            {
                const float ua = x0 + w * static_cast<float>(c)     / kCols;
                const float ub = x0 + w * static_cast<float>(c + 1) / kCols;

                add({ ua, va }); add({ ub, va }); add({ ua, vb });
                add({ ub, va }); add({ ub, vb }); add({ ua, vb });
            }
        }
    }

    // Where a texture-space point ends up with the current pose (muzzle, face, ...).
    sf::Vector2f deform(sf::Vector2f p) const
    {
        sf::Vector2f rel = p - m_pivot;

        const float k  = std::clamp(-rel.y / m_height, 0.f, 1.f);   // 0 at the pivot, 1 at the top
        const float w  = k * k;                                      // head moves most
        const float rw = m_pose.rippleBase + (1.f - m_pose.rippleBase) * k;

        rel.x = rel.x * m_pose.scaleX + m_pose.shear * w;
        rel.y = rel.y * m_pose.scaleY;

        rel.y += m_pose.ripple * rw        * std::sin(m_pose.phase * 2.3f + p.x * 0.06f);
        rel.x += m_pose.ripple * rw * 0.5f * std::sin(m_pose.phase * 1.7f + p.y * 0.05f);

        rel = rel.rotatedBy(sf::degrees(m_pose.rotation));

        return m_pivot + rel + m_pose.offset;
    }

private:
    static constexpr int kCols = 6;
    static constexpr int kRows = 24;

    const sf::Texture* m_texture;
    sf::IntRect   m_bounds;
    sf::Vector2f  m_pivot;
    sf::Color     m_tint;
    float         m_height = 1.f;
    Pose          m_pose;
    sf::VertexArray m_vertices;
    sf::VertexArray m_flashVertices;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.texture = m_texture;          // keeps the caller's shader and transform
        target.draw(m_vertices, states);

        if (m_pose.flash > 0.002f)
        {
            states.blendMode = sf::BlendAdd;
            target.draw(m_flashVertices, states);
        }
    }
};

// ============================================================================
//  Splat: little green droplets when the pea hits (image-space)
// ============================================================================

class Splat : public sf::Drawable
{
public:
    Splat() : m_rng(std::random_device{}()) {}

    void emit(sf::Vector2f pos, int count = 18)
    {
        for (int i = 0; i < count; ++i)
        {
            const float angle = rnd(2.6f, 5.0f);           // mostly up and away to the left
            const float speed = rnd(80.f, 260.f);

            Drop d;
            d.pos    = pos;
            d.vel    = { std::cos(angle) * speed, std::sin(angle) * speed };
            d.life   = rnd(0.35f, 0.7f);
            d.radius = rnd(1.8f, 4.5f);
            m_drops.push_back(d);
        }
    }

    void update(float dt)
    {
        for (std::size_t i = 0; i < m_drops.size();)
        {
            Drop& d = m_drops[i];
            d.age += dt;
            if (d.age >= d.life)
            {
                d = m_drops.back();
                m_drops.pop_back();
                continue;
            }

            d.vel.y += 700.f * dt;
            d.pos   += d.vel * dt;
            ++i;
        }
    }

private:
    struct Drop
    {
        sf::Vector2f pos, vel;
        float age = 0.f, life = 0.f, radius = 0.f;
    };

    std::vector<Drop> m_drops;
    mutable sf::CircleShape m_shape;
    std::mt19937 m_rng;

    float rnd(float a, float b) { return std::uniform_real_distribution<float>(a, b)(m_rng); }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        for (const auto& d : m_drops)
        {
            const float t = 1.f - d.age / d.life;
            m_shape.setRadius(d.radius);
            m_shape.setOrigin({ d.radius, d.radius });
            m_shape.setPosition(d.pos);
            m_shape.setFillColor(sf::Color(130, 205, 70, static_cast<std::uint8_t>(230.f * t)));
            target.draw(m_shape, states);
        }
    }
};

// ============================================================================
//  PvZ scene
// ============================================================================

class PvZ : public Scene
{
    private:
        struct Light
        {
            sf::Vector2f pos;      // image-space
            float        radius;
            sf::Color    color;
        };

        static constexpr float kSceneW = 1920.f;
        static constexpr float kSceneH = 1080.f;

        // ---- "Play" cinematic timeline (seconds after the click) ----------
        static constexpr float kUiFadeEnd   = 0.45f;   // menu UI finishes fading out
        static constexpr float kWindupStart = 0.30f;   // peashooter starts leaning back
        static constexpr float kFire        = 0.80f;   // pea leaves the mouth
        static constexpr float kHit         = 1.15f;   // pea reaches the zombie
        static constexpr float kFadeOutBeg  = 2.00f;   // screen starts fading to black
        static constexpr float kEnd         = 2.55f;   // switch to gameplay

        Rain      m_rain = Rain(sf::Vector2f{ kSceneW, kSceneH });
        Lightning m_lightning;

        std::optional<sf::Sprite> m_background;
        std::optional<Actor>      m_zombie;
        std::optional<Actor>      m_peashooter;
        std::optional<Leaves>     m_leaves;
        Splat                     m_splat;

        sf::Shader      m_shader;
        bool            m_shaderOk = false;

        sf::Shader m_actorShader;
        bool       m_actorShaderOk = false;

        sf::Texture               m_glowTexture;
        std::optional<sf::Sprite> m_glow;
        std::vector<Light>        m_lights;

        sf::Transform m_imageToScene;   // image pixels -> 1920x1080 scene
        float         m_time = 0.f;

        // ---- play sequence state ------------------------------------------
        bool          m_playing  = false;
        bool          m_fired    = false;
        bool          m_hitDone  = false;
        bool          m_finished = false;
        float         m_playTime = 0.f;
        sf::Vector2f  m_muzzlePos;      // frozen at the moment of firing (image-space)
        sf::Vector2f  m_targetPos;

        static sf::Texture makeGlowTexture(unsigned size = 64);

        Actor::Pose zombiePose() const;
        Actor::Pose peashooterPose() const;
        sf::Vector2f shakeOffset() const;
        void updatePlaySequence();
        void onPlayFinished();
        void drawPea(sf::RenderWindow& window);

    public:
        // Call this from your Play button later (or press Enter for testing).
        void startPlaySequence();

        void sDoAction(RefinedAction& input) override ;
        void sUpdate(sf::Time dt) override ;
        void sRender(sf::RenderWindow& gameWindow)  override; 
        void onLoad() override ;
        void onDelete()  override; 
        bool renderBelow() override ;
        bool updateBelow() override ;
};