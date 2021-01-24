#pragma once

namespace mlbb {

// The scales need to match what is in the scenes, otherwise the graphics don't line up
constexpr auto PADDLE_HEIGHT_PIXELS = static_cast<int>(50 * 0.9f);
constexpr auto PADDLE_WIDTH_PIXELS = static_cast<int>(323 * 0.9f);

constexpr auto BRICK_WIDTH_PIXELS = static_cast<int>(250 * 0.48f);
constexpr auto BRICK_HEIGHT_PIXELS = static_cast<int>(85 * 0.48f);

// Ball is a square
constexpr auto BALL_SIZE_PIXELS = 50;

// For now simulation sizes are the same as pixel sizes

constexpr auto PADDLE_HEIGHT = PADDLE_HEIGHT_PIXELS;
constexpr auto PADDLE_WIDTH = PADDLE_WIDTH_PIXELS;
constexpr auto BRICK_WIDTH = BRICK_WIDTH_PIXELS;
constexpr auto BRICK_HEIGHT = BRICK_HEIGHT_PIXELS;
constexpr auto BALL_SIZE = BALL_SIZE_PIXELS;

constexpr float PADDLE_SPEED = 2700.f;
constexpr float BALL_SPEED = 1100.f;

constexpr int STARTING_LIVES = 3;
constexpr int SCORE_PER_BROKEN_BRICK = 20;
constexpr int LEVEL_CLEAR_SCORE = 500;

constexpr float DISALLOWED_BALL_LAUNCH_HORIZONTAL_THRESHOLD = 5;

//! How long the AI is allowed to play before ending the match automatically
constexpr float MAX_AI_PLAY_TIME = 120;

constexpr float ELAPSED_TIME_SCORE_PENALTY = 0.5f;

constexpr float PI = 3.14159265359f;

} // namespace mlbb
