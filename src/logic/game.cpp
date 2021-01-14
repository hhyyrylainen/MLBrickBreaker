#include "game.h"

using namespace mlbb;
using namespace godot;

void Game::_register_methods() {
    register_method("_process", &Game::_process);
}

Game::Game() {
}

Game::~Game() {
    // add your cleanup here
}

void Game::_init() {
    // initialize any variables here
    time_passed = 0.0;
}

void Game::_process(float delta) {
    time_passed += delta;

    // Vector2 new_position = Vector2(10.0 + (10.0 * sin(time_passed * 2.0)), 10.0 + (10.0 * cos(time_passed * 1.5)));

    // set_position(new_position);
}
