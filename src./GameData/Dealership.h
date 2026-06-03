#pragma once

#include "GameEnums.h"

#include <string>
#include <vector>

namespace GameData
{
    class GameDatabase;
    class Player;
    struct Car;

    struct DealershipCar
    {
        int carId = 0;
        int carPrice = 0;
        PriceType priceType = PriceType::Money;
        std::string carName;
    };

    class Dealership
    {
    public:
        std::vector<DealershipCar> inventory;

        void addCar(const DealershipCar& car);
        bool removeCar(int carId);
        bool hasCar(int carId) const;
        const DealershipCar* getCar(int carId) const;
        bool buyCar(Player& player, int carId, const GameDatabase& database) const;
    };
}
