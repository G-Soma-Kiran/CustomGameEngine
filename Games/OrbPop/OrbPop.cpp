#include"OrbPop.hpp"
#include<algorithm>
#include<cmath>
#include<cstdint>
#include<iostream>

namespace
{
    constexpr float kPi = 3.14159265f;
    constexpr float kRoundTime = 45.f;
    constexpr float kMaxTime = 60.f;

    float mixf(float a , float b , float t) { return a + (b - a) * t; }
    float clamp01(float t) { return std::clamp(t , 0.f , 1.f); }
    std::uint8_t toByte(float v) { return static_cast<std::uint8_t>(std::clamp(v , 0.f , 255.f)); }

    sf::Color mix(const sf::Color& a , const sf::Color& b , float t)
    {
        t = clamp01(t);
        return sf::Color(toByte(mixf(a.r , b.r , t)) , toByte(mixf(a.g , b.g , t)) ,
                         toByte(mixf(a.b , b.b , t)) , toByte(mixf(a.a , b.a , t)));
    }

    sf::Color withAlpha(sf::Color c , float alpha)
    {
        c.a = toByte(alpha);
        return c;
    }

    float easeOutCubic(float t)
    {
        const float u = 1.f - clamp01(t);
        return 1.f - u * u * u;
    }

    const sf::Color kOrbColors[] =
    {
        sf::Color( 70 , 220 , 255),   // cyan
        sf::Color(255 ,  90 , 170),   // pink
        sf::Color(150 , 255 , 110),   // lime
        sf::Color(170 , 120 , 255),   // violet
        sf::Color(255 , 160 ,  70),   // orange
    };
    const sf::Color kGold(255 , 214 , 70);
    const sf::Color kBombBody(34 , 24 , 48);
    const sf::Color kBombGlow(255 , 50 , 70);
}

bool OrbPop::renderBelow() { return false; }
bool OrbPop::updateBelow() { return false; }
void OrbPop::onDelete() {}

float OrbPop::rnd(float lo , float hi)
{
    return std::uniform_real_distribution<float>(lo , hi)(m_rng);
}

// -------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------

void OrbPop::onLoad()
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
        // Plain button press, not the Click gesture: Click is rejected when the mouse moves
        // more than 5 px between press and release, which happens all the time with moving targets.
        ActionBinding& binding = this->registerActionBinding("LeftDown");
        binding.addCondition<MouseKeyCondition>(sf::Mouse::Button::Left);
    }
    {
        ActionBinding& binding = this->registerActionBinding("Restart");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::R);
        binding.addCondition<ModifierCondition>(false , false , false);
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

    m_additive.blendMode = sf::BlendAdd;
    m_circle.setPointCount(44);
    loadFont();

    for(int i = 0 ; i < 220 ; i++)
    {
        Star star;
        star.pos = {rnd(0.f , 1.f) , rnd(0.f , 1.f)};
        star.depth = rnd(0.f , 1.f);
        star.size = mixf(0.8f , 2.4f , star.depth);
        star.phase = rnd(0.f , 2.f * kPi);
        star.speed = rnd(0.8f , 3.2f);
        m_stars.push_back(star);
    }

    resetRound();
}

void OrbPop::loadFont()
{
    // The first one that exists wins. Drop your own font at Assets/Fonts/Game.ttf to override.
    const char* candidates[] =
    {
        "Assets/Fonts/Game.ttf",
        "C:/Windows/Fonts/segoeuib.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    };
    for(const char* path : candidates)
    {
        sf::Font font;
        if(font.openFromFile(path))
        {
            m_font = std::move(font);
            return;
        }
    }
    std::cerr << "OrbPop: no font found, the HUD will show only the timer bar.\n";
}

void OrbPop::resetRound()
{
    for(const auto& e : m_entities.getEntities("Orb"))
        e->destroy();

    m_state = State::Playing;
    m_score = 0;
    m_combo = 0;
    m_newBest = false;
    m_timeLeft = kRoundTime;
    m_elapsed = 0.f;
    m_overTime = 0.f;
    m_spawnTimer = 0.6f;
    m_shake = 0.f;
    m_flash = 0.f;
    m_comboPulse = 0.f;
    m_particles.clear();
    m_rings.clear();
    m_popups.clear();
}

void OrbPop::endRound()
{
    m_state = State::GameOver;
    m_timeLeft = 0.f;
    m_overTime = 0.f;
    m_combo = 0;
    if(m_score > m_best)
    {
        m_best = m_score;
        m_newBest = true;
    }
}

// -------------------------------------------------------------------
// Actions
// -------------------------------------------------------------------

void OrbPop::sDoAction(RefinedAction& input)
{
    const std::string& name = input.getName();
    const ActionType type = input.getType();

    if(name == "Close" && type == ActionType::Started)
    {
        m_requests.push_back(Request(RequestCommand::Pop , "OrbPop"));
    }
    else if(name == "Restart" && type == ActionType::Started)
    {
        resetRound();
    }
    else if(name == "MouseOver")
    {
        handleMouseOver(type , input.getTriggerEntityId());
    }
    else if(name == "MouseMoved")
    {
        handleMouseMoved(sf::Vector2f(input.getTriggerMousePos()));
    }
    else if(name == "LeftDown" && type == ActionType::Started)
    {
        handleClick(sf::Vector2f(input.getTriggerMousePos()));
    }
    else if(name == "NextScene" && type == ActionType::Started)
    {
        m_requests.push_back(Request(RequestCommand::Push , "PvZ"));
    }
    else if(name == "PreviousScene" && type == ActionType::Started)
    {
        m_requests.push_back(Request(RequestCommand::Pop , "OrbPop"));
    }
}

void OrbPop::handleMouseOver(ActionType type , size_t entityId)
{
    // Ended / Cancelled carry the entity that was left, Started the one that was entered.
    auto& orbs = m_entities.storage<COrb>();
    if(!orbs.hasComponent(entityId)) return;

    COrb& orb = orbs.getComponent(entityId);
    if(type == ActionType::Started)
        orb.hovered = true;
    else if(type == ActionType::Ended || type == ActionType::Cancelled)
        orb.hovered = false;
}

void OrbPop::handleMouseMoved(sf::Vector2f position)
{
    m_mouse = position;
    m_hasMouse = true;

    Particle p;
    p.pos = position;
    p.vel = {rnd(-25.f , 25.f) * m_scale , rnd(-25.f , 25.f) * m_scale};
    p.color = mix(sf::Color(120 , 220 , 255) , sf::Color::White , rnd(0.f , 1.f));
    p.life = p.maxLife = rnd(0.25f , 0.45f);
    p.size = rnd(2.f , 4.f) * m_scale;
    p.gravity = 0.f;
    m_particles.push_back(p);
}

void OrbPop::handleClick(sf::Vector2f position)
{
    if(m_state == State::GameOver)
    {
        if(m_overTime > 0.7f) resetRound();
        return;
    }

    // Don't trust the engine's top-most entity: its hit box is a square, so at overlaps it can
    // name a neighbour whose corner covers the cursor. Test the real circles and take the orb
    // the cursor is most clearly inside (smallest distance relative to its radius).
    auto& orbs = m_entities.storage<COrb>();
    size_t best = NoEntity;
    float bestRatio = 1.25f;                // a little forgiveness: orbs move between frames
    for(const auto& e : m_entities.getEntities("Orb"))
    {
        if(!e->isAlive()) continue;
        const size_t id = e->getId();
        if(!orbs.hasComponent(id)) continue;

        const COrb& orb = orbs.getComponent(id);
        if(orb.popped) continue;

        const sf::Vector2f diff = position - orb.center;
        const float ratio = std::hypot(diff.x , diff.y) / orb.radius;
        if(ratio <= bestRatio)
        {
            bestRatio = ratio;
            best = id;
        }
    }

    if(best != NoEntity)
        popOrb(best);
    else
        addRing(position , sf::Color(200 , 200 , 255) , 0.3f , 26.f * m_scale , 2.f);
}

int OrbPop::multiplier() const
{
    return 1 + std::min(m_combo / 5 , 4);
}

void OrbPop::popOrb(size_t id)
{
    COrb& orb = m_entities.storage<COrb>().getComponent(id);
    orb.popped = true;
    m_entities.storage<CMouseInteraction>().getComponent(id).enabled = false;   // no double pops this frame

    switch(orb.kind)
    {
        case OrbKind::Normal:
        {
            const int points = 10 * multiplier();
            m_score += points;
            m_combo++;
            m_comboPulse = 1.f;
            burst(orb.center , orb.color , 22 , 120.f , 380.f , 5.f);
            addRing(orb.center , orb.color , 0.45f , orb.radius * 2.2f , 5.f);
            addPopup(orb.center , "+" + std::to_string(points) , sf::Color::White , 34.f);
            break;
        }
        case OrbKind::Golden:
        {
            const int points = 50 * multiplier();
            m_score += points;
            m_combo++;
            m_comboPulse = 1.f;
            m_timeLeft = std::min(m_timeLeft + 3.f , kMaxTime);
            burst(orb.center , kGold , 44 , 140.f , 520.f , 6.f);
            addRing(orb.center , kGold , 0.6f , orb.radius * 3.4f , 6.f);
            addRing(orb.center , sf::Color::White , 0.4f , orb.radius * 2.0f , 3.f);
            addPopup(orb.center , "+" + std::to_string(points) , kGold , 44.f);
            addPopup(orb.center + sf::Vector2f(0.f , 44.f * m_scale) , "+3s" , sf::Color(130 , 255 , 170) , 32.f);
            m_flash = 0.6f;
            m_flashColor = kGold;
            break;
        }
        case OrbKind::Bomb:
        {
            m_timeLeft -= 4.f;
            m_combo = 0;
            m_shake = 1.f;
            burst(orb.center , kBombGlow , 36 , 160.f , 560.f , 6.f);
            burst(orb.center , sf::Color(255 , 190 , 80) , 18 , 80.f , 300.f , 4.f);
            addRing(orb.center , kBombGlow , 0.55f , orb.radius * 4.0f , 8.f);
            addPopup(orb.center , "-4s" , kBombGlow , 46.f);
            m_flash = 1.f;
            m_flashColor = kBombGlow;
            break;
        }
    }
}

// -------------------------------------------------------------------
// Spawning
// -------------------------------------------------------------------

void OrbPop::spawnOrb()
{
    const float s = m_scale;
    const float difficulty = clamp01(m_elapsed / kRoundTime);
    const float roll = rnd(0.f , 1.f);

    COrb data;
    if(roll < 0.08f && m_elapsed > 4.f)
    {
        data.kind = OrbKind::Bomb;
        data.radius = 46.f * s;
        data.color = kBombBody;
    }
    else if(roll < 0.14f)
    {
        data.kind = OrbKind::Golden;
        data.radius = 40.f * s;
        data.color = kGold;
    }
    else
    {
        data.kind = OrbKind::Normal;
        data.radius = rnd(34.f , 62.f) * s;
        data.color = kOrbColors[static_cast<size_t>(rnd(0.f , 4.999f))];
    }

    data.speed = rnd(120.f , 210.f) * s * (1.f + 0.9f * difficulty);
    if(data.kind == OrbKind::Golden) data.speed *= 1.35f;
    data.swayAmp = rnd(15.f , 55.f) * s;
    data.swayFreq = rnd(0.8f , 1.8f);
    data.phase = rnd(0.f , 2.f * kPi);
    const float margin = data.radius + data.swayAmp + 20.f;
    data.baseX = rnd(margin , std::max(margin + 1.f , m_size.x - margin));
    data.center = {data.baseX , m_size.y + data.radius + 10.f};

    auto entity = m_entities.addEntity("Orb");
    const size_t id = entity->getId();

    m_entities.storage<CTransform>().addComponent(id , data.center - sf::Vector2f(data.radius , data.radius));
    m_entities.storage<CRender>().addComponent(id).zIndex = (data.kind == OrbKind::Bomb) ? 3 : (data.kind == OrbKind::Golden ? 2 : 1);
    m_entities.storage<CMouseInteraction>().addComponent(id , sf::FloatRect({0.f , 0.f} , {data.radius * 2.f , data.radius * 2.f}));
    m_entities.storage<COrb>().addComponent(id) = data;
}

// -------------------------------------------------------------------
// Update
// -------------------------------------------------------------------

void OrbPop::sUpdate(sf::Time dt)
{
    // The entity flush happens in SceneManager::sceneUpdate before this call.
    const float d = std::min(dt.asSeconds() , 0.1f);
    m_time += d;

    if(m_state == State::Playing)
    {
        m_elapsed += d;
        m_timeLeft -= d;

        m_spawnTimer -= d;
        if(m_spawnTimer <= 0.f)
        {
            const float difficulty = clamp01(m_elapsed / kRoundTime);
            spawnOrb();
            if(difficulty > 0.5f && rnd(0.f , 1.f) < 0.25f) spawnOrb();
            m_spawnTimer = mixf(0.8f , 0.34f , difficulty) * rnd(0.7f , 1.3f);
        }

        if(m_timeLeft <= 0.f) endRound();
    }
    else
    {
        m_overTime += d;
    }

    updateOrbs(d);
    updateEffects(d);
}

void OrbPop::updateOrbs(float dt)
{
    auto& orbs = m_entities.storage<COrb>();
    auto& transforms = m_entities.storage<CTransform>();

    for(const auto& e : m_entities.getEntities("Orb"))
    {
        if(!e->isAlive()) continue;
        const size_t id = e->getId();
        if(!orbs.hasComponent(id)) continue;

        COrb& orb = orbs.getComponent(id);
        if(orb.popped)
        {
            e->destroy();
            continue;
        }

        orb.age += dt;
        orb.center.y -= orb.speed * dt;
        orb.center.x = orb.baseX + std::sin(orb.age * orb.swayFreq + orb.phase) * orb.swayAmp;

        const float target = orb.hovered && m_state == State::Playing ? 1.f : 0.f;
        orb.hover += (target - orb.hover) * std::min(1.f , dt * 14.f);

        if(transforms.hasComponent(id))
            transforms.getComponent(id).position = orb.center - sf::Vector2f(orb.radius , orb.radius);

        if(orb.center.y + orb.radius < -20.f)
        {
            if(orb.kind == OrbKind::Normal && m_state == State::Playing)
                m_combo = 0;                                     // a normal orb got away
            m_entities.storage<CMouseInteraction>().getComponent(id).enabled = false;
            e->destroy();
        }
    }
}

void OrbPop::updateEffects(float dt)
{
    m_shake = std::max(0.f , m_shake - dt * 2.2f);
    m_flash = std::max(0.f , m_flash - dt * 2.5f);
    m_comboPulse = std::max(0.f , m_comboPulse - dt * 4.f);

    for(Particle& p : m_particles)
    {
        p.life -= dt;
        p.vel.y += p.gravity * dt;
        p.vel *= std::max(0.f , 1.f - 1.6f * dt);
        p.pos += p.vel * dt;
    }
    std::erase_if(m_particles , [](const Particle& p) { return p.life <= 0.f; });

    for(Ring& r : m_rings) r.age += dt;
    std::erase_if(m_rings , [](const Ring& r) { return r.age >= r.duration; });

    for(Popup& p : m_popups) p.age += dt;
    std::erase_if(m_popups , [](const Popup& p) { return p.age >= p.duration; });

    // shooting stars
    m_shootTimer -= dt;
    if(m_shootTimer <= 0.f)
    {
        const float angle = rnd(20.f , 38.f) * kPi / 180.f;
        ShootingStar star;
        star.pos = {rnd(m_size.x * 0.25f , m_size.x * 1.05f) , rnd(0.f , m_size.y * 0.35f)};
        star.dir = {-std::cos(angle) , std::sin(angle)};
        star.life = rnd(0.7f , 1.1f);
        star.length = rnd(180.f , 320.f) * m_scale;
        m_shootingStars.push_back(star);
        m_shootTimer = rnd(2.5f , 6.f);
    }
    for(ShootingStar& s : m_shootingStars)
    {
        s.age += dt;
        s.pos += s.dir * (950.f * m_scale * dt);
    }
    std::erase_if(m_shootingStars , [](const ShootingStar& s) { return s.age >= s.life; });
}

void OrbPop::burst(sf::Vector2f pos , const sf::Color& color , int count , float speedMin , float speedMax , float size)
{
    for(int i = 0 ; i < count ; i++)
    {
        const float angle = rnd(0.f , 2.f * kPi);
        const float speed = rnd(speedMin , speedMax) * m_scale;
        Particle p;
        p.pos = pos;
        p.vel = {std::cos(angle) * speed , std::sin(angle) * speed};
        p.color = mix(color , sf::Color::White , rnd(0.f , 0.5f));
        p.life = p.maxLife = rnd(0.5f , 1.0f);
        p.size = rnd(size * 0.5f , size) * m_scale;
        p.gravity = 260.f * m_scale;
        m_particles.push_back(p);
    }
}

void OrbPop::addRing(sf::Vector2f pos , const sf::Color& color , float duration , float maxRadius , float thickness)
{
    m_rings.push_back(Ring{pos , color , 0.f , duration , maxRadius , thickness});
}

void OrbPop::addPopup(sf::Vector2f pos , const std::string& text , const sf::Color& color , float size)
{
    m_popups.push_back(Popup{pos , text , color , 0.f , 0.9f , size});
}

// -------------------------------------------------------------------
// Drawing primitives
// -------------------------------------------------------------------

void OrbPop::drawCircle(sf::RenderWindow& w , sf::Vector2f center , float radius , const sf::Color& fill , bool additive , float outline , const sf::Color& outlineColor)
{
    if(radius <= 0.f) return;
    m_circle.setRadius(radius);
    m_circle.setOrigin({radius , radius});
    m_circle.setPosition(center);
    m_circle.setFillColor(fill);
    m_circle.setOutlineThickness(outline);
    m_circle.setOutlineColor(outlineColor);
    w.draw(m_circle , additive ? m_additive : m_normal);
}

void OrbPop::drawRect(sf::RenderWindow& w , sf::Vector2f center , sf::Vector2f size , float rotationDeg , const sf::Color& color , bool additive)
{
    m_rect.setSize(size);
    m_rect.setOrigin(size / 2.f);
    m_rect.setPosition(center);
    m_rect.setRotation(sf::degrees(rotationDeg));
    m_rect.setFillColor(color);
    w.draw(m_rect , additive ? m_additive : m_normal);
}

void OrbPop::drawText(sf::RenderWindow& w , const std::string& str , float size , sf::Vector2f pos , const sf::Color& color , TextAlign align , float scale)
{
    if(!m_font) return;
    sf::Text text(*m_font , str , static_cast<unsigned>(std::max(size , 1.f)));
    text.setFillColor(color);
    text.setOutlineColor(sf::Color(0 , 0 , 0 , static_cast<std::uint8_t>(color.a / 2)));
    text.setOutlineThickness(std::max(1.5f , size * 0.05f));

    const sf::FloatRect bounds = text.getLocalBounds();
    float originX = bounds.position.x;
    if(align == TextAlign::Center) originX += bounds.size.x / 2.f;
    else if(align == TextAlign::Right) originX += bounds.size.x;

    text.setOrigin({originX , bounds.position.y + bounds.size.y / 2.f});
    text.setPosition(pos);
    text.setScale({scale , scale});
    w.draw(text);
}

// -------------------------------------------------------------------
// Scene drawing
// -------------------------------------------------------------------

void OrbPop::drawHills(sf::RenderWindow& w , float baseY , float amp , float freq , float phase , const sf::Color& top , const sf::Color& bottom)
{
    m_vertices.clear();
    const float step = 20.f;
    for(float x = -40.f ; x <= m_size.x + 40.f + step ; x += step)
    {
        const float y = baseY + amp * (std::sin(x * freq + phase) * 0.6f + std::sin(x * freq * 2.3f + phase * 1.7f) * 0.4f);
        m_vertices.push_back(sf::Vertex{sf::Vector2f(x , y) , top});
        m_vertices.push_back(sf::Vertex{sf::Vector2f(x , m_size.y + 60.f) , bottom});
    }
    w.draw(m_vertices.data() , m_vertices.size() , sf::PrimitiveType::TriangleStrip , m_normal);
}

void OrbPop::drawBackground(sf::RenderWindow& w)
{
    const float W = m_size.x;
    const float H = m_size.y;
    const float s = m_scale;

    // sky gradient (the middle stop breathes slowly)
    const float breathe = 0.5f + 0.5f * std::sin(m_time * 0.25f);
    struct Stop { float t; sf::Color color; };
    const Stop stops[] =
    {
        {0.00f , sf::Color(  8 ,  8 , 32)},
        {0.45f , mix(sf::Color(40 , 18 , 92) , sf::Color(58 , 24 , 110) , breathe)},
        {0.75f , mix(sf::Color(130 , 42 , 122) , sf::Color(160 , 52 , 130) , breathe)},
        {1.00f , sf::Color(255 , 120 , 110)},
    };
    m_vertices.clear();
    for(const Stop& stop : stops)
    {
        const float y = -40.f + (H + 80.f) * stop.t;
        m_vertices.push_back(sf::Vertex{sf::Vector2f(-40.f , y) , stop.color});
        m_vertices.push_back(sf::Vertex{sf::Vector2f(W + 40.f , y) , stop.color});
    }
    w.draw(m_vertices.data() , m_vertices.size() , sf::PrimitiveType::TriangleStrip , m_normal);

    // stars: twinkle and drift sideways at a speed set by their depth
    for(const Star& star : m_stars)
    {
        const float twinkle = 0.55f + 0.45f * std::sin(m_time * star.speed + star.phase);
        const float x = std::fmod(star.pos.x * W + m_time * (3.f + star.depth * 9.f) * s , W);
        const float y = star.pos.y * H * 0.78f;
        const float alpha = 255.f * twinkle * (0.35f + 0.65f * star.depth);
        const float r = star.size * s * (0.7f + 0.3f * twinkle);
        drawCircle(w , {x , y} , r , withAlpha(sf::Color(235 , 235 , 255) , alpha) , true);
        if(star.depth > 0.85f)
            drawCircle(w , {x , y} , r * 3.f , withAlpha(sf::Color(170 , 180 , 255) , alpha * 0.18f) , true);
    }

    // moon with a soft glow
    const sf::Vector2f moon(W * 0.82f , H * 0.18f + std::sin(m_time * 0.4f) * 5.f * s);
    const float moonR = 70.f * s;
    for(int i = 0 ; i < 5 ; i++)
        drawCircle(w , moon , moonR * (1.35f + 0.55f * i) , sf::Color(255 , 240 , 210 , 14) , true);
    drawCircle(w , moon , moonR , sf::Color(250 , 244 , 220));
    drawCircle(w , moon + sf::Vector2f(-0.30f , -0.15f) * moonR , moonR * 0.20f , sf::Color(228 , 220 , 194));
    drawCircle(w , moon + sf::Vector2f( 0.28f ,  0.25f) * moonR , moonR * 0.28f , sf::Color(232 , 224 , 198));
    drawCircle(w , moon + sf::Vector2f( 0.05f , -0.45f) * moonR , moonR * 0.12f , sf::Color(228 , 220 , 194));

    // shooting stars: a quad that fades to nothing at the tail
    for(const ShootingStar& star : m_shootingStars)
    {
        const float fade = std::sin(kPi * clamp01(star.age / star.life));
        const sf::Vector2f tail = star.pos - star.dir * star.length;
        const sf::Vector2f perp(-star.dir.y , star.dir.x);
        const float width = 2.5f * s;
        m_vertices.clear();
        m_vertices.push_back(sf::Vertex{tail + perp * width , sf::Color(255 , 255 , 255 , 0)});
        m_vertices.push_back(sf::Vertex{tail - perp * width , sf::Color(255 , 255 , 255 , 0)});
        m_vertices.push_back(sf::Vertex{star.pos + perp * width , withAlpha(sf::Color(255 , 255 , 255) , 255.f * fade)});
        m_vertices.push_back(sf::Vertex{star.pos - perp * width , withAlpha(sf::Color(255 , 255 , 255) , 255.f * fade)});
        w.draw(m_vertices.data() , m_vertices.size() , sf::PrimitiveType::TriangleStrip , m_additive);
    }

    // far hills (the orbs rise from behind the near ones, drawn after the orbs)
    drawHills(w , H * 0.80f , H * 0.045f , 0.0035f , 1.3f + m_time * 0.03f , sf::Color(78 , 32 , 116) , sf::Color(34 , 16 , 66));
}

void OrbPop::drawOrb(sf::RenderWindow& w , const COrb& orb)
{
    const float s = m_scale;
    const float pulse = 0.5f + 0.5f * std::sin(m_time * 4.f + orb.phase);
    const float r = orb.radius * (1.f + 0.14f * orb.hover);
    const sf::Vector2f c = orb.center;
    const bool bomb = orb.kind == OrbKind::Bomb;
    const bool golden = orb.kind == OrbKind::Golden;

    // glow
    const sf::Color glow = bomb ? kBombGlow : orb.color;
    const float glowStrength = (golden ? 1.5f : 1.f) * (0.8f + 0.4f * pulse + 0.6f * orb.hover) * (bomb ? 0.8f : 1.f);
    for(int i = 0 ; i < 4 ; i++)
        drawCircle(w , c , r * (1.25f + 0.35f * i) , withAlpha(glow , 34.f * glowStrength / (1.f + 0.6f * i)) , true);

    // body: dark base, lighter core offset to the light, highlights
    if(bomb)
    {
        drawCircle(w , c , r , kBombBody , false , 3.f * s , withAlpha(kBombGlow , 120.f + 135.f * pulse));
        drawCircle(w , c + sf::Vector2f(-0.12f , -0.12f) * r , r * 0.72f , sf::Color(58 , 42 , 78));
        drawRect(w , c , {r * 1.0f , r * 0.17f} ,  45.f , sf::Color(255 , 110 , 110 , 235));
        drawRect(w , c , {r * 1.0f , r * 0.17f} , -45.f , sf::Color(255 , 110 , 110 , 235));
        // little fuse spark
        const sf::Vector2f spark = c + sf::Vector2f(r * 0.55f + std::sin(m_time * 30.f) * 2.f , -r * 1.05f);
        drawCircle(w , spark , 5.f * s , sf::Color(255 , 200 , 90) , true);
        drawCircle(w , spark , 11.f * s , sf::Color(255 , 140 , 40 , 90) , true);
    }
    else
    {
        drawCircle(w , c , r , mix(orb.color , sf::Color::Black , 0.30f) , false , 2.f * s , withAlpha(mix(orb.color , sf::Color::White , 0.5f) , 150.f));
        drawCircle(w , c + sf::Vector2f(-0.10f , -0.12f) * r , r * 0.80f , mix(orb.color , sf::Color::White , 0.18f));
        drawCircle(w , c + sf::Vector2f(-0.34f , -0.40f) * r , r * 0.22f , sf::Color(255 , 255 , 255 , 210));
        drawCircle(w , c + sf::Vector2f( 0.36f ,  0.38f) * r , r * 0.09f , sf::Color(255 , 255 , 255 , 90));
    }

    // golden orbs get a slowly rotating sparkle
    if(golden)
    {
        const float spin = m_time * 70.f;
        for(int i = 0 ; i < 2 ; i++)
            drawRect(w , c , {r * 3.0f , 3.f * s} , spin + 90.f * static_cast<float>(i) , withAlpha(sf::Color(255 , 240 , 170) , 130.f * pulse + 40.f) , true);
    }

    // hover ring
    if(orb.hover > 0.01f)
        drawCircle(w , c , r * 1.18f , sf::Color::Transparent , false , 3.f * s ,
                   withAlpha(bomb ? kBombGlow : sf::Color::White , 220.f * orb.hover));
}

void OrbPop::drawEffects(sf::RenderWindow& w)
{
    for(const Particle& p : m_particles)
    {
        const float t = clamp01(p.life / p.maxLife);
        drawCircle(w , p.pos , p.size * (0.4f + 0.6f * t) , withAlpha(p.color , 255.f * t) , true);
    }

    for(const Ring& ring : m_rings)
    {
        const float t = clamp01(ring.age / ring.duration);
        drawCircle(w , ring.pos , ring.maxRadius * easeOutCubic(t) , sf::Color::Transparent , true ,
                   ring.thickness * (1.f - t) + 1.f , withAlpha(ring.color , 255.f * (1.f - t)));
    }

    // soft glow around the mouse so the player always sees where they aim
    if(m_hasMouse && m_state == State::Playing)
    {
        drawCircle(w , m_mouse , 30.f * m_scale , sf::Color(120 , 200 , 255 , 26) , true);
        drawCircle(w , m_mouse , 16.f * m_scale , sf::Color(160 , 230 , 255 , 40) , true);
    }

    for(const Popup& popup : m_popups)
    {
        const float t = clamp01(popup.age / popup.duration);
        const sf::Vector2f pos = popup.pos + sf::Vector2f(0.f , -80.f * m_scale * easeOutCubic(t));
        drawText(w , popup.text , popup.size * m_scale , pos , withAlpha(popup.color , 255.f * (1.f - t * t)) , TextAlign::Center , 1.f + 0.35f * (1.f - t));
    }
}

void OrbPop::drawHud(sf::RenderWindow& w)
{
    const float s = m_scale;
    const float W = m_size.x;
    const float margin = 36.f * s;
    const sf::Color label(190 , 180 , 235);

    // score and best
    drawText(w , "SCORE" , 22.f * s , {margin , margin + 8.f * s} , label , TextAlign::Left);
    drawText(w , std::to_string(m_score) , 64.f * s , {margin , margin + 58.f * s} , sf::Color::White , TextAlign::Left);
    drawText(w , "BEST  " + std::to_string(m_best) , 26.f * s , {W - margin , margin + 20.f * s} , label , TextAlign::Right);

    // time bar (drawn even without a font)
    const float frac = clamp01(m_timeLeft / kRoundTime);
    const sf::Vector2f barSize(W * 0.34f , 16.f * s);
    const sf::Vector2f barCenter(W / 2.f , margin + 14.f * s);
    const sf::Color barColor = frac > 0.5f ? mix(sf::Color(255 , 215 , 80) , sf::Color(90 , 240 , 150) , (frac - 0.5f) * 2.f)
                                            : mix(sf::Color(255 , 70 , 70) , sf::Color(255 , 215 , 80) , frac * 2.f);
    const float lowPulse = (m_timeLeft < 8.f && m_state == State::Playing) ? 0.6f + 0.4f * std::sin(m_time * 12.f) : 1.f;

    drawRect(w , barCenter , barSize + sf::Vector2f(8.f * s , 8.f * s) , 0.f , sf::Color(0 , 0 , 0 , 110));
    if(frac > 0.f)
    {
        const sf::Vector2f fillSize(barSize.x * frac , barSize.y);
        const sf::Vector2f fillCenter(barCenter.x - barSize.x / 2.f + fillSize.x / 2.f , barCenter.y);
        drawRect(w , fillCenter , fillSize , 0.f , withAlpha(barColor , 255.f * lowPulse));
        drawRect(w , fillCenter , fillSize + sf::Vector2f(0.f , 10.f * s) , 0.f , withAlpha(barColor , 40.f * lowPulse) , true);
    }
    drawText(w , std::to_string(static_cast<int>(std::ceil(m_timeLeft))) + "s" , 28.f * s , {W / 2.f , margin + 52.f * s} , sf::Color::White);

    // combo
    if(m_combo >= 2 && m_state == State::Playing)
    {
        std::string text = "COMBO " + std::to_string(m_combo);
        if(multiplier() > 1) text += "   x" + std::to_string(multiplier());
        drawText(w , text , 34.f * s , {W / 2.f , margin + 98.f * s} , kGold , TextAlign::Center , 1.f + 0.25f * m_comboPulse);
    }

    // hint during the first seconds
    if(m_state == State::Playing && m_elapsed < 4.f)
    {
        const float a = clamp01(std::min(m_elapsed / 0.5f , (4.f - m_elapsed) / 0.8f));
        drawText(w , "POP THE ORBS!" , 76.f * s , {W / 2.f , m_size.y * 0.42f} , withAlpha(sf::Color::White , 255.f * a));
        drawText(w , "catch the golden ones   -   avoid the bombs" , 28.f * s , {W / 2.f , m_size.y * 0.42f + 66.f * s} , withAlpha(label , 255.f * a));
    }
}

void OrbPop::drawGameOver(sf::RenderWindow& w)
{
    const float s = m_scale;
    const float t = clamp01(m_overTime / 0.5f);
    const sf::Vector2f center = m_size / 2.f;

    drawRect(w , center , m_size , 0.f , sf::Color(5 , 3 , 20 , static_cast<std::uint8_t>(175.f * t)));

    const sf::Vector2f panel(580.f * s , 380.f * s);
    drawRect(w , center , panel + sf::Vector2f(14.f , 14.f) * s , 0.f , sf::Color(150 , 110 , 255 , static_cast<std::uint8_t>(60.f * t)) , true);
    drawRect(w , center , panel , 0.f , sf::Color(20 , 14 , 46 , static_cast<std::uint8_t>(240.f * t)));

    const float a = 255.f * t;
    drawText(w , "TIME IS UP" , 60.f * s , center + sf::Vector2f(0.f , -128.f * s) , withAlpha(sf::Color::White , a));
    drawText(w , "SCORE" , 22.f * s , center + sf::Vector2f(0.f , -62.f * s) , withAlpha(sf::Color(190 , 180 , 235) , a));
    drawText(w , std::to_string(m_score) , 96.f * s , center + sf::Vector2f(0.f , 4.f * s) , withAlpha(kGold , a));

    if(m_newBest)
        drawText(w , "NEW BEST!" , 34.f * s , center + sf::Vector2f(0.f , 78.f * s) , withAlpha(sf::Color(130 , 255 , 170) , a) , TextAlign::Center , 1.f + 0.08f * std::sin(m_time * 8.f));
    else
        drawText(w , "BEST  " + std::to_string(m_best) , 30.f * s , center + sf::Vector2f(0.f , 78.f * s) , withAlpha(sf::Color(190 , 180 , 235) , a));

    if(m_overTime > 0.7f)
    {
        const float blink = 150.f + 105.f * std::sin(m_time * 4.f);
        drawText(w , "click or press R to play again   -   Esc to quit" , 24.f * s , center + sf::Vector2f(0.f , 148.f * s) , withAlpha(sf::Color::White , blink));
    }
}

void OrbPop::sRender(sf::RenderWindow& gameWindow)
{
    m_size = sf::Vector2f(gameWindow.getSize());
    m_scale = std::min(m_size.x , m_size.y) / 1080.f;

    // screen shake moves the world, the HUD stays put
    sf::View view = gameWindow.getDefaultView();
    if(m_shake > 0.f)
        view.setCenter(view.getCenter() + sf::Vector2f(rnd(-1.f , 1.f) , rnd(-1.f , 1.f)) * (m_shake * 16.f * m_scale));
    gameWindow.setView(view);

    drawBackground(gameWindow);

    auto& orbs = m_entities.storage<COrb>();
    for(const auto& e : m_entities.getEntities("Orb"))
    {
        if(!e->isAlive() || !orbs.hasComponent(e->getId())) continue;
        const COrb& orb = orbs.getComponent(e->getId());
        if(!orb.popped) drawOrb(gameWindow , orb);
    }

    // near hills hide the spot where orbs are born
    drawHills(gameWindow , m_size.y * 0.93f , m_size.y * 0.022f , 0.0058f , 4.1f , sf::Color(24 , 11 , 48) , sf::Color(8 , 4 , 22));

    drawEffects(gameWindow);

    gameWindow.setView(gameWindow.getDefaultView());

    if(m_flash > 0.f)
        drawRect(gameWindow , m_size / 2.f , m_size , 0.f , withAlpha(m_flashColor , 110.f * m_flash) , true);

    drawHud(gameWindow);
    if(m_state == State::GameOver) drawGameOver(gameWindow);
}