#include "Game.h"

#include "Constants.h"
#include "../Scenes/Scene.h"
#include "../Scenes/GarageScene.h"
#include "../Scenes/MainMenuScene.h"
#include "../Scenes/RaceScene.h"
#include "../Scenes/SettingsScene.h"

#include <SFML/Audio.hpp>
#include <algorithm>
#include <optional>
#include <string>

Game::Game()
    : m_view({Constants::DesignSize.x / 2.f, Constants::DesignSize.y / 2.f}, Constants::DesignSize)
{
    m_settings.load();
    m_assets.load();
    m_database.initialize();

    recreateWindow();
    sf::Listener::setGlobalVolume(m_settings.volume * 100.f);

    setScene(SceneType::MainMenu);
}

Game::~Game() = default;

void Game::run()
{
    sf::Clock clock;

    while (m_window.isOpen())
    {
        processEvents();
        update(clock.restart().asSeconds());
        draw();
    }
}

void Game::close()
{
    m_settings.save();
    applyPendingRaceReward();
    m_database.savePlayer();
    m_window.close();
}

void Game::changeScene(SceneType sceneType)
{
    m_pendingScene = sceneType;
}

void Game::setNextRaceMode(RaceMode mode)
{
    m_nextRaceMode = mode;
}

RaceMode Game::raceMode() const
{
    return m_nextRaceMode;
}

void Game::queueRaceReward(int baseMoney, int baseXp)
{
    if (baseMoney <= 0 && baseXp <= 0)
        return;

    m_pendingRewardMoney += std::max(0, baseMoney);
    m_pendingRewardXp += std::max(0, baseXp);
    m_hasPendingReward = true;
}

void Game::applyPendingRaceReward()
{
    if (!m_hasPendingReward)
        return;

    const int rewardMoney = m_pendingRewardMoney;
    const int rewardXp = m_pendingRewardXp;

    // Clear first, so even if save fails we do not apply the same reward twice.
    m_pendingRewardMoney = 0;
    m_pendingRewardXp = 0;
    m_hasPendingReward = false;

    if (rewardMoney > 0)
        m_database.currentPlayer.addMoney(rewardMoney);

    if (rewardXp > 0)
        m_database.currentPlayer.addExperience(rewardXp);

    m_database.savePlayer();
}

void Game::setScene(SceneType sceneType)
{
    switch (sceneType)
    {
    case SceneType::MainMenu:
        m_currentScene = std::make_unique<MainMenuScene>(*this);
        break;
    case SceneType::Settings:
        m_currentScene = std::make_unique<SettingsScene>(*this);
        break;
    case SceneType::Garage:
        m_currentScene = std::make_unique<GarageScene>(*this);
        break;
    case SceneType::Race:
        m_currentScene = std::make_unique<RaceScene>(*this);
        break;
    }
}

void Game::recreateWindow()
{
    m_settings.refreshAvailableResolutions();

    if (m_settings.fullscreen)
    {
        m_settings.setToMaxResolution();
        m_window.create(
            sf::VideoMode(m_settings.currentResolution()),
            Constants::WindowTitle,
            sf::Style::None,
            sf::State::Fullscreen
        );
    }
    else
    {
        m_window.create(
            sf::VideoMode(m_settings.currentResolution()),
            Constants::WindowTitle,
            sf::Style::Titlebar | sf::Style::Close,
            sf::State::Windowed
        );
    }

    m_window.setFramerateLimit(m_settings.currentFps());
    m_window.setVerticalSyncEnabled(false);
    m_window.setView(m_view);
}

sf::RenderWindow& Game::window()
{
    return m_window;
}

Settings& Game::settings()
{
    return m_settings;
}

AssetManager& Game::assets()
{
    return m_assets;
}

GameData::GameDatabase& Game::database()
{
    return m_database;
}

const GameData::GameDatabase& Game::database() const
{
    return m_database;
}

sf::Vector2f Game::mouseDesignPosition(sf::Vector2i pixelPosition) const
{
    return m_window.mapPixelToCoords(pixelPosition, m_view);
}

void Game::drawBackground(sf::RenderTarget& target, const sf::Texture& texture, sf::Color fallbackColor) const
{
    target.clear(fallbackColor);

    if (texture.getSize().x == 0 || texture.getSize().y == 0)
        return;

    sf::Sprite sprite(texture);
    const sf::Vector2u textureSize = texture.getSize();
    sprite.setScale({
        Constants::DesignSize.x / static_cast<float>(textureSize.x),
        Constants::DesignSize.y / static_cast<float>(textureSize.y)
    });

    target.draw(sprite);
}

void Game::drawText(sf::RenderTarget& target,
                    const std::string& text,
                    sf::Vector2f position,
                    unsigned int characterSize,
                    sf::Color color,
                    bool centered) const
{
    sf::Text drawableText(m_assets.mainFont(), text, characterSize);
    drawableText.setFillColor(color);
    drawableText.setOutlineColor(sf::Color(0, 0, 0, 100));
    drawableText.setOutlineThickness(2.f);

    if (centered)
    {
        const sf::FloatRect bounds = drawableText.getLocalBounds();
        drawableText.setOrigin({
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        });
    }

    drawableText.setPosition(position);
    target.draw(drawableText);
}

void Game::applyPendingSceneChange()
{
    if (!m_pendingScene)
        return;

    const SceneType sceneType = *m_pendingScene;
    m_pendingScene.reset();

    if (sceneType == SceneType::Garage)
        applyPendingRaceReward();

    setScene(sceneType);
}

void Game::handleGlobalEvent(const sf::Event& event)
{
    if (event.is<sf::Event::Closed>())
        close();
}

void Game::processEvents()
{
    while (const std::optional<sf::Event> event = m_window.pollEvent())
    {
        handleGlobalEvent(*event);

        if (m_currentScene)
            m_currentScene->handleEvent(*event);

        applyPendingSceneChange();
    }
}

void Game::update(float dt)
{
    if (m_currentScene)
        m_currentScene->update(dt);

    applyPendingSceneChange();
}

void Game::draw()
{
    m_window.setView(m_view);

    if (m_currentScene)
        m_currentScene->draw(m_window);

    m_window.display();
}
