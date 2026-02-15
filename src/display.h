#pragma once

#include <array>
#include <stdint.h>

class Display {
    private:
        std::array<uint8_t, 64 * 32> vram;

    public:
        void clear();
};
