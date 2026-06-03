#include "TextButton.h"

#include <utility>

TextButton::TextButton(const sf::Font& font,
                       const std::string& label,
                       sf::Vector2f position,
                       unsigned int characterSize,
                       std::function<void()> callback)
    : m_text(font, label, characterSize),
      m_callback(std::move(callback))
{
    m_text.setFillColor(m_defaultColor);
    m_text.setOutlineColor(sf::Color(0, 0, 0, 90));
    m_text.setOutlineThickness(2.f);
    centerOrigin();
    m_text.setPosition(position);
}

void TextButton::setLabel(const std::string& label)
{
    const sf::Vector2f oldPosition = m_text.getPosition();
    m_text.setString(label);
    centerOrigin();
    m_text.setPosition(oldPosition);
}

void TextButton::setEnabled(bool enabled)
{
    m_enabled = enabled;
    m_text.setFillColor(m_enabled ? m_defaultColor : m_disabledColor);
}

void TextButton::update(sf::Vector2f mousePosition)
{
    if (!m_enabled)
    {
        m_text.setFillColor(m_disabledColor);
        m_text.setScale({1.f, 1.f});
        return;
    }

    const bool hovered = contains(mousePosition);
    m_text.setFillColor(hovered ? m_hoverColor : m_defaultColor);
    m_text.setScale(hovered ? sf::Vector2f{1.04f, 1.04f} : sf::Vector2f{1.f, 1.f});
}

void TextButton::click(sf::Vector2f mousePosition)
{
    if (m_enabled && contains(mousePosition) && m_callback)
        m_callback();
}

void TextButton::draw(sf::RenderTarget& target) const
{
    target.draw(m_text);
}

void TextButton::centerOrigin()
{
    const sf::FloatRect bounds = m_text.getLocalBounds();
    m_text.setOrigin({
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f
    });
}

bool TextButton::contains(sf::Vector2f point) const
{
    sf::FloatRect bounds = m_text.getGlobalBounds();
    bounds.position.x -= 25.f;
    bounds.position.y -= 12.f;
    bounds.size.x += 50.f;
    bounds.size.y += 24.f;
    return bounds.contains(point);
}
