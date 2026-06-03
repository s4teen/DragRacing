#pragma once

#include "AssetManager.h"
#include "Settings.h"
#include "SceneType.h"
#include "RaceMode.h"
#include "../GameData/GameDatabase.h"

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>

class Scene;

class Game
{
public:
    Game();
    ~Game();

    void run();
    void close();
    void changeScene(SceneType sceneType);
    void setNextRaceMode(RaceMode mode);
    RaceMode raceMode() const;
    void queueRaceReward(int baseMoney, int baseXp);
    void applyPendingRaceReward();
    void recreateWindow();

    sf::RenderWindow& window();
    Settings& settings();
    AssetManager& assets();
    GameData::GameDatabase& database();
    const GameData::GameDatabase& database() const;

    sf::Vector2f mouseDesignPosition(sf::Vector2i pixelPosition) const;

    void drawBackground(sf::RenderTarget& target, const sf::Texture& texture, sf::Color fallbackColor) const;
    void drawText(sf::RenderTarget& target,
                  const std::string& text,
                  sf::Vector2f position,
                  unsigned int characterSize,
                  sf::Color color = sf::Color::White,
                  bool centered = false) const;

private:
    Settings m_settings;
    AssetManager m_assets;
    GameData::GameDatabase m_database;
    sf::RenderWindow m_window;
    sf::View m_view;
    std::unique_ptr<Scene> m_currentScene;
    std::optional<SceneType> m_pendingScene;
    RaceMode m_nextRaceMode = RaceMode::FreeRide;
    int m_pendingRewardMoney = 0;
    int m_pendingRewardXp = 0;
    bool m_hasPendingReward = false;

    void setScene(SceneType sceneType);
    void applyPendingSceneChange();
    void handleGlobalEvent(const sf::Event& event);
    void processEvents();
    void update(float dt);
    void draw();
};
