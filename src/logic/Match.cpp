#include "Match.h"

using namespace mlbb;

//! This is used to cap the update simulation time to 50 ms in case the game lags too much
constexpr auto MAX_ELAPSED_TIME_PER_UPDATE = 0.05f;

constexpr auto NEXT_BALL_DELAY = 0.2f;

constexpr float BALL_SPEED = 600.f;

constexpr auto BALL_LAUNCH_UPWARDS = -130;

constexpr auto SIDE_WALL_CALCULATION_THICKNESS = 10000;
constexpr auto SIDE_WALL_OVERLAP = 100;


Match::Match(int width, int height) : Width(width), Height(height)
{
    // Setup random number generator
    std::random_device randomDevice;
    std::seed_seq seed{randomDevice(), randomDevice(), randomDevice(), randomDevice(),
        randomDevice(), randomDevice(), randomDevice(), randomDevice()};
    RandomEngine.seed(seed);
}

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

    if(elapsed > MAX_ELAPSED_TIME_PER_UPDATE)
        elapsed = MAX_ELAPSED_TIME_PER_UPDATE;

    TotalElapsed += elapsed;
    CurrentStateElapsed += elapsed;

    HandlePaddleMove(elapsed, input);
    HandleBallMovement(elapsed);

    switch(State) {
    case MatchState::Starting: {
        HandleStartup();
        break;
    }
    case MatchState::Serving: {
        ServeBall();
        MoveToState(MatchState::Playing);
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
        } else if(paddle.X + PADDLE_WIDTH > Width) {
            paddle.X = Width - PADDLE_WIDTH;
        }
    }
}

void Match::HandleBallMovement(float elapsed)
{
    switch(State) {
    case MatchState::Ended: return;
    default: break;
    }

    for(auto iter = Balls.begin(); iter != Balls.end();) {
        auto& ball = *iter;

        auto newPos = ball.PositionAsVector() + ball.Direction * BALL_SPEED * elapsed;

        ball.X = newPos.x;
        ball.Y = newPos.y;

        // Bottom side collision
        if(ball.Y >= Height - BALL_SIZE) {
            // Ball went through the bottom of the playing field
            iter = Balls.erase(iter);
            // TODO: handle losing a life
            continue;
        }

        // Wall collisions
        if(ball.X < 0) {
            // Left side collision
            HandleBallCollision(
                ball, GameElement(-SIDE_WALL_CALCULATION_THICKNESS, -SIDE_WALL_OVERLAP,
                          SIDE_WALL_CALCULATION_THICKNESS, Height + SIDE_WALL_OVERLAP * 2));
        }

        if(ball.Y < 0) {
            // Top collision
            HandleBallCollision(
                ball, GameElement(-SIDE_WALL_OVERLAP, -SIDE_WALL_CALCULATION_THICKNESS,
                          Width + SIDE_WALL_OVERLAP * 2, SIDE_WALL_CALCULATION_THICKNESS));
        }

        if(ball.X + BALL_SIZE > Width) {
            // Right side collision
            HandleBallCollision(
                ball, GameElement(Width, -SIDE_WALL_OVERLAP, SIDE_WALL_CALCULATION_THICKNESS,
                          Height + SIDE_WALL_OVERLAP * 2));
        }

        for(const auto& paddle : Paddles) {
            if(ball.OverlapsWith(paddle)) {
                HandleBallCollision(ball, paddle);
            }
        }

        // TODO: brick collisions

        for(auto iter2 = Balls.begin(); iter2 != Balls.end(); ++iter2) {
            if(iter == iter2)
                continue;

            if(ball.OverlapsWith(*iter2)) {
                HandleBallCollision(ball, *iter2);
            }
        }

        ++iter;
    }

    // Start serving next ball if all balls are gone
    if(Balls.empty() && State == MatchState::Playing) {
        MoveToState(MatchState::WaitingForNextServe);
    }
}

void Match::HandleStartup()
{
    // Clear old game objects
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

void Match::ServeBall()
{
    auto [x, y] = CalculateBallStartPosition();
    auto ball = Ball(x, y);
    ball.Direction = CreateRandomInitialBallDirection();

    Balls.push_back(ball);
}

std::tuple<int, int> Match::CalculatePaddleStartPosition() const
{
    return std::tuple<int, int>((Width / 2) - (PADDLE_WIDTH / 2), Height - PADDLE_HEIGHT);
}

std::tuple<int, int> Match::CalculateBallStartPosition() const
{
    return std::tuple<int, int>((Width / 2) - (BALL_SIZE / 2), (Height / 2) - (BALL_SIZE / 2));
}

godot::Vector2 Match::CreateRandomInitialBallDirection()
{
    return godot::Vector2(BallHorizontalDistribution(RandomEngine), BALL_LAUNCH_UPWARDS)
        .normalized();
}

void Match::HandleBallCollision(Ball& ball, const GameElement& collidedAgainst)
{
    const auto objectCenter = collidedAgainst.GetCenterPoint();
    const auto ballCenter = ball.GetCenterPoint();

    godot::Vector2 normal;

    const auto angle = objectCenter.angle_to(ballCenter);

    // TODO: is there a better way to calculate this?
    if(angle < 0) {
        // Up angle
        if(angle > -PI / 4) {
            // Right
            normal = godot::Vector2(1, 0);
        } else if(angle > -PI * (3 / 4.f)) {
            // Up
            normal = godot::Vector2(0, -1);
        } else {
            // Left
            normal = godot::Vector2(-1, 0);
        }
    } else {
        // Down angle
        if(angle < PI / 4) {
            // Right
            normal = godot::Vector2(1, 0);
        } else if(angle < PI * (3 / 4.f)) {
            // Down
            normal = godot::Vector2(0, 1);
        } else {
            // Left
            normal = godot::Vector2(-1, 0);
        }
    }

    // TODO: normal variability for the paddle based on how center the ball hit

    // TODO: maybe the - should be removed
    ball.Direction = -ball.Direction.reflect(normal);

    return;

    // Push the ball back by at least 1 pixel to make sure it isn't colliding again

    int xPush;
    int yPush;

    for(int multiplier = 1; multiplier < 100; ++multiplier) {
        const auto scaled = normal * multiplier;
        xPush = static_cast<int>(std::round(scaled.x));
        yPush = static_cast<int>(std::round(scaled.y));

        if(xPush != 0 || yPush != 0)
            break;
    }

    ball.X += xPush;
    ball.Y += yPush;
}
