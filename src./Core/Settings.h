#pragma once

#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

class Settings
{
public:
    bool fullscreen = true;
    bool metric = true;
    float volume = 1.f;
    int resolutionIndex = 0;
    int fpsIndex = 2;

    std::vector<sf::Vector2u> resolutions;

    const std::vector<unsigned int> fpsLimits{
        60, 75, 120, 144, 165, 240, 360
    };

    void load();
    void save() const;

    sf::Vector2u currentResolution() const;
    unsigned int currentFps() const;

    void refreshAvailableResolutions();
    void setResolution(sf::Vector2u size);
    void setToMaxResolution();

private:
    static std::string filePath();
    void normalize();
    int findResolutionIndex(sf::Vector2u size) const;
};
