#include "game.h"

using namespace mlbb;
using namespace godot;

void Game::_register_methods()
{
    register_method("_process", &Game::_process);
    register_method("_ready", &Game::_ready);

    register_property<Game, decltype(BrickScene)>("BrickScene", &Game::BrickScene, nullptr);
}

void Game::_init() {}

void Game::_ready()
{
    GameVisuals = get_node("VisualizedGame");

    GameVisuals->add_child(BrickScene->instance());
}

void Game::_process(float delta) {}
