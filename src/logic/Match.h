#pragma once

#include "Definitions.h"
#include "GameElements.h"
#include "Input.h"

#include <optional>
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

    bool HasEnded() const
    {
        return State == MatchState::Ended;
    }

    float GetElapsedTime() const
    {
        return TotalElapsed;
    }

    void Forfeit()
    {
        MoveToState(MatchState::Ended);
    }

    int GetScore() const
    {
        return Score;
    }

    int GetTimedScore() const
    {
        return std::max(
            0, static_cast<int>(GetScore() - (TotalElapsed * ELAPSED_TIME_SCORE_PENALTY)));
    }

    int GetLivesLeft() const
    {
        return LivesLeft;
    }


    // None of the objects in these should be modified, only used for drawing
    // And for AI to read their state, might be a bit cleaner if that had separate named
    // methods for it
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
    void ResetPaddleVelocity();
    void HandleBallMovement(float elapsed);
    void HandleBrickBreaking();

    void HandleStartup();
    void SetupLevel();

    void ServeBall();

    std::tuple<int, int> CalculatePaddleStartPosition() const;
    std::tuple<int, int> CalculateBallStartPosition() const;
    godot::Vector2 CreateRandomInitialBallDirection();

    static void HandleBallCollision(Ball& ball, const GameElement& collidedAgainst,
        std::optional<godot::Vector2> extraVelocity = {});

private:
    const int Width;
    const int Height;

    //! The state of the match. Don't assign directly, use MoveToState
    MatchState State = MatchState::Starting;

    float TotalElapsed = 0;
    float CurrentStateElapsed = 0;

    int Score = 0;
    int LivesLeft = STARTING_LIVES;

    std::mt19937 RandomEngine;
    std::normal_distribution<float> BallHorizontalDistribution{0, 80};

    std::vector<Ball> Balls;
    std::vector<Paddle> Paddles;
    std::vector<Brick> Bricks;

    //! Bricks that were hit this frame and should be destroyed
    std::vector<const Brick*> HitBricks;
};

} // namespace mlbb
