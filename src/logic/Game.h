#pragma once

#include "Match.h"
#include "NodeHolder.h"

#include <Godot.hpp>
#include <Node.hpp>
#include <Node2D.hpp>
#include <PackedScene.hpp>

#include <memory>
#include <optional>

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
    void DrawGame();
    void DrawPaddles(const std::vector<Paddle>& paddles);
    void DrawBalls(const std::vector<Ball>& balls);
    void DrawBricks(const std::vector<Brick>& bricks);

private:
    godot::Ref<godot::PackedScene> BrickScene = nullptr;
    godot::Ref<godot::PackedScene> BallScene = nullptr;
    godot::Ref<godot::PackedScene> PaddleScene = nullptr;

    godot::Node2D* GameVisuals = nullptr;

    std::optional<NodeHolder<godot::Node2D>> Paddles;
    std::optional<NodeHolder<godot::Node2D>> Balls;
    std::optional<NodeHolder<godot::Node2D>> Bricks;

    bool PlayerControlled = false;

    float TimePassed = 0.f;

    std::shared_ptr<Match> ActiveMatch;
};

} // namespace mlbb
