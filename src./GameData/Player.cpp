#include "Player.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace GameData
{
    Car* Player::getSelectedCar()
    {
        if (carInventory.empty())
            return nullptr;

        if (selectedCarIndex < 0 || selectedCarIndex >= static_cast<int>(carInventory.size()))
            selectedCarIndex = 0;

        return &carInventory[static_cast<std::size_t>(selectedCarIndex)];
    }

    const Car* Player::getSelectedCar() const
    {
        if (carInventory.empty())
            return nullptr;

        int safeIndex = selectedCarIndex;
        if (safeIndex < 0 || safeIndex >= static_cast<int>(carInventory.size()))
            safeIndex = 0;

        return &carInventory[static_cast<std::size_t>(safeIndex)];
    }

    void Player::addPartToInventory(int partId, int amount)
    {
        if (partId <= 0 || amount <= 0)
            return;

        auto it = std::find_if(inventory.begin(), inventory.end(), [&](const InventoryPart& item)
        {
            return item.partId == partId;
        });

        if (it == inventory.end())
            inventory.push_back({partId, amount});
        else
            it->count += amount;
    }

    bool Player::removePartFromInventory(int partId, int amount)
    {
        if (partId <= 0 || amount <= 0)
            return false;

        auto it = std::find_if(inventory.begin(), inventory.end(), [&](const InventoryPart& item)
        {
            return item.partId == partId;
        });

        if (it == inventory.end() || it->count < amount)
            return false;

        it->count -= amount;
        if (it->count <= 0)
            inventory.erase(it);

        return true;
    }

    int Player::getPartCount(int partId) const
    {
        auto it = std::find_if(inventory.begin(), inventory.end(), [&](const InventoryPart& item)
        {
            return item.partId == partId;
        });

        return it == inventory.end() ? 0 : it->count;
    }

    int Player::addMoney(int amount)
    {
        const int delta = amount > 0 ? applyPositivePercentBonus(amount, moneyBonus) : amount;
        money = std::max(0, money + delta);
        return delta;
    }

    int Player::addGold(int amount)
    {
        const int delta = amount > 0 ? applyPositivePercentBonus(amount, goldBonus) : amount;
        gold = std::max(0, gold + delta);
        return delta;
    }

    int Player::addExperience(int amount)
    {
        const int delta = amount > 0 ? applyPositivePercentBonus(amount, experienceBonus) : amount;

        const long long nextExperience = static_cast<long long>(experience) + static_cast<long long>(delta);
        experience = static_cast<int>(std::clamp(nextExperience, 0LL, static_cast<long long>(std::numeric_limits<int>::max())));

        int safetyCounter = 0;
        while (level < 200 && safetyCounter < 200 && experience >= totalExperienceRequiredForLevel(level + 1))
        {
            ++level;
            ++skillPoints;
            addGold(100);
            ++safetyCounter;
        }

        return delta;
    }

    bool Player::spendSkillPoint(int& skillValue)
    {
        if (skillPoints <= 0)
            return false;

        --skillPoints;
        ++skillValue;
        return true;
    }

    int Player::experienceRequiredForNextLevel(int currentLevel)
    {
        const int safeLevel = std::max(1, currentLevel);
        const double requirement = 1000.0 * std::pow(1.15, static_cast<double>(safeLevel - 1));
        return std::max(1, static_cast<int>(std::round(requirement)));
    }

    int Player::totalExperienceRequiredForLevel(int targetLevel)
    {
        if (targetLevel <= 1)
            return 0;

        int total = 0;
        for (int levelToAdvanceFrom = 1; levelToAdvanceFrom < targetLevel; ++levelToAdvanceFrom)
            total += experienceRequiredForNextLevel(levelToAdvanceFrom);
        return total;
    }

    int Player::applyPositivePercentBonus(int amount, int percent)
    {
        if (amount <= 0 || percent <= 0)
            return amount;

        return static_cast<int>(std::round(static_cast<double>(amount) * (1.0 + static_cast<double>(percent) / 100.0)));
    }
}
