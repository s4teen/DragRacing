#include "RaceVehiclePhysics.h"

#include <algorithm>
#include <cmath>

namespace Race
{
    namespace
    {
        constexpr float Pi = 3.1415926535f;
        constexpr float Gravity = 9.81f;
        constexpr float HorsepowerToWatts = 745.7f;
        constexpr float BaseDrivetrainEfficiency = 0.62f;
        constexpr float HighPowerEfficiencyBonus = 0.10f;
        constexpr float AerodynamicCdA = 0.78f;
        constexpr float RollingResistanceCoefficient = 0.017f;

        // Чем меньше это значение, тем слабее кузов клюет носом / приседает при разгоне.
        constexpr float BodyPitchStrength = 0.05f;
        constexpr float MaxAccelerationPitchDegrees = 1.1f;
        constexpr float MaxBrakePitchDegrees = 1.2f;
    }

    void RaceVehiclePhysics::setup(const GameData::Car& car, const GameData::GameDatabase& database)
    {
        m_powerHp = static_cast<float>(std::max(60, car.getPower(database)));
        m_maxRpm = static_cast<float>(std::max(3000, car.getMaxRPM(database)));
        m_idleRpm = static_cast<float>(std::clamp(car.getIdleRPM(database), 500, static_cast<int>(m_maxRpm) - 300));
        m_massKg = static_cast<float>(std::max(600, car.getWeight(database)));
        m_grip = static_cast<float>(std::max(1, car.getGrip(database)));
        m_shiftTimeMs = std::max(1, car.getShiftTimeMs(database));
        m_gearRatios = car.getGearRatios(database);
        if (m_gearRatios.empty())
            m_gearRatios = {3.5f, 2.1f, 1.5f, 1.1f, 0.8f};
        m_finalRatio = std::max(0.1f, car.getFinalRatio(database));
        m_powerCurve = car.getPowerCurveMultipliers(database);

        // Не вся паспортная мощность превращается в полезное ускорение.
        // У маломощных машин потери/провалы сильнее, у мощных машин запас тяги выше.
        const float hpPerTon = m_powerHp / std::max(0.1f, m_massKg / 1000.f);
        const float highPower01 = std::clamp((hpPerTon - 150.f) / 300.f, 0.f, 1.f);
        m_drivetrainEfficiency = BaseDrivetrainEfficiency + HighPowerEfficiencyBonus * highPower01;

        reset();
    }


    void RaceVehiclePhysics::setVisualTuning(float pixelsPerMeter, float visualWheelRadiusPixels)
    {
        m_visualPixelsPerMeter = std::max(1.f, pixelsPerMeter);
        m_visualWheelRadiusPixels = std::max(1.f, visualWheelRadiusPixels);
    }

    void RaceVehiclePhysics::applyPerformanceMultipliers(float powerMultiplier, float gripMultiplier, float massMultiplier, float shiftTimeMultiplier)
    {
        m_powerHp = std::max(1.f, m_powerHp * std::max(0.05f, powerMultiplier));
        m_grip = std::max(1.f, m_grip * std::max(0.05f, gripMultiplier));
        m_massKg = std::max(100.f, m_massKg * std::max(0.05f, massMultiplier));
        m_shiftTimeMs = std::max(1, static_cast<int>(std::round(static_cast<float>(m_shiftTimeMs) * std::max(0.01f, shiftTimeMultiplier))));

        const float hpPerTon = m_powerHp / std::max(0.1f, m_massKg / 1000.f);
        const float highPower01 = std::clamp((hpPerTon - 150.f) / 300.f, 0.f, 1.f);
        m_drivetrainEfficiency = BaseDrivetrainEfficiency + HighPowerEfficiencyBonus * highPower01;
    }

    void RaceVehiclePhysics::reset()
    {
        m_speedMps = 0.f;
        m_rpm = m_idleRpm;
        m_gear = 1;
        m_distanceMeters = 0.f;
        m_wheelRotationDegrees = 0.f;
        m_bodyDynamicRotationDegrees = 0.f;
        m_cameraOffsetX = 0.f;
        m_shiftTimer = 0.f;
        m_slip01 = 0.f;
        m_accelerationMps2 = 0.f;
        m_lastThrottle = 0.f;
    }

    void RaceVehiclePhysics::update(float dt, RaceVehicleInput input)
    {
        dt = std::clamp(dt, 0.f, 1.f / 20.f);
        input.throttle = std::clamp(input.throttle, 0.f, 1.f);
        input.brake = std::clamp(input.brake, 0.f, 1.f);

        if (m_shiftTimer > 0.f)
        {
            m_shiftTimer = std::max(0.f, m_shiftTimer - dt);
            input.throttle = 0.f;
        }

        if (m_gear > 0)
            updateRpmFromSpeed();
        else
            updateNeutralRpm(dt, input.throttle);

        const float rpmSafe = std::max(m_rpm, m_idleRpm);
        const float curve = powerCurveAt(rpmSafe);
        const float enginePowerW = m_powerHp * HorsepowerToWatts * curve * input.throttle * m_drivetrainEfficiency;

        float driveForce = 0.f;
        if (m_gear > 0 && m_gear <= static_cast<int>(m_gearRatios.size()) && input.throttle > 0.f && m_shiftTimer <= 0.f)
        {
            const float currentGearRatio = m_gearRatios[static_cast<std::size_t>(m_gear - 1)];
            const float totalRatio = currentGearRatio * m_finalRatio;
            const float firstGearTotalRatio = m_gearRatios.empty() ? totalRatio : m_gearRatios.front() * m_finalRatio;

            const float engineAngularSpeed = std::max(90.f, rpmSafe * 2.f * Pi / 60.f);
            const float engineTorqueNm = enginePowerW / engineAngularSpeed;
            const float wheelTorqueNm = engineTorqueNm * totalRatio;

            // На очень низкой скорости сцепление/гидротрансформатор не должен передавать
            // одинаковую тягу на 1-й и 5-й передаче. Чем выше передача, тем сильнее машина
            // "душится" до скорости, на которой колесо уже может синхронно крутить мотор выше idleRPM.
            const float idleSyncSpeedMps = (m_idleRpm / 60.f) * (2.f * Pi * m_wheelRadiusM) / std::max(totalRatio, 0.01f);
            const float clutchEngagement01 = std::clamp(m_speedMps / std::max(idleSyncSpeedMps, 0.1f), 0.f, 1.f);
            const float mechanicalLeverage01 = std::clamp(totalRatio / std::max(firstGearTotalRatio, 0.01f), 0.08f, 1.f);
            const float lowSpeedLaunchFactor = clutchEngagement01 + (1.f - clutchEngagement01) * (0.38f * mechanicalLeverage01);

            driveForce = std::max(0.f, wheelTorqueNm / m_wheelRadiusM) * lowSpeedLaunchFactor;
        }

        const float gripCoefficient = std::clamp(0.55f + m_grip / 95.f, 0.70f, 1.45f);
        const float maxTractionForce = m_massKg * Gravity * gripCoefficient;
        const bool slipping = driveForce > maxTractionForce;
        const float tractionForce = std::min(driveForce, maxTractionForce);
        const float desiredSlip = slipping ? std::clamp((driveForce - maxTractionForce) / std::max(maxTractionForce, 1.f), 0.f, 1.f) : 0.f;
        m_slip01 = approach(m_slip01, desiredSlip, desiredSlip > m_slip01 ? 6.f : 3.5f, dt);

        const float dragForce = 0.5f * 1.225f * AerodynamicCdA * m_speedMps * m_speedMps;
        const float rollingForce = m_speedMps > 0.02f ? RollingResistanceCoefficient * m_massKg * Gravity : 0.f;
        const float brakeForce = input.brake * m_massKg * Gravity * 1.15f;

        float netForce = tractionForce - dragForce - rollingForce - brakeForce;
        if (m_speedMps <= 0.01f && netForce < 0.f)
            netForce = 0.f;

        m_accelerationMps2 = netForce / m_massKg;
        m_speedMps = std::max(0.f, m_speedMps + m_accelerationMps2 * dt);

        if (m_gear > 0 && m_gear <= static_cast<int>(m_gearRatios.size()))
        {
            const float gearMaxSpeed = maxSpeedForGear(m_gear);
            if (m_speedMps > gearMaxSpeed)
                m_speedMps = gearMaxSpeed;
        }

        if (m_gear > 0)
            updateRpmFromSpeed();
        else
            updateNeutralRpm(dt, input.throttle);

        if (m_speedMps < 0.3f && input.throttle < 0.05f && input.brake > 0.05f)
        {
            m_speedMps = 0.f;
            m_accelerationMps2 = 0.f;
        }

        const float distanceDeltaMeters = m_speedMps * dt;
        m_distanceMeters += distanceDeltaMeters;

        // Визуальная синхронизация колеса с дорожным полотном:
        // сколько пикселей проехала машина, настолько и проворачиваем колесо.
        const float roadPixelsDelta = distanceDeltaMeters * m_visualPixelsPerMeter;
        const float slipSpinMultiplier = 1.f + m_slip01 * input.throttle * 1.35f;
        m_wheelRotationDegrees += (roadPixelsDelta / m_visualWheelRadiusPixels) * 180.f / Pi * slipSpinMultiplier;

        const float pitchTarget = std::clamp(-m_accelerationMps2 * BodyPitchStrength, -MaxAccelerationPitchDegrees, MaxBrakePitchDegrees);
        m_bodyDynamicRotationDegrees = approach(m_bodyDynamicRotationDegrees, pitchTarget, 7.f, dt);

        const float cameraTarget = std::clamp(-m_accelerationMps2 * 1.75f, -10.f, 10.f);
        m_cameraOffsetX = approach(m_cameraOffsetX, cameraTarget, 5.5f, dt);

        m_lastThrottle = input.throttle;
    }

    void RaceVehiclePhysics::shiftUp()
    {
        if (m_shiftTimer > 0.f)
            return;

        if (m_gear < static_cast<int>(m_gearRatios.size()))
        {
            ++m_gear;
            m_shiftTimer = static_cast<float>(m_shiftTimeMs) / 1000.f;
            updateRpmFromSpeed();
        }
    }

    void RaceVehiclePhysics::shiftDown()
    {
        if (m_shiftTimer > 0.f || m_gear <= 0)
            return;

        if (m_gear == 1)
        {
            if (m_speedMps < 2.f)
                m_gear = 0;
            return;
        }

        const int targetGear = m_gear - 1;
        const float targetGearMaxSpeed = maxSpeedForGear(targetGear) * 0.98f;
        if (m_speedMps > targetGearMaxSpeed)
            return;

        m_gear = targetGear;
        m_shiftTimer = static_cast<float>(m_shiftTimeMs) / 1000.f;
        updateRpmFromSpeed();
    }

    float RaceVehiclePhysics::speedMps() const { return m_speedMps; }
    float RaceVehiclePhysics::speedKmh() const { return m_speedMps * 3.6f; }
    float RaceVehiclePhysics::speedMph() const { return m_speedMps * 2.236936f; }
    float RaceVehiclePhysics::rpm() const { return m_rpm; }
    int RaceVehiclePhysics::gear() const { return m_gear; }
    float RaceVehiclePhysics::distanceMeters() const { return m_distanceMeters; }
    float RaceVehiclePhysics::wheelRotationDegrees() const { return m_wheelRotationDegrees; }
    float RaceVehiclePhysics::bodyDynamicRotationDegrees() const { return m_bodyDynamicRotationDegrees; }
    float RaceVehiclePhysics::cameraOffsetX() const { return m_cameraOffsetX; }
    bool RaceVehiclePhysics::isShifting() const { return m_shiftTimer > 0.f; }
    float RaceVehiclePhysics::slip01() const { return m_slip01; }
    float RaceVehiclePhysics::accelerationMps2() const { return m_accelerationMps2; }
    float RaceVehiclePhysics::currentPowerCurveMultiplier() const { return powerCurveAt(m_rpm); }
    float RaceVehiclePhysics::maxRpm() const { return m_maxRpm; }
    int RaceVehiclePhysics::maxGear() const { return static_cast<int>(m_gearRatios.size()); }

    float RaceVehiclePhysics::powerCurveAt(float rpm) const
    {
        const float maxRpm = std::max(m_maxRpm, 1000.f);
        const float normalized = std::clamp(rpm / maxRpm, 0.f, 1.f);
        const float scaled = normalized * 5.f;
        const int index = std::clamp(static_cast<int>(std::floor(scaled)), 0, 4);
        const float t = std::clamp(scaled - static_cast<float>(index), 0.f, 1.f);
        const float a = std::clamp(m_powerCurve[static_cast<std::size_t>(index)], 0.f, 1.f);
        const float b = std::clamp(m_powerCurve[static_cast<std::size_t>(index + 1)], 0.f, 1.f);
        return a + (b - a) * t;
    }

    float RaceVehiclePhysics::rpmForSpeedAndGear(float speedMps, int gear) const
    {
        if (gear <= 0 || gear > static_cast<int>(m_gearRatios.size()))
            return m_idleRpm;

        const float wheelRps = speedMps / (2.f * Pi * m_wheelRadiusM);
        const float engineRpm = wheelRps * m_gearRatios[static_cast<std::size_t>(gear - 1)] * m_finalRatio * 60.f;
        return std::clamp(engineRpm, m_idleRpm, m_maxRpm);
    }

    float RaceVehiclePhysics::maxSpeedForGear(int gear) const
    {
        if (gear <= 0 || gear > static_cast<int>(m_gearRatios.size()))
            return 0.f;

        const float totalRatio = m_gearRatios[static_cast<std::size_t>(gear - 1)] * m_finalRatio;
        if (totalRatio <= 0.f)
            return 0.f;

        return (m_maxRpm / 60.f) * (2.f * Pi * m_wheelRadiusM) / totalRatio;
    }

    void RaceVehiclePhysics::updateRpmFromSpeed()
    {
        if (m_gear > 0)
            m_rpm = rpmForSpeedAndGear(m_speedMps, m_gear);
    }

    void RaceVehiclePhysics::updateNeutralRpm(float dt, float throttle)
    {
        // На нейтрали мотор крутится свободно, но не уходит в бесконечность.
        // Газ ведет обороты к redline, отпускание газа возвращает к idleRPM.
        const float limiterRpm = m_maxRpm * 0.985f;
        const float targetRpm = throttle > 0.05f
            ? m_idleRpm + (limiterRpm - m_idleRpm) * std::clamp(throttle, 0.f, 1.f)
            : m_idleRpm;

        const float response = throttle > 0.05f ? 5.0f : 6.0f;
        m_rpm = std::clamp(approach(m_rpm, targetRpm, response, dt), m_idleRpm, limiterRpm);
    }

    float RaceVehiclePhysics::approach(float current, float target, float speed, float dt)
    {
        const float factor = 1.f - std::exp(-speed * dt);
        return current + (target - current) * factor;
    }
}
