#pragma once

#include <SFML/Graphics.hpp>

class Game;

class Scene
{
public:
    explicit Scene(Game& game);
    virtual ~Scene() = default;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderTarget& target) = 0;

protected:
    Game& game();

private:
    Game& m_game;
};
