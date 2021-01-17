#include "Globals.h"

using namespace mlbb;

void Globals::_register_methods()
{
    register_method("_ready", &Globals::_ready);

    register_property("is_player", &Globals::IsPlayer, false);
}

void Globals::_init() {}

void Globals::_ready() {}
