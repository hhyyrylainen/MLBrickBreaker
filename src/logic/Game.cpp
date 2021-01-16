#include "Game.h"
#include "Definitions.h"

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
    const auto& windowSize = get_viewport()->get_size();
    GameVisuals = get_node<Node2D>("VisualizedGame");

    auto brick = static_cast<Node2D*>(BrickScene->instance());
    GameVisuals->add_child(brick);

    auto brick2 = static_cast<Node2D*>(BrickScene->instance());
    GameVisuals->add_child(brick2);

    auto brick3 = static_cast<Node2D*>(BrickScene->instance());
    GameVisuals->add_child(brick3);

    brick->set_position(Vector2(200, 50));
    brick2->set_position(Vector2(200 + BRICK_WIDTH_PIXELS, 50));
    brick3->set_position(Vector2(200, 50 + BRICK_HEIGHT_PIXELS));

    auto paddle = static_cast<Node2D*>(PaddleScene->instance());
    GameVisuals->add_child(paddle);

    auto paddle2 = static_cast<Node2D*>(PaddleScene->instance());
    GameVisuals->add_child(paddle2);

    paddle->set_position(Vector2(windowSize.x / 2, windowSize.y - PADDLE_HEIGHT_PIXELS));
    paddle2->set_position(
        Vector2(windowSize.x - PADDLE_WIDTH_PIXELS, windowSize.y - PADDLE_HEIGHT_PIXELS));

    auto ball = static_cast<Node2D*>(BallScene->instance());
    GameVisuals->add_child(ball);

    auto ball2 = static_cast<Node2D*>(BallScene->instance());
    GameVisuals->add_child(ball2);

    const auto firstBallPos = Vector2(windowSize.x / 2, windowSize.y / 2);
    ball->set_position(firstBallPos);
    ball2->set_position(firstBallPos + Vector2(BALL_SIZE_PIXELS, 0));
}

void Game::_process(float delta) {}
