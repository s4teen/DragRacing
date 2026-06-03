#include "GarageScene.h"

#include "../Core/Constants.h"
#include "../Core/Game.h"
#include "../Core/RaceMode.h"
#include "../Core/SceneType.h"
#include "../GameData/Car.h"
#include "../GameData/GameDatabase.h"
#include "../GameData/Part.h"
#include "../GameData/Player.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace
{
    constexpr sf::Color PanelColor = sf::Color(0, 0, 0, 85);
    constexpr sf::Color DarkPanelColor = sf::Color(0, 0, 0, 115);

    const std::vector<GameData::PartType> GaragePartTypes{
        GameData::PartType::Engine,
        GameData::PartType::Turbo,
        GameData::PartType::EngineBlock,
        GameData::PartType::Pistons,
        GameData::PartType::AirFilter,
        GameData::PartType::Intercooler,
        GameData::PartType::ECU,
        GameData::PartType::ExhaustSystem,
        GameData::PartType::Suspension,
        GameData::PartType::Tires,
        GameData::PartType::BodyWeightReduction,
        GameData::PartType::Transmission,
        GameData::PartType::Gearbox,
        GameData::PartType::Clutch
    };
}

GarageScene::GarageScene(Game& game)
    : Scene(game)
{
    showPlayerCar();
    buildUi();
    buildSkillButtons();
}

void GarageScene::handleEvent(const sf::Event& event)
{
    if (skillChoiceOpen())
    {
        if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (pressed->button == sf::Mouse::Button::Left)
                handleSkillButtonClick(game().mouseDesignPosition(pressed->position));
        }
        return;
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->scancode == sf::Keyboard::Scancode::Escape)
        {
            switch (m_currentMenu)
            {
            case GarageMenu::Main:
                game().changeScene(SceneType::MainMenu);
                break;
            case GarageMenu::Tuning:
                requestMenu(GarageMenu::Parking);
                break;
            default:
                requestMenu(GarageMenu::Main);
                break;
            }
        }
    }

    if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (pressed->button == sf::Mouse::Button::Left)
        {
            const sf::Vector2f mouse = game().mouseDesignPosition(pressed->position);
            handleButtonClick(mouse);
        }
    }

    applyPendingMenuChange();
}

void GarageScene::update(float)
{
    const sf::Vector2f mouse = game().mouseDesignPosition(sf::Mouse::getPosition(game().window()));

    if (skillChoiceOpen())
    {
        updateSkillButtons(mouse);
        return;
    }

    updateButtons(mouse);
    applyPendingMenuChange();
}

void GarageScene::draw(sf::RenderTarget& target)
{
    game().drawBackground(target, game().assets().garageBackground(), sf::Color(60, 60, 60));

    if (skillChoiceOpen())
    {
        drawPlayerResources(target);
        drawSkillChoiceOverlay(target);
        for (const TextButton& button : m_skillButtons)
            button.draw(target);
        return;
    }

    drawCarPreview(target);
    drawTopTitle(target);
    drawPlayerResources(target);
    drawCarStats(target);
    drawCurrentMenuContent(target);
    drawButtons(target);
}

void GarageScene::buildUi()
{
    m_menuButtons.clear();
    m_actionButtons.clear();
    m_partTypeButtons.clear();

    switch (m_currentMenu)
    {
    case GarageMenu::Main:
        buildMainMenu();
        break;
    case GarageMenu::Dealership:
        buildCarBrowserMenu(false);
        break;
    case GarageMenu::Parking:
        buildCarBrowserMenu(true);
        break;
    case GarageMenu::Tuning:
        buildPartBrowserMenu(true);
        break;
    case GarageMenu::Shop:
        buildPartBrowserMenu(false);
        break;
    case GarageMenu::RaceTypes:
        buildRaceTypesMenu();
        break;
    }
}

void GarageScene::buildSkillButtons()
{
    m_skillButtons.clear();

    const sf::Font& font = game().assets().mainFont();
    GameData::Player& player = game().database().currentPlayer;

    const float x = Constants::DesignSize.x / 2.f;
    float y = 390.f;
    const float step = 58.f;

    const auto label = [](const std::string& name, int points)
    {
        return name + ": " + std::to_string(points) + " (+" + std::to_string(points) + "%)";
    };

    m_skillButtons.emplace_back(font, label("Money", player.moneyBonus), sf::Vector2f{x, y}, 34, [this] { requestSpendSkill(SkillType::Money); });
    y += step;
    m_skillButtons.emplace_back(font, label("Gold", player.goldBonus), sf::Vector2f{x, y}, 34, [this] { requestSpendSkill(SkillType::Gold); });
    y += step;
    m_skillButtons.emplace_back(font, label("Experience", player.experienceBonus), sf::Vector2f{x, y}, 34, [this] { requestSpendSkill(SkillType::Experience); });
    y += step;
    m_skillButtons.emplace_back(font, label("Power", player.powerBonus), sf::Vector2f{x, y}, 34, [this] { requestSpendSkill(SkillType::Power); });
    y += step;
    m_skillButtons.emplace_back(font, label("Grip", player.gripBonus), sf::Vector2f{x, y}, 34, [this] { requestSpendSkill(SkillType::Grip); });
    y += step;
    m_skillButtons.emplace_back(font, label("Shop Discount", player.shopDiscount), sf::Vector2f{x, y}, 34, [this] { requestSpendSkill(SkillType::ShopDiscount); });
}

bool GarageScene::skillChoiceOpen()
{
    return game().database().currentPlayer.skillPoints > 0;
}

void GarageScene::requestSpendSkill(SkillType skill)
{
    // Do not rebuild/clear m_skillButtons directly from a TextButton callback.
    // The click loop may still be iterating over the same vector.
    m_pendingSkillSpend = skill;
}

void GarageScene::applyPendingSkillSpend()
{
    if (!m_pendingSkillSpend)
        return;

    const SkillType skill = *m_pendingSkillSpend;
    m_pendingSkillSpend.reset();

    GameData::Player& player = game().database().currentPlayer;

    switch (skill)
    {
    case SkillType::Money:
        spendSkill(player.moneyBonus);
        break;
    case SkillType::Gold:
        spendSkill(player.goldBonus);
        break;
    case SkillType::Experience:
        spendSkill(player.experienceBonus);
        break;
    case SkillType::Power:
        spendSkill(player.powerBonus);
        break;
    case SkillType::Grip:
        spendSkill(player.gripBonus);
        break;
    case SkillType::ShopDiscount:
        spendSkill(player.shopDiscount);
        break;
    }
}

void GarageScene::spendSkill(int& skillValue)
{
    if (!game().database().currentPlayer.spendSkillPoint(skillValue))
        return;

    autosave();
    buildSkillButtons();
    showPlayerCar();
}

void GarageScene::handleSkillButtonClick(sf::Vector2f mousePosition)
{
    m_pendingSkillSpend.reset();

    for (TextButton& button : m_skillButtons)
        button.click(mousePosition);

    applyPendingSkillSpend();
}

void GarageScene::updateSkillButtons(sf::Vector2f mousePosition)
{
    if (m_skillButtons.empty())
        buildSkillButtons();

    for (TextButton& button : m_skillButtons)
        button.update(mousePosition);
}

void GarageScene::requestMenu(GarageMenu menu)
{
    m_pendingMenu = menu;
}

void GarageScene::applyPendingMenuChange()
{
    if (!m_pendingMenu)
        return;

    const GarageMenu nextMenu = *m_pendingMenu;
    m_pendingMenu.reset();

    switch (nextMenu)
    {
    case GarageMenu::Main:
        closeToMainGarageMenu();
        break;
    case GarageMenu::Dealership:
        openDealership();
        break;
    case GarageMenu::Parking:
        openParking();
        break;
    case GarageMenu::Tuning:
        openTuning();
        break;
    case GarageMenu::Shop:
        openShop();
        break;
    case GarageMenu::RaceTypes:
        m_currentMenu = GarageMenu::RaceTypes;
        m_previewCar.reset();
        showPlayerCar();
        buildUi();
        break;
    }
}

void GarageScene::buildMainMenu()
{
    const sf::Font& font = game().assets().mainFont();

    m_menuButtons.reserve(5);
    m_menuButtons.emplace_back(font, "Dealership", sf::Vector2f{105.f, 133.f}, 31, [this]
    {
        requestMenu(GarageMenu::Dealership);
    });
    m_menuButtons.emplace_back(font, "Parking", sf::Vector2f{105.f, 184.f}, 34, [this]
    {
        requestMenu(GarageMenu::Parking);
    });
    m_menuButtons.emplace_back(font, "Shop", sf::Vector2f{105.f, 235.f}, 34, [this]
    {
        requestMenu(GarageMenu::Shop);
    });
    m_menuButtons.emplace_back(font, "Race", sf::Vector2f{105.f, 292.f}, 45, [this]
    {
        requestMenu(GarageMenu::RaceTypes);
    });
    m_menuButtons.emplace_back(font, "MENU", sf::Vector2f{92.f, 36.f}, 40, [this]
    {
        autosave();
        game().changeScene(SceneType::MainMenu);
    });
}

void GarageScene::buildCarBrowserMenu(bool isParking)
{
    const sf::Font& font = game().assets().mainFont();
    addBackButton();

    m_actionButtons.reserve(5);
    m_actionButtons.emplace_back(font, "Prev", sf::Vector2f{885.f, 935.f}, 54, [this, isParking]
    {
        if (isParking)
            parkingPrev();
        else
            dealershipPrev();
    });
    m_actionButtons.emplace_back(font, "Next", sf::Vector2f{1035.f, 935.f}, 54, [this, isParking]
    {
        if (isParking)
            parkingNext();
        else
            dealershipNext();
    });

    if (isParking)
    {
        m_actionButtons.emplace_back(font, "Tuning", sf::Vector2f{118.f, 858.f}, 54, [this]
        {
            requestMenu(GarageMenu::Tuning);
        });
        m_actionButtons.emplace_back(font, "Sell", sf::Vector2f{95.f, 935.f}, 54, [this]
        {
            sellSelectedParkingCar();
        });
    }
    else
    {
        m_actionButtons.emplace_back(font, "Buy", sf::Vector2f{105.f, 935.f}, 54, [this]
        {
            buySelectedDealershipCar();
        });
    }
}

void GarageScene::buildPartBrowserMenu(bool isTuning)
{
    const sf::Font& font = game().assets().mainFont();
    addBackButton();

    m_partTypeButtons.reserve(GaragePartTypes.size());
    float y = 228.f;
    for (GameData::PartType type : GaragePartTypes)
    {
        m_partTypeButtons.emplace_back(font, GameData::toString(type), sf::Vector2f{146.f, y}, 25, [this, type, isTuning]
        {
            if (isTuning)
                selectTuningPartType(type);
            else
                selectShopPartType(type);
        });
        y += 39.f;
    }

    m_actionButtons.reserve(5);
    m_actionButtons.emplace_back(font, "Prev", sf::Vector2f{885.f, 935.f}, 54, [this, isTuning]
    {
        if (isTuning)
            tuningPrev();
        else
            shopPrev();
    });
    m_actionButtons.emplace_back(font, "Next", sf::Vector2f{1035.f, 935.f}, 54, [this, isTuning]
    {
        if (isTuning)
            tuningNext();
        else
            shopNext();
    });

    if (isTuning)
    {
        m_actionButtons.emplace_back(font, "Install", sf::Vector2f{118.f, 858.f}, 54, [this]
        {
            installSelectedTuningPart();
        });
        m_actionButtons.emplace_back(font, "Sell", sf::Vector2f{95.f, 935.f}, 54, [this]
        {
            sellSelectedTuningPart();
        });
    }
    else
    {
        m_actionButtons.emplace_back(font, "Buy", sf::Vector2f{105.f, 935.f}, 54, [this]
        {
            buySelectedShopPart();
        });
    }
}

void GarageScene::buildRaceTypesMenu()
{
    const sf::Font& font = game().assets().mainFont();
    addBackButton();

    m_actionButtons.reserve(3);
    m_actionButtons.emplace_back(font, "Free Ride", sf::Vector2f{210.f, 280.f}, 34, [this]
    {
        autosave();
        game().setNextRaceMode(RaceMode::FreeRide);
        game().changeScene(SceneType::Race);
    });
    m_actionButtons.emplace_back(font, "Against Bot", sf::Vector2f{210.f, 330.f}, 34, [this]
    {
        autosave();
        game().setNextRaceMode(RaceMode::AgainstBot);
        game().changeScene(SceneType::Race);
    });
}

void GarageScene::addBackButton()
{
    const sf::Font& font = game().assets().mainFont();

    m_menuButtons.reserve(2);
    m_menuButtons.emplace_back(font, "MENU", sf::Vector2f{92.f, 36.f}, 40, [this]
    {
        autosave();
        game().changeScene(SceneType::MainMenu);
    });
    m_menuButtons.emplace_back(font, "Back", sf::Vector2f{76.f, 105.f}, 31, [this]
    {
        if (m_currentMenu == GarageMenu::Tuning)
            requestMenu(GarageMenu::Parking);
        else
            requestMenu(GarageMenu::Main);
    });
}

void GarageScene::handleButtonClick(sf::Vector2f mousePosition)
{
    for (auto& button : m_menuButtons)
        button.click(mousePosition);

    for (auto& button : m_actionButtons)
        button.click(mousePosition);

    for (auto& button : m_partTypeButtons)
        button.click(mousePosition);
}

void GarageScene::updateButtons(sf::Vector2f mousePosition)
{
    for (auto& button : m_menuButtons)
        button.update(mousePosition);

    for (auto& button : m_actionButtons)
        button.update(mousePosition);

    for (auto& button : m_partTypeButtons)
        button.update(mousePosition);
}

void GarageScene::drawButtons(sf::RenderTarget& target)
{
    for (const auto& button : m_menuButtons)
        button.draw(target);

    for (const auto& button : m_actionButtons)
        button.draw(target);

    for (const auto& button : m_partTypeButtons)
        button.draw(target);
}

void GarageScene::openDealership()
{
    m_currentMenu = GarageMenu::Dealership;
    m_dealershipIndex = 0;
    showDealershipCar(0);
    buildUi();
}

void GarageScene::openParking()
{
    m_currentMenu = GarageMenu::Parking;
    m_parkingIndex = game().database().currentPlayer.selectedCarIndex;
    showParkingCar(m_parkingIndex);
    buildUi();
}

void GarageScene::openShop()
{
    m_currentMenu = GarageMenu::Shop;
    m_previewCar.reset();
    m_selectedPartType = GameData::PartType::Engine;
    m_shopIndex = 0;
    showPlayerCar();
    showShopPart(0);
    buildUi();
}

void GarageScene::openTuning()
{
    m_currentMenu = GarageMenu::Tuning;
    m_selectedPartType = GameData::PartType::Engine;
    m_tuningIndex = 0;
    showTuningPart(0);
    buildUi();
}

void GarageScene::closeToMainGarageMenu()
{
    m_currentMenu = GarageMenu::Main;
    m_previewCar.reset();
    showPlayerCar();
    buildUi();
    buildSkillButtons();
}

void GarageScene::dealershipNext()
{
    const auto& inventory = game().database().dealership.inventory;
    if (inventory.empty())
        return;

    m_dealershipIndex = (m_dealershipIndex + 1) % static_cast<int>(inventory.size());
    showDealershipCar(m_dealershipIndex);
}

void GarageScene::dealershipPrev()
{
    const auto& inventory = game().database().dealership.inventory;
    if (inventory.empty())
        return;

    m_dealershipIndex = (m_dealershipIndex - 1 + static_cast<int>(inventory.size())) % static_cast<int>(inventory.size());
    showDealershipCar(m_dealershipIndex);
}

void GarageScene::showDealershipCar(int index)
{
    const auto& inventory = game().database().dealership.inventory;
    m_selectedDealershipCar = nullptr;
    m_previewCar.reset();

    if (inventory.empty() || index < 0 || index >= static_cast<int>(inventory.size()))
    {
        updateStatsFromCar(nullptr);
        return;
    }

    const int carId = inventory[static_cast<std::size_t>(index)].carId;
    m_selectedDealershipCar = game().database().getCarById(carId);
    updateStatsFromCar(m_selectedDealershipCar);
}

void GarageScene::buySelectedDealershipCar()
{
    const auto& inventory = game().database().dealership.inventory;
    if (inventory.empty() || m_dealershipIndex < 0 || m_dealershipIndex >= static_cast<int>(inventory.size()))
        return;

    if (game().database().dealership.buyCar(game().database().currentPlayer, inventory[static_cast<std::size_t>(m_dealershipIndex)].carId, game().database()))
    {
        autosave();
        showDealershipCar(m_dealershipIndex);
    }
}

void GarageScene::parkingNext()
{
    auto& cars = game().database().currentPlayer.carInventory;
    if (cars.empty())
        return;

    m_parkingIndex = (m_parkingIndex + 1) % static_cast<int>(cars.size());
    showParkingCar(m_parkingIndex);
}

void GarageScene::parkingPrev()
{
    auto& cars = game().database().currentPlayer.carInventory;
    if (cars.empty())
        return;

    m_parkingIndex = (m_parkingIndex - 1 + static_cast<int>(cars.size())) % static_cast<int>(cars.size());
    showParkingCar(m_parkingIndex);
}

void GarageScene::showParkingCar(int index)
{
    GameData::Player& player = game().database().currentPlayer;
    m_selectedParkingCar = nullptr;
    m_previewCar.reset();

    if (player.carInventory.empty() || index < 0 || index >= static_cast<int>(player.carInventory.size()))
    {
        updateStatsFromCar(nullptr);
        return;
    }

    player.selectedCarIndex = index;
    m_parkingIndex = index;
    m_selectedParkingCar = &player.carInventory[static_cast<std::size_t>(index)];
    updateStatsFromCar(m_selectedParkingCar);
}

void GarageScene::sellSelectedParkingCar()
{
    GameData::Player& player = game().database().currentPlayer;
    if (!m_selectedParkingCar || player.carInventory.size() <= 1)
        return;

    const int index = player.selectedCarIndex;
    if (index < 0 || index >= static_cast<int>(player.carInventory.size()))
        return;

    player.addMoney(player.carInventory[static_cast<std::size_t>(index)].price / 2);
    player.carInventory.erase(player.carInventory.begin() + index);
    player.selectedCarIndex = std::clamp(index, 0, static_cast<int>(player.carInventory.size()) - 1);
    m_parkingIndex = player.selectedCarIndex;
    showParkingCar(m_parkingIndex);
    autosave();
}

void GarageScene::shopNext()
{
    const auto parts = game().database().getShopPartsOfType(m_selectedPartType);
    if (parts.empty())
        return;

    m_shopIndex = (m_shopIndex + 1) % static_cast<int>(parts.size());
    showShopPart(m_shopIndex);
}

void GarageScene::shopPrev()
{
    const auto parts = game().database().getShopPartsOfType(m_selectedPartType);
    if (parts.empty())
        return;

    m_shopIndex = (m_shopIndex - 1 + static_cast<int>(parts.size())) % static_cast<int>(parts.size());
    showShopPart(m_shopIndex);
}

void GarageScene::showShopPart(int index)
{
    const auto parts = game().database().getShopPartsOfType(m_selectedPartType);
    m_selectedShopPart = nullptr;
    m_previewCar.reset();

    GameData::Car* selectedCar = game().database().currentPlayer.getSelectedCar();
    if (!selectedCar)
    {
        updateStatsFromCar(nullptr);
        return;
    }

    if (parts.empty() || index < 0 || index >= static_cast<int>(parts.size()))
    {
        updateStatsFromCar(selectedCar);
        return;
    }

    m_shopIndex = index;
    m_selectedShopPart = parts[static_cast<std::size_t>(index)];

    // Shop preview: temporarily install the selected shop part into a copy
    // of the current player car and show the resulting stats/sprite preview.
    m_previewCar = *selectedCar;
    m_previewCar->installPart(*m_selectedShopPart, game().database());
    updateStatsFromCar(&*m_previewCar);
}

void GarageScene::selectShopPartType(GameData::PartType type)
{
    m_selectedPartType = type;
    m_shopIndex = 0;
    showShopPart(0);
}

void GarageScene::buySelectedShopPart()
{
    if (!m_selectedShopPart)
        return;

    if (game().database().partShop.buyPart(game().database().currentPlayer, m_selectedShopPart->id, game().database()))
    {
        autosave();
        showShopPart(m_shopIndex);
    }
}

void GarageScene::tuningNext()
{
    const auto parts = game().database().getPlayerPartsOfType(m_selectedPartType);
    if (parts.empty())
        return;

    m_tuningIndex = (m_tuningIndex + 1) % static_cast<int>(parts.size());
    showTuningPart(m_tuningIndex);
}

void GarageScene::tuningPrev()
{
    const auto parts = game().database().getPlayerPartsOfType(m_selectedPartType);
    if (parts.empty())
        return;

    m_tuningIndex = (m_tuningIndex - 1 + static_cast<int>(parts.size())) % static_cast<int>(parts.size());
    showTuningPart(m_tuningIndex);
}

void GarageScene::showTuningPart(int index)
{
    const auto parts = game().database().getPlayerPartsOfType(m_selectedPartType);
    m_selectedTuningPart = nullptr;
    m_previewCar.reset();

    GameData::Car* selectedCar = game().database().currentPlayer.getSelectedCar();
    if (!selectedCar)
    {
        updateStatsFromCar(nullptr);
        return;
    }

    if (parts.empty() || index < 0 || index >= static_cast<int>(parts.size()))
    {
        updateStatsFromCar(selectedCar);
        return;
    }

    m_tuningIndex = index;
    m_selectedTuningPart = parts[static_cast<std::size_t>(index)];
    m_previewCar = *selectedCar;
    m_previewCar->installPart(*m_selectedTuningPart, game().database());
    updateStatsFromCar(&*m_previewCar);
}

void GarageScene::selectTuningPartType(GameData::PartType type)
{
    m_selectedPartType = type;
    m_tuningIndex = 0;
    showTuningPart(0);
}

void GarageScene::installSelectedTuningPart()
{
    if (!m_selectedTuningPart)
        return;

    GameData::Player& player = game().database().currentPlayer;
    GameData::Car* car = player.getSelectedCar();
    if (!car || player.getPartCount(m_selectedTuningPart->id) <= 0)
        return;

    if (const GameData::Part* oldPart = car->getPart(m_selectedTuningPart->type, game().database()))
    {
        car->removePart(oldPart->type, game().database());
        player.addPartToInventory(oldPart->id);
    }

    car->installPart(*m_selectedTuningPart, game().database());
    player.removePartFromInventory(m_selectedTuningPart->id);

    const auto parts = game().database().getPlayerPartsOfType(m_selectedPartType);
    if (m_tuningIndex >= static_cast<int>(parts.size()))
        m_tuningIndex = std::max(0, static_cast<int>(parts.size()) - 1);

    showTuningPart(m_tuningIndex);
    autosave();
}

void GarageScene::sellSelectedTuningPart()
{
    if (!m_selectedTuningPart)
        return;

    GameData::Player& player = game().database().currentPlayer;
    GameData::Car* car = player.getSelectedCar();

    if (car)
    {
        const GameData::Part* installedPart = car->getPart(m_selectedTuningPart->type, game().database());
        if (installedPart && installedPart->id == m_selectedTuningPart->id)
            car->removePart(m_selectedTuningPart->type, game().database());
    }

    if (player.removePartFromInventory(m_selectedTuningPart->id))
        player.addMoney(m_selectedTuningPart->price / 2);

    const auto parts = game().database().getPlayerPartsOfType(m_selectedPartType);
    if (m_tuningIndex >= static_cast<int>(parts.size()))
        m_tuningIndex = std::max(0, static_cast<int>(parts.size()) - 1);

    showTuningPart(m_tuningIndex);
    autosave();
}

void GarageScene::updateStatsFromCar(const GameData::Car* car)
{
    if (!car)
    {
        m_carStats = {};
        return;
    }

    const GameData::Player& player = game().database().currentPlayer;
    m_carStats.power = static_cast<int>(std::round(static_cast<float>(car->getPower(game().database())) * (1.f + static_cast<float>(player.powerBonus) * 0.01f)));
    m_carStats.maxRpm = car->getMaxRPM(game().database());
    m_carStats.weight = car->getWeight(game().database());
    m_carStats.drive = GameData::toShortString(car->getTransmissionType(game().database()));
    m_carStats.grip = static_cast<int>(std::round(static_cast<float>(car->getGrip(game().database())) * (1.f + static_cast<float>(player.gripBonus) * 0.01f)));
    m_carStats.shiftTimeMs = std::max(1, car->getShiftTimeMs(game().database()));
}

void GarageScene::showPlayerCar()
{
    m_previewCar.reset();
    updateStatsFromCar(game().database().currentPlayer.getSelectedCar());
}

void GarageScene::autosave()
{
    game().database().savePlayer();
}

void GarageScene::drawTopTitle(sf::RenderTarget&)
{
}

void GarageScene::drawPlayerResources(sf::RenderTarget& target)
{
    constexpr float rightMargin = 32.f;
    constexpr float iconSize = 40.f;
    constexpr float rowHeight = 40.f;
    constexpr float rowGap = 10.f;
    constexpr float iconX = Constants::DesignSize.x - rightMargin - iconSize;
    constexpr float textRightX = iconX - 18.f;
    constexpr float startY = 26.f;

    const GameData::Player& player = game().database().currentPlayer;

    struct ResourceLine
    {
        int value;
        const sf::Texture& icon;
    };

    const ResourceLine lines[] = {
        {player.money, game().assets().moneyIcon()},
        {player.gold, game().assets().goldIcon()},
        {player.level, game().assets().levelIcon()}
    };

    for (int i = 0; i < 3; ++i)
    {
        const float y = startY + i * (rowHeight + rowGap);
        drawRightAlignedText(target, std::to_string(lines[i].value), sf::Vector2f{textRightX, y + rowHeight / 2.f}, 34);
        drawIcon(target, lines[i].icon, sf::Vector2f{iconX, y}, iconSize);
    }
}

void GarageScene::drawCarStats(sf::RenderTarget& target)
{
    drawPanel(target, sf::Vector2f{1375.f, 385.f}, sf::Vector2f{480.f, 354.f}, DarkPanelColor);

    const std::vector<std::string> lines{
        "Power: " + padNumber(m_carStats.power, 4) + " (hp)",
        "Max RPM: " + padNumber(m_carStats.maxRpm, 5) + " (rpm)",
        "Weight: " + padNumber(m_carStats.weight, 4) + " (kg)",
        "Drive: " + m_carStats.drive,
        "Grip: " + padNumber(m_carStats.grip, 4),
        "Shift Time: " + padNumber(m_carStats.shiftTimeMs, 4) + " (ms)"
    };

    float y = 405.f;
    for (const std::string& line : lines)
    {
        drawText(target, line, sf::Vector2f{1398.f, y}, 24);
        y += 52.f;
    }
}

void GarageScene::drawCurrentMenuContent(sf::RenderTarget& target)
{
    switch (m_currentMenu)
    {
    case GarageMenu::Main:
        drawMainMenuContent(target);
        break;
    case GarageMenu::Dealership:
        drawCarBrowserContent(target, false);
        break;
    case GarageMenu::Parking:
        drawCarBrowserContent(target, true);
        break;
    case GarageMenu::Tuning:
        drawPartBrowserContent(target, true);
        break;
    case GarageMenu::Shop:
        drawPartBrowserContent(target, false);
        break;
    case GarageMenu::RaceTypes:
        drawRaceTypesContent(target);
        break;
    }
}

void GarageScene::drawCarPreview(sf::RenderTarget& target)
{
    const GameData::Car* car = nullptr;
    if (m_previewCar)
        car = &*m_previewCar;
    else if (m_currentMenu == GarageMenu::Dealership && m_selectedDealershipCar)
        car = m_selectedDealershipCar;
    else if (m_currentMenu == GarageMenu::Parking && m_selectedParkingCar)
        car = m_selectedParkingCar;
    else
        car = game().database().currentPlayer.getSelectedCar();

    if (!car)
        return;

    const sf::Vector2f scale = car->carScale;
    const sf::Vector2f bodySize{car->bodySize.x * scale.x, car->bodySize.y * scale.y};
    const sf::Vector2f wheelSize{car->wheelSize.x * scale.x, car->wheelSize.y * scale.y};
    const sf::Vector2f frontWheelPos = car->bodyPosition + sf::Vector2f{car->frontWheelPos.x * scale.x, car->frontWheelPos.y * scale.y};
    const sf::Vector2f rearWheelPos = car->bodyPosition + sf::Vector2f{car->rearWheelPos.x * scale.x, car->rearWheelPos.y * scale.y};
    const sf::Angle bodyRotation = sf::degrees(car->bodyRotationDegrees);
    const float bodyRotationRadians = car->bodyRotationDegrees * 3.1415926535f / 180.f;
    const auto rotateBodyOffset = [&](sf::Vector2f offset)
    {
        const float cosA = std::cos(bodyRotationRadians);
        const float sinA = std::sin(bodyRotationRadians);
        return sf::Vector2f{
            offset.x * cosA - offset.y * sinA,
            offset.x * sinA + offset.y * cosA
        };
    };

    const GameData::Part* tirePart = car->getPart(GameData::PartType::Tires, game().database());
    const std::string wheelPath = tirePart ? tirePart->wheelSpritePath : "";
    const sf::Texture* wheelTexture = game().assets().texture(wheelPath);

    const auto drawWheel = [&](sf::Vector2f center)
    {
        if (wheelTexture)
        {
            sf::Sprite wheel(*wheelTexture);
            const sf::Vector2u textureSize = wheelTexture->getSize();
            wheel.setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});
            wheel.setScale({wheelSize.x / static_cast<float>(textureSize.x), wheelSize.y / static_cast<float>(textureSize.y)});
            wheel.setPosition(center);
            target.draw(wheel);
        }
        else
        {
            sf::CircleShape wheel(wheelSize.x / 2.f);
            wheel.setOrigin({wheelSize.x / 2.f, wheelSize.x / 2.f});
            wheel.setPosition(center);
            wheel.setFillColor(sf::Color(22, 22, 22, 230));
            wheel.setOutlineColor(sf::Color(190, 190, 190, 180));
            wheel.setOutlineThickness(4.f);
            target.draw(wheel);
        }
    };

    // Draw order: rear wheel, front wheel, then body.
    // This keeps both wheels visually below the body.
    drawWheel(rearWheelPos);
    drawWheel(frontWheelPos);

    if (const sf::Texture* bodyTexture = game().assets().texture(car->bodySpritePath))
    {
        sf::Sprite body(*bodyTexture);
        const sf::Vector2u textureSize = bodyTexture->getSize();
        body.setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});
        body.setScale({bodySize.x / static_cast<float>(textureSize.x), bodySize.y / static_cast<float>(textureSize.y)});
        body.setRotation(bodyRotation);
        body.setPosition(car->bodyPosition);
        target.draw(body);
    }
    else
    {
        sf::RectangleShape body(bodySize);
        body.setOrigin({bodySize.x / 2.f, bodySize.y / 2.f});
        body.setRotation(bodyRotation);
        body.setPosition(car->bodyPosition);
        body.setFillColor(sf::Color(225, 225, 225, 210));
        body.setOutlineColor(sf::Color(40, 40, 40, 230));
        body.setOutlineThickness(5.f);
        target.draw(body);

        sf::RectangleShape cabin({bodySize.x * 0.36f, bodySize.y * 0.34f});
        cabin.setOrigin({bodySize.x * 0.18f, bodySize.y * 0.17f});
        cabin.setRotation(bodyRotation);
        cabin.setPosition(car->bodyPosition + rotateBodyOffset(sf::Vector2f{bodySize.x * 0.05f, -bodySize.y * 0.25f}));
        cabin.setFillColor(sf::Color(90, 140, 160, 200));
        cabin.setOutlineColor(sf::Color(40, 40, 40, 220));
        cabin.setOutlineThickness(4.f);
        target.draw(cabin);
    }

}

void GarageScene::drawMainMenuContent(sf::RenderTarget& target)
{
    drawPanel(target, sf::Vector2f{0.f, 96.f}, sf::Vector2f{210.f, 235.f}, PanelColor);
}

void GarageScene::drawCarBrowserContent(sf::RenderTarget& target, bool isParking)
{
    if (isParking)
        drawPanel(target, sf::Vector2f{0.f, 816.f}, sf::Vector2f{240.f, 180.f}, PanelColor);

    drawText(target, currentCarPrice(isParking), sf::Vector2f{960.f, 822.f}, 31, true);
    drawText(target, currentCarName(isParking), sf::Vector2f{960.f, 876.f}, 36, true);
}

void GarageScene::drawPartBrowserContent(sf::RenderTarget& target, bool isTuning)
{
    drawPanel(target, sf::Vector2f{0.f, 145.f}, sf::Vector2f{290.f, 620.f}, PanelColor);
    drawText(target, "Parts", sf::Vector2f{145.f, 185.f}, 43, true);

    if (isTuning)
    {
        drawPanel(target, sf::Vector2f{0.f, 816.f}, sf::Vector2f{240.f, 180.f}, PanelColor);
        drawText(target, currentTuningAmount(), sf::Vector2f{960.f, 772.f}, 31, true);
    }

    drawText(target, currentPartPrice(isTuning), sf::Vector2f{960.f, 822.f}, 31, true);
    drawText(target, currentPartName(isTuning), sf::Vector2f{960.f, 876.f}, 36, true);
}

void GarageScene::drawRaceTypesContent(sf::RenderTarget& target)
{
    drawPanel(target, sf::Vector2f{0.f, 160.f}, sf::Vector2f{395.f, 220.f}, PanelColor);
    drawText(target, "Select Race", sf::Vector2f{205.f, 205.f}, 45, true);
}

void GarageScene::drawSkillChoiceOverlay(sf::RenderTarget& target)
{
    drawPanel(target, sf::Vector2f{530.f, 195.f}, sf::Vector2f{860.f, 690.f}, sf::Color(0, 0, 0, 175));
    drawText(target, "Choose skills!", sf::Vector2f{960.f, 255.f}, 52, true);
    drawText(target, "Points left: " + std::to_string(game().database().currentPlayer.skillPoints), sf::Vector2f{960.f, 315.f}, 36, true);
}

void GarageScene::drawPanel(sf::RenderTarget& target, sf::Vector2f position, sf::Vector2f size, sf::Color color)
{
    sf::RectangleShape panel(size);
    panel.setPosition(position);
    panel.setFillColor(color);
    target.draw(panel);
}

void GarageScene::drawText(sf::RenderTarget& target, const std::string& text, sf::Vector2f position, unsigned int size, bool centered, sf::Color color)
{
    sf::Text drawableText(game().assets().mainFont(), text, size);
    drawableText.setFillColor(color);
    drawableText.setOutlineColor(sf::Color(0, 0, 0, 110));
    drawableText.setOutlineThickness(2.f);

    if (centered)
    {
        const sf::FloatRect bounds = drawableText.getLocalBounds();
        drawableText.setOrigin({
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        });
    }

    drawableText.setPosition(position);
    target.draw(drawableText);
}

void GarageScene::drawRightAlignedText(sf::RenderTarget& target, const std::string& text, sf::Vector2f rightMiddle, unsigned int size, sf::Color color)
{
    sf::Text drawableText(game().assets().mainFont(), text, size);
    drawableText.setFillColor(color);
    drawableText.setOutlineColor(sf::Color(0, 0, 0, 110));
    drawableText.setOutlineThickness(2.f);

    const sf::FloatRect bounds = drawableText.getLocalBounds();
    drawableText.setOrigin({
        bounds.position.x + bounds.size.x,
        bounds.position.y + bounds.size.y / 2.f
    });
    drawableText.setPosition(rightMiddle);
    target.draw(drawableText);
}

void GarageScene::drawIcon(sf::RenderTarget& target, const sf::Texture& texture, sf::Vector2f position, float targetHeight)
{
    const sf::Vector2u textureSize = texture.getSize();
    if (textureSize.x == 0 || textureSize.y == 0)
        return;

    sf::Sprite sprite(texture);
    const float scale = targetHeight / static_cast<float>(textureSize.y);
    sprite.setScale({scale, scale});
    sprite.setPosition(position);
    target.draw(sprite);
}

std::string GarageScene::currentCarName(bool isParking)
{
    if (isParking)
        return m_selectedParkingCar ? m_selectedParkingCar->carName : "No Cars";

    return m_selectedDealershipCar ? m_selectedDealershipCar->carName : "No Cars";
}

std::string GarageScene::currentCarPrice(bool isParking)
{
    if (isParking)
        return m_selectedParkingCar ? formatPrice(m_selectedParkingCar->price, m_selectedParkingCar->priceType) : "";

    const auto& inventory = game().database().dealership.inventory;
    if (inventory.empty() || m_dealershipIndex < 0 || m_dealershipIndex >= static_cast<int>(inventory.size()))
        return "";

    const auto& dealerCar = inventory[static_cast<std::size_t>(m_dealershipIndex)];
    int price = dealerCar.carPrice;
    if (dealerCar.priceType == GameData::PriceType::Money)
        price = std::max(0, price - price * game().database().currentPlayer.shopDiscount / 100);

    return formatPrice(price, dealerCar.priceType);
}

std::string GarageScene::currentPartName(bool isTuning)
{
    const GameData::Part* part = isTuning ? m_selectedTuningPart : m_selectedShopPart;
    return part ? part->partName : "No parts";
}

std::string GarageScene::currentPartPrice(bool isTuning)
{
    const GameData::Part* part = isTuning ? m_selectedTuningPart : m_selectedShopPart;
    if (!part)
        return "";

    int price = part->price;
    if (!isTuning)
        price = std::max(0, price - price * game().database().currentPlayer.shopDiscount / 100);

    return formatPrice(price, GameData::PriceType::Money);
}

std::string GarageScene::currentTuningAmount()
{
    if (!m_selectedTuningPart)
        return "Amount: 0";

    return "Amount: " + std::to_string(game().database().currentPlayer.getPartCount(m_selectedTuningPart->id));
}

std::string GarageScene::padNumber(int value, int width)
{
    (void)width;
    return std::to_string(std::max(0, value));
}

std::string GarageScene::formatPrice(int price, GameData::PriceType type)
{
    if (type == GameData::PriceType::Gold)
        return std::to_string(price) + " G";

    return std::to_string(price) + " $";
}
