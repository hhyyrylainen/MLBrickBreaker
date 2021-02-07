#include "Match.h"

#include <algorithm>

using namespace mlbb;

constexpr auto NEXT_BALL_DELAY = 0.25f;

constexpr auto BALL_LAUNCH_UPWARDS = -130;
constexpr auto BALL_OUT_OF_BOUNDS = 10000;
constexpr bool USE_BALL_PUSH = true;
constexpr bool BALL_CAN_BREAK_ONLY_ONE_BRICK_PER_UPDATE = true;
constexpr float BALL_HORIZONTAL_STUCK_THRESHOLD = 0.15f;
constexpr float BALL_VERTICAL_STUCK_THRESHOLD = 0.15f;
constexpr auto BALL_HORIZONTAL_STUCK_NUDGE_DEFAULT = -0.0035f;
constexpr auto BALL_HORIZONTAL_STUCK_NUDGE_MULTIPLIER = 2.5f;
constexpr auto BALL_VERTICAL_STUCK_NUDGE = 0.04f;

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
    // Time doesn't pass once match has ended
    if(State != MatchState::Ended)
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
        // Check for next level
        if(Bricks.empty()) {

            // Won the current level
            MoveToState(MatchState::Starting);
            Score += LEVEL_CLEAR_SCORE;
        }

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
    switch(State) {
    case MatchState::Starting:
        ResetPaddleVelocity();
    case MatchState::Ended: return;
    default: break;
    }

    const float paddleAccelerator = static_cast<float>(PaddleSpeed) / PADDLE_TIME_TO_FULL_SPEED * elapsed;
    const float maxSpeed = PADDLE_SPEED;
    for (auto& paddle : Paddles) {
        if(input.GetLeftPressed() && paddle.Velocity.x > 0){
            paddle.Velocity.x = 0;
        }
        if(input.GetRightPressed() && paddle.Velocity.x < 0){
            paddle.Velocity.x = 0;
        }
        if(input.GetLeftPressed()){
            paddle.Velocity.x -= paddleAccelerator;
        }
        if(input.GetRightPressed()){
            paddle.Velocity.x += paddleAccelerator;
        }
        if (!input.GetRightPressed() && !input.GetLeftPressed()){
            /* decelerate */
            if (std::abs(paddle.Velocity.x) < (2 * paddleAccelerator)){
                paddle.Velocity.x = 0;
            }
            else if (paddle.Velocity.x > 0 ){
                paddle.Velocity.x -= paddleAccelerator;
            }
            else if (paddle.Velocity.x < 0 ){
                paddle.Velocity.x += paddleAccelerator;
            }
        }
        if (std::abs(paddle.Velocity.x) > maxSpeed){
            std::clamp(
                paddle.Velocity.x,
                -maxSpeed,
                maxSpeed
            ); 
        }
        paddle.X += paddle.Velocity.x * elapsed;
        if(paddle.X < 0) {
            paddle.X = 0;
            paddle.Velocity.x = 0;
        } else if(paddle.X + PADDLE_WIDTH > Width) {
            paddle.X = Width - PADDLE_WIDTH;
            paddle.Velocity.x = 0;
        }
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

        auto newPos = ball.PositionAsVector() + ball.Direction * BallSpeed * elapsed;

        ball.X = newPos.x;
        ball.Y = newPos.y;

        // Bottom side collision
        // And also out of bounds checks
        if((ball.Y >= Height) ||
            (ball.Y < -BALL_OUT_OF_BOUNDS || ball.Y > BALL_OUT_OF_BOUNDS ||
                ball.X < -BALL_OUT_OF_BOUNDS || ball.X > BALL_OUT_OF_BOUNDS)) {
            // Ball went through the bottom of the playing field (or out of bounds)
            iter = Balls.erase(iter);
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
                HandleBallCollision(ball, paddle,
                    paddle.Velocity * PADDLE_VELOCITY_TRANSFER_TO_BALL_FRACTION, true);
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

            auto nudge = ball.Direction;
            nudge.y *= BALL_HORIZONTAL_STUCK_NUDGE_MULTIPLIER;

            if(std::abs(nudge.y) < std::abs(BALL_HORIZONTAL_STUCK_NUDGE_DEFAULT))
                nudge.y = BALL_HORIZONTAL_STUCK_NUDGE_DEFAULT;

            ball.Direction = (ball.Direction + nudge).normalized();
        }

        // If exactly vertical, give the ball a little push
        if(std::abs(ball.Direction.x) < BALL_VERTICAL_STUCK_THRESHOLD) {

            auto nudge = godot::Vector2(BALL_VERTICAL_STUCK_NUDGE, 0);

            if(ball.X > Width / 2) {
                nudge = godot::Vector2(-BALL_VERTICAL_STUCK_NUDGE, 0);
            }

            ball.Direction = (ball.Direction + nudge).normalized();
        }

        ++iter;
    }

    // Start serving next ball if all balls are gone
    if(Balls.empty() && State == MatchState::Playing) {
        --LivesLeft;

        if(LivesLeft >= 0) {
            MoveToState(MatchState::WaitingForNextServe);
        } else {
            // Ran out of lives
            MoveToState(MatchState::Ended);
        }
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
                Score += SCORE_PER_BROKEN_BRICK;
                break;
            }
        }
    }
}

void Match::HandleStartup()
{
    // Clear old game objects
    Balls.clear();
    Bricks.clear();

    // Paddles are not cleared so that the game remembers their position between levels
    if(!Paddles.empty()) {
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
    float horizontal;

    // Make sure that ball doesn't start too vertically
    do {
        horizontal = BallHorizontalDistribution(RandomEngine);
    } while(std::abs(horizontal) < DISALLOWED_BALL_LAUNCH_HORIZONTAL_THRESHOLD);

    return godot::Vector2(horizontal, BALL_LAUNCH_UPWARDS).normalized();
}

void Match::HandleBallCollision(Ball& ball, const GameElement& collidedAgainst,
    std::optional<godot::Vector2> extraVelocity, bool useAngleAdjustment)
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

    std::optional<float> adjustmentAngle;

    if(left == lowest) {
        // Left
        normal = godot::Vector2(-1, 0);
    } else if(right == lowest) {
        // Right
        normal = godot::Vector2(1, 0);
    } else if(top == lowest) {
        // Up
        normal = godot::Vector2(0, -1);

        if(useAngleAdjustment) {
            // Normal variability for the paddle based on how center the ball hit
            const float hitPointPercentage =
                std::clamp((ball.GetCenterPoint().x - collidedAgainst.X) /
                               static_cast<float>(collidedAgainst.Width),
                    0.f, 1.f);

            // Don't change bounce angle if hit close enough to the center
            if(hitPointPercentage < 0.5f - PADDLE_CENTER_FLAT_BOUNCE_AREA ||
                hitPointPercentage > 0.5f + PADDLE_CENTER_FLAT_BOUNCE_AREA) {
                // TODO: could use a non-linear function here
                // Multiply by two is used to from maximum of 0.5 to 1 so that then max angle
                // can be reached
                if(hitPointPercentage <= 0.5f) {
                    adjustmentAngle =
                        (0.5f - hitPointPercentage) * 2 * -PADDLE_MAX_ANGLE_DEVIATION_RADIANS;
                } else {
                    adjustmentAngle =
                        (hitPointPercentage - 0.5f) * 2 * PADDLE_MAX_ANGLE_DEVIATION_RADIANS;
                }
            }
        }

    } else {
        // Down
        normal = godot::Vector2(0, 1);
    }

    if(adjustmentAngle) {
        normal = normal.rotated(*adjustmentAngle);
    }

    ball.Direction = -ball.Direction.reflect(normal);

    if(adjustmentAngle) {
        const auto up = godot::Vector2(0, -1);

        const auto angle = ball.Direction.angle_to(up);

        // Prevent the ball from deflecting so much that it goes through and bounces to the
        // wrong side
        if(std::abs(angle) > PADDLE_ANGLE_DEVIATION_RESULTING_MAX_ANGLE) {

            ball.Direction = ball.Direction.bounce(up);
        }
    }

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
        ball.Direction = ((ball.Direction * BallSpeed) + *extraVelocity).normalized();
    }
}
