#pragma once

#include <array>
#include <mutex>
#include <stdint.h>

/// Video memory for the CHIP-8 emulator.
class Display {
    private:
        /// Video memory as an array of bytes.
        ///
        /// Each entry should either be 1 for a lit pixel, or 0 for an unlit one.
        /// Calculate the index for a pixel at (x, y) as `y * Display::WIDTH + x`
        ///
        /// \important Acquire the [lock](#lock) before accessing this field!
        std::array<uint8_t, 64 * 32> vram;

        /// A mutex lock for [vram](#vram).
        mutable std::mutex lock;


    public:
        /// Width of the display in pixels.
        static constexpr int WIDTH = 64;

        /// Height of the display in pixels.
        static constexpr int HEIGHT = 32;

        Display() = default;
        Display(const Display &other);
        Display& operator=(Display other);

        /// Clear the screen.
        void clear();

        /// Draw a single byte of sprite data.
        ///
        /// \param x Leftmost x position of where to draw the sprite.
        /// \param y Y position of where to draw the sprite.
        /// \param data The actual sprite data to be written. Each bit in this
        /// byte represents one pixel. If the bit is set, the pixel in video
        /// RAM will be toggled. If the bit is unset, it will remain as-is.
        ///
        /// \return A boolean indicating whether any bits were unset by this
        /// operation. Commonly used for collision detection.
        bool draw_byte(int x, int y, uint8_t data);

        /// Get a copy of the current vram.
        ///
        /// \return A copt of the current vram.
        std::array<uint8_t, Display::WIDTH * Display::HEIGHT> get_vram() const;
};
