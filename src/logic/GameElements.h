#pragma once

#include "Definitions.h"

#include <Vector2.hpp>

#include <tuple>

namespace mlbb {

enum class PaddleSize { Normal };

struct GameElement {
public:
    GameElement(int x, int y, int width, int height) : X(x), Y(y), Width(width), Height(height)
    {}

    godot::Vector2 PositionAsVector() const
    {
        return godot::Vector2(X, Y);
    }

    godot::Vector2 GetCenterPoint() const
    {
        return godot::Vector2(X + Width * 0.5f, Y + Height * 0.5f);
    }

    std::tuple<int, int> GetCenterPointInt() const
    {
        return {X + Width / 2, Y + Height / 2};
    }

    bool OverlapsWith(const GameElement& other) const
    {
        if(other.X + other.Width < X || other.Y + other.Height < Y)
            return false;

        if(X + Width < other.X || Y + Height < other.Y)
            return false;

        return true;
    }

    int X;
    int Y;
    int Width;
    int Height;
};

struct Brick : public GameElement {
public:
    Brick(int x, int y) : GameElement(x, y, BRICK_WIDTH, BRICK_HEIGHT) {}

    int Durability = 1;
};

struct Paddle : public GameElement {
public:
    Paddle(int x, int y) : GameElement(x, y, PADDLE_WIDTH, PADDLE_HEIGHT) {}

    PaddleSize Size = PaddleSize::Normal;
    godot::Vector2 Velocity = godot::Vector2(0, 0);
};

struct Ball : public GameElement {
public:
    Ball(int x, int y) : GameElement(x, y, BALL_SIZE, BALL_SIZE) {}

    godot::Vector2 Direction = godot::Vector2(0, 0);
};
} // namespace mlbb
