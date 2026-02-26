#include "display.h"
#include <mutex>

Display::Display(const Display &other) {
    std::lock_guard<std::mutex> lock(other.lock);

    std::copy(other.vram.begin(), other.vram.end(), this->vram.begin());
}

Display& Display::operator=(Display other) {
    std::lock_guard<std::mutex> lock(this->lock);

    std::swap(this->vram, other.vram);
    return *this;
}

void Display::clear() {
    this->vram.fill(0);
}

bool Display::draw_byte(int x, int y, uint8_t data) {
    std::lock_guard<std::mutex> lock(this->lock);

    bool ret = false;

    for (int bit = 0; bit < 8; ++bit) {
        int vram_x = (x + bit) % Display::WIDTH;

        int idx = y * Display::WIDTH + vram_x;
        int old_val = this->vram[idx];
        int xor_val = (data >> (7 - bit)) & 0x01;

        if ((old_val & xor_val) == 1) {
            ret = true;
        }

        this->vram[idx] = old_val ^ xor_val;
    }

    return ret;
}

std::array<uint8_t, Display::WIDTH * Display::HEIGHT> Display::get_vram() const {
    std::lock_guard<std::mutex> lock(this->lock);

    return this->vram;
}
