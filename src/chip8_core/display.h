#pragma once

#include <array>
#include <mutex>
#include <stdint.h>

class Display {
    private:

    public:
        static const int WIDTH = 64;
        static const int HEIGHT = 32;

        std::mutex lock;

        std::array<uint8_t, 64 * 32> vram;

        void clear();
        bool draw_byte(int x, int y, uint8_t data);
};
