#pragma once

#include <SFML/System/Vector2.hpp>
#include <string>

namespace Format
{
    std::string resolution(sf::Vector2u value);
    std::string fps(unsigned int value);
    std::string percent(float value01);
}
