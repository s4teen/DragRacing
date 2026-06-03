#include "AssetManager.h"

#include <random>
#include <utility>
#include <vector>

bool AssetManager::load()
{
    m_fontMissing = !loadMainFont();
    loadSecondFont();

    loadRandomMenuSkybox();
    loadRandomRaceSkybox();

    loadFirstAvailable(m_settingsFallbackBackground, {
        "assets/backgrounds/skybox_day.png",
        "assets/backgrounds/skybox_evening.png",
        "assets/backgrounds/skybox_rain.png",
        "assets/screens/skybox_day.png",
        "assets/screens/skybox_evening.png",
        "assets/screens/skybox_rain.png",
        "assets/screens/SettingsMenu.png",
        "assets/screens/MainMenu.png"
    });

    loadFirstAvailable(m_garageBackground, {
        "assets/backgrounds/garage.png",
        "assets/screens/garage.png",
        "assets/screens/GarageMain.png"
    });

    loadFirstAvailable(m_raceRoadTile, {
        "assets/race/roadtile.png",
        "assets/roadtile.png",
        "assets/screens/roadtile.png"
    });

    loadFirstAvailable(m_dashboardTexture, {
        "assets/race/dashboard.png",
        "assets/dashboard.png",
        "assets/screens/dashboard.png"
    });

    loadFirstAvailable(m_moneyIcon, {
        "assets/icons/money.png",
        "assets/money.png",
        "assets/backgrounds/money.png",
        "assets/screens/money.png"
    });

    loadFirstAvailable(m_goldIcon, {
        "assets/icons/gold.png",
        "assets/gold.png",
        "assets/backgrounds/gold.png",
        "assets/screens/gold.png"
    });

    loadFirstAvailable(m_levelIcon, {
        "assets/icons/level.png",
        "assets/level.png",
        "assets/backgrounds/level.png",
        "assets/screens/level.png"
    });

    return !m_fontMissing;
}

const sf::Font& AssetManager::mainFont() const
{
    return m_mainFont;
}

const sf::Font& AssetManager::secondFont() const
{
    return m_secondFont;
}

const sf::Texture& AssetManager::mainMenuBackground() const
{
    return m_menuBackground;
}

const sf::Texture& AssetManager::settingsBackground() const
{
    if (m_menuBackground.getSize().x != 0 && m_menuBackground.getSize().y != 0)
        return m_menuBackground;

    return m_settingsFallbackBackground;
}

const sf::Texture& AssetManager::garageBackground() const
{
    return m_garageBackground;
}

const sf::Texture& AssetManager::raceSkybox() const
{
    return m_raceSkybox;
}

void AssetManager::reloadRaceSkybox()
{
    loadRandomRaceSkybox();
}

const sf::Texture& AssetManager::raceRoadTile() const
{
    return m_raceRoadTile;
}

const sf::Texture& AssetManager::dashboardTexture() const
{
    return m_dashboardTexture;
}

const sf::Texture& AssetManager::moneyIcon() const
{
    return m_moneyIcon;
}

const sf::Texture& AssetManager::goldIcon() const
{
    return m_goldIcon;
}

const sf::Texture& AssetManager::levelIcon() const
{
    return m_levelIcon;
}

const sf::Texture* AssetManager::texture(const std::string& path)
{
    if (path.empty())
        return nullptr;

    auto existing = m_textureCache.find(path);
    if (existing != m_textureCache.end())
        return &existing->second;

    if (m_missingTexturePaths.find(path) != m_missingTexturePaths.end())
        return nullptr;

    sf::Texture loaded;
    if (!loadTexture(loaded, path))
    {
        m_missingTexturePaths.insert(path);
        return nullptr;
    }

    auto inserted = m_textureCache.emplace(path, std::move(loaded));
    return &inserted.first->second;
}

bool AssetManager::isFontMissing() const
{
    return m_fontMissing;
}

bool AssetManager::loadTexture(sf::Texture& texture, const std::string& path)
{
    if (!texture.loadFromFile(path))
        return false;

    texture.setSmooth(true);
    return true;
}

bool AssetManager::loadFirstAvailable(sf::Texture& texture, const std::vector<std::string>& paths)
{
    for (const std::string& path : paths)
    {
        if (loadTexture(texture, path))
            return true;
    }

    return false;
}

bool AssetManager::loadMainFont()
{
    const std::vector<std::string> candidates{
        "assets/fonts/fontMain.ttf",
        "assets/fonts/fontMain.otf",
        "assets/fonts/fontSecond.ttf",
        "assets/fonts/fontSecond.otf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf"
    };

    for (const auto& path : candidates)
    {
        if (m_mainFont.openFromFile(path))
            return true;
    }

    return false;
}

bool AssetManager::loadSecondFont()
{
    const std::vector<std::string> candidates{
        "assets/fonts/fontSecond.ttf",
        "assets/fonts/fontSecond.otf",
        "assets/fonts/fontMain.ttf",
        "assets/fonts/fontMain.otf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf"
    };

    for (const auto& path : candidates)
    {
        if (m_secondFont.openFromFile(path))
            return true;
    }

    // SFML fonts are not copyable, so if second font cannot be loaded we keep it empty.
    // RaceScene falls back visually by also trying common system fonts above.
    return false;
}

void AssetManager::loadRandomMenuSkybox()
{
    const std::vector<std::string> skyboxPaths{
        "assets/backgrounds/skybox_day.png",
        "assets/backgrounds/skybox_evening.png",
        "assets/backgrounds/skybox_rain.png",
        "assets/screens/skybox_day.png",
        "assets/screens/skybox_evening.png",
        "assets/screens/skybox_rain.png"
    };

    std::vector<sf::Texture> loadedSkyboxes;

    for (const std::string& path : skyboxPaths)
    {
        sf::Texture texture;
        if (loadTexture(texture, path))
            loadedSkyboxes.push_back(std::move(texture));
    }

    if (!loadedSkyboxes.empty())
    {
        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::uniform_int_distribution<std::size_t> distribution(0, loadedSkyboxes.size() - 1);

        m_menuBackground = std::move(loadedSkyboxes[distribution(generator)]);
        return;
    }

    loadFirstAvailable(m_menuBackground, {
        "assets/screens/MainMenu.png",
        "assets/screens/SettingsMenu.png"
    });
}

void AssetManager::loadRandomRaceSkybox()
{
    const std::vector<std::string> skyboxPaths{
        "assets/backgrounds/skybox_day.png",
        "assets/backgrounds/skybox_evening.png",
        "assets/backgrounds/skybox_rain.png",
        "assets/screens/skybox_day.png",
        "assets/screens/skybox_evening.png",
        "assets/screens/skybox_rain.png"
    };

    std::vector<sf::Texture> loadedSkyboxes;

    for (const std::string& path : skyboxPaths)
    {
        sf::Texture texture;
        if (loadTexture(texture, path))
            loadedSkyboxes.push_back(std::move(texture));
    }

    if (!loadedSkyboxes.empty())
    {
        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::uniform_int_distribution<std::size_t> distribution(0, loadedSkyboxes.size() - 1);
        m_raceSkybox = std::move(loadedSkyboxes[distribution(generator)]);
    }
}
