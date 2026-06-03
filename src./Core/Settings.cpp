#include "Settings.h"

#include <SFML/Window/VideoMode.hpp>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>

namespace fs = std::filesystem;

namespace
{
    std::uint64_t resolutionArea(sf::Vector2u size)
    {
        return static_cast<std::uint64_t>(size.x) * static_cast<std::uint64_t>(size.y);
    }

    bool sameResolution(sf::Vector2u a, sf::Vector2u b)
    {
        return a.x == b.x && a.y == b.y;
    }
}

std::string Settings::filePath()
{
    fs::create_directories("save");
    return "save/settings.ini";
}

void Settings::load()
{
    refreshAvailableResolutions();

    const sf::Vector2u defaultResolution = currentResolution();
    sf::Vector2u loadedResolution = defaultResolution;
    bool hasSavedResolution = false;
    int loadedResolutionIndex = resolutionIndex;

    std::ifstream input(filePath());
    if (!input)
    {
        normalize();
        if (fullscreen)
            setToMaxResolution();
        return;
    }

    std::string key;
    while (input >> key)
    {
        if (key == "fullscreen")
            input >> fullscreen;
        else if (key == "metric")
            input >> metric;
        else if (key == "volume")
            input >> volume;
        else if (key == "resolutionIndex")
            input >> loadedResolutionIndex;
        else if (key == "resolutionWidth")
        {
            input >> loadedResolution.x;
            hasSavedResolution = true;
        }
        else if (key == "resolutionHeight")
        {
            input >> loadedResolution.y;
            hasSavedResolution = true;
        }
        else if (key == "fpsIndex")
            input >> fpsIndex;
    }

    if (hasSavedResolution)
        setResolution(loadedResolution);
    else
        resolutionIndex = loadedResolutionIndex;

    normalize();

    if (fullscreen)
        setToMaxResolution();
}

void Settings::save() const
{
    const sf::Vector2u resolution = currentResolution();

    std::ofstream output(filePath());
    output << "fullscreen " << fullscreen << '\n';
    output << "metric " << metric << '\n';
    output << "volume " << volume << '\n';
    output << "resolutionIndex " << resolutionIndex << '\n';
    output << "resolutionWidth " << resolution.x << '\n';
    output << "resolutionHeight " << resolution.y << '\n';
    output << "fpsIndex " << fpsIndex << '\n';
}

sf::Vector2u Settings::currentResolution() const
{
    if (resolutions.empty())
        return {1920u, 1080u};

    const int index = std::clamp(resolutionIndex, 0, static_cast<int>(resolutions.size()) - 1);
    return resolutions[index];
}

unsigned int Settings::currentFps() const
{
    const int index = std::clamp(fpsIndex, 0, static_cast<int>(fpsLimits.size()) - 1);
    return fpsLimits[index];
}

void Settings::refreshAvailableResolutions()
{
    const sf::Vector2u previous = currentResolution();
    resolutions.clear();

    const std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
    for (const sf::VideoMode& mode : modes)
    {
        const sf::Vector2u size = mode.size;
        if (size.x == 0 || size.y == 0)
            continue;

        const auto duplicate = std::find_if(resolutions.begin(), resolutions.end(), [&](sf::Vector2u existing)
        {
            return sameResolution(existing, size);
        });

        if (duplicate == resolutions.end())
            resolutions.push_back(size);
    }

    const sf::Vector2u desktopSize = sf::VideoMode::getDesktopMode().size;
    if (desktopSize.x > 0 && desktopSize.y > 0)
    {
        const auto duplicate = std::find_if(resolutions.begin(), resolutions.end(), [&](sf::Vector2u existing)
        {
            return sameResolution(existing, desktopSize);
        });

        if (duplicate == resolutions.end())
            resolutions.push_back(desktopSize);
    }

    if (resolutions.empty())
        resolutions.push_back({1920u, 1080u});

    std::sort(resolutions.begin(), resolutions.end(), [](sf::Vector2u a, sf::Vector2u b)
    {
        const std::uint64_t areaA = resolutionArea(a);
        const std::uint64_t areaB = resolutionArea(b);

        if (areaA != areaB)
            return areaA > areaB;

        if (a.x != b.x)
            return a.x > b.x;

        return a.y > b.y;
    });

    setResolution(previous);
}

void Settings::setResolution(sf::Vector2u size)
{
    if (resolutions.empty())
        refreshAvailableResolutions();

    resolutionIndex = findResolutionIndex(size);
}

void Settings::setToMaxResolution()
{
    if (resolutions.empty())
        refreshAvailableResolutions();

    resolutionIndex = 0;
}

void Settings::normalize()
{
    if (resolutions.empty())
        refreshAvailableResolutions();

    volume = std::clamp(volume, 0.f, 1.f);
    resolutionIndex = std::clamp(resolutionIndex, 0, static_cast<int>(resolutions.size()) - 1);
    fpsIndex = std::clamp(fpsIndex, 0, static_cast<int>(fpsLimits.size()) - 1);
}

int Settings::findResolutionIndex(sf::Vector2u size) const
{
    if (resolutions.empty())
        return 0;

    int bestIndex = 0;
    std::uint64_t bestDifference = std::numeric_limits<std::uint64_t>::max();
    const std::uint64_t targetArea = resolutionArea(size);

    for (int i = 0; i < static_cast<int>(resolutions.size()); ++i)
    {
        if (sameResolution(resolutions[i], size))
            return i;

        const std::uint64_t area = resolutionArea(resolutions[i]);
        const std::uint64_t difference = area > targetArea ? area - targetArea : targetArea - area;
        if (difference < bestDifference)
        {
            bestDifference = difference;
            bestIndex = i;
        }
    }

    return bestIndex;
}
