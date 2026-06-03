#pragma once

#include "Scene.h"
#include "../UI/TextButton.h"

#include <vector>

class MainMenuScene : public Scene
{
public:
    explicit MainMenuScene(Game& game);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;

private:
    std::vector<TextButton> m_buttons;

    void buildUi();
};
