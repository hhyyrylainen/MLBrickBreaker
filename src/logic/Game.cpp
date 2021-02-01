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

    register_method("set_speed", &Game::SetSpeed);
    register_method("set_threads", &Game::SetTrainingThreads);
    register_method("save_top_ai", &Game::SaveTopAI);
    register_method("set_paddle_speed", &Game::SetPaddleSpeed);
    register_method("set_ball_speed", &Game::SetBallSpeed);
    register_method("set_ghost_count", &Game::SetGhostCount);

    register_property<Game, decltype(BrickScene)>("BrickScene", &Game::BrickScene, nullptr);
    register_property<Game, decltype(BallScene)>("BallScene", &Game::BallScene, nullptr);
    register_property<Game, decltype(PaddleScene)>("PaddleScene", &Game::PaddleScene, nullptr);

    register_property<Game, decltype(PaddleScene)>(
        "GhostPaddleScene", &Game::GhostPaddleScene, nullptr);
    register_property<Game, decltype(PaddleScene)>(
        "GhostBallScene", &Game::GhostBallScene, nullptr);
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

    GhostPaddles = NodeHolder<godot::Node2D>(
        GameVisuals, [this]() { return static_cast<Node2D*>(GhostPaddleScene->instance()); });

    GhostBalls = NodeHolder<godot::Node2D>(
        GameVisuals, [this]() { return static_cast<Node2D*>(GhostBallScene->instance()); });

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
    ControlPanel->connect("training_speed_changed", this, "set_speed");
    ControlPanel->connect("threads_changed", this, "set_threads");
    ControlPanel->connect("save_ai_pressed", this, "save_top_ai");
    ControlPanel->connect("paddle_speed_changed", this, "set_paddle_speed");
    ControlPanel->connect("ball_speed_changed", this, "set_ball_speed");
    ControlPanel->connect("ghost_count_changed", this, "set_ghost_count");

    ControlPanel->call("on_start");
}

void Game::_process(float delta)
{
    if(delta <= 0)
        return;

    if(delta > MAX_ELAPSED_TIME_PER_UPDATE)
        delta = MAX_ELAPSED_TIME_PER_UPDATE;

    const Clock::time_point start = Clock::now();
    Clock::duration aiDuration{};

    AdditionalAIMatchesToShow.clear();

    if(PlayerControlled) {
        if(ActiveMatch) {
            ActiveMatch->SetGameVariables(PaddleSpeed, BallSpeed);
            ActiveMatch->Update(delta, UserInput());
        }
    } else {
        const Clock::time_point aiStart = Clock::now();

        float aiDelta = delta;

        if(SpeedMultiplier > USE_60_FPS_DELTA_WITH_SPED_UP_TRAINING_THRESHOLD) {
            aiDelta = DELTA_FOR_60_FPS;
        }

        // Run AI
        AI->Update(aiDelta, SpeedMultiplier, TrainingThreads, PaddleSpeed, BallSpeed);

        aiDuration = Clock::now() - aiStart;

        int aiID = -1;
        std::tie(ActiveMatch, aiID) = AI->GetAIMatch(&AdditionalAIMatchesToShow, Ghosts);

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

void Game::SaveTopAI()
{
    if(!AI)
        return;

    AI->SetGenerationEndCallback([this]() { OnPerformSave(); });

    ControlPanel->call("show_status", godot::String("Saving on generation end"));
}

void Game::OnPerformSave()
{
    const std::string targetFile = "best_species.txt";
    const std::string individualTarget = "best_organism.txt";

    AI->WriteSpeciesToFile(targetFile, AIType::Best);
    AI->WriteOrganismToFile(individualTarget, AIType::Best);

    ControlPanel->call("show_status", godot::String("Wrote AI to files"));

    godot::Godot::print(
        godot::String("Wrote: ") + targetFile.c_str() + " and: " + individualTarget.c_str());
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

    DrawGhosts(AdditionalAIMatchesToShow);
}

template<class T, class HolderT>
void DrawHelperInner(const std::vector<T>& gameObjects, HolderT& container)
{
    for(const auto& object : gameObjects) {
        auto graphics = container.GetNext();

        graphics->set_position(object.PositionAsVector());
    }
}

template<class T, class HolderT>
void DrawHelper(const std::vector<T>& gameObjects, HolderT& container)
{
    container.UnMarkAll();

    DrawHelperInner(gameObjects, container);

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

void Game::DrawGhosts(const std::vector<std::shared_ptr<Match>>& matches)
{
    GhostPaddles->UnMarkAll();
    GhostBalls->UnMarkAll();

    for(const auto& match : matches) {
        if(!match->HasEnded()) {
            DrawHelperInner(match->GetBallsForDrawing(), *GhostBalls);
            DrawHelperInner(match->GetPaddlesForDrawing(), *GhostPaddles);
        }
    }

    GhostPaddles->FreeUnmarked();
    GhostBalls->FreeUnmarked();
}

void Game::LoadNEAT()
{
    // TODO: this will break in packaged game
    NEAT::load_neat_params("config/NeatParams.ne", false);
}
