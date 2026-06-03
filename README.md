# DragRacing — SFML/C++ Racing Game

**DragRacing** — 2D racing / drag racing game на **C++** и **SFML 3.1**.

Проект разрабатывается под **Visual Studio 2026** и использует ручную систему сцен, UI, сохранений, гаража, магазина, физики автомобиля и гонок.

---

## Текущий статус проекта

На данный момент реализованы:

- главное меню;
- меню настроек;
- полноэкранный и оконный режим;
- выбор разрешения из доступных режимов ОС;
- ограничение FPS;
- настройка громкости;
- выбор метрической / имперской системы;
- гараж;
- автосалон;
- парковка;
- магазин деталей;
- меню тюнинга;
- free ride;
- гонка против AI;
- базовая система наград;
- система опыта, уровней и скиллов;
- система сохранений игрока и настроек;
- data-driven база машин и деталей через `GameDatabase`.

---

## Технологии

- **C++17**
- **SFML 3.1**
- **Visual Studio 2026**
- Dynamic linking SFML
- Ручная архитектура сцен и UI

---

## Структура проекта

```txt
src/
  main.cpp

  Core/
    Game.h / Game.cpp
    Settings.h / Settings.cpp
    AssetManager.h / AssetManager.cpp
    SceneType.h
    RaceMode.h
    Constants.h

  Scenes/
    Scene.h / Scene.cpp
    MainMenuScene.h / MainMenuScene.cpp
    SettingsScene.h / SettingsScene.cpp
    GarageScene.h / GarageScene.cpp
    RaceScene.h / RaceScene.cpp

  UI/
    TextButton.h / TextButton.cpp
    Slider.h / Slider.cpp

  Utils/
    Format.h / Format.cpp

  GameData/
    GameDatabase.h / GameDatabase.cpp
    Player.h / Player.cpp
    Car.h / Car.cpp
    Part.h
    PartShop.h
    Dealership.h
    SaveManager.h / SaveManager.cpp

  Race/
    RaceVehiclePhysics.h / RaceVehiclePhysics.cpp
```

---

## Запуск

`main.cpp` намеренно минимальный:

```cpp
#include "Core/Game.h"

int main()
{
    Game game;
    game.run();
    return 0;
}
```

Вся логика запуска, оконного режима, сцен, ресурсов и игрового цикла находится внутри `Game`.

---

## Подключение SFML 3.1 в Visual Studio 2026

Для проекта нужно подключить SFML в свойствах проекта.

### Include directories

```txt
C:\SFML-3.1.0\include
```

### Library directories

```txt
C:\SFML-3.1.0\lib
```

### Debug dependencies

```txt
sfml-graphics-d.lib
sfml-window-d.lib
sfml-system-d.lib
sfml-audio-d.lib
```

### Release dependencies

```txt
sfml-graphics.lib
sfml-window.lib
sfml-system.lib
sfml-audio.lib
```

DLL-файлы из `SFML-3.1.0\bin` должны лежать рядом с `.exe`.

---

## Управление

### Меню

- **ЛКМ** — нажать кнопку
- **Esc** — вернуться назад / выйти из меню

### Гонка

- **W / Up** — газ
- **S / Down** — тормоз
- **E** — передача вверх
- **Q** — передача вниз
- **Esc** — назад в гараж

---

## Сцены

### Main Menu

Главное меню игры. Использует случайный skybox:

- `skybox_day.png`
- `skybox_evening.png`
- `skybox_rain.png`

### Settings

Меню настроек:

- разрешение;
- refresh rate / FPS limit;
- fullscreen;
- volume;
- metric / imperial.

Если fullscreen включён, разрешение автоматически ставится в максимальный режим монитора.

### Garage

Основной центр игры. Включает:

- главное меню гаража;
- Dealership;
- Parking;
- Shop;
- Tuning;
- Race Types;
- окно выбора скиллов после level up.

В гараже отображаются:

- деньги;
- золото;
- уровень;
- выбранная машина;
- характеристики автомобиля;
- текущие детали.

### Race

Сцена гонки поддерживает:

- Free Ride;
- Against AI;
- бесконечную трассу;
- параллакс skybox;
- dashboard;
- физику автомобиля;
- вращение колёс;
- наклон кузова при разгоне и торможении;
- AI-бота на такой же машине с небольшим случайным разбросом параметров.

---

## База машин и деталей

Все машины и детали прописываются в:

```txt
src/GameData/GameDatabase.cpp
```

Основной метод:

```cpp
GameDatabase::buildDefaultDatabase()
```

Там задаются:

- детали;
- машины;
- стартовая машина игрока;
- стартовые детали;
- магазин;
- автосалон.

---

## Детали

Каждая деталь имеет тип:

- Engine
- Turbo
- EngineBlock
- Pistons
- AirFilter
- Intercooler
- ECU
- ExhaustSystem
- Suspension
- Tires
- BodyWeightReduction
- Transmission
- Gearbox
- Clutch

Детали могут влиять на:

- мощность;
- максимальные обороты;
- idle RPM;
- кривую мощности;
- вес;
- grip;
- тип привода;
- количество передач;
- gear ratios;
- final ratio;
- shift time;
- спрайт колёс.

---

## Машины

Машина состоит из трёх визуальных элементов:

1. заднее колесо;
2. переднее колесо;
3. кузов.

Колёса рисуются на слое ниже кузова.

Для каждой машины можно настроить:

- название;
- цену;
- тип цены;
- вес кузова;
- путь к спрайту кузова;
- позицию кузова;
- размер кузова;
- позицию переднего и заднего колеса;
- размер колёс;
- общий scale;
- угол поворота кузова;
- установленные детали.

---

## Физика автомобиля

Физика находится в:

```txt
src/Race/RaceVehiclePhysics.cpp
```

Учитываются:

- масса автомобиля;
- мощность;
- кривая мощности двигателя;
- RPM;
- idle RPM;
- max RPM;
- передаточные числа;
- final ratio;
- wheel radius;
- grip;
- wheel slip;
- drag;
- rolling resistance;
- brake force;
- drivetrain efficiency;
- shift time;
- отключение мощности во время переключения.

Мощность двигателя рассчитывается по кривой из 6 точек:

```txt
0%
20%
40%
60%
80%
100%
```

Между точками используется интерполяция.

---

## Гонка против AI

В режиме Against AI:

1. Игрок и бот появляются на трассе.
2. Бот использует такую же машину, как игрок.
3. У бота есть небольшой случайный разброс параметров:
   - мощность;
   - grip;
   - масса;
   - скорость переключения;
   - реакция переключения.
4. Перед стартом идёт отсчёт:
   - `3`
   - `2`
   - `1`
   - `GO!`
5. Во время отсчёта машины не двигаются.
6. После старта гонка длится 15 секунд.
7. Победитель определяется по пройденной дистанции.
8. Награда показывается игроку.
9. Награда применяется безопасно через очередь в `Game`, чтобы не сохранять игрока в середине активной гонки.

---

## Система игрока

Игрок хранит:

- деньги;
- золото;
- опыт;
- уровень;
- skill points;
- инвентарь деталей;
- инвентарь машин;
- выбранную машину;
- скиллы.

Стартовый игрок получает:

```txt
money = 10000
gold = 100
level = 1
```

---

## Уровни и опыт

Для получения 2 уровня нужно:

```txt
1000 xp
```

Каждый следующий уровень требует опыта больше на коэффициент:

```txt
1.15
```

После каждого повышения уровня игрок получает:

```txt
+1 skill point
+100 gold
```

Бонус к золоту учитывается при получении золота за level up.

---

## Скиллы

После повышения уровня в гараже принудительно открывается окно:

```txt
Choose skills!
Points left: X
```

Пока у игрока есть очки, окно остаётся открытым.

Каждый скилл даёт `+1%`.

Доступные скиллы:

- Money Bonus;
- Gold Bonus;
- XP Bonus;
- Power Bonus;
- Grip Bonus;
- Shop Discount.

---

## Сохранения

Сохранения лежат в:

```txt
save/
  settings.ini
  player.ini
```

`settings.ini` хранит:

- разрешение;
- fullscreen;
- FPS limit;
- громкость;
- metric / imperial.

`player.ini` хранит:

- валюту;
- опыт;
- уровень;
- skill points;
- скиллы;
- машины игрока;
- детали игрока;
- выбранную машину.

Сохранение игрока сделано через безопасную запись во временный файл с последующей заменой основного файла.

---

## Ассеты

Ожидаемая структура ассетов:

```txt
assets/
  backgrounds/
    skybox_day.png
    skybox_evening.png
    skybox_rain.png
    garage.png

  race/
    roadtile.png
    dashboard.png

  icons/
    money.png
    gold.png
    level.png

  fonts/
    fontMain.ttf
    fontSecond.ttf

  cars/
    ae86body.png
    ae86wheels.png
    r34body.png
    r34wheels.png
    oldmustang.png
    oldmustangwheels.png
    stage1wheels.png
    stage2wheels.png
```

---

## Особенности архитектуры

Проект построен так, чтобы `main.cpp` только запускал игру.

Основные системы разделены:

- `Game` — окно, игровой цикл, смена сцен, режим гонки, отложенные награды.
- `Scene` — базовый класс сцены.
- `GarageScene` — гараж, магазин, тюнинг, скиллы.
- `RaceScene` — гонка, трасса, dashboard, бот.
- `RaceVehiclePhysics` — физика машины.
- `GameDatabase` — база данных машин и деталей.
- `Player` — состояние игрока.
- `SaveManager` — сохранение и загрузка.

---

## Планы на развитие

Возможные следующие шаги:

- полноценный race result menu;
- больше типов гонок;
- баланс наград;
- улучшение AI;
- больше машин;
- больше деталей;
- визуальный редактор позиций машины;
- hot-reload конфигов;
- JSON/INI база деталей вместо ручного C++;
- звуки двигателя, шин и переключения;
- музыка;
- анимации UI;
- улучшение физики сцепления и турболага.
