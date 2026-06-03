#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class TextButton
{
public:
    TextButton(const sf::Font& font,
               const std::string& label,
               sf::Vector2f position,
               unsigned int characterSize,
               std::function<void()> callback = nullptr);

    void setLabel(const std::string& label);
    void setEnabled(bool enabled);
    void update(sf::Vector2f mousePosition);
    void click(sf::Vector2f mousePosition);
    void draw(sf::RenderTarget& target) const;

private:
    sf::Text m_text;
    std::function<void()> m_callback;
    bool m_enabled = true;
    sf::Color m_defaultColor = sf::Color::White;
    sf::Color m_disabledColor = sf::Color(135, 135, 135);
    sf::Color m_hoverColor = sf::Color(255, 235, 150);

    void centerOrigin();
    bool contains(sf::Vector2f point) const;
};
