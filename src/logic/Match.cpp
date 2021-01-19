#include "Match.h"

using namespace mlbb;

//! This is used to cap the update simulation time to 30 ms in case the game lags too much
constexpr auto MAX_ELAPSED_TIME_PER_UPDATE = 0.03f;

constexpr auto NEXT_BALL_DELAY = 0.25f;

constexpr float BALL_SPEED = 900.f;
constexpr auto BALL_LAUNCH_UPWARDS = -130;
constexpr auto BALL_OUT_OF_BOUNDS = 10000;
constexpr bool USE_BALL_PUSH = true;
constexpr bool BALL_CAN_BREAK_ONLY_ONE_BRICK_PER_UPDATE = true;
constexpr float BALL_HORIZONTAL_STUCK_THRESHOLD = 0.5f;

constexpr auto PADDLE_VELOCITY_TRANSFER_TO_BALL_FRACTION = 0.2f;

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
    RandomEngine = std::move(other.RandomEngine);
    BallHorizontalDistribution = std::move(other.BallHorizontalDistribution);
    HitBricks = std::move(other.HitBricks);
}

Match::Match(const Match& other) : Width(other.Width), Height(other.Height)
{
    State = other.State;
    TotalElapsed = other.TotalElapsed;
    CurrentStateElapsed = other.CurrentStateElapsed;
    Balls = other.Balls;
    Paddles = other.Paddles;
    Bricks = other.Bricks;
    RandomEngine = other.RandomEngine;
    BallHorizontalDistribution = other.BallHorizontalDistribution;
    HitBricks = other.HitBricks;
}

void Match::Update(float elapsed, const Input& input)
{
    if(elapsed <= 0)
        return;

    if(elapsed > MAX_ELAPSED_TIME_PER_UPDATE)
        elapsed = MAX_ELAPSED_TIME_PER_UPDATE;

    TotalElapsed += elapsed;
    CurrentStateElapsed += elapsed;

    HitBricks.clear();

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

    HandleBrickBreaking();
}

void Match::MoveToState(MatchState newState)
{
    State = newState;
    CurrentStateElapsed = 0;
}

void Match::HandlePaddleMove(float elapsed, const Input& input)
{
    ResetPaddleVelocity();

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
        const auto previous = paddle.PositionAsVector();

        paddle.X += static_cast<int>(movement);

        if(paddle.X < 0) {
            paddle.X = 0;
        } else if(paddle.X + PADDLE_WIDTH > Width) {
            paddle.X = Width - PADDLE_WIDTH;
        }

        paddle.Velocity = (paddle.PositionAsVector() - previous) / elapsed;
    }
}

void Match::ResetPaddleVelocity()
{
    for(auto& paddle : Paddles) {
        paddle.Velocity = godot::Vector2(0, 0);
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
        // And also out of bounds checks
        if((ball.Y >= Height) ||
            (ball.Y < -BALL_OUT_OF_BOUNDS || ball.Y > BALL_OUT_OF_BOUNDS ||
                ball.X < -BALL_OUT_OF_BOUNDS || ball.X > BALL_OUT_OF_BOUNDS)) {
            // Ball went through the bottom of the playing field (or out of bounds)
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

        // Paddle collision
        for(const auto& paddle : Paddles) {
            if(ball.OverlapsWith(paddle)) {
                HandleBallCollision(
                    ball, paddle, paddle.Velocity * PADDLE_VELOCITY_TRANSFER_TO_BALL_FRACTION);
            }
        }

        // Brick collision
        for(const auto& brick : Bricks) {
            if(ball.OverlapsWith(brick)) {
                HandleBallCollision(ball, brick);

                if(!BALL_CAN_BREAK_ONLY_ONE_BRICK_PER_UPDATE || HitBricks.empty()) {
                    HitBricks.push_back(&brick);
                }
            }
        }

        // Ball to ball collision
        for(auto iter2 = Balls.begin(); iter2 != Balls.end(); ++iter2) {
            if(iter == iter2)
                continue;

            if(ball.OverlapsWith(*iter2)) {
                HandleBallCollision(ball, *iter2);
            }
        }

        // If stuck exactly horizontal, add a bit of upwards motion
        if(std::abs(ball.Direction.y) < BALL_HORIZONTAL_STUCK_THRESHOLD) {
            ball.Direction = (ball.Direction + godot::Vector2(0, -0.5)).normalized();
        }

        ++iter;
    }

    // Start serving next ball if all balls are gone
    if(Balls.empty() && State == MatchState::Playing) {
        MoveToState(MatchState::WaitingForNextServe);
    }
}

void Match::HandleBrickBreaking()
{
    if(State == MatchState::Ended || State == MatchState::Starting)
        return;

    for(auto brick : HitBricks) {
        // Find the matching brick to destroy
        for(auto iter = Bricks.begin(); iter != Bricks.end(); ++iter) {
            if(&*iter == brick) {
                // TODO: break animation?
                Bricks.erase(iter);
                break;
            }
        }

        // TODO: add score
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

    SetupLevel();

    MoveToState(MatchState::Serving);
}

void Match::SetupLevel()
{
    // TODO: make different brick configurations

    const auto columns = Width / BRICK_WIDTH;

    const auto bricksStartY = 80;
    const auto bricksStartX = 0;

    for(int row = 0; row < 5; ++row) {
        for(int column = 0; column < columns; ++column) {
            Bricks.emplace_back(
                bricksStartX + (column * BRICK_WIDTH), bricksStartY + (row * BRICK_HEIGHT));
        }
    }
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

void Match::HandleBallCollision(Ball& ball, const GameElement& collidedAgainst,
    std::optional<godot::Vector2> extraVelocity)
{
    godot::Vector2 normal;

    // Find the lowest amount of penetration
    int left = collidedAgainst.X - (ball.X + ball.Width);
    int top = collidedAgainst.Y - (ball.Y + ball.Height);
    int right = ball.X - (collidedAgainst.X + collidedAgainst.Width);
    int bottom = ball.Y - (collidedAgainst.Y + collidedAgainst.Height);

    // If no penetration, exclude those
    if(left > 0)
        left = std::numeric_limits<int>::min();
    if(right > 0)
        right = std::numeric_limits<int>::min();
    if(top > 0)
        top = std::numeric_limits<int>::min();
    if(bottom > 0)
        bottom = std::numeric_limits<int>::min();

    int lowest = std::max(left, std::max(right, std::max(top, bottom)));

    if(left == lowest) {
        // Left
        normal = godot::Vector2(-1, 0);
    } else if(right == lowest) {
        // Right
        normal = godot::Vector2(1, 0);
    } else if(top == lowest) {
        // Up
        // TODO: this is not tested as bricks aren't added yet
        normal = godot::Vector2(0, -1);
    } else {
        // Down
        normal = godot::Vector2(0, 1);
    }

    // TODO: normal variability for the paddle based on how center the ball hit

    ball.Direction = -ball.Direction.reflect(normal);

    if(USE_BALL_PUSH) {
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

    // Extra velocity when hitting a moving paddle to make the player have more impact on the
    // ball direction
    if(extraVelocity) {
        ball.Direction = ((ball.Direction * BALL_SPEED) + *extraVelocity).normalized();
    }
}
