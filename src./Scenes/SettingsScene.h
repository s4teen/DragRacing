#pragma once

#include "Scene.h"
#include "../UI/Slider.h"
#include "../UI/TextButton.h"

#include <vector>

class SettingsScene : public Scene
{
public:
    explicit SettingsScene(Game& game);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;

private:
    std::vector<TextButton> m_buttons;
    Slider m_volumeSlider;

    TextButton* m_resolutionLeft = nullptr;
    TextButton* m_resolutionRight = nullptr;
    TextButton* m_fullscreenToggle = nullptr;
    TextButton* m_metricToggle = nullptr;

    void buildUi();
    void refreshControls();

    void changeResolution(int delta);
    void changeFps(int delta);
    void toggleFullscreen();
    void toggleMetric();
    void saveVolume();
};
