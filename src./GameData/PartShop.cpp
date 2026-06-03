#include "PartShop.h"

#include "GameDatabase.h"
#include "Player.h"

#include <algorithm>

namespace GameData
{
    void PartShop::addPart(const PartShopPart& part)
    {
        inventory.push_back(part);
    }

    bool PartShop::removePart(int partId)
    {
        const auto oldSize = inventory.size();
        inventory.erase(std::remove_if(inventory.begin(), inventory.end(), [&](const PartShopPart& part)
        {
            return part.partId == partId;
        }), inventory.end());
        return inventory.size() != oldSize;
    }

    bool PartShop::hasPart(int partId) const
    {
        return getPart(partId) != nullptr;
    }

    const PartShopPart* PartShop::getPart(int partId) const
    {
        auto it = std::find_if(inventory.begin(), inventory.end(), [&](const PartShopPart& part)
        {
            return part.partId == partId;
        });
        return it == inventory.end() ? nullptr : &*it;
    }

    bool PartShop::buyPart(Player& player, int partId, const GameDatabase& database) const
    {
        const PartShopPart* shopPart = getPart(partId);
        if (!shopPart || !database.getPartById(partId))
            return false;

        const int price = std::max(0, shopPart->partPrice - (shopPart->partPrice * player.shopDiscount / 100));
        if (player.money < price)
            return false;

        player.addMoney(-price);
        player.addPartToInventory(partId);
        return true;
    }
}
