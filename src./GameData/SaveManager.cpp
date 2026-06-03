#include "SaveManager.h"

#include "GameDatabase.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace GameData
{
    namespace
    {
        std::string trim(const std::string& text)
        {
            const auto first = text.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
                return "";
            const auto last = text.find_last_not_of(" \t\r\n");
            return text.substr(first, last - first + 1);
        }

        int parseInt(const std::unordered_map<std::string, std::string>& values,
                     const std::string& key,
                     int fallback)
        {
            auto it = values.find(key);
            if (it == values.end())
                return fallback;

            try
            {
                return std::stoi(it->second);
            }
            catch (...)
            {
                return fallback;
            }
        }

        std::vector<std::string> split(const std::string& text, char delimiter)
        {
            std::vector<std::string> result;
            std::stringstream stream(text);
            std::string item;
            while (std::getline(stream, item, delimiter))
                result.push_back(item);
            return result;
        }

        std::string joinPartIds(const std::vector<int>& ids)
        {
            std::ostringstream stream;
            for (std::size_t i = 0; i < ids.size(); ++i)
            {
                if (i > 0)
                    stream << ',';
                stream << ids[i];
            }
            return stream.str();
        }

        bool writePlayerToStream(std::ostream& file, const Player& player)
        {
            if (!file.good())
                return false;

            file << "nickname=" << player.nickname << '\n';
            file << "money=" << player.money << '\n';
            file << "gold=" << player.gold << '\n';
            file << "experience=" << player.experience << '\n';
            file << "level=" << player.level << '\n';
            file << "skillPoints=" << player.skillPoints << '\n';
            file << "moneyBonus=" << player.moneyBonus << '\n';
            file << "goldBonus=" << player.goldBonus << '\n';
            file << "experienceBonus=" << player.experienceBonus << '\n';
            file << "powerBonus=" << player.powerBonus << '\n';
            file << "gripBonus=" << player.gripBonus << '\n';
            file << "shopDiscount=" << player.shopDiscount << '\n';
            file << "selectedCarIndex=" << player.selectedCarIndex << '\n';

            file << "inventory=";
            for (std::size_t i = 0; i < player.inventory.size(); ++i)
            {
                if (i > 0)
                    file << ',';
                file << player.inventory[i].partId << ':' << player.inventory[i].count;
            }
            file << '\n';

            file << "carCount=" << player.carInventory.size() << '\n';
            for (std::size_t i = 0; i < player.carInventory.size(); ++i)
            {
                const Car& car = player.carInventory[i];
                file << "car" << i << "=" << car.id << '|' << joinPartIds(car.installedPartIds) << '\n';
            }

            file.flush();
            return file.good();
        }
    }

    bool SaveManager::save(const Player& player, const std::string& path)
    {
        try
        {
            const std::filesystem::path savePath(path);
            std::error_code ec;

            if (savePath.has_parent_path())
            {
                std::filesystem::create_directories(savePath.parent_path(), ec);
                if (ec)
                    return false;
            }

            const std::filesystem::path tempPath = savePath.string() + ".tmp";

            {
                std::ofstream file(tempPath, std::ios::trunc);
                if (!file.is_open())
                    return false;

                if (!writePlayerToStream(file, player))
                    return false;
            }

            std::filesystem::rename(tempPath, savePath, ec);
            if (ec)
            {
                ec.clear();
                std::filesystem::remove(savePath, ec);
                ec.clear();
                std::filesystem::rename(tempPath, savePath, ec);
                if (ec)
                    return false;
            }

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool SaveManager::load(Player& player, const GameDatabase& database, const std::string& path)
    {
        try
        {
            std::ifstream file(path);
            if (!file.is_open())
                return false;

            std::unordered_map<std::string, std::string> values;
            std::string line;
            while (std::getline(file, line))
            {
                const auto equals = line.find('=');
                if (equals == std::string::npos)
                    continue;

                values[trim(line.substr(0, equals))] = trim(line.substr(equals + 1));
            }

            Player loaded;
            if (auto it = values.find("nickname"); it != values.end() && !it->second.empty())
                loaded.nickname = it->second;

            loaded.money = parseInt(values, "money", 10000);
            loaded.gold = parseInt(values, "gold", 100);
            loaded.experience = parseInt(values, "experience", 0);
            loaded.level = std::clamp(parseInt(values, "level", 1), 1, 200);
            loaded.skillPoints = std::clamp(parseInt(values, "skillPoints", 0), 0, 10000);
            loaded.moneyBonus = std::clamp(parseInt(values, "moneyBonus", 0), 0, 10000);
            loaded.goldBonus = std::clamp(parseInt(values, "goldBonus", 0), 0, 10000);
            loaded.experienceBonus = std::clamp(parseInt(values, "experienceBonus", 0), 0, 10000);
            loaded.powerBonus = std::clamp(parseInt(values, "powerBonus", 0), 0, 10000);
            loaded.gripBonus = std::clamp(parseInt(values, "gripBonus", 0), 0, 10000);
            loaded.shopDiscount = std::clamp(parseInt(values, "shopDiscount", 0), 0, 90);
            loaded.selectedCarIndex = parseInt(values, "selectedCarIndex", 0);

            if (auto it = values.find("inventory"); it != values.end() && !it->second.empty())
            {
                for (const std::string& entry : split(it->second, ','))
                {
                    const auto colon = entry.find(':');
                    if (colon == std::string::npos)
                        continue;

                    try
                    {
                        const int id = std::stoi(entry.substr(0, colon));
                        const int count = std::stoi(entry.substr(colon + 1));
                        if (id > 0 && count > 0 && database.getPartById(id))
                            loaded.inventory.push_back({id, count});
                    }
                    catch (...)
                    {
                    }
                }
            }

            const int carCount = std::clamp(parseInt(values, "carCount", 0), 0, 10000);
            for (int i = 0; i < carCount; ++i)
            {
                const std::string key = "car" + std::to_string(i);
                auto it = values.find(key);
                if (it == values.end())
                    continue;

                const auto pipe = it->second.find('|');
                if (pipe == std::string::npos)
                    continue;

                try
                {
                    const int carId = std::stoi(it->second.substr(0, pipe));
                    Car car = database.cloneCar(carId);
                    if (car.id == 0)
                        continue;

                    car.installedPartIds.clear();
                    const std::string parts = it->second.substr(pipe + 1);
                    for (const std::string& partIdText : split(parts, ','))
                    {
                        if (partIdText.empty())
                            continue;
                        const int partId = std::stoi(partIdText);
                        if (database.getPartById(partId))
                            car.installedPartIds.push_back(partId);
                    }

                    loaded.carInventory.push_back(car);
                }
                catch (...)
                {
                }
            }

            if (loaded.selectedCarIndex < 0 || loaded.selectedCarIndex >= static_cast<int>(loaded.carInventory.size()))
                loaded.selectedCarIndex = 0;

            player = loaded;
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool SaveManager::hasSave(const std::string& path)
    {
        std::error_code ec;
        return std::filesystem::exists(path, ec) && !ec;
    }

    void SaveManager::deleteSave(const std::string& path)
    {
        std::error_code ignored;
        std::filesystem::remove(path, ignored);
    }
}
