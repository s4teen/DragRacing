#pragma once

#include "Scene.h"
#include "../Core/RaceMode.h"
#include "../GameData/Car.h"
#include "../Race/RaceVehiclePhysics.h"
#include "../UI/TextButton.h"

#include <SFML/Graphics.hpp>
#include <optional>
#include <random>
#include <string>
#include <vector>

class RaceScene : public Scene
{
public:
    explicit RaceScene(Game& game);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;

private:
    enum class RaceState
    {
        FreeRide,
        Countdown,
        Racing,
        ResultPause
    };

    GameData::Car m_car;
    GameData::Car m_botCar;
    bool m_hasCar = false;
    bool m_hasBot = false;
    RaceMode m_mode = RaceMode::FreeRide;
    RaceState m_state = RaceState::FreeRide;

    Race::RaceVehiclePhysics m_physics;
    Race::RaceVehiclePhysics m_botPhysics;
    std::vector<TextButton> m_buttons;

    sf::View m_screenView;
    sf::View m_worldView;

    float m_raceTime = 0.f;
    float m_stateTimer = 0.f;
    float m_goTextTimer = 0.f;
    float m_botShiftDelayTimer = 0.f;
    float m_botShiftReactionDelay = 0.12f;
    float m_botShiftAtRpm01 = 0.925f;
    float m_botThrottle = 1.f;
    float m_botLaunchMistake = 0.f;

    int m_rewardMoney = 0;
    int m_rewardXp = 0;
    int m_pendingBaseRewardMoney = 0;
    int m_pendingBaseRewardXp = 0;
    bool m_rewardGranted = false;
    bool m_pendingRewardApplied = false;
    std::string m_resultText;

    float m_pixelsPerMeter = 85.f; // визуальная скорость трассы/камеры; больше = быстрее выглядит движение
    sf::Vector2f m_carScreenPosition{530.f, 560.f};
    sf::Vector2f m_botScreenPosition{530.f, 455.f};
    float m_skyboxParallax = 0.035f;
    static constexpr float BotRaceDurationSeconds = 15.f;

    void setupUi();
    void setupCar();
    void setupBot();
    void startRace();
    void retryRace();
    void finishBotRace();
    void grantPendingRewardAndGoGarage();

    Race::RaceVehicleInput readInput() const;
    Race::RaceVehicleInput readBotInput(float dt);
    void applyPlayerSkillBonuses();

    void drawSkybox(sf::RenderTarget& target);
    void drawTrack(sf::RenderTarget& target);
    void drawCar(sf::RenderTarget& target);
    void drawVehicle(sf::RenderTarget& target, const GameData::Car& car, const Race::RaceVehiclePhysics& physics, float bodyCenterY);
    void drawDashboard(sf::RenderTarget& target);
    void drawDebugHints(sf::RenderTarget& target);
    void drawRaceOverlay(sf::RenderTarget& target);

    void drawText(sf::RenderTarget& target,
                  const sf::Font& font,
                  const std::string& text,
                  sf::Vector2f position,
                  unsigned int size,
                  bool centered = false,
                  sf::Color color = sf::Color::White,
                  float outline = 2.f);
    void drawFallbackCar(sf::RenderTarget& target, sf::Vector2f bodyCenter, sf::Vector2f bodySize, float rotationDegrees);

    std::string gearText() const;
    std::string timeText() const;
    float carWorldX() const;
    float botWorldX() const;
};
