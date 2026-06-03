#pragma once

#include "Car.h"
#include "Dealership.h"
#include "Part.h"
#include "PartShop.h"
#include "Player.h"

#include <string>
#include <vector>

namespace GameData
{
    class GameDatabase
    {
    public:
        std::vector<Car> allCars;
        std::vector<Part> allParts;
        std::vector<int> shopPartIds;
        int starterCarId = 1;
        std::vector<int> starterPartIds;
        Dealership dealership;
        PartShop partShop;
        Player currentPlayer;

        int startMoney = 10000;
        int startGold = 100;
        int startExperience = 0;
        int startLevel = 1;

        void initialize();
        void createNewPlayer();
        void savePlayer() const;
        bool loadPlayer();
        void deletePlayerSave();

        const Car* getCarById(int id) const;
        const Part* getPartById(int id) const;
        std::vector<const Part*> getShopParts() const;
        std::vector<const Part*> getShopPartsOfType(PartType type) const;
        std::vector<const Part*> getPlayerPartsOfType(PartType type) const;
        Car cloneCar(int carId) const;

    private:
        void buildDefaultDatabase();
        void rebuildPartShopFromIds();
    };
}
