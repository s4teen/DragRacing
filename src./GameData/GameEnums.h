#pragma once

#include <string>

namespace GameData
{
    enum class PartType
    {
        Engine = 0,
        Turbo,
        EngineBlock,
        Pistons,
        AirFilter,
        Intercooler,
        ECU,
        ExhaustSystem,
        Suspension,
        Tires,
        BodyWeightReduction,
        Transmission,
        Gearbox,
        Clutch
    };

    enum class TransmissionType
    {
        RWD,
        AWD,
        FWD
    };

    enum class PriceType
    {
        Money = 1,
        Gold = 2
    };

    enum class RaceType
    {
        Short = 1,
        Medium = 2,
        Long = 3,
        Free = 4,
        Bot = 5
    };

    enum class PlayerSkill
    {
        MoneyBonus = 1,
        GoldBonus = 2,
        ExperienceBonus = 3,
        ShopDiscount = 4
    };

    std::string toString(PartType type);
    std::string toShortString(TransmissionType type);
    std::string toString(PriceType type);

    bool partTypeFromIndex(int index, PartType& type);
    int partTypeIndex(PartType type);
}
