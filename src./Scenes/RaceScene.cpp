#include "RaceScene.h"

#include "../Core/AssetManager.h"
#include "../Core/Constants.h"
#include "../Core/Game.h"
#include "../Core/SceneType.h"
#include "../Core/Settings.h"
#include "../GameData/GameDatabase.h"
#include "../GameData/Part.h"
#include "../GameData/Player.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>

namespace
{
    constexpr float Pi = 3.1415926535f;

    sf::Vector2f rotateOffset(sf::Vector2f offset, float degrees)
    {
        const float radians = degrees * Pi / 180.f;
        const float cosA = std::cos(radians);
        const float sinA = std::sin(radians);
        return {
            offset.x * cosA - offset.y * sinA,
            offset.x * sinA + offset.y * cosA
        };
    }

    float randomFloat(float minValue, float maxValue)
    {
        static std::random_device randomDevice;
        static std::mt19937 generator(randomDevice());
        std::uniform_real_distribution<float> distribution(minValue, maxValue);
        return distribution(generator);
    }

    int applyPositivePercentBonusPreview(int amount, int percent)
    {
        if (amount <= 0 || percent <= 0)
            return amount;

        return static_cast<int>(std::round(static_cast<double>(amount) * (1.0 + static_cast<double>(percent) / 100.0)));
    }
}

RaceScene::RaceScene(Game& game)
    : Scene(game),
      m_mode(game.raceMode()),
      m_state(m_mode == RaceMode::AgainstBot ? RaceState::Countdown : RaceState::FreeRide),
      m_screenView({Constants::DesignSize.x / 2.f, Constants::DesignSize.y / 2.f}, Constants::DesignSize),
      m_worldView({Constants::DesignSize.x / 2.f, Constants::DesignSize.y / 2.f}, Constants::DesignSize)
{
    this->game().assets().reloadRaceSkybox();
    setupCar();
    setupUi();

    if (m_mode == RaceMode::AgainstBot)
        startRace();
}

void RaceScene::setupUi()
{
    const sf::Font& font = game().assets().mainFont();
    m_buttons.emplace_back(font, "GARAGE", sf::Vector2f{104.f, 48.f}, 32, [this]
    {
        game().changeScene(SceneType::Garage);
    });
}

void RaceScene::setupCar()
{
    const GameData::Car* selectedCar = game().database().currentPlayer.getSelectedCar();
    if (!selectedCar)
        return;

    m_car = *selectedCar;
    m_hasCar = true;
    m_physics.setup(m_car, game().database());
    applyPlayerSkillBonuses();

    const float visualWheelRadiusPixels = std::max(1.f, m_car.wheelSize.x * m_car.carScale.x * 0.5f);
    m_physics.setVisualTuning(m_pixelsPerMeter, visualWheelRadiusPixels);

    if (m_mode == RaceMode::AgainstBot)
        setupBot();
}

void RaceScene::setupBot()
{
    if (!m_hasCar)
        return;

    m_botCar = m_car;
    m_hasBot = true;
    m_botPhysics.setup(m_botCar, game().database());

    // Bot is the same car, but with a small random tuning/driver spread.
    const float powerMultiplier = randomFloat(0.94f, 1.08f);
    const float gripMultiplier = randomFloat(0.93f, 1.09f);
    const float massMultiplier = randomFloat(0.97f, 1.04f);
    const float shiftMultiplier = randomFloat(0.84f, 1.22f);
    m_botPhysics.applyPerformanceMultipliers(powerMultiplier, gripMultiplier, massMultiplier, shiftMultiplier);

    const float visualWheelRadiusPixels = std::max(1.f, m_botCar.wheelSize.x * m_botCar.carScale.x * 0.5f);
    m_botPhysics.setVisualTuning(m_pixelsPerMeter, visualWheelRadiusPixels);

    m_botShiftReactionDelay = randomFloat(0.05f, 0.22f);
    m_botShiftAtRpm01 = randomFloat(0.90f, 0.965f);
    m_botThrottle = randomFloat(0.96f, 1.0f);
    m_botLaunchMistake = randomFloat(0.f, 0.18f);
}

void RaceScene::applyPlayerSkillBonuses()
{
    const GameData::Player& player = game().database().currentPlayer;
    const float powerMultiplier = 1.f + static_cast<float>(player.powerBonus) * 0.01f;
    const float gripMultiplier = 1.f + static_cast<float>(player.gripBonus) * 0.01f;
    m_physics.applyPerformanceMultipliers(powerMultiplier, gripMultiplier, 1.f, 1.f);
}

void RaceScene::startRace()
{
    m_state = RaceState::Countdown;
    m_stateTimer = 0.f;
    m_goTextTimer = 0.f;
    m_raceTime = 0.f;
    m_rewardMoney = 0;
    m_rewardXp = 0;
    m_pendingBaseRewardMoney = 0;
    m_pendingBaseRewardXp = 0;
    m_rewardGranted = false;
    m_pendingRewardApplied = false;
    m_resultText.clear();
    m_botShiftDelayTimer = 0.f;

    if (m_hasCar)
    {
        m_physics.setup(m_car, game().database());
        applyPlayerSkillBonuses();
        const float visualWheelRadiusPixels = std::max(1.f, m_car.wheelSize.x * m_car.carScale.x * 0.5f);
        m_physics.setVisualTuning(m_pixelsPerMeter, visualWheelRadiusPixels);
    }

    setupBot();
}

void RaceScene::retryRace()
{
    startRace();
}

void RaceScene::finishBotRace()
{
    if (m_rewardGranted)
        return;

    const float playerDistance = m_physics.distanceMeters();
    const float botDistance = m_botPhysics.distanceMeters();
    const float difference = playerDistance - botDistance;

    int baseMoney = 120;
    int baseXp = 35;

    if (difference > 1.5f)
    {
        m_resultText = "WINNER!";
        baseMoney = 650;
        baseXp = 160;
    }
    else if (difference < -1.5f)
    {
        m_resultText = "LOSER!";
        baseMoney = 120;
        baseXp = 35;
    }
    else
    {
        m_resultText = "DRAW!";
        baseMoney = 300;
        baseXp = 80;
    }

    const GameData::Player& player = game().database().currentPlayer;
    m_pendingBaseRewardMoney = baseMoney;
    m_pendingBaseRewardXp = baseXp;
    m_rewardMoney = applyPositivePercentBonusPreview(baseMoney, player.moneyBonus);
    m_rewardXp = applyPositivePercentBonusPreview(baseXp, player.experienceBonus);

    m_rewardGranted = true;
    m_pendingRewardApplied = false;
    m_state = RaceState::ResultPause;
    m_stateTimer = 0.f;
}

void RaceScene::grantPendingRewardAndGoGarage()
{
    if (!m_pendingRewardApplied && m_rewardGranted)
    {
        game().queueRaceReward(m_pendingBaseRewardMoney, m_pendingBaseRewardXp);
        m_pendingRewardApplied = true;
    }

    game().changeScene(SceneType::Garage);
}

void RaceScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        const bool canDrive = m_state != RaceState::Countdown;

        if (key->scancode == sf::Keyboard::Scancode::Escape)
        {
            if (m_state == RaceState::ResultPause)
                grantPendingRewardAndGoGarage();
            else
                game().changeScene(SceneType::Garage);
        }
        else if (canDrive && key->scancode == sf::Keyboard::Scancode::E)
            m_physics.shiftUp();
        else if (canDrive && key->scancode == sf::Keyboard::Scancode::Q)
            m_physics.shiftDown();
    }

    if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (pressed->button == sf::Mouse::Button::Left)
        {
            const sf::Vector2f mouse = game().mouseDesignPosition(pressed->position);
            if (m_state != RaceState::ResultPause)
            {
                for (TextButton& button : m_buttons)
                    button.click(mouse);
            }
        }
    }
}

void RaceScene::update(float dt)
{
    dt = std::clamp(dt, 0.f, 1.f / 20.f);

    const sf::Vector2f mouse = game().mouseDesignPosition(sf::Mouse::getPosition(game().window()));
    for (TextButton& button : m_buttons)
        button.update(mouse);

    if (!m_hasCar)
        return;

    if (m_state == RaceState::Countdown)
    {
        m_stateTimer += dt;
        if (m_stateTimer >= 3.f)
        {
            m_state = RaceState::Racing;
            m_stateTimer = 0.f;
            m_goTextTimer = 0.75f;
        }
        return;
    }

    // В FreeRide, Racing и после результата физика продолжает обновляться одинаково.
    // После 15 секунд мы только фиксируем результат/награду и показываем UI поверх сцены.
    m_raceTime += dt;

    if (m_goTextTimer > 0.f)
        m_goTextTimer = std::max(0.f, m_goTextTimer - dt);

    m_physics.update(dt, readInput());
    if (m_hasBot)
        m_botPhysics.update(dt, readBotInput(dt));

    if (m_state == RaceState::Racing && m_raceTime >= BotRaceDurationSeconds)
    {
        finishBotRace();
        return;
    }

    if (m_state == RaceState::ResultPause)
    {
        m_stateTimer += dt;
        if (m_stateTimer >= 3.0f)
            grantPendingRewardAndGoGarage();
    }
}

Race::RaceVehicleInput RaceScene::readInput() const
{
    Race::RaceVehicleInput input;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        input.throttle = 1.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        input.brake = 1.f;

    return input;
}

Race::RaceVehicleInput RaceScene::readBotInput(float dt)
{
    Race::RaceVehicleInput input;
    input.throttle = std::max(0.f, m_botThrottle - m_botLaunchMistake * std::max(0.f, 1.f - m_raceTime));

    if (!m_botPhysics.isShifting() && m_botPhysics.gear() > 0 && m_botPhysics.gear() < m_botPhysics.maxGear())
    {
        const float rpm01 = m_botPhysics.rpm() / std::max(1.f, m_botPhysics.maxRpm());
        if (rpm01 >= m_botShiftAtRpm01)
        {
            if (m_botShiftDelayTimer <= 0.f)
                m_botShiftDelayTimer = m_botShiftReactionDelay;
        }
    }

    if (m_botShiftDelayTimer > 0.f)
    {
        m_botShiftDelayTimer = std::max(0.f, m_botShiftDelayTimer - dt);
        if (m_botShiftDelayTimer <= 0.f)
            m_botPhysics.shiftUp();
    }

    return input;
}

void RaceScene::draw(sf::RenderTarget& target)
{
    target.setView(m_screenView);
    drawSkybox(target);

    const float desiredCarScreenX = m_carScreenPosition.x + m_physics.cameraOffsetX();
    const float cameraX = carWorldX() - desiredCarScreenX + Constants::DesignSize.x / 2.f;
    m_worldView.setCenter({cameraX, Constants::DesignSize.y / 2.f});

    target.setView(m_worldView);
    drawTrack(target);
    if (m_hasBot)
        drawVehicle(target, m_botCar, m_botPhysics, m_botScreenPosition.y);
    drawCar(target);

    target.setView(m_screenView);
    drawDashboard(target);
    drawDebugHints(target);
    drawRaceOverlay(target);

    if (m_state != RaceState::ResultPause)
    {
        for (TextButton& button : m_buttons)
            button.draw(target);
    }
}

void RaceScene::drawSkybox(sf::RenderTarget& target)
{
    target.clear(sf::Color(35, 45, 55));

    const sf::Texture& texture = game().assets().raceSkybox();
    if (texture.getSize().x == 0 || texture.getSize().y == 0)
        return;

    const float scale = Constants::DesignSize.y / static_cast<float>(texture.getSize().y);
    const float tileWidth = static_cast<float>(texture.getSize().x) * scale;
    if (tileWidth <= 1.f)
        return;

    const float offset = std::fmod(carWorldX() * m_skyboxParallax, tileWidth);
    for (int i = -1; i <= static_cast<int>(Constants::DesignSize.x / tileWidth) + 2; ++i)
    {
        sf::Sprite sprite(texture);
        sprite.setScale({scale, scale});
        sprite.setPosition({i * tileWidth - offset, 0.f});
        target.draw(sprite);
    }
}

void RaceScene::drawTrack(sf::RenderTarget& target)
{
    const sf::Texture& texture = game().assets().raceRoadTile();
    const float viewLeft = m_worldView.getCenter().x - Constants::DesignSize.x / 2.f;

    if (texture.getSize().x == 0 || texture.getSize().y == 0)
    {
        sf::RectangleShape fallback({Constants::DesignSize.x * 3.f, 430.f});
        fallback.setPosition({viewLeft - Constants::DesignSize.x, 585.f});
        fallback.setFillColor(sf::Color(70, 70, 70));
        target.draw(fallback);
        return;
    }

    const float scale = Constants::DesignSize.y / static_cast<float>(texture.getSize().y);
    const float tileWidth = static_cast<float>(texture.getSize().x) * scale;
    const int firstTile = static_cast<int>(std::floor(viewLeft / tileWidth)) - 1;
    const int count = static_cast<int>(Constants::DesignSize.x / tileWidth) + 4;

    for (int i = 0; i < count; ++i)
    {
        sf::Sprite sprite(texture);
        sprite.setScale({scale, scale});
        sprite.setPosition({(firstTile + i) * tileWidth, 0.f});
        target.draw(sprite);
    }
}

void RaceScene::drawCar(sf::RenderTarget& target)
{
    if (!m_hasCar)
        return;

    drawVehicle(target, m_car, m_physics, m_carScreenPosition.y);
}

void RaceScene::drawVehicle(sf::RenderTarget& target, const GameData::Car& car, const Race::RaceVehiclePhysics& physics, float bodyCenterY)
{
    const sf::Vector2f scale = car.carScale;
    const sf::Vector2f bodySize{car.bodySize.x * scale.x, car.bodySize.y * scale.y};
    const sf::Vector2f wheelSize{car.wheelSize.x * scale.x, car.wheelSize.y * scale.y};
    const sf::Vector2f bodyCenter{physics.distanceMeters() * m_pixelsPerMeter, bodyCenterY};
    const sf::Vector2f frontWheelPos = bodyCenter + sf::Vector2f{car.frontWheelPos.x * scale.x, car.frontWheelPos.y * scale.y};
    const sf::Vector2f rearWheelPos = bodyCenter + sf::Vector2f{car.rearWheelPos.x * scale.x, car.rearWheelPos.y * scale.y};
    const float bodyRotation = car.bodyRotationDegrees + physics.bodyDynamicRotationDegrees();

    const GameData::Part* tirePart = car.getPart(GameData::PartType::Tires, game().database());
    const std::string wheelPath = tirePart ? tirePart->wheelSpritePath : "";
    const sf::Texture* wheelTexture = game().assets().texture(wheelPath);

    const auto drawWheel = [&](sf::Vector2f center)
    {
        if (wheelTexture)
        {
            sf::Sprite wheel(*wheelTexture);
            const sf::Vector2u textureSize = wheelTexture->getSize();
            wheel.setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});
            wheel.setScale({wheelSize.x / static_cast<float>(textureSize.x), wheelSize.y / static_cast<float>(textureSize.y)});
            wheel.setPosition(center);
            wheel.setRotation(sf::degrees(physics.wheelRotationDegrees()));
            target.draw(wheel);
        }
        else
        {
            sf::CircleShape wheel(wheelSize.x / 2.f);
            wheel.setOrigin({wheelSize.x / 2.f, wheelSize.x / 2.f});
            wheel.setPosition(center);
            wheel.setRotation(sf::degrees(physics.wheelRotationDegrees()));
            wheel.setFillColor(sf::Color(20, 20, 20));
            wheel.setOutlineColor(sf::Color(180, 180, 180));
            wheel.setOutlineThickness(4.f);
            target.draw(wheel);
        }
    };

    drawWheel(rearWheelPos);
    drawWheel(frontWheelPos);

    if (const sf::Texture* bodyTexture = game().assets().texture(car.bodySpritePath))
    {
        sf::Sprite body(*bodyTexture);
        const sf::Vector2u textureSize = bodyTexture->getSize();
        body.setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});
        body.setScale({bodySize.x / static_cast<float>(textureSize.x), bodySize.y / static_cast<float>(textureSize.y)});
        body.setPosition(bodyCenter);
        body.setRotation(sf::degrees(bodyRotation));
        target.draw(body);
    }
    else
    {
        drawFallbackCar(target, bodyCenter, bodySize, bodyRotation);
    }
}

void RaceScene::drawDashboard(sf::RenderTarget& target)
{
    const sf::Texture& dashboard = game().assets().dashboardTexture();
    const sf::Vector2f panelPos{1115.f, 757.f};
    const sf::Vector2f panelSize{635.f, 232.f};

    if (dashboard.getSize().x != 0 && dashboard.getSize().y != 0)
    {
        sf::Sprite sprite(dashboard);
        sprite.setPosition(panelPos);
        sprite.setScale({panelSize.x / static_cast<float>(dashboard.getSize().x), panelSize.y / static_cast<float>(dashboard.getSize().y)});
        target.draw(sprite);
    }
    else
    {
        sf::RectangleShape fallback(panelSize);
        fallback.setPosition(panelPos);
        fallback.setFillColor(sf::Color(0, 0, 0, 150));
        target.draw(fallback);
    }

    const sf::Font& font = game().assets().secondFont();
    const bool metric = game().settings().metric;
    const int speed = static_cast<int>(std::round(metric ? m_physics.speedKmh() : m_physics.speedMph()));
    const int rpm = static_cast<int>(std::round(m_physics.rpm()));
    const std::string speedUnit = metric ? "km/h" : "mi/h";

    const sf::Color labelColor(220, 220, 220);
    const sf::Vector2f p = panelPos;

    drawText(target, font, "SPEED", {p.x + 72.f, p.y + 32.f}, 16, true, labelColor, 1.f);
    drawText(target, font, std::to_string(speed), {p.x + 166.f, p.y + 70.f}, 70, true);
    drawText(target, font, speedUnit, {p.x + 292.f, p.y + 100.f}, 15, true, labelColor, 1.f);

    drawText(target, font, "RPM", {p.x + 570.f, p.y + 32.f}, 16, true, labelColor, 1.f);
    drawText(target, font, std::to_string(rpm), {p.x + 458.f, p.y + 49.f}, 38, true);

    drawText(target, font, "TIME", {p.x + 292.f, p.y + 129.f}, 16, true, labelColor, 1.f);
    drawText(target, font, timeText(), {p.x + 166.f, p.y + 166.f}, 40, true);

    drawText(target, font, "GEAR", {p.x + 342.f, p.y + 198.f}, 16, true, labelColor, 1.f);
    drawText(target, font, gearText(), {p.x + 470.f, p.y + 144.f}, 92, true);
}

void RaceScene::drawDebugHints(sf::RenderTarget& target)
{
    if (m_physics.isShifting())
        drawText(target, game().assets().mainFont(), "SHIFTING", {900.f, 80.f}, 36, true, sf::Color(255, 220, 80));

    if (m_physics.slip01() > 0.08f)
        drawText(target, game().assets().mainFont(), "WHEEL SLIP", {900.f, 128.f}, 28, true, sf::Color(255, 105, 75));

    drawText(target, game().assets().mainFont(), "W/UP - gas   S/DOWN - brake   E/Q - shift   ESC - garage", {500.f, 982.f}, 20, true, sf::Color(230, 230, 230), 1.f);
}

void RaceScene::drawRaceOverlay(sf::RenderTarget& target)
{
    if (m_state == RaceState::Countdown)
    {
        const int number = std::max(1, 3 - static_cast<int>(std::floor(m_stateTimer)));
        drawText(target, game().assets().mainFont(), std::to_string(number), {Constants::DesignSize.x / 2.f, 270.f}, 110, true, sf::Color(255, 240, 170), 4.f);
        return;
    }

    if (m_goTextTimer > 0.f)
        drawText(target, game().assets().mainFont(), "GO!", {Constants::DesignSize.x / 2.f, 270.f}, 96, true, sf::Color(120, 255, 120), 4.f);

    if (m_state == RaceState::ResultPause)
    {
        sf::RectangleShape panel({680.f, 350.f});
        panel.setOrigin({340.f, 175.f});
        panel.setPosition({Constants::DesignSize.x / 2.f, 515.f});
        panel.setFillColor(sf::Color(0, 0, 0, 175));
        target.draw(panel);

        sf::Color resultColor = sf::Color::White;
        if (m_resultText == "WINNER!")
            resultColor = sf::Color(120, 255, 120);
        else if (m_resultText == "LOSER!")
            resultColor = sf::Color(255, 110, 90);
        else if (m_resultText == "DRAW!")
            resultColor = sf::Color(255, 220, 110);

        drawText(target, game().assets().mainFont(), m_resultText, {Constants::DesignSize.x / 2.f, 420.f}, 72, true, resultColor, 4.f);
        drawText(target, game().assets().mainFont(), std::to_string(m_rewardMoney) + "$", {Constants::DesignSize.x / 2.f, 505.f}, 40, true, sf::Color(240, 240, 240), 3.f);
        drawText(target, game().assets().mainFont(), std::to_string(m_rewardXp) + "xp", {Constants::DesignSize.x / 2.f, 560.f}, 40, true, sf::Color(240, 240, 240), 3.f);
    }
}

void RaceScene::drawText(sf::RenderTarget& target,
                         const sf::Font& font,
                         const std::string& text,
                         sf::Vector2f position,
                         unsigned int size,
                         bool centered,
                         sf::Color color,
                         float outline)
{
    sf::Text drawableText(font, text, size);
    drawableText.setFillColor(color);
    drawableText.setOutlineColor(sf::Color(0, 0, 0, 150));
    drawableText.setOutlineThickness(outline);

    if (centered)
    {
        const sf::FloatRect bounds = drawableText.getLocalBounds();
        drawableText.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
    }

    drawableText.setPosition(position);
    target.draw(drawableText);
}

void RaceScene::drawFallbackCar(sf::RenderTarget& target, sf::Vector2f bodyCenter, sf::Vector2f bodySize, float rotationDegrees)
{
    sf::RectangleShape body(bodySize);
    body.setOrigin({bodySize.x / 2.f, bodySize.y / 2.f});
    body.setPosition(bodyCenter);
    body.setRotation(sf::degrees(rotationDegrees));
    body.setFillColor(sf::Color(230, 230, 230, 230));
    body.setOutlineColor(sf::Color(40, 40, 40, 240));
    body.setOutlineThickness(5.f);
    target.draw(body);

    sf::RectangleShape cabin({bodySize.x * 0.34f, bodySize.y * 0.33f});
    cabin.setOrigin({bodySize.x * 0.17f, bodySize.y * 0.165f});
    cabin.setPosition(bodyCenter + rotateOffset({bodySize.x * 0.06f, -bodySize.y * 0.25f}, rotationDegrees));
    cabin.setRotation(sf::degrees(rotationDegrees));
    cabin.setFillColor(sf::Color(90, 150, 170, 220));
    cabin.setOutlineColor(sf::Color(30, 30, 30, 230));
    cabin.setOutlineThickness(4.f);
    target.draw(cabin);
}

std::string RaceScene::gearText() const
{
    if (m_physics.gear() <= 0)
        return "N";
    return std::to_string(m_physics.gear());
}

std::string RaceScene::timeText() const
{
    const int minutes = static_cast<int>(m_raceTime / 60.f);
    const int seconds = static_cast<int>(m_raceTime) % 60;
    const int millis = static_cast<int>((m_raceTime - std::floor(m_raceTime)) * 1000.f);

    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(2) << minutes << ':'
           << std::setw(2) << seconds << ','
           << std::setw(3) << millis;
    return stream.str();
}

float RaceScene::carWorldX() const
{
    return m_physics.distanceMeters() * m_pixelsPerMeter;
}

float RaceScene::botWorldX() const
{
    return m_botPhysics.distanceMeters() * m_pixelsPerMeter;
}
