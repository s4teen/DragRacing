#pragma once

#include "GameEnums.h"

#include <vector>

namespace GameData
{
    class GameDatabase;
    class Player;

    struct PartShopPart
    {
        int partId = 0;
        int partPrice = 0;
        PartType partType = PartType::Engine;
    };

    class PartShop
    {
    public:
        std::vector<PartShopPart> inventory;

        void addPart(const PartShopPart& part);
        bool removePart(int partId);
        bool hasPart(int partId) const;
        const PartShopPart* getPart(int partId) const;
        bool buyPart(Player& player, int partId, const GameDatabase& database) const;
    };
}
