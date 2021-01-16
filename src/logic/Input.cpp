#include "Input.h"

#include <Input.hpp>

using namespace mlbb;

bool UserInput::GetLeftPressed() const
{
    return godot::Input::get_singleton()->is_action_pressed("g_left");
}

bool UserInput::GetRightPressed() const
{
    return godot::Input::get_singleton()->is_action_pressed("g_right");
}

bool UserInput::GetSpecialPressed() const
{
    return godot::Input::get_singleton()->is_action_pressed("g_special");
}
