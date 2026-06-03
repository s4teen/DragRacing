#include "GameEnums.h"

namespace GameData
{
    std::string toString(PartType type)
    {
        switch (type)
        {
        case PartType::Engine: return "Engine";
        case PartType::Turbo: return "Turbo";
        case PartType::EngineBlock: return "Engine Block";
        case PartType::Pistons: return "Pistons";
        case PartType::AirFilter: return "Air Filter";
        case PartType::Intercooler: return "Intercooler";
        case PartType::ECU: return "ECU";
        case PartType::ExhaustSystem: return "Exhaust";
        case PartType::Suspension: return "Suspension";
        case PartType::Tires: return "Tires";
        case PartType::BodyWeightReduction: return "Weight Reduction";
        case PartType::Transmission: return "Transmission";
        case PartType::Gearbox: return "Gearbox";
        case PartType::Clutch: return "Clutch";
        default: return "Unknown";
        }
    }

    std::string toShortString(TransmissionType type)
    {
        switch (type)
        {
        case TransmissionType::RWD: return "RWD";
        case TransmissionType::AWD: return "AWD";
        case TransmissionType::FWD: return "FWD";
        default: return "-";
        }
    }

    std::string toString(PriceType type)
    {
        switch (type)
        {
        case PriceType::Money: return "Money";
        case PriceType::Gold: return "Gold";
        default: return "Money";
        }
    }

    bool partTypeFromIndex(int index, PartType& type)
    {
        if (index < 0 || index > static_cast<int>(PartType::Clutch))
            return false;

        type = static_cast<PartType>(index);
        return true;
    }

    int partTypeIndex(PartType type)
    {
        return static_cast<int>(type);
    }
}
