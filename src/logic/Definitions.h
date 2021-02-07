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

constexpr float BALL_SPEED = 1100.f;
constexpr float PADDLE_TIME_TO_FULL_SPEED = 0.15f;
constexpr float PADDLE_SPEED = 2700.f;

constexpr int STARTING_LIVES = 3;
constexpr int SCORE_PER_BROKEN_BRICK = 20;
constexpr int LEVEL_CLEAR_SCORE = 500;

constexpr float DISALLOWED_BALL_LAUNCH_HORIZONTAL_THRESHOLD = 20;

//! How long the AI is allowed to play before ending the match automatically
constexpr float MAX_AI_PLAY_TIME = 75;

constexpr float ELAPSED_TIME_SCORE_PENALTY = 0.5f;

//! This is used to cap the update simulation time to 25 ms in case the game lags too much
constexpr auto MAX_ELAPSED_TIME_PER_UPDATE = 0.025f;
constexpr auto DELTA_FOR_60_FPS = 1/60.f;
constexpr int USE_60_FPS_DELTA_WITH_SPED_UP_TRAINING_THRESHOLD = 1;

constexpr float PI = 3.14159265359f;

constexpr float PADDLE_CENTER_FLAT_BOUNCE_AREA = 0.15f;
constexpr float PADDLE_MAX_ANGLE_DEVIATION_RADIANS = PI / 6;
constexpr float PADDLE_ANGLE_DEVIATION_RESULTING_MAX_ANGLE = (PI / 2) - (PI / 180.f);


} // namespace mlbb
