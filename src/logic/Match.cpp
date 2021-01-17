#include "Match.h"

using namespace mlbb;

constexpr auto NEXT_BALL_DELAY = 0.2f;

Match::Match(Match&& other) : Width(other.Width), Height(other.Height)
{
    State = std::move(other.State);
    TotalElapsed = std::move(other.TotalElapsed);
    CurrentStateElapsed = std::move(other.CurrentStateElapsed);
    Balls = std::move(other.Balls);
    Paddles = std::move(other.Paddles);
    Bricks = std::move(other.Bricks);
}

Match::Match(const Match& other) : Width(other.Width), Height(other.Height)
{
    State = other.State;
    TotalElapsed = other.TotalElapsed;
    CurrentStateElapsed = other.CurrentStateElapsed;
    Balls = other.Balls;
    Paddles = other.Paddles;
    Bricks = other.Bricks;
}

void Match::Update(float elapsed, const Input& input)
{
    if(elapsed <= 0)
        return;

    TotalElapsed += elapsed;
    CurrentStateElapsed += elapsed;

    HandlePaddleMove(elapsed, input);

    switch(State) {
    case MatchState::Starting: {
        HandleStartup();
        break;
    }
    case MatchState::Serving: {

        break;
    }
    case MatchState::WaitingForNextServe: {
        if(CurrentStateElapsed > NEXT_BALL_DELAY) {
            MoveToState(MatchState::Serving);
        }
        break;
    }
    case MatchState::Playing: {
        break;
    }
    case MatchState::Ended: break;
    }
}

void Match::MoveToState(MatchState newState)
{
    State = newState;
    CurrentStateElapsed = 0;
}

void Match::HandlePaddleMove(float elapsed, const Input& input)
{
    switch(State) {
    case MatchState::Starting:
    case MatchState::Ended: return;
    default: break;
    }

    float movement = 0;

    if(input.GetLeftPressed()) {
        movement -= PADDLE_SPEED * elapsed;
    }

    if(input.GetRightPressed()) {
        movement += PADDLE_SPEED * elapsed;
    }

    for(auto& paddle : Paddles) {
        paddle.X += static_cast<int>(movement);

        if(paddle.X < 0) {
            paddle.X = 0;
        } else if(paddle.X + PADDLE_WIDTH > Width){
            paddle.X = Width - PADDLE_WIDTH;
        }
    }
}

void Match::HandleStartup()
{
    // Clear old state, if there is one
    Balls.clear();
    Bricks.clear();

    // Paddles are not cleared so that the game remembers their position between levels
    if(Paddles.size() > 1) {
        while(Paddles.size() > 1) {
            Paddles.pop_back();
        }
    } else {
        auto [x, y] = CalculatePaddleStartPosition();
        Paddles.emplace_back(x, y);
    }

    // TODO: create bricks

    MoveToState(MatchState::Serving);
}

std::tuple<int, int> Match::CalculatePaddleStartPosition() const
{
    return std::tuple<int, int>((Width / 2) - (PADDLE_WIDTH / 2), Height - PADDLE_HEIGHT);
}
