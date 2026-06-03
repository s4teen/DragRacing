#include "Scene.h"

Scene::Scene(Game& game)
    : m_game(game)
{
}

Game& Scene::game()
{
    return m_game;
}
