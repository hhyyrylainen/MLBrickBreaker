#pragma once

#include <Godot.hpp>
#include <Node.hpp>
#include <PackedScene.hpp>

namespace mlbb {

//! \brief Main class handling the game scene
class Game : public godot::Node {
    GODOT_CLASS(Game, Node);

private:
    float TimePassed = 0.f;
    godot::Ref<godot::PackedScene> BrickScene = nullptr;

public:
    static void _register_methods();

    Game() = default;
    ~Game() = default;

    void _init();
    void _ready();

    void _process(float delta);
};

} // namespace mlbb
