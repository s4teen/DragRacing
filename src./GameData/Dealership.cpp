#include "Dealership.h"

#include "GameDatabase.h"
#include "Player.h"

#include <algorithm>

namespace GameData
{
    void Dealership::addCar(const DealershipCar& car)
    {
        inventory.push_back(car);
    }

    bool Dealership::removeCar(int carId)
    {
        const auto oldSize = inventory.size();
        inventory.erase(std::remove_if(inventory.begin(), inventory.end(), [&](const DealershipCar& car)
        {
            return car.carId == carId;
        }), inventory.end());
        return inventory.size() != oldSize;
    }

    bool Dealership::hasCar(int carId) const
    {
        return getCar(carId) != nullptr;
    }

    const DealershipCar* Dealership::getCar(int carId) const
    {
        auto it = std::find_if(inventory.begin(), inventory.end(), [&](const DealershipCar& car)
        {
            return car.carId == carId;
        });
        return it == inventory.end() ? nullptr : &*it;
    }

    bool Dealership::buyCar(Player& player, int carId, const GameDatabase& database) const
    {
        const DealershipCar* dealerCar = getCar(carId);
        const Car* sourceCar = database.getCarById(carId);
        if (!dealerCar || !sourceCar)
            return false;

        const int discount = dealerCar->priceType == PriceType::Money ? player.shopDiscount : 0;
        const int price = std::max(0, dealerCar->carPrice - (dealerCar->carPrice * discount / 100));

        if (dealerCar->priceType == PriceType::Money)
        {
            if (player.money < price)
                return false;
            player.addMoney(-price);
        }
        else
        {
            if (player.gold < dealerCar->carPrice)
                return false;
            player.addGold(-dealerCar->carPrice);
        }

        Car newCar = *sourceCar;
        newCar.price = dealerCar->carPrice;
        newCar.priceType = dealerCar->priceType;
        if (!dealerCar->carName.empty())
            newCar.carName = dealerCar->carName;

        player.carInventory.push_back(newCar);
        player.selectedCarIndex = static_cast<int>(player.carInventory.size()) - 1;
        return true;
    }
}
