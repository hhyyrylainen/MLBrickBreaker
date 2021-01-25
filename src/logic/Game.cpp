#include "Game.h"
#include "Definitions.h"
#include "GameElements.h"
#include "Globals.h"

#include <Viewport.hpp>
#include <neat.h>

#include <chrono>

using namespace mlbb;
using namespace godot;

using Clock = std::chrono::high_resolution_clock;
using Seconds = std::chrono::duration<double, std::ratio<1>>;
using MilliSeconds = std::chrono::duration<int64_t, std::milli>;

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
        // Load AI
        LoadNEAT();

        // The used NEAT params are optimized for 1000, so that's used for now.
        // At least on my computer (hh) I could probably run around 5000 population without
        // requiring threading to speed things up.
        int startingPopulation = 1000;
        AI.emplace(windowSize.x, windowSize.y, startingPopulation);

        AI->Begin();
    }

    ControlPanel->set("is_player", PlayerControlled);
    ControlPanel->call("on_start");
}

void Game::_process(float delta)
{
    const Clock::time_point start = Clock::now();
    Clock::duration aiDuration{};

    if(PlayerControlled) {
        if(ActiveMatch) {
            ActiveMatch->Update(delta, UserInput());
        }
    } else {
        const Clock::time_point aiStart = Clock::now();

        // Run AI
        AI->Update(delta);

        aiDuration = Clock::now() - aiStart;

        int aiID = -1;
        std::tie(ActiveMatch, aiID) = AI->GetAIMatch();

        // Update AI stats to GUI
        ControlPanel->set("generation", AI->GetGenerationNumber());
        ControlPanel->set("ai_id", aiID);
        ControlPanel->set("alive_ais", AI->CountActiveAIMatches());
    }

    if(ActiveMatch) {
        // Update GUI
        ControlPanel->set("elapsed_match_time", ActiveMatch->GetElapsedTime());
        ControlPanel->set("lives", ActiveMatch->GetLivesLeft());
        ControlPanel->set("score", ActiveMatch->GetTimedScore());
    }

    DrawGame();

    const auto updateDuration = Clock::now() - start;

    ControlPanel->set("update_performance",
        std::chrono::duration_cast<Seconds>(updateDuration).count() * 1000.f);

    if(!PlayerControlled) {
        ControlPanel->set("ai_performance",
            std::chrono::duration_cast<Seconds>(aiDuration).count() * 1000.f);
    }
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

void Game::LoadNEAT()
{
    // TODO: this will break in packaged game
    NEAT::load_neat_params("config/NeatParams.ne", false);
}
