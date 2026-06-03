#pragma once

#include "Scene.h"
#include "../GameData/Car.h"
#include "../GameData/GameEnums.h"
#include "../UI/TextButton.h"

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

namespace GameData
{
    struct Car;
    struct Part;
}

class GarageScene : public Scene
{
public:
    explicit GarageScene(Game& game);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;

private:
    enum class GarageMenu
    {
        Main,
        Dealership,
        Parking,
        Tuning,
        Shop,
        RaceTypes
    };

    enum class SkillType
    {
        Money,
        Gold,
        Experience,
        Power,
        Grip,
        ShopDiscount
    };

    struct CarStats
    {
        int power = 0;
        int maxRpm = 0;
        int weight = 0;
        std::string drive = "-";
        int grip = 0;
        int shiftTimeMs = 0;
    };

    GarageMenu m_currentMenu = GarageMenu::Main;
    std::optional<GarageMenu> m_pendingMenu;
    std::optional<SkillType> m_pendingSkillSpend;

    std::vector<TextButton> m_menuButtons;
    std::vector<TextButton> m_actionButtons;
    std::vector<TextButton> m_partTypeButtons;
    std::vector<TextButton> m_skillButtons;

    int m_dealershipIndex = 0;
    int m_parkingIndex = 0;
    int m_shopIndex = 0;
    int m_tuningIndex = 0;
    GameData::PartType m_selectedPartType = GameData::PartType::Engine;

    const GameData::Car* m_selectedDealershipCar = nullptr;
    GameData::Car* m_selectedParkingCar = nullptr;
    const GameData::Part* m_selectedShopPart = nullptr;
    const GameData::Part* m_selectedTuningPart = nullptr;
    std::optional<GameData::Car> m_previewCar;
    CarStats m_carStats;

    void buildUi();
    void buildSkillButtons();
    void requestMenu(GarageMenu menu);
    void applyPendingMenuChange();

    void buildMainMenu();
    void buildCarBrowserMenu(bool isParking);
    void buildPartBrowserMenu(bool isTuning);
    void buildRaceTypesMenu();
    void addBackButton();

    void handleButtonClick(sf::Vector2f mousePosition);
    void handleSkillButtonClick(sf::Vector2f mousePosition);
    void updateButtons(sf::Vector2f mousePosition);
    void updateSkillButtons(sf::Vector2f mousePosition);
    void drawButtons(sf::RenderTarget& target);
    void drawSkillChoiceOverlay(sf::RenderTarget& target);

    void openDealership();
    void openParking();
    void openShop();
    void openTuning();
    void closeToMainGarageMenu();

    void dealershipNext();
    void dealershipPrev();
    void showDealershipCar(int index);
    void buySelectedDealershipCar();

    void parkingNext();
    void parkingPrev();
    void showParkingCar(int index);
    void sellSelectedParkingCar();

    void shopNext();
    void shopPrev();
    void showShopPart(int index);
    void selectShopPartType(GameData::PartType type);
    void buySelectedShopPart();

    void tuningNext();
    void tuningPrev();
    void showTuningPart(int index);
    void selectTuningPartType(GameData::PartType type);
    void installSelectedTuningPart();
    void sellSelectedTuningPart();

    void updateStatsFromCar(const GameData::Car* car);
    void showPlayerCar();
    void autosave();
    bool skillChoiceOpen();
    void requestSpendSkill(SkillType skill);
    void applyPendingSkillSpend();
    void spendSkill(int& skillValue);

    void drawTopTitle(sf::RenderTarget& target);
    void drawPlayerResources(sf::RenderTarget& target);
    void drawCarStats(sf::RenderTarget& target);
    void drawCurrentMenuContent(sf::RenderTarget& target);
    void drawCarPreview(sf::RenderTarget& target);

    void drawMainMenuContent(sf::RenderTarget& target);
    void drawCarBrowserContent(sf::RenderTarget& target, bool isParking);
    void drawPartBrowserContent(sf::RenderTarget& target, bool isTuning);
    void drawRaceTypesContent(sf::RenderTarget& target);

    void drawPanel(sf::RenderTarget& target, sf::Vector2f position, sf::Vector2f size, sf::Color color = sf::Color(0, 0, 0, 85));
    void drawText(sf::RenderTarget& target, const std::string& text, sf::Vector2f position, unsigned int size, bool centered = false, sf::Color color = sf::Color::White);
    void drawRightAlignedText(sf::RenderTarget& target, const std::string& text, sf::Vector2f rightMiddle, unsigned int size, sf::Color color = sf::Color::White);
    void drawIcon(sf::RenderTarget& target, const sf::Texture& texture, sf::Vector2f position, float targetHeight);

    std::string currentCarName(bool isParking);
    std::string currentCarPrice(bool isParking);
    std::string currentPartName(bool isTuning);
    std::string currentPartPrice(bool isTuning);
    std::string currentTuningAmount();

    static std::string padNumber(int value, int width);
    static std::string formatPrice(int price, GameData::PriceType type);
};
