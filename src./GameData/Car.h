#pragma once

#include "GameEnums.h"

#include <SFML/System/Vector2.hpp>
#include <array>
#include <string>
#include <vector>

namespace GameData
{
    class GameDatabase;
    struct Part;

    struct Car
    {
        int id = 0;
        std::string carName = "Car";
        int price = 0;
        PriceType priceType = PriceType::Money;
        int bodyWeight = 0;
        std::string bodySpritePath;
        sf::Vector2f bodyPosition{960.f, 560.f};
        sf::Vector2f bodySize{520.f, 240.f};
        float bodyRotationDegrees = 0.f;
        // Визуально машина состоит только из трёх отдельных элементов:
        // заднее колесо, переднее колесо и кузов. Колёса рисуются ниже кузова.
        sf::Vector2f rearWheelPos{-135.f, 72.f};
        sf::Vector2f frontWheelPos{150.f, 72.f};
        sf::Vector2f wheelSize{78.f, 78.f};
        sf::Vector2f carScale{1.f, 1.f};

        std::vector<int> installedPartIds;

        const Part* getPart(PartType type, const GameDatabase& database) const;
        std::vector<const Part*> getInstalledParts(const GameDatabase& database) const;

        int getPower(const GameDatabase& database) const;
        int getMaxRPM(const GameDatabase& database) const;
        int getIdleRPM(const GameDatabase& database) const;
        std::array<float, 6> getPowerCurveMultipliers(const GameDatabase& database) const;
        int getWeight(const GameDatabase& database) const;
        int getGrip(const GameDatabase& database) const;
        int getShiftTimeMs(const GameDatabase& database) const;
        TransmissionType getTransmissionType(const GameDatabase& database) const;
        int getGearCount(const GameDatabase& database) const;
        std::vector<float> getGearRatios(const GameDatabase& database) const;
        float getFinalRatio(const GameDatabase& database) const;

        void installPart(const Part& part, const GameDatabase& database);
        void removePart(PartType type, const GameDatabase& database);
    };
}
