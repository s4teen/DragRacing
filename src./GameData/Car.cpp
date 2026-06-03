#include "Car.h"

#include "GameDatabase.h"
#include "Part.h"

#include <algorithm>
#include <cmath>

namespace GameData
{
    const Part* Car::getPart(PartType type, const GameDatabase& database) const
    {
        for (int partId : installedPartIds)
        {
            const Part* part = database.getPartById(partId);
            if (part && part->type == type)
                return part;
        }
        return nullptr;
    }

    std::vector<const Part*> Car::getInstalledParts(const GameDatabase& database) const
    {
        std::vector<const Part*> result;
        for (int partId : installedPartIds)
        {
            if (const Part* part = database.getPartById(partId))
                result.push_back(part);
        }
        return result;
    }

    int Car::getPower(const GameDatabase& database) const
    {
        int sum = 0;
        for (const Part* part : getInstalledParts(database))
        {
            sum += part->power;
            sum += part->turboPowerBonus;
            sum += part->airPowerBonus;
            sum += part->intercoolerPowerBonus;
            sum += part->ecuPowerBonus;
            sum += part->exhaustPowerBonus;
        }
        return sum;
    }

    int Car::getMaxRPM(const GameDatabase& database) const
    {
        int sum = 0;
        for (const Part* part : getInstalledParts(database))
        {
            sum += part->maxRPM;
            sum += part->pistonRPMIncrease;
            sum += part->ecuRPMIncrease;
        }
        return sum == 0 ? 7000 : sum;
    }


    int Car::getIdleRPM(const GameDatabase& database) const
    {
        if (const Part* engine = getPart(PartType::Engine, database))
            return std::clamp(engine->idleRPM, 500, std::max(600, getMaxRPM(database) - 250));
        return 900;
    }

    std::array<float, 6> Car::getPowerCurveMultipliers(const GameDatabase& database) const
    {
        if (const Part* engine = getPart(PartType::Engine, database))
            return engine->powerCurveMultipliers;
        return {0.f, 0.45f, 0.70f, 0.95f, 1.00f, 0.78f};
    }

    int Car::getWeight(const GameDatabase& database) const
    {
        float sum = static_cast<float>(bodyWeight);
        float multiplier = 1.f;

        for (const Part* part : getInstalledParts(database))
        {
            sum += static_cast<float>(part->engineWeight);
            sum += static_cast<float>(part->blockWeight);
            sum += static_cast<float>(part->suspensionWeight);
            sum += static_cast<float>(part->transmissionWeight);

            if (part->weightMultiplier > 0.f && part->weightMultiplier < 1.f)
                multiplier = part->weightMultiplier;
        }

        return static_cast<int>(std::round(sum * multiplier));
    }

    int Car::getGrip(const GameDatabase& database) const
    {
        int sum = 0;
        for (const Part* part : getInstalledParts(database))
        {
            sum += part->grip;
            sum += part->tireGrip;
        }
        return sum;
    }

    int Car::getShiftTimeMs(const GameDatabase& database) const
    {
        int gearboxShiftTimeMs = 1000;
        int shiftReductionMs = 0;

        if (const Part* gearbox = getPart(PartType::Gearbox, database))
            gearboxShiftTimeMs = gearbox->shiftTimeMs;

        if (const Part* clutch = getPart(PartType::Clutch, database))
            shiftReductionMs = clutch->shiftTimeReductionMs;

        // Keep a small minimum delay so shifting cannot become instant/negative.
        return std::max(1, gearboxShiftTimeMs - shiftReductionMs);
    }

    TransmissionType Car::getTransmissionType(const GameDatabase& database) const
    {
        if (const Part* part = getPart(PartType::Transmission, database))
            return part->transmissionType;
        return TransmissionType::RWD;
    }

    int Car::getGearCount(const GameDatabase& database) const
    {
        if (const Part* part = getPart(PartType::Gearbox, database))
            return part->gearCount;
        return 5;
    }

    std::vector<float> Car::getGearRatios(const GameDatabase& database) const
    {
        if (const Part* part = getPart(PartType::Gearbox, database); part && !part->gearRatios.empty())
            return part->gearRatios;
        return {3.5f, 2.1f, 1.5f, 1.1f, 0.8f};
    }

    float Car::getFinalRatio(const GameDatabase& database) const
    {
        if (const Part* part = getPart(PartType::Gearbox, database))
            return part->finalRatio;
        return 3.5f;
    }

    void Car::installPart(const Part& part, const GameDatabase& database)
    {
        removePart(part.type, database);
        installedPartIds.push_back(part.id);
    }

    void Car::removePart(PartType type, const GameDatabase& database)
    {
        installedPartIds.erase(
            std::remove_if(installedPartIds.begin(), installedPartIds.end(), [&](int id)
            {
                const Part* installedPart = database.getPartById(id);
                return installedPart && installedPart->type == type;
            }),
            installedPartIds.end()
        );
    }
}
