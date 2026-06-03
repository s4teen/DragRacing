#include "MainMenuScene.h"

#include "../Core/Game.h"
#include "../Core/SceneType.h"

MainMenuScene::MainMenuScene(Game& game)
    : Scene(game)
{
    buildUi();
}

void MainMenuScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->scancode == sf::Keyboard::Scancode::Escape)
            game().close();
    }

    if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (pressed->button != sf::Mouse::Button::Left)
            return;

        const sf::Vector2f mouse = game().mouseDesignPosition(pressed->position);
        for (auto& button : m_buttons)
            button.click(mouse);
    }
}

void MainMenuScene::update(float)
{
    const sf::Vector2f mouse = game().mouseDesignPosition(sf::Mouse::getPosition(game().window()));
    for (auto& button : m_buttons)
        button.update(mouse);
}

void MainMenuScene::draw(sf::RenderTarget& target)
{
    game().drawBackground(target, game().assets().mainMenuBackground(), sf::Color(100, 170, 235));

    for (const auto& button : m_buttons)
        button.draw(target);
}

void MainMenuScene::buildUi()
{
    const sf::Font& font = game().assets().mainFont();
    m_buttons.reserve(3);

    m_buttons.emplace_back(font, "Play", sf::Vector2f{900.f, 445.f}, 52, [&]
    {
        game().changeScene(SceneType::Garage);
    });

    m_buttons.emplace_back(font, "Settings", sf::Vector2f{900.f, 530.f}, 52, [&]
    {
        game().changeScene(SceneType::Settings);
    });

    m_buttons.emplace_back(font, "Exit", sf::Vector2f{900.f, 615.f}, 52, [&]
    {
        game().close();
    });
}
