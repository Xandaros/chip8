#include "display.h"

#include <SDL3/SDL.h>

void Display::clear() {
    SDL_Log("Screen cleared");
    this->vram.fill(0);
}
