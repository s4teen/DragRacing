#pragma once

#include "../GameData/Car.h"
#include "../GameData/GameDatabase.h"

#include <array>
#include <vector>

namespace Race
{
    struct RaceVehicleInput
    {
        float throttle = 0.f; // 0..1
        float brake = 0.f;    // 0..1
    };

    class RaceVehiclePhysics
    {
    public:
        void setup(const GameData::Car& car, const GameData::GameDatabase& database);
        void setVisualTuning(float pixelsPerMeter, float visualWheelRadiusPixels);
        void applyPerformanceMultipliers(float powerMultiplier, float gripMultiplier, float massMultiplier, float shiftTimeMultiplier);
        void reset();
        void update(float dt, RaceVehicleInput input);

        void shiftUp();
        void shiftDown();

        float speedMps() const;
        float speedKmh() const;
        float speedMph() const;
        float rpm() const;
        int gear() const;
        float distanceMeters() const;
        float wheelRotationDegrees() const;
        float bodyDynamicRotationDegrees() const;
        float cameraOffsetX() const;
        bool isShifting() const;
        float slip01() const;
        float accelerationMps2() const;
        float currentPowerCurveMultiplier() const;
        float maxRpm() const;
        int maxGear() const;

    private:
        float m_speedMps = 0.f;
        float m_rpm = 900.f;
        int m_gear = 0;
        float m_distanceMeters = 0.f;
        float m_wheelRotationDegrees = 0.f;
        float m_bodyDynamicRotationDegrees = 0.f;
        float m_cameraOffsetX = 0.f;
        float m_shiftTimer = 0.f;
        float m_slip01 = 0.f;
        float m_accelerationMps2 = 0.f;
        float m_lastThrottle = 0.f;

        float m_powerHp = 100.f;
        float m_maxRpm = 7000.f;
        float m_idleRpm = 900.f;
        float m_massKg = 1200.f;
        float m_grip = 40.f;
        float m_wheelRadiusM = 0.34f;
        float m_visualPixelsPerMeter = 72.f;
        float m_visualWheelRadiusPixels = 42.f;
        float m_finalRatio = 3.5f;
        float m_drivetrainEfficiency = 0.62f;
        int m_shiftTimeMs = 1000;
        std::vector<float> m_gearRatios{3.5f, 2.1f, 1.5f, 1.1f, 0.8f};
        std::array<float, 6> m_powerCurve{0.f, 0.45f, 0.70f, 0.95f, 1.00f, 0.78f};

        float powerCurveAt(float rpm) const;
        float rpmForSpeedAndGear(float speedMps, int gear) const;
        float maxSpeedForGear(int gear) const;
        void updateRpmFromSpeed();
        void updateNeutralRpm(float dt, float throttle);
        static float approach(float current, float target, float speed, float dt);
    };
}
