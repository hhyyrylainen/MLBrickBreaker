#pragma once

#include <Godot.hpp>
// #include <Node.hpp>
#include <Sprite.hpp>

namespace mlbb {

//! \brief Main class handling the game scene
class Game : public godot::Sprite {
    GODOT_CLASS(Game, Sprite);

private:
    float time_passed;

public:
    static void _register_methods();

    Game();
    ~Game();

    void _init(); // our initializer called by Godot

    void _process(float delta);
};

} // namespace mlbb
