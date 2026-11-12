#include "PvZ.hpp"

namespace
{
const char* kActorShader = R"(
uniform sampler2D texture;
uniform float flash;
uniform float grade;     // 0 = keep colours, 1 = night grade + outline
uniform vec2  texel;     // 1 / texture size

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 c  = texture2D(texture, uv);
    float lum = dot(c.rgb, vec3(0.30, 0.59, 0.11));

    // Night grade (only as strong as 'grade')
    vec3 graded = mix(vec3(lum), c.rgb, 0.55);
    graded *= mix(vec3(0.55, 0.68, 0.95), vec3(0.85, 0.95, 1.00), smoothstep(0.2, 0.8, lum));
    graded *= 0.85;
    vec3 col = mix(c.rgb, graded, grade);

    // Lightning as exposure: darks stay dark, lit areas jump
    col *= 1.0 + flash * 1.2;
    col  = mix(col, col * vec3(0.85, 0.95, 1.25), flash * 0.5);
    col += vec3(0.50, 0.60, 0.90) * flash * 0.3 * smoothstep(0.35, 0.9, lum);

    // Dark green-black outline, applied last so lightning never washes it out
    vec2 px = texel * 2.0;
    float a1 = texture2D(texture, uv + vec2( px.x, 0.0)).a;
    float a2 = texture2D(texture, uv + vec2(-px.x, 0.0)).a;
    float a3 = texture2D(texture, uv + vec2(0.0,  px.y)).a;
    float a4 = texture2D(texture, uv + vec2(0.0, -px.y)).a;
    float edge = c.a * (1.0 - min(min(a1, a2), min(a3, a4)));
    col = mix(col, vec3(0.02, 0.07, 0.03), clamp(edge, 0.0, 1.0) * 0.85 * grade);

    gl_FragColor = vec4(col, c.a) * gl_Color;   // alpha is never touched
}
)";
// ---------------------------------------------------------------------------
//  Shader (unchanged): cloud drift + lightning, limited to the sky by the mask.
//  The actors are drawn with it too (same UV space as the background), so
//  lightning also brightens them a little.
// ---------------------------------------------------------------------------
const char* kSceneShader = R"(
uniform sampler2D texture;
uniform sampler2D mask;
uniform float time;
uniform float flash;

float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

float noise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i),                  hash(i + vec2(1.0, 0.0)), f.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), f.x), f.y);
}

float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; ++i) { v += a * noise(p); p *= 2.0; a *= 0.5; }
    return v;
}

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    float m = texture2D(mask, uv).r;

    vec2 q = vec2(uv.x * 3.0 - time * 0.03, uv.y * 3.0);
    vec2 warp = vec2(fbm(q + time * 0.02),
                     fbm(q + vec2(5.2, 1.3) - time * 0.02)) - 0.5;
    vec4 col = texture2D(texture, uv + warp * 0.008 * m);

    float bands = fbm(vec2(uv.x * 2.5 - time * 0.04, uv.y * 4.0));
    col.rgb *= 1.0 + (bands - 0.5) * 0.45 * m;

    float lum = dot(col.rgb, vec3(0.30, 0.59, 0.11));
    vec3 flashCol = vec3(0.55, 0.65, 1.0);
    col.rgb += flashCol * flash * (m * (0.25 + lum * 1.6) + 0.05 * lum);

    gl_FragColor = col * gl_Color;
}
)";

// ---------------------------------------------------------------------------
//  Placement of the cut-outs, in IMAGE pixels (the 1376x768 extracts).
//  Measured from your PNGs: zombie solid pixels (673,348)-(839,609),
//  peashooter solid pixels (845,461)-(1083,758).
// ---------------------------------------------------------------------------
const sf::IntRect     kZombieBounds    { {667, 342}, {179, 274} };
const sf::Vector2f    kZombiePivot     { 748.f, 604.f };            // between his feet
const sf::Color       kZombieTint      { 225, 230, 245 };

const sf::IntRect     kPeaBounds       { {839, 455}, {251, 311} };
const sf::Vector2f    kPeaPivot        { 975.f, 752.f };            // base of the stem
const sf::Color       kPeaTint         { 170, 190, 215 };           // tames the bright green to match the night

const sf::Vector2f    kMuzzle          { 852.f, 545.f };            // peashooter's mouth
const sf::Vector2f    kZombieFace      { 778.f, 432.f };            // where the pea lands

// Soft contact shadows (image px): centre and half-size
const sf::Vector2f    kZombieShadowPos { 748.f, 606.f };
const sf::Vector2f    kZombieShadowR   { 62.f, 11.f };
const sf::Vector2f    kPeaShadowPos    { 968.f, 752.f };
const sf::Vector2f    kPeaShadowR      { 88.f, 13.f };
constexpr float       kShadowAlpha     = 150.f;

float smoothstep(float a, float b, float x)
{
    const float t = std::clamp((x - a) / (b - a), 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

// Damped oscillation that starts at 1 and rings out (0 before t = 0).
float spring(float t, float decay, float freq)
{
    return t < 0.f ? 0.f : std::exp(-decay * t) * std::cos(freq * t);
}
}

sf::Texture PvZ::makeGlowTexture(unsigned size)
{
    sf::Image img(sf::Vector2u{ size, size }, sf::Color::Transparent);
    const float c = static_cast<float>(size) * 0.5f;

    for (unsigned y = 0; y < size; ++y)
        for (unsigned x = 0; x < size; ++x)
        {
            const float d = std::hypot(x + 0.5f - c, y + 0.5f - c) / c;
            float a = std::clamp(1.f - d, 0.f, 1.f);
            a *= a;
            img.setPixel({ x, y }, sf::Color(255, 255, 255, static_cast<std::uint8_t>(a * 255.f)));
        }

    sf::Texture tex(img);
    tex.setSmooth(true);
    return tex;
}

bool PvZ::renderBelow() 
{
    return false;
}

bool PvZ::updateBelow()
{
    return false;
}

void PvZ::onDelete()
{
    //Have to implement
}

void PvZ::onLoad()
{

    {
        ActionBinding& binding = this->registerActionBinding("Close");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::Escape);
        binding.addCondition<ModifierCondition>(false , false , false);
    }

    {
        // Test trigger. Later, call startPlaySequence() from your Play button instead.
        ActionBinding& binding = this->registerActionBinding("Play");
        binding.addCondition<KeyCondition>(sf::Keyboard::Key::Enter);
        binding.addCondition<ModifierCondition>(false , false , false);
    }

    // ---- Assets: "Background" (the NO-zombie image), "SkyMask", "Zombie", "Peashooter"
    const sf::Texture& bgTex     = m_assetManager.getTexture("Background");
    const sf::Texture& maskTex   = m_assetManager.getTexture("SkyMask");
    const sf::Texture& zombieTex = m_assetManager.getTexture("Zombie");
    const sf::Texture& peaTex    = m_assetManager.getTexture("Peashooter");

    const sf::Vector2f imgSize{ bgTex.getSize() };

    m_background.emplace(bgTex);
    m_background->setScale({ kSceneW / imgSize.x, kSceneH / imgSize.y });

    m_imageToScene = sf::Transform();
    m_imageToScene.scale({ kSceneW / imgSize.x, kSceneH / imgSize.y });

    // ---- Shader -------------------------------------------------------------
    m_shaderOk = m_shader.loadFromMemory(kSceneShader, sf::Shader::Type::Fragment);
    if (m_shaderOk)
    {
        m_shader.setUniform("texture", sf::Shader::CurrentTexture);
        m_shader.setUniform("mask", maskTex);
        m_shader.setUniform("time", 0.f);
        m_shader.setUniform("flash", 0.f);
    }

    m_actorShaderOk = m_actorShader.loadFromMemory(kActorShader, sf::Shader::Type::Fragment);
    if (m_actorShaderOk)
    {
        m_actorShader.setUniform("texture", sf::Shader::CurrentTexture);
        m_actorShader.setUniform("flash", 0.f);
    }

    // ---- Actors -------------------------------------------------------------
    m_zombie.emplace(zombieTex, kZombieBounds, kZombiePivot, kZombieTint);
    m_peashooter.emplace(peaTex, kPeaBounds, kPeaPivot, kPeaTint);

    // ---- Leaves (spawn area = tree canopy, image-space) ---------------------
    m_leaves.emplace(sf::FloatRect({ 0.f, 20.f }, { 800.f, 110.f }), imgSize);

    // ---- Glows (windows, lantern) and soft shadows share one texture --------
    m_glowTexture = makeGlowTexture();
    m_glow.emplace(m_glowTexture);
    m_glow->setOrigin({ 32.f, 32.f });

    m_lights = {
        { {468.f, 392.f}, 40.f, {255, 170, 60} },   // round window
        { {620.f, 401.f}, 38.f, {255, 170, 60} },
        { {565.f, 462.f}, 30.f, {255, 170, 60} },
        { {634.f, 452.f}, 30.f, {255, 170, 60} },
        { {680.f, 450.f}, 36.f, {255, 170, 60} },
        { {576.f, 432.f}, 26.f, {255, 190, 90} },   // lantern
    };
}

// ============================================================================
//  Play sequence
// ============================================================================

void PvZ::startPlaySequence()
{
    if (m_playing)
        return;

    m_playing  = true;
    m_fired    = false;
    m_hitDone  = false;
    m_finished = false;
    m_playTime = 0.f;
}

void PvZ::updatePlaySequence()
{
    if (!m_playing)
        return;

    const float T = m_playTime;

    if (!m_fired && T >= kFire)
    {
        m_fired     = true;
        m_muzzlePos = m_peashooter->deform(kMuzzle);      // freeze the flight path
        m_targetPos = m_zombie->deform(kZombieFace);
        // TODO: play "pea shoot" sound
    }

    if (!m_hitDone && T >= kHit)
    {
        m_hitDone = true;
        m_splat.emit(m_targetPos);
        // TODO: play "pea hit" sound
    }

    if (!m_finished && T >= kEnd)
    {
        m_finished = true;
        onPlayFinished();
    }
}

void PvZ::onPlayFinished()
{
    // TODO: switch to the gameplay scene with whatever request your engine uses, e.g.
    // m_requests.push_back(Request(RequestCommand::???, "Gameplay"));
}

// ---- Poses -----------------------------------------------------------------

Actor::Pose PvZ::zombiePose() const
{
    const float t = m_time;

    Actor::Pose p;
    // Idle: slow sway of the upper body, tiny breathing, feet stay planted.
    p.shear    = 3.2f * (std::sin(t * 1.05f) + 0.35f * std::sin(t * 2.4f + 1.3f));
    p.scaleY   = 1.f + 0.007f * std::sin(t * 1.6f);
    p.scaleX   = 1.f - 0.003f * std::sin(t * 1.6f);
    p.rotation = 0.35f * std::sin(t * 0.8f + 0.5f);
    p.ripple   = 0.9f;                 // arms and jacket wobble slightly
    p.phase    = t;

    if (m_playing)
    {
        const float th = m_playTime - kHit;
        if (th >= 0.f)
        {
            // Hit reaction: head snaps away from the pea, then rings out.
            const float rec = smoothstep(0.f, 0.04f, th) * spring(th, 4.5f, 13.f);

            p.shear      -= 18.f * rec;
            p.offset.x   -= 5.f  * rec;
            p.rotation   -= 2.5f * rec;
            p.scaleY     *= 1.f - 0.04f * rec;
            p.flash       = 0.85f * std::exp(-th * 10.f);
        }
    }
    return p;
}

Actor::Pose PvZ::peashooterPose() const
{
    const float t = m_time;

    Actor::Pose p;
    // Idle: a little livelier than the zombie, like a plant in the wind.
    p.shear      = -4.0f * (std::sin(t * 1.25f + 0.8f) + 0.3f * std::sin(t * 2.9f));
    p.scaleY     = 1.f + 0.012f * std::sin(t * 1.9f + 0.6f);
    p.scaleX     = 1.f - 0.006f * std::sin(t * 1.9f + 0.6f);
    p.ripple     = 1.4f;
    p.rippleBase = 0.5f;               // leaves at the base flutter too
    p.phase      = t * 1.2f;

    if (m_playing)
    {
        const float T = m_playTime;

        // 0 -> 1 while winding up, then the release rings out as a spring
        // (it starts at 1, so there is no pop at the moment of firing).
        const float charge = T < kFire
            ? smoothstep(kWindupStart, kFire - 0.05f, T)
            : spring(T - kFire, 7.f, 20.f);

        p.shear  += 10.f * charge;                // leans back, then snaps forward
        p.scaleY *= 1.f - 0.05f * charge;
        p.scaleX *= 1.f + 0.04f * charge;
    }
    return p;
}

sf::Vector2f PvZ::shakeOffset() const
{
    if (!m_playing)
        return { 0.f, 0.f };

    const float th = m_playTime - kHit;
    if (th < 0.f)
        return { 0.f, 0.f };

    const float amp = 7.f * std::exp(-th * 9.f);
    return { std::sin(th * 70.f) * amp, std::cos(th * 83.f) * amp * 0.6f };
}

// ============================================================================
//  Actions / update / render
// ============================================================================

void PvZ::sDoAction(RefinedAction& input) 
{
    if(input.getName() == "Close" && input.getType() == ActionType::Started)
    {
        m_requests.push_back(Request(RequestCommand::Pop , "InputTest"));
    }

    if(input.getName() == "Play" && input.getType() == ActionType::Started)
    {
        startPlaySequence();
    }
}

void PvZ::sUpdate(sf::Time dt)
{   
    const float d = std::min(dt.asSeconds(), 1.f / 20.f);
    m_time += d;

    if (m_playing)
        m_playTime += d;

    m_lightning.update(d);
    if (m_lightning.takeStrike())
    {
        // TODO: play thunder sound after a 0.5 - 2 s delay
    }

    updatePlaySequence();

    if (m_zombie)     m_zombie->setPose(zombiePose());
    if (m_peashooter) m_peashooter->setPose(peashooterPose());
    if (m_leaves)     m_leaves->update(d);
    m_splat.update(d);
    m_rain.update(dt);

    // Fade the menu UI out when the play sequence starts.
    if (m_playing)
    {
        const float a = 1.f - smoothstep(0.f, kUiFadeEnd, m_playTime);
        for(auto entity : m_entities.getEntities())
        {
            auto& sprite = m_entities.storage<CVisual>().getComponent(entity->getId()).sprite;
            sf::Color c = sprite.getColor();
            c.a = static_cast<std::uint8_t>(255.f * a);
            sprite.setColor(c);
        }
    }

    if (m_shaderOk)
    {
        m_shader.setUniform("time", m_time);
        m_shader.setUniform("flash", m_lightning.flash());
    }

    if (m_actorShaderOk)
        m_actorShader.setUniform("flash", m_lightning.flash());
}

void PvZ::drawPea(sf::RenderWindow& window)
{
    if (!m_playing || m_playTime < kFire || m_playTime >= kHit)
        return;

    // Position and size of the pea at time t: it shrinks as it flies "away" from the camera.
    auto peaAt = [&](float t, float& radius)
    {
        const float u = std::clamp((t - kFire) / (kHit - kFire), 0.f, 1.f);
        radius = 11.f * (1.15f - 0.5f * u);

        sf::Vector2f p = m_muzzlePos + (m_targetPos - m_muzzlePos) * u;
        p.y -= 40.f * u * (1.f - u);                 // slight arc
        return p;
    };

    sf::RenderStates states;
    states.transform = m_imageToScene;

    sf::CircleShape shape;

    // Motion trail
    for (int i = 5; i >= 1; --i)
    {
        const float t = m_playTime - 0.02f * static_cast<float>(i);
        if (t < kFire)
            continue;

        float r = 0.f;
        const sf::Vector2f p = peaAt(t, r);
        r *= 1.f - 0.1f * static_cast<float>(i);

        shape.setRadius(r);
        shape.setOrigin({ r, r });
        shape.setPosition(p);
        shape.setFillColor(sf::Color(130, 205, 70, static_cast<std::uint8_t>(130 - 22 * i)));
        window.draw(shape, states);
    }

    // The pea itself: body, dark rim, small highlight
    float r = 0.f;
    const sf::Vector2f p = peaAt(m_playTime, r);

    shape.setRadius(r);
    shape.setOrigin({ r, r });
    shape.setPosition(p);
    shape.setFillColor(sf::Color(125, 205, 65));
    shape.setOutlineThickness(1.5f);
    shape.setOutlineColor(sf::Color(40, 100, 30));
    window.draw(shape, states);

    const float hr = r * 0.32f;
    shape.setRadius(hr);
    shape.setOrigin({ hr, hr });
    shape.setPosition(p + sf::Vector2f{ -r * 0.3f, -r * 0.3f });
    shape.setOutlineThickness(0.f);
    shape.setFillColor(sf::Color(230, 255, 200, 200));
    window.draw(shape, states);
}

void PvZ::sRender(sf::RenderWindow& gameWindow)  
{
    // Screen shake on impact
    const sf::View originalView = gameWindow.getView();
    sf::View shaken = originalView;
    shaken.move(shakeOffset());
    gameWindow.setView(shaken);

    // ---- Shaded scene -------------------------------------------------------
    sf::RenderStates shaded;
    if (m_shaderOk) shaded.shader = &m_shader;

    sf::RenderStates shadedImageSpace = shaded;
    shadedImageSpace.transform = m_imageToScene;

    sf::RenderStates imageSpace;
    imageSpace.transform = m_imageToScene;

    sf::RenderStates add;
    add.blendMode = sf::BlendAdd;
    add.transform = m_imageToScene;

    if (m_background) gameWindow.draw(*m_background, shaded);

    // ---- Window glows -------------------------------------------------------
    if (m_glow)
    {
        for (std::size_t i = 0; i < m_lights.size(); ++i)
        {
            const float fi = static_cast<float>(i);
            const float flicker = 0.85f + 0.15f * std::sin(m_time * 7.f + fi * 1.7f)
                                                * std::sin(m_time * 2.3f + fi);

            m_glow->setPosition(m_lights[i].pos);
            m_glow->setScale({ m_lights[i].radius / 32.f, m_lights[i].radius / 32.f });

            sf::Color c = m_lights[i].color;
            c.a = static_cast<std::uint8_t>(110.f * flicker);
            m_glow->setColor(c);
            gameWindow.draw(*m_glow, add);
        }

        // Moon halo: slow pulse, brighter during lightning
        m_glow->setPosition({ 978.f, 106.f });
        m_glow->setScale({ 2.8f, 2.8f });
        m_glow->setColor(sf::Color(170, 195, 255,
            static_cast<std::uint8_t>(std::clamp(70.f + 15.f * std::sin(m_time * 0.6f)
                                                 + 120.f * m_lightning.flash(), 0.f, 255.f))));
        gameWindow.draw(*m_glow, add);

        // ---- Contact shadows (so the cut-outs sit on the dirt) --------------
        m_glow->setColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(kShadowAlpha)));

        m_glow->setPosition(kZombieShadowPos);
        m_glow->setScale({ kZombieShadowR.x / 32.f, kZombieShadowR.y / 32.f });
        gameWindow.draw(*m_glow, imageSpace);

        m_glow->setPosition(kPeaShadowPos);
        m_glow->setScale({ kPeaShadowR.x / 32.f, kPeaShadowR.y / 32.f });
        gameWindow.draw(*m_glow, imageSpace);
    }

    // ---- Actors: zombie is further back, peashooter in front ----------------

    // if (m_zombie)     gameWindow.draw(*m_zombie,     shadedImageSpace);
    // // if (m_peashooter) gameWindow.draw(*m_peashooter, shadedImageSpace);
    // sf::RenderStates peaStates;
    // peaStates.transform = m_imageToScene;
    // if (m_actorShaderOk) peaStates.shader = &m_actorShader;
    // if (m_peashooter) gameWindow.draw(*m_peashooter, peaStates);
    sf::RenderStates actorStates;
    actorStates.transform = m_imageToScene;
    actorStates.shader    = &m_actorShader;

    m_actorShader.setUniform("texel", sf::Vector2f{ 1.f / 1376.f, 1.f / 768.f });

    m_actorShader.setUniform("grade", 0.f);      // zombie: colours as painted
    if (m_zombie)     gameWindow.draw(*m_zombie, actorStates);

    m_actorShader.setUniform("grade", 1.f);      // peashooter: full grade + outline
    if (m_peashooter) gameWindow.draw(*m_peashooter, actorStates);
    // ---- Shot effects -------------------------------------------------------
    if (m_playing && m_fired && m_glow)
    {
        const float tf = m_playTime - kFire;
        if (tf < 0.3f)
        {
            // Muzzle puff
            const float radius = 22.f + 90.f * tf;
            m_glow->setPosition(m_muzzlePos);
            m_glow->setScale({ radius / 32.f, radius / 32.f });
            m_glow->setColor(sf::Color(150, 255, 90,
                static_cast<std::uint8_t>(210.f * std::exp(-tf * 12.f))));
            gameWindow.draw(*m_glow, add);
        }
    }

    drawPea(gameWindow);
    gameWindow.draw(m_splat, imageSpace);

    // ---- Leaves + rain ------------------------------------------------------
    if (m_leaves)
        gameWindow.draw(*m_leaves, imageSpace);

    gameWindow.draw(m_rain);   // already in 1920x1080 scene space

    // ---- Regular entities (buttons, title, ...) on top ----------------------
    for(auto entity : m_entities.getEntities())
    {
        gameWindow.draw(m_entities.storage<CVisual>().getComponent(entity->getId()).sprite);
    }

    gameWindow.setView(originalView);

    // ---- Fade to black before switching scenes ------------------------------
    if (m_playing && m_playTime > kFadeOutBeg)
    {
        sf::RectangleShape overlay({ kSceneW, kSceneH });
        overlay.setFillColor(sf::Color(0, 0, 0,
            static_cast<std::uint8_t>(255.f * smoothstep(kFadeOutBeg, kEnd, m_playTime))));
        gameWindow.draw(overlay);
    }
}