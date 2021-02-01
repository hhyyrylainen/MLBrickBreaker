#pragma once

#include "AITrainer.h"
#include "Match.h"
#include "NodeHolder.h"

#include <Godot.hpp>
#include <Node.hpp>
#include <Node2D.hpp>
#include <PackedScene.hpp>

#include <Control.hpp>
#include <memory>
#include <optional>

namespace mlbb {


//! \brief Main class handling the game scene
class Game : public godot::Node {
    GODOT_CLASS(Game, godot::Node);

public:
    Game() = default;
    ~Game() = default;

    void _init();
    void _ready();

    void _process(float delta);

    void SetSpeed(int speedMultiplier)
    {
        SpeedMultiplier = std::max(speedMultiplier, 1);
    }

    void SetTrainingThreads(int threads)
    {
        TrainingThreads = std::max(threads, 1);
    }

    void SaveTopAI();

    static void _register_methods();

private:
    void DrawGame();
    void DrawPaddles(const std::vector<Paddle>& paddles);
    void DrawBalls(const std::vector<Ball>& balls);
    void DrawBricks(const std::vector<Brick>& bricks);

    void DrawGhosts(const std::vector<std::shared_ptr<Match>>& matches);

    void LoadNEAT();

    void OnPerformSave();

private:
    godot::Ref<godot::PackedScene> BrickScene = nullptr;
    godot::Ref<godot::PackedScene> BallScene = nullptr;
    godot::Ref<godot::PackedScene> PaddleScene = nullptr;

    godot::Ref<godot::PackedScene> GhostPaddleScene = nullptr;
    godot::Ref<godot::PackedScene> GhostBallScene = nullptr;

    godot::Node2D* GameVisuals = nullptr;
    godot::Control* ControlPanel = nullptr;

    std::optional<NodeHolder<godot::Node2D>> Paddles;
    std::optional<NodeHolder<godot::Node2D>> Balls;
    std::optional<NodeHolder<godot::Node2D>> Bricks;

    std::optional<NodeHolder<godot::Node2D>> GhostPaddles;
    std::optional<NodeHolder<godot::Node2D>> GhostBalls;

    bool PlayerControlled = false;

    float TimePassed = 0.f;
    int SpeedMultiplier = 1;
    int TrainingThreads = 1;

    std::shared_ptr<Match> ActiveMatch;
    std::optional<AITrainer> AI;
    std::vector<std::shared_ptr<Match>> AdditionalAIMatchesToShow;
};

} // namespace mlbb
