#include "SettingsScene.h"

#include "../Core/Game.h"
#include "../Core/SceneType.h"
#include "../Utils/Format.h"

#include <SFML/Audio.hpp>

namespace
{
    constexpr float LabelX = 690.f;
    constexpr float LeftButtonX = 975.f;
    constexpr float ValueX = 1110.f;
    constexpr float RightButtonX = 1250.f;
    constexpr float ToggleX = 1125.f;

    constexpr float ResolutionRowY = 300.f;
    constexpr float RefreshRateRowY = 365.f;
    constexpr float FullscreenRowY = 430.f;
    constexpr float VolumeRowY = 505.f;
    constexpr float MetricRowY = 585.f;

    constexpr float LabelTopOffset = 28.f;
}

SettingsScene::SettingsScene(Game& game)
    : Scene(game),
      m_volumeSlider({975.f, VolumeRowY}, 300.f, game.settings().volume)
{
    buildUi();
    refreshControls();
}

void SettingsScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->scancode == sf::Keyboard::Scancode::Escape)
            game().changeScene(SceneType::MainMenu);
    }

    if (const auto* moved = event.getIf<sf::Event::MouseMoved>())
    {
        const sf::Vector2f mouse = game().mouseDesignPosition(moved->position);
        m_volumeSlider.handleMouseMove(mouse);
    }

    if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (pressed->button != sf::Mouse::Button::Left)
            return;

        const sf::Vector2f mouse = game().mouseDesignPosition(pressed->position);
        m_volumeSlider.handleMouseDown(mouse);

        for (auto& button : m_buttons)
            button.click(mouse);
    }

    if (event.is<sf::Event::MouseButtonReleased>())
    {
        m_volumeSlider.handleMouseUp();
        saveVolume();
    }
}

void SettingsScene::update(float)
{
    const sf::Vector2f mouse = game().mouseDesignPosition(sf::Mouse::getPosition(game().window()));

    for (auto& button : m_buttons)
        button.update(mouse);

    game().settings().volume = m_volumeSlider.value();
    sf::Listener::setGlobalVolume(game().settings().volume * 100.f);
}

void SettingsScene::draw(sf::RenderTarget& target)
{
    Settings& settings = game().settings();

    game().drawBackground(target, game().assets().settingsBackground(), sf::Color(100, 170, 235));

    const sf::Color disabledColor(135, 135, 135);
    const sf::Color resolutionColor = settings.fullscreen ? disabledColor : sf::Color::White;

    game().drawText(target, "Resolution:", {LabelX, ResolutionRowY - LabelTopOffset}, 38, resolutionColor);
    game().drawText(target, Format::resolution(settings.currentResolution()), {ValueX, ResolutionRowY}, 38, resolutionColor, true);

    game().drawText(target, "Refresh Rate:", {LabelX, RefreshRateRowY - LabelTopOffset}, 38);
    game().drawText(target, Format::fps(settings.currentFps()), {ValueX, RefreshRateRowY}, 38, sf::Color::White, true);

    game().drawText(target, "Fullscreen:", {LabelX, FullscreenRowY - LabelTopOffset}, 42);

    game().drawText(target, "Volume:", {LabelX, VolumeRowY - LabelTopOffset}, 42);
    game().drawText(target, Format::percent(settings.volume), {1310.f, VolumeRowY - 18.f}, 32);
    m_volumeSlider.draw(target);

    game().drawText(target, "Metric:", {LabelX, MetricRowY - LabelTopOffset}, 42);

    for (const auto& button : m_buttons)
        button.draw(target);
}

void SettingsScene::buildUi()
{
    const sf::Font& font = game().assets().mainFont();
    m_buttons.reserve(7);

    m_buttons.emplace_back(font, "<", sf::Vector2f{675.f, 195.f}, 34, [&]
    {
        game().changeScene(SceneType::MainMenu);
    });

    m_buttons.emplace_back(font, "<", sf::Vector2f{LeftButtonX, ResolutionRowY}, 34, [&]
    {
        changeResolution(-1);
    });
    m_resolutionLeft = &m_buttons.back();

    m_buttons.emplace_back(font, ">", sf::Vector2f{RightButtonX, ResolutionRowY}, 34, [&]
    {
        changeResolution(1);
    });
    m_resolutionRight = &m_buttons.back();

    m_buttons.emplace_back(font, "<", sf::Vector2f{LeftButtonX, RefreshRateRowY}, 34, [&]
    {
        changeFps(-1);
    });

    m_buttons.emplace_back(font, ">", sf::Vector2f{RightButtonX, RefreshRateRowY}, 34, [&]
    {
        changeFps(1);
    });

    m_buttons.emplace_back(font, "OFF", sf::Vector2f{ToggleX, FullscreenRowY}, 36, [&]
    {
        toggleFullscreen();
    });
    m_fullscreenToggle = &m_buttons.back();

    m_buttons.emplace_back(font, "OFF", sf::Vector2f{ToggleX, MetricRowY}, 36, [&]
    {
        toggleMetric();
    });
    m_metricToggle = &m_buttons.back();
}

void SettingsScene::refreshControls()
{
    Settings& settings = game().settings();

    if (m_resolutionLeft)
        m_resolutionLeft->setEnabled(!settings.fullscreen);
    if (m_resolutionRight)
        m_resolutionRight->setEnabled(!settings.fullscreen);

    if (m_fullscreenToggle)
        m_fullscreenToggle->setLabel(settings.fullscreen ? "ON" : "OFF");
    if (m_metricToggle)
        m_metricToggle->setLabel(settings.metric ? "ON" : "OFF");

    m_volumeSlider.setValue(settings.volume);
}

void SettingsScene::changeResolution(int delta)
{
    Settings& settings = game().settings();
    if (settings.fullscreen || settings.resolutions.empty())
        return;

    settings.resolutionIndex = (settings.resolutionIndex + delta + static_cast<int>(settings.resolutions.size())) % static_cast<int>(settings.resolutions.size());
    settings.save();

    game().recreateWindow();
    refreshControls();
}

void SettingsScene::changeFps(int delta)
{
    Settings& settings = game().settings();
    settings.fpsIndex = (settings.fpsIndex + delta + static_cast<int>(settings.fpsLimits.size())) % static_cast<int>(settings.fpsLimits.size());
    settings.save();

    game().window().setFramerateLimit(settings.currentFps());
}

void SettingsScene::toggleFullscreen()
{
    Settings& settings = game().settings();
    settings.fullscreen = !settings.fullscreen;

    if (settings.fullscreen)
        settings.setToMaxResolution();

    game().recreateWindow();
    settings.save();
    refreshControls();
}

void SettingsScene::toggleMetric()
{
    Settings& settings = game().settings();
    settings.metric = !settings.metric;
    settings.save();

    refreshControls();
}

void SettingsScene::saveVolume()
{
    Settings& settings = game().settings();
    settings.volume = m_volumeSlider.value();
    sf::Listener::setGlobalVolume(settings.volume * 100.f);
    settings.save();
}
