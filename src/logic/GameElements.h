#pragma once

#include <Vector2.hpp>
namespace mlbb {

enum class PaddleSize { Normal };

struct GameElement {
public:
    GameElement(int x, int y) : X(x), Y(y) {}

    godot::Vector2 PositionAsVector() const{
        return godot::Vector2(X, Y);
    }

    int X;
    int Y;
};

struct Brick : public GameElement {
public:
    Brick(int x, int y) : GameElement(x, y) {}

    int Durability = 1;
};

struct Paddle : public GameElement {
public:
    Paddle(int x, int y) : GameElement(x, y) {}

    PaddleSize Size = PaddleSize::Normal;
};

struct Ball : public GameElement {
public:
    Ball(int x, int y) : GameElement(x, y) {}
};
} // namespace mlbb
