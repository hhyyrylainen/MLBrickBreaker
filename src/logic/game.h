#pragma once

#include <Godot.hpp>
#include <Node.hpp>
#include <Node2D.hpp>
#include <PackedScene.hpp>

namespace mlbb {

//! \brief Main class handling the game scene
class Game : public godot::Node {
    GODOT_CLASS(Game, Node);

public:
    Game() = default;
    ~Game() = default;

    void _init();
    void _ready();

    void _process(float delta);

    static void _register_methods();

private:
    float TimePassed = 0.f;
    godot::Ref<godot::PackedScene> BrickScene = nullptr;
    godot::Node2D* GameVisuals = nullptr;
};

} // namespace mlbb
