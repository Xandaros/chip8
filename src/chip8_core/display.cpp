#include "display.h"

void Display::clear() {
    this->vram.fill(0);
}

void Display::draw_byte(int x, int y, uint8_t data) {
    for (int bit = 0; bit < 8; ++bit) {
        int vram_x = (x + bit) % Display::WIDTH;

        int idx = y * Display::WIDTH + vram_x;
        int old_val = this->vram[idx];
        int xor_val = (data >> (7 - bit)) & 0x01;

        this->vram[idx] = old_val ^ xor_val;
    }
}
