#include "Format.h"

#include <cmath>

namespace Format
{
    std::string resolution(sf::Vector2u value)
    {
        return std::to_string(value.x) + "x" + std::to_string(value.y);
    }

    std::string fps(unsigned int value)
    {
        return std::to_string(value) + "hz";
    }

    std::string percent(float value01)
    {
        return std::to_string(static_cast<int>(std::round(value01 * 100.f))) + "%";
    }
}
