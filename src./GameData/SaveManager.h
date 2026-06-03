#pragma once

#include "Player.h"

#include <string>

namespace GameData
{
    class GameDatabase;

    class SaveManager
    {
    public:
        static bool save(const Player& player, const std::string& path = "save/player.ini");
        static bool load(Player& player, const GameDatabase& database, const std::string& path = "save/player.ini");
        static bool hasSave(const std::string& path = "save/player.ini");
        static void deleteSave(const std::string& path = "save/player.ini");
    };
}
