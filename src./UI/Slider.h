#pragma once

#include <SFML/Graphics.hpp>

class Slider
{
public:
    Slider(sf::Vector2f position = {0.f, 0.f}, float width = 100.f, float value = 1.f);

    void setValue(float value);
    float value() const;

    void handleMouseDown(sf::Vector2f mousePosition);
    void handleMouseUp();
    void handleMouseMove(sf::Vector2f mousePosition);

    void draw(sf::RenderTarget& target) const;

private:
    sf::RectangleShape m_bar;
    sf::CircleShape m_knob;
    float m_value = 1.f;
    bool m_dragging = false;

    bool hit(sf::Vector2f point) const;
    void setByMouse(sf::Vector2f mousePosition);
};
