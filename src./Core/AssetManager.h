#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class AssetManager
{
public:
    bool load();

    const sf::Font& mainFont() const;
    const sf::Font& secondFont() const;
    const sf::Texture& mainMenuBackground() const;
    const sf::Texture& settingsBackground() const;
    const sf::Texture& garageBackground() const;
    const sf::Texture& raceSkybox() const;
    void reloadRaceSkybox();
    const sf::Texture& raceRoadTile() const;
    const sf::Texture& dashboardTexture() const;
    const sf::Texture& moneyIcon() const;
    const sf::Texture& goldIcon() const;
    const sf::Texture& levelIcon() const;
    const sf::Texture* texture(const std::string& path);

    bool isFontMissing() const;

private:
    sf::Font m_mainFont;
    sf::Font m_secondFont;
    sf::Texture m_menuBackground;
    sf::Texture m_settingsFallbackBackground;
    sf::Texture m_garageBackground;
    sf::Texture m_raceSkybox;
    sf::Texture m_raceRoadTile;
    sf::Texture m_dashboardTexture;
    sf::Texture m_moneyIcon;
    sf::Texture m_goldIcon;
    sf::Texture m_levelIcon;
    std::unordered_map<std::string, sf::Texture> m_textureCache;
    std::unordered_set<std::string> m_missingTexturePaths;
    bool m_fontMissing = false;

    static bool loadTexture(sf::Texture& texture, const std::string& path);
    static bool loadFirstAvailable(sf::Texture& texture, const std::vector<std::string>& paths);
    bool loadMainFont();
    bool loadSecondFont();
    void loadRandomMenuSkybox();
    void loadRandomRaceSkybox();
};
