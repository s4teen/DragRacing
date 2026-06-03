#include "GameDatabase.h"

#include "SaveManager.h"

#include <algorithm>

namespace GameData
{
    namespace
    {
        Part makeBasePart(int id, const std::string& name, int price, PartType type)
        {
            Part part;
            part.id = id;
            part.partName = name;
            part.price = price;
            part.type = type;
            return part;
        }
    }

    void GameDatabase::initialize()
    {
        buildDefaultDatabase();
        rebuildPartShopFromIds();

        if (!loadPlayer())
        {
            createNewPlayer();
            savePlayer();
        }
    }

    void GameDatabase::createNewPlayer()
    {
        currentPlayer = Player{};
        currentPlayer.money = startMoney;
        currentPlayer.gold = startGold;
        currentPlayer.experience = startExperience;
        currentPlayer.level = startLevel;
        currentPlayer.selectedCarIndex = 0;

        Car starterCar = cloneCar(starterCarId);
        if (starterCar.id != 0)
        {
            starterCar.installedPartIds = starterPartIds;
            currentPlayer.carInventory.push_back(starterCar);
        }
    }

    void GameDatabase::savePlayer() const
    {
        SaveManager::save(currentPlayer);
    }

    bool GameDatabase::loadPlayer()
    {
        Player loaded;
        if (!SaveManager::load(loaded, *this))
            return false;

        currentPlayer = loaded;
        if (currentPlayer.carInventory.empty())
            createNewPlayer();

        return true;
    }

    void GameDatabase::deletePlayerSave()
    {
        SaveManager::deleteSave();
        createNewPlayer();
        savePlayer();
    }

    const Car* GameDatabase::getCarById(int id) const
    {
        auto it = std::find_if(allCars.begin(), allCars.end(), [&](const Car& car)
        {
            return car.id == id;
        });
        return it == allCars.end() ? nullptr : &*it;
    }

    const Part* GameDatabase::getPartById(int id) const
    {
        auto it = std::find_if(allParts.begin(), allParts.end(), [&](const Part& part)
        {
            return part.id == id;
        });
        return it == allParts.end() ? nullptr : &*it;
    }

    std::vector<const Part*> GameDatabase::getShopParts() const
    {
        std::vector<const Part*> result;
        for (const PartShopPart& shopPart : partShop.inventory)
        {
            if (const Part* part = getPartById(shopPart.partId))
                result.push_back(part);
        }
        return result;
    }

    std::vector<const Part*> GameDatabase::getShopPartsOfType(PartType type) const
    {
        std::vector<const Part*> result;
        for (const Part* part : getShopParts())
        {
            if (part && part->type == type)
                result.push_back(part);
        }
        return result;
    }

    std::vector<const Part*> GameDatabase::getPlayerPartsOfType(PartType type) const
    {
        std::vector<const Part*> result;
        std::vector<int> addedIds;

        for (const InventoryPart& item : currentPlayer.inventory)
        {
            if (item.count <= 0)
                continue;

            const Part* part = getPartById(item.partId);
            if (!part || part->type != type)
                continue;

            if (std::find(addedIds.begin(), addedIds.end(), part->id) == addedIds.end())
            {
                addedIds.push_back(part->id);
                result.push_back(part);
            }
        }

        return result;
    }

    Car GameDatabase::cloneCar(int carId) const
    {
        if (const Car* source = getCarById(carId))
            return *source;
        return Car{};
    }

    void GameDatabase::buildDefaultDatabase()
    {
        allCars.clear();
        allParts.clear();
        shopPartIds.clear();
        starterPartIds.clear();
        dealership.inventory.clear();


        startMoney = 10000;
        startGold = 100;
        startExperience = 0;
        startLevel = 1;

        // AE86 PARTS AE86 PARTS AE86 PARTS AE86 PARTS AE86 PARTS AE86 PARTS AE86 PARTS AE86 PARTS AE86 PARTS

        Part ae86engine = makeBasePart(101, "Stock AE86 Engine", 100, PartType::Engine);
        ae86engine.power = 165;
        ae86engine.maxRPM = 6400;
        ae86engine.idleRPM = 900;
        ae86engine.powerCurveMultipliers = {0.f, 0.42f, 0.68f, 0.92f, 1.00f, 0.82f};
        ae86engine.engineWeight = 205;
        allParts.push_back(ae86engine);

        Part ae86turbo = makeBasePart(102, "Stock Turbo", 250, PartType::Turbo);
        ae86turbo.turboPowerBonus = 0;
        allParts.push_back(ae86turbo);

        Part ae86block = makeBasePart(103, "Stock Engine Block", 300, PartType::EngineBlock);
        ae86block.blockWeight = 0;
        allParts.push_back(ae86block);

        Part ae86pistons = makeBasePart(104, "Stock Pistons", 180, PartType::Pistons);
        ae86pistons.pistonRPMIncrease = 0;
        allParts.push_back(ae86pistons);

        Part ae86airFilter = makeBasePart(105, "Stock Air Filter", 80, PartType::AirFilter);
        ae86airFilter.airPowerBonus = 0;
        allParts.push_back(ae86airFilter);

        Part ae86intercooler = makeBasePart(106, "Stock Intercooler", 140, PartType::Intercooler);
        ae86intercooler.intercoolerPowerBonus = 0;
        allParts.push_back(ae86intercooler);

        Part ae86ecu = makeBasePart(107, "Stock ECU", 200, PartType::ECU);
        ae86ecu.ecuPowerBonus = 0;
        ae86ecu.ecuRPMIncrease = 0;
        allParts.push_back(ae86ecu);

        Part ae86exhaust = makeBasePart(108, "Stock Exhaust", 150, PartType::ExhaustSystem);
        ae86exhaust.exhaustPowerBonus = 0;
        allParts.push_back(ae86exhaust);

        Part ae86suspension = makeBasePart(109, "Stock Suspension", 220, PartType::Suspension);
        ae86suspension.grip = 22;
        ae86suspension.suspensionWeight = 75;
        allParts.push_back(ae86suspension);

        Part ae86tires = makeBasePart(110, "Street Tires", 180, PartType::Tires);
        ae86tires.tireGrip = 18;
        ae86tires.wheelSpritePath = "assets/cars/ae86wheels.png";
        allParts.push_back(ae86tires);

        Part ae86weight = makeBasePart(111, "Stock Body Weight", 0, PartType::BodyWeightReduction);
        ae86weight.weightMultiplier = 1.f;
        allParts.push_back(ae86weight);

        Part ae86transmission = makeBasePart(112, "RWD Transmission", 500, PartType::Transmission);
        ae86transmission.transmissionType = TransmissionType::RWD;
        ae86transmission.transmissionWeight = 85;
        allParts.push_back(ae86transmission);

        Part ae86gearbox = makeBasePart(113, "5-Speed Gearbox", 650, PartType::Gearbox);
        ae86gearbox.gearCount = 5;
        ae86gearbox.gearRatios = {3.5f, 2.1f, 1.5f, 1.1f, 0.8f};
        ae86gearbox.finalRatio = 3.5f;
        ae86gearbox.shiftTimeMs = 1000;
        allParts.push_back(ae86gearbox);

        Part ae86clutch = makeBasePart(114, "Stock Clutch", 200, PartType::Clutch);
        ae86clutch.shiftTimeReductionMs = 0;
        allParts.push_back(ae86clutch);

        // R34 PARTS R34 PARTS R34 PARTS R34 PARTS R34 PARTS R34 PARTS R34 PARTS R34 PARTS R34 PARTS R34 PARTS 
        Part r34Engine = makeBasePart(201, "Stock Engine", 100, PartType::Engine);
        r34Engine.power = 260;
        r34Engine.maxRPM = 7800;
        r34Engine.idleRPM = 950;
        r34Engine.powerCurveMultipliers = {0.f, 0.36f, 0.64f, 0.90f, 1.00f, 0.88f};
        r34Engine.engineWeight = 255;
        allParts.push_back(r34Engine);

        Part r34Turbo = makeBasePart(202, "Stock Turbo", 100, PartType::Turbo);
        r34Turbo.turboPowerBonus = 80;
        allParts.push_back(r34Turbo);

        Part r34Block = makeBasePart(203, "Stock Engine Block", 100, PartType::EngineBlock);
        r34Block.blockWeight = 0;
        allParts.push_back(r34Block);

        Part r34Pistons = makeBasePart(204, "Stock Pistons", 100, PartType::Pistons);
        r34Pistons.pistonRPMIncrease = 0;
        allParts.push_back(r34Pistons);

        Part r34Air = makeBasePart(205, "Stock Air Filter", 100, PartType::AirFilter);
        r34Air.airPowerBonus = 0;
        allParts.push_back(r34Air);

        Part r34Intercooler = makeBasePart(206, "Stock Intercooler", 100, PartType::Intercooler);
        r34Intercooler.intercoolerPowerBonus = 30;
        allParts.push_back(r34Intercooler);

        Part r34Ecu = makeBasePart(207, "Stock ECU", 100, PartType::ECU);
        r34Ecu.ecuPowerBonus = 0;
        r34Ecu.ecuRPMIncrease = 0;
        allParts.push_back(r34Ecu);

        Part r34Exhaust = makeBasePart(208, "Stock Exhaust", 100, PartType::ExhaustSystem);
        r34Exhaust.exhaustPowerBonus = 0;
        allParts.push_back(r34Exhaust);

        Part r34Suspension = makeBasePart(209, "Stock Suspension", 100, PartType::Suspension);
        r34Suspension.grip = 0;
        r34Suspension.suspensionWeight = 0;
        allParts.push_back(r34Suspension);

        Part r34tires = makeBasePart(210, "Stock Tires", 100, PartType::Tires);
        r34tires.tireGrip = 87;
        r34tires.wheelSpritePath = "assets/cars/r34wheels.png";
        allParts.push_back(r34tires);

        Part r34Weight = makeBasePart(211, "Stock Weight", 100, PartType::BodyWeightReduction);
        r34Weight.weightMultiplier = 1.f;
        allParts.push_back(r34Weight);

        Part r34Transmission = makeBasePart(212, "Stock Transmission", 100, PartType::Transmission);
        r34Transmission.transmissionType = TransmissionType::AWD;
        r34Transmission.transmissionWeight = 0;
        allParts.push_back(r34Transmission);

        Part r34Speed = makeBasePart(213, "Stock Gearbox", 100, PartType::Gearbox);
        r34Speed.gearCount = 6;
        r34Speed.gearRatios = {3.7f, 2.4f, 1.7f, 1.25f, 1.0f, 0.82f};
        r34Speed.finalRatio = 3.9f;
        r34Speed.shiftTimeMs = 700;
        allParts.push_back(r34Speed);

        Part r34Clutch = makeBasePart(214, "Stock Clutch", 100, PartType::Clutch);
        r34Clutch.shiftTimeReductionMs = 0;
        allParts.push_back(r34Clutch);

        // BETTER PARTS | BETTER PARTS | BETTER PARTS | BETTER PARTS | BETTER PARTS | BETTER PARTS | BETTER PARTS | BETTER PARTS |

        // ENGINES ENGINES ENGINES ENGINES ENGINES ENGINES ENGINES ENGINES ENGINES ENGINES ENGINES ENGINES ENGINES ENGINES

        Part V8HEMI500 = makeBasePart(1, "V8 HEMI 500 CUI", 200000, PartType::Engine);
        V8HEMI500.power = 520;
        V8HEMI500.maxRPM = 5500;
        V8HEMI500.idleRPM = 800;
        V8HEMI500.powerCurveMultipliers = {0.f, 0.55f, 0.80f, 1.00f, 0.92f, 0.70f};
        V8HEMI500.engineWeight = 300;
        allParts.push_back(V8HEMI500);

        Part RB26DETT = makeBasePart(2, "I6 RB26DETT", 85000, PartType::Engine);
        RB26DETT.power = 320;
        RB26DETT.maxRPM = 8200;
        RB26DETT.idleRPM = 950;
        RB26DETT.powerCurveMultipliers = { 0.f, 0.34f, 0.62f, 0.88f, 1.0f, 0.9f };
        RB26DETT.engineWeight = 255;
        allParts.push_back(RB26DETT);

        Part JZ2GTE = makeBasePart(3, "I6 2JZ-GTE", 125000, PartType::Engine);
        JZ2GTE.power = 380;
        JZ2GTE.maxRPM = 7600;
        JZ2GTE.idleRPM = 900;
        JZ2GTE.powerCurveMultipliers = { 0.f, 0.38f, 0.68f, 0.92f, 1.00f, 0.86f };
        JZ2GTE.engineWeight = 275;
        allParts.push_back(JZ2GTE);

        Part LS7V8 = makeBasePart(4, "V8 LS7", 170000, PartType::Engine);
        LS7V8.power = 495;
        LS7V8.maxRPM = 7000;
        LS7V8.idleRPM = 800;
        LS7V8.powerCurveMultipliers = { 0.f, 0.55f, 0.80f, 1.00f, 0.92f, 0.70f };
        LS7V8.engineWeight = 210;
        allParts.push_back(LS7V8);

        // TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS TURBOS

        Part OEMTurbo = makeBasePart(5, "Small OEM Turbo", 5000, PartType::Turbo);
        OEMTurbo.turboPowerBonus = 40;
        allParts.push_back(OEMTurbo);

        Part SSTurbo = makeBasePart(6, "Sport Street Turbo", 9500, PartType::Turbo);
        SSTurbo.turboPowerBonus = 75;
        allParts.push_back(SSTurbo);

        Part GT2860RS = makeBasePart(7, "Garrett GT2860RS Turbo", 16000, PartType::Turbo);
        GT2860RS.turboPowerBonus = 120;
        allParts.push_back(GT2860RS);

        Part GTX3071R = makeBasePart(8, "Garrett GTX3071R Turbo", 24400, PartType::Turbo);
        GTX3071R.turboPowerBonus = 180;
        allParts.push_back(GTX3071R);

        Part Precision6266 = makeBasePart(9, "Precision 6266 Turbo", 37800, PartType::Turbo);
        Precision6266.turboPowerBonus = 220;
        allParts.push_back(Precision6266);

        Part GTX3582R = makeBasePart(11, "Garrett GTX3582R Turbo", 69000, PartType::Turbo);
        GTX3582R.turboPowerBonus = 310;
        allParts.push_back(GTX3582R);

        Part EFR9180 = makeBasePart(12, "BorgWarner EFR 9180 Turbo", 118000, PartType::Turbo);
        EFR9180.turboPowerBonus = 400;
        allParts.push_back(EFR9180);

        Part Precision7675 = makeBasePart(13, "Precision 7675 Turbo", 225000, PartType::Turbo);
        Precision7675.turboPowerBonus = 650;
        allParts.push_back(Precision7675);

        Part TDTurbo = makeBasePart(14, "Top Drag Turbo", 420000, PartType::Turbo);
        TDTurbo.turboPowerBonus = 1050;
        allParts.push_back(TDTurbo);

        // ENGINE BLOCKS ENGINE BLOCKS ENGINE BLOCKS ENGINE BLOCKS ENGINE BLOCKS ENGINE BLOCKS ENGINE BLOCKS

        Part ForgedBlock = makeBasePart(15, "ForgedBlock Engine Block", 17500, PartType::EngineBlock);
        ForgedBlock.blockWeight = -20;
        allParts.push_back(ForgedBlock);

        Part ReinforcedBlock = makeBasePart(16, "Reinforced Engine Block", 37500, PartType::EngineBlock);
        ReinforcedBlock.blockWeight = -40;
        allParts.push_back(ReinforcedBlock);

        Part BilletBlock = makeBasePart(17, "Billet Engine Block", 75000, PartType::EngineBlock);
        BilletBlock.blockWeight = -60;
        allParts.push_back(BilletBlock);

        // PISTONS PISTONS PISTONS PISTONS PISTONS PISTONS PISTONS PISTONS PISTONS PISTONS PISTONS

        Part ForgedPistons = makeBasePart(18, "Forged Pistons", 21000, PartType::Pistons);
        ForgedPistons.pistonRPMIncrease = 300;
        allParts.push_back(ForgedPistons);

        Part ReinforcedPistons = makeBasePart(19, "Reinforced Pistons", 71000, PartType::Pistons);
        ReinforcedPistons.pistonRPMIncrease = 800;
        allParts.push_back(ReinforcedPistons);

        Part BilletPistons = makeBasePart(20, "Billet Pistons", 189000, PartType::Pistons);
        BilletPistons.pistonRPMIncrease = 1100;
        allParts.push_back(BilletPistons);

        // AIR FILTERS AIR FILTERS AIR FILTERS AIR FILTERS AIR FILTERS AIR FILTERS AIR FILTERS AIR FILTERS

        Part AirFilterStage1 = makeBasePart(21, "Stage 1 Air Filter", 15000, PartType::AirFilter);
        AirFilterStage1.airPowerBonus = 30;
        allParts.push_back(AirFilterStage1);

        Part AirFilterStage2 = makeBasePart(22, "Stage 2 Air Filter", 45000, PartType::AirFilter);
        AirFilterStage2.airPowerBonus = 75;
        allParts.push_back(AirFilterStage2);

        // INTERCOOLERS  INTERCOOLERS INTERCOOLERS INTERCOOLERS INTERCOOLERS INTERCOOLERS INTERCOOLERS

        Part IntercoolerStage1 = makeBasePart(23, "Stage 1 Intercooler", 9000, PartType::Intercooler);
        IntercoolerStage1.intercoolerPowerBonus = 15;
        allParts.push_back(IntercoolerStage1);

        Part IntercoolerStage2 = makeBasePart(24, "Stage 2 Intercooler", 19000, PartType::Intercooler);
        IntercoolerStage2.intercoolerPowerBonus = 35;
        allParts.push_back(IntercoolerStage2);

        // ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU ECU

        Part ECUstage1 = makeBasePart(25, "Stage 1 ECU", 27000, PartType::ECU);
        ECUstage1.ecuPowerBonus = 20;
        ECUstage1.ecuRPMIncrease = 250;
        allParts.push_back(ECUstage1);

        Part ECUstage2 = makeBasePart(26, "Stage 2 ECU", 65500, PartType::ECU);
        ECUstage2.ecuPowerBonus = 85;
        ECUstage2.ecuRPMIncrease = 400;
        allParts.push_back(ECUstage2);

        Part ECUstage3 = makeBasePart(27, "Stage 3 ECU", 155000, PartType::ECU);
        ECUstage3.ecuPowerBonus = 155;
        ECUstage3.ecuRPMIncrease = 750;
        allParts.push_back(ECUstage3);

        Part ECUstage4 = makeBasePart(28, "Stage 4 ECU", 325000, PartType::ECU);
        ECUstage4.ecuPowerBonus = 250;
        ECUstage4.ecuRPMIncrease = 1250;
        allParts.push_back(ECUstage4);

        // EXHAUSTS EXHAUSTS EXHAUSTS EXHAUSTS EXHAUSTS EXHAUSTS EXHAUSTS EXHAUSTS EXHAUSTS EXHAUSTS EXHAUSTS

        Part ExhaustStage1 = makeBasePart(29, "Stage 1 Exhaust", 20500, PartType::ExhaustSystem);
        ExhaustStage1.exhaustPowerBonus = 45;
        allParts.push_back(ExhaustStage1);

        Part ExhaustStage2 = makeBasePart(30, "Stage 2 Exhaust", 55000, PartType::ExhaustSystem);
        ExhaustStage2.exhaustPowerBonus = 95;
        allParts.push_back(ExhaustStage2);

        // SUSPENSION SUSPENSION SUSPENSION SUSPENSION SUSPENSION SUSPENSION SUSPENSION SUSPENSION SUSPENSION

        Part SuspensionStage1 = makeBasePart(31, "Stage 1 Suspension", 27000, PartType::Suspension);
        SuspensionStage1.grip = 110;
        SuspensionStage1.suspensionWeight = -10;
        allParts.push_back(SuspensionStage1);

        Part SuspensionStage2 = makeBasePart(32, "Stage 2 Suspension", 77000, PartType::Suspension);
        SuspensionStage2.grip = 220;
        SuspensionStage2.suspensionWeight = -20;
        allParts.push_back(SuspensionStage2);

        // TIRES TIRES TIRES TIRES TIRES TIRES TIRES TIRES TIRES TIRES TIRES TIRES TIRES TIRES TIRES TIRES

        Part TiresStage1 = makeBasePart(33, "Stage 1 Tires", 30000, PartType::Tires);
        TiresStage1.tireGrip = 140;
        TiresStage1.wheelSpritePath = "assets/cars/stage1wheels.png";
        allParts.push_back(TiresStage1);

        Part TiresStage2 = makeBasePart(34, "Stage 2 Tires", 80000, PartType::Tires);
        TiresStage2.tireGrip = 260;
        TiresStage2.wheelSpritePath = "assets/cars/stage2wheels.png";
        allParts.push_back(TiresStage2);

        // WEIGHT REDUCTION WEIGHT REDUCTION WEIGHT REDUCTION WEIGHT REDUCTION WEIGHT REDUCTION WEIGHT REDUCTION

        Part WeightStage1 = makeBasePart(35, "Stage 1 Weight", 46000, PartType::BodyWeightReduction);
        WeightStage1.weightMultiplier = 0.95f;
        allParts.push_back(WeightStage1);

        Part WeightStage2 = makeBasePart(36, "Stage 2 Weight", 97000, PartType::BodyWeightReduction);
        WeightStage2.weightMultiplier = 0.9f;
        allParts.push_back(WeightStage2);

        Part WeightStage3 = makeBasePart(37, "Stage 3 Weight", 200000, PartType::BodyWeightReduction);
        WeightStage3.weightMultiplier = 0.85f;
        allParts.push_back(WeightStage3);

        Part WeightStage4 = makeBasePart(38, "Stage 4 Weight", 400100, PartType::BodyWeightReduction);
        WeightStage4.weightMultiplier = 0.8f;
        allParts.push_back(WeightStage4);

        Part WeightAlien = makeBasePart(39, "Alien Weight", 1000000, PartType::BodyWeightReduction);
        WeightAlien.weightMultiplier = 0.65f;
        allParts.push_back(WeightAlien);

        // GEARBOXES GEARBOXES GEARBOXES GEARBOXES GEARBOXES GEARBOXES GEARBOXES GEARBOXES GEARBOXES GEARBOXES GEARBOXES

        Part CD009 = makeBasePart(40, "CD009 5MT", 2200, PartType::Gearbox);
        CD009.gearCount = 5;
        CD009.gearRatios = { 3.7f, 2.02f, 1.38f, 1.0f, 0.86f };
        CD009.finalRatio = 3.5f;
        CD009.shiftTimeMs = 650;
        allParts.push_back(CD009);

        Part GetragV160 = makeBasePart(41, "Getrag V160 6MT", 8400, PartType::Gearbox);
        GetragV160.gearCount = 6;
        GetragV160.gearRatios = { 3.82f, 2.36f, 1.68f, 1.31f, 1.0f, 0.79f };
        GetragV160.finalRatio = 3.26f;
        GetragV160.shiftTimeMs = 550;
        allParts.push_back(GetragV160);

        Part TREMECT56 = makeBasePart(42, "TREMEC T-56 6MT", 18200, PartType::Gearbox);
        TREMECT56.gearCount = 6;
        TREMECT56.gearRatios = { 2.66f, 1.78f, 1.3f, 1.0f, 0.8f, 0.63f };
        TREMECT56.finalRatio = 3.1f;
        TREMECT56.shiftTimeMs = 400;
        allParts.push_back(TREMECT56);

        Part Quaife60G = makeBasePart(43, "Quaife 60G 6SQ", 46000, PartType::Gearbox);
        Quaife60G.gearCount = 6;
        Quaife60G.gearRatios = { 2.69f, 2.0f, 1.6f, 1.33f, 1.14f, 1.0f };
        Quaife60G.finalRatio = 2.6f;
        Quaife60G.shiftTimeMs = 230;
        allParts.push_back(Quaife60G);

        Part Xtrac1080 = makeBasePart(44, "Xtrac 1080 8SQ", 120000, PartType::Gearbox);
        Xtrac1080.gearCount = 8;
        Xtrac1080.gearRatios = { 4.5f, 3.3f, 2.61f, 2.05f, 1.66f, 1.38f, 1.16f, 1.0f };
        Xtrac1080.finalRatio = 2.0f;
        Xtrac1080.shiftTimeMs = 120;
        allParts.push_back(Xtrac1080);

        // CLUTCHES CLUTCHES CLUTCHES CLUTCHES CLUTCHES CLUTCHES CLUTCHES CLUTCHES CLUTCHES CLUTCHES CLUTCHES CLUTCHES

        Part ULWClutch = makeBasePart(45, "Ultra Light Weight Clutch", 80000, PartType::Clutch);
        ULWClutch.shiftTimeReductionMs = 110;
        allParts.push_back(ULWClutch);

        Part LWClutch = makeBasePart(46, "Light Weight Clutch", 25000, PartType::Clutch);
        LWClutch.shiftTimeReductionMs = 55;
        allParts.push_back(LWClutch);


        // CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS CARS
        Car ae86;
        ae86.id = 1;
        ae86.carName = "AE86";
        ae86.price = 10000;
        ae86.priceType = PriceType::Money;
        ae86.bodyWeight = 640;
        ae86.bodySpritePath = "assets/cars/ae86body.png";
        ae86.bodyPosition = {960.f, 620.f};
        ae86.bodySize = {512.f, 440.f};
        ae86.rearWheelPos = {-131.4f, 55.f};
        ae86.frontWheelPos = {149.4f, 55.f};
        ae86.wheelSize = {77.f, 77.f};
        ae86.carScale = {1.3f, 1.3f};
        ae86.bodyRotationDegrees = 0.2f;
        ae86.installedPartIds = {101,102,103,104,105,106,107,108,109,110,111,112,113,114};
        allCars.push_back(ae86);

        Car r34;
        r34.id = 2;
        r34.carName = "R34";
        r34.price = 2500;
        r34.priceType = PriceType::Gold;
        r34.bodyWeight = 1050;
        r34.bodySpritePath = "assets/cars/r34body.png";
        r34.bodyPosition = { 960.f, 611.f };
        r34.bodySize = {560.f, 490.f};
        r34.frontWheelPos = {164.6f, 52.f};
        r34.rearWheelPos = {-152.4f, 52.f};
        r34.wheelSize = {82.5f, 82.5f};
        r34.carScale = { 1.4f, 1.4f };
        r34.bodyRotationDegrees = 0.2f;
        r34.installedPartIds = {201,202,203,204,205,206,207,208,209,210,211,212,213,214};
        allCars.push_back(r34);

        starterCarId = 1;
        starterPartIds = ae86.installedPartIds;

        shopPartIds = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46};

        dealership.addCar({1, 10000, PriceType::Money, "AE86"});
        dealership.addCar({2, 2500, PriceType::Gold, "R34"});
    }

    void GameDatabase::rebuildPartShopFromIds()
    {
        partShop.inventory.clear();
        for (int id : shopPartIds)
        {
            const Part* part = getPartById(id);
            if (part)
                partShop.addPart({id, part->price, part->type});
        }
    }
}
