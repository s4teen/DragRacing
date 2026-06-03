#pragma once

#include "Car.h"
#include "Part.h"

#include <string>
#include <vector>

namespace GameData
{
    class GameDatabase;

    struct InventoryPart
    {
        int partId = 0;
        int count = 0;
    };

    class Player
    {
    public:
        std::string nickname = "Player";
        int money = 10000;
        int gold = 100;
        int experience = 0;
        int level = 1;
        int skillPoints = 0;

        // Each point = 1% bonus.
        int moneyBonus = 0;
        int goldBonus = 0;
        int experienceBonus = 0;
        int powerBonus = 0;
        int gripBonus = 0;
        int shopDiscount = 0;

        std::vector<InventoryPart> inventory;
        std::vector<Car> carInventory;
        int selectedCarIndex = 0;

        Car* getSelectedCar();
        const Car* getSelectedCar() const;

        void addPartToInventory(int partId, int amount = 1);
        bool removePartFromInventory(int partId, int amount = 1);
        int getPartCount(int partId) const;

        int addMoney(int amount);
        int addGold(int amount);
        int addExperience(int amount);
        bool spendSkillPoint(int& skillValue);

        static int experienceRequiredForNextLevel(int currentLevel);
        static int totalExperienceRequiredForLevel(int targetLevel);

    private:
        static int applyPositivePercentBonus(int amount, int percent);
    };
}
