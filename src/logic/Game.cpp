#include "Game.h"
#include "Definitions.h"
#include "GameElements.h"
#include "Globals.h"

#include <Viewport.hpp>

using namespace mlbb;
using namespace godot;

void Game::_register_methods()
{
    register_method("_process", &Game::_process);
    register_method("_ready", &Game::_ready);

    register_property<Game, decltype(BrickScene)>("BrickScene", &Game::BrickScene, nullptr);
    register_property<Game, decltype(BallScene)>("BallScene", &Game::BallScene, nullptr);
    register_property<Game, decltype(PaddleScene)>("PaddleScene", &Game::PaddleScene, nullptr);
}

void Game::_init() {}

void Game::_ready()
{
    // Read globals
    auto globals = get_node<Globals>("/root/Globals");
    PlayerControlled = globals->GetIsPlayer();

    // Size override gives the wanted (logical size) 1920x1080
    const auto& windowSize = get_viewport()->get_size_override();
    GameVisuals = get_node<Node2D>("VisualizedGame");

    if(!GameVisuals)
        throw std::runtime_error("couldn't get GameVisuals");

    ControlPanel = get_node<Control>("GameControlPanel");

    if(!ControlPanel)
        throw std::runtime_error("couldn't get ControlPanel");

    Paddles = NodeHolder<godot::Node2D>(
        GameVisuals, [this]() { return static_cast<Node2D*>(PaddleScene->instance()); });

    Balls = NodeHolder<godot::Node2D>(
        GameVisuals, [this]() { return static_cast<Node2D*>(BallScene->instance()); });

    Bricks = NodeHolder<godot::Node2D>(
        GameVisuals, [this]() { return static_cast<Node2D*>(BrickScene->instance()); });

    if(PlayerControlled) {
        ActiveMatch = std::make_shared<Match>(windowSize.x, windowSize.y);
    } else {
        // TODO: AI training
        ActiveMatch = std::make_shared<Match>(windowSize.x, windowSize.y);
    }

    ControlPanel->set("is_player", PlayerControlled);
}

void Game::_process(float delta)
{
    if(PlayerControlled) {
        if(ActiveMatch) {
            ActiveMatch->Update(delta, UserInput());
        }
    } else {
        // TODO: run AI
        if(ActiveMatch) {
            ActiveMatch->Update(delta, ProgrammaticInput());
        }
    }

    DrawGame();
}

void Game::DrawGame()
{
    if(!ActiveMatch) {
        DrawPaddles({});
        DrawBalls({});
        DrawBricks({});
    } else {
        DrawPaddles(ActiveMatch->GetPaddlesForDrawing());
        DrawBalls(ActiveMatch->GetBallsForDrawing());
        DrawBricks(ActiveMatch->GetBricksForDrawing());
    }
}

template<class T, class HolderT>
void DrawHelper(const std::vector<T>& gameObjects, HolderT& container)
{
    container.UnMarkAll();

    for(const auto& object : gameObjects) {
        auto graphics = container.GetNext();

        graphics->set_position(object.PositionAsVector());
    }

    container.FreeUnmarked();
}

void Game::DrawPaddles(const std::vector<Paddle>& paddles)
{
    DrawHelper(paddles, *Paddles);
}

void Game::DrawBalls(const std::vector<Ball>& balls)
{
    DrawHelper(balls, *Balls);
}

void Game::DrawBricks(const std::vector<Brick>& bricks)
{
    DrawHelper(bricks, *Bricks);
}