#pragma once

namespace mlbb {

//! \brief Base type for input handling to the game
class Input {
public:
    virtual bool GetLeftPressed() const = 0;
    virtual bool GetRightPressed() const = 0;
    virtual bool GetSpecialPressed() const = 0;
};

class ProgrammaticInput : public Input {
public:
    bool GetLeftPressed() const override
    {
        return Left;
    }

    bool GetRightPressed() const override
    {
        return Right;
    }

    bool GetSpecialPressed() const override
    {
        return Special;
    }

    bool Left = false;
    bool Right = false;
    bool Special = false;
};

// \brief Reads user input from the keyboard for game input
class UserInput : public Input {
public:
    bool GetLeftPressed() const override;
    bool GetRightPressed() const override;
    bool GetSpecialPressed() const override;

public:
};
} // namespace mlbb
