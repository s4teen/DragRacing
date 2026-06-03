#include "Slider.h"

#include <algorithm>

Slider::Slider(sf::Vector2f position, float width, float value)
    : m_bar({width, 8.f}),
      m_knob(16.f)
{
    m_bar.setOrigin({0.f, 4.f});
    m_bar.setPosition(position);
    m_bar.setFillColor(sf::Color(180, 180, 180));

    m_knob.setOrigin({16.f, 16.f});
    m_knob.setFillColor(sf::Color::White);

    setValue(value);
}

void Slider::setValue(float value)
{
    m_value = std::clamp(value, 0.f, 1.f);

    const sf::FloatRect bounds = m_bar.getGlobalBounds();
    m_knob.setPosition({
        bounds.position.x + bounds.size.x * m_value,
        bounds.position.y + bounds.size.y / 2.f
    });
}

float Slider::value() const
{
    return m_value;
}

void Slider::handleMouseDown(sf::Vector2f mousePosition)
{
    if (!hit(mousePosition))
        return;

    m_dragging = true;
    setByMouse(mousePosition);
}

void Slider::handleMouseUp()
{
    m_dragging = false;
}

void Slider::handleMouseMove(sf::Vector2f mousePosition)
{
    if (m_dragging)
        setByMouse(mousePosition);
}

void Slider::draw(sf::RenderTarget& target) const
{
    target.draw(m_bar);
    target.draw(m_knob);
}

bool Slider::hit(sf::Vector2f point) const
{
    sf::FloatRect bounds = m_bar.getGlobalBounds();
    bounds.position.y -= 20.f;
    bounds.size.y += 40.f;

    return bounds.contains(point) || m_knob.getGlobalBounds().contains(point);
}

void Slider::setByMouse(sf::Vector2f mousePosition)
{
    const sf::FloatRect bounds = m_bar.getGlobalBounds();
    setValue((mousePosition.x - bounds.position.x) / bounds.size.x);
}
