#pragma once

#include "Definitions.h"
#include "GameElements.h"
#include "Input.h"

#include <random>
#include <tuple>
#include <vector>

namespace mlbb {

enum class MatchState { Starting, Serving, WaitingForNextServe, Ended, Playing };


//! \brief A single match of brick breaker
//!
//! Separate class from Game to allow running this in the background and also multiple at the
//! same time
class Match {
public:
    //! \brief Creates a match with specified size. Due to the way rendering is done in Godot
    //! We don't need to resize even when window size changes
    Match(int width, int height);

    Match(Match&& other);
    Match(const Match& other);

    Match& operator=(const Match& other) = delete;
    Match& operator=(Match&& other) = delete;

    //! \brief Updates the game state
    void Update(float elapsed, const Input& input);


    // None of the objects in these should be modified, only used for drawing
    const std::vector<Ball>& GetBallsForDrawing() const
    {
        return Balls;
    }
    const std::vector<Paddle>& GetPaddlesForDrawing() const
    {
        return Paddles;
    }
    const std::vector<Brick>& GetBricksForDrawing() const
    {
        return Bricks;
    }

private:
    void MoveToState(MatchState newState);

    void HandlePaddleMove(float elapsed, const Input& input);
    void HandleBallMovement(float elapsed);

    void HandleStartup();

    void ServeBall();

    std::tuple<int, int> CalculatePaddleStartPosition() const;
    std::tuple<int, int> CalculateBallStartPosition() const;
    godot::Vector2 CreateRandomInitialBallDirection();

    static void HandleBallCollision(Ball& ball, const GameElement& collidedAgainst);

private:
    const int Width;
    const int Height;

    //! The state of the match. Don't assign directly, use MoveToState
    MatchState State = MatchState::Starting;

    float TotalElapsed = 0;
    float CurrentStateElapsed = 0;

    std::mt19937 RandomEngine;
    std::normal_distribution<float> BallHorizontalDistribution{0, 80};

    std::vector<Ball> Balls;
    std::vector<Paddle> Paddles;
    std::vector<Brick> Bricks;
};

} // namespace mlbb
