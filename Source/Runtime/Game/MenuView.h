#pragma once

#include "Audio/AudioSystem.h"
#include "Core/Menu.h"
#include "Game/Controls.h"
#include "Render/BitmapFont.h"

#include <SDL3/SDL.h>

namespace iron {

enum class MenuResult { None, Moved, Changed, Chosen, Back };

// Shared menu behaviour: Up/Down move, Left/Right change a setting, Confirm
// chooses an action, Back/Pause leave. Plays the menu sounds.
MenuResult navigate(Menu& menu, const MenuEvent& event, AudioSystem& audio);

struct MenuStyle {
    float x = 320, y = 160; // centre of the list's top line
    float width = 260;      // settings: label on the left, value on the right
    float spacing = 14;
    float noteGap = 6;      // below the last line
};

// Draws a menu on the small font: the focused line on a brass bar, disabled
// lines in steel, settings as "LABEL   < VALUE >", the focused note below.
void drawMenu(SDL_Renderer* r, const Menu& menu, const BitmapFont& font, const MenuStyle& style, int blink);

} // namespace iron
