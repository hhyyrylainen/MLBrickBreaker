#pragma once

#include <Godot.hpp>
#include <Node.hpp>

namespace mlbb {

//! \brief Global, non scene specific data. This is in C++ because for some reason gdscript
//! globals don't update until some time passes
class Globals : public godot::Node {
    GODOT_CLASS(Globals, godot::Node);

public:
    void _init();
    void _ready();

    bool GetIsPlayer() const
    {
        return IsPlayer;
    }

    static void _register_methods();

private:
    bool IsPlayer = false;
};
} // namespace mlbb
