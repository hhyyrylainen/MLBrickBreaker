#pragma once

#include <Godot.hpp>
#include <Node.hpp>

namespace mlbb{

//! \brief Main class handling the game scene
class Game : public godot::Node {
    GODOT_CLASS(Game, Node);

private:
        float time_passed;

public:
    static void _register_methods();

    Game();
    ~Game();

    void _init(); // our initializer called by Godot

    void _process(float delta);
};

}
