#pragma once

#include "GameEnums.h"

#include <array>
#include <string>
#include <vector>

namespace GameData
{
    struct Part
    {
        int id = 0;
        std::string partName;
        int price = 0;
        PartType type = PartType::Engine;

        // Engine
        int power = 0;
        int maxRPM = 0;
        int idleRPM = 900;
        int engineWeight = 0;
        // Power curve points by RPM percent:
        // 0%, 20%, 40%, 60%, 80%, 100%. Values are multipliers from 0.00 to 1.00.
        std::array<float, 6> powerCurveMultipliers{0.f, 0.45f, 0.70f, 0.95f, 1.00f, 0.78f};

        // Turbo
        int turboPowerBonus = 0;

        // EngineBlock
        int blockWeight = 0;

        // Pistons
        int pistonRPMIncrease = 0;

        // AirFilter
        int airPowerBonus = 0;

        // Intercooler
        int intercoolerPowerBonus = 0;

        // ECU
        int ecuPowerBonus = 0;
        int ecuRPMIncrease = 0;

        // Exhaust
        int exhaustPowerBonus = 0;

        // Suspension
        int grip = 0;
        int suspensionWeight = 0;

        // Tires
        int tireGrip = 0;
        std::string wheelSpritePath;

        // BodyWeightReduction
        float weightMultiplier = 1.f;

        // Transmission
        TransmissionType transmissionType = TransmissionType::RWD;
        int transmissionWeight = 0;

        // Gearbox
        int gearCount = 5;
        std::vector<float> gearRatios;
        float finalRatio = 3.5f;
        // Shift time is stored in milliseconds.
        // Example: 1000 = 1 second without power while shifting.
        int shiftTimeMs = 1000;

        // Clutch reduces gearbox shift time in milliseconds.
        int shiftTimeReductionMs = 0;

    };
}
