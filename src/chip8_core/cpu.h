#pragma once

#include "display.h"

#include<array>
#include <atomic>
#include <stdint.h>

/// Main CHIP-8 implementation.
///
/// Responsible for fetching and executing instructions.
class CPU {
    private:
        /// Array of keys, indicating whether the corresponding key is pressed.
        bool keys[16];
        /// When waiting for a key, this attribute indicates which register the pressed key should be written to.
        /// \note Using 0xFF as a special "not waiting" value
        uint8_t key_wait_register = 0xFF;

    public:
        /// Static font data.
        ///
        /// Copied into the CPU's memory when the CPU is created.
        static constexpr std::array<uint8_t, 80> FONT = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80, // F
        };

        /// Display containing the video memory.
        Display *display;

        /// Internal memory, visible to the running ROM.
        std::array<uint8_t, 4096> memory;

        /// State of the CPU registers.
        uint8_t registers[16];

        /// Program counter.
        uint16_t pc;

        /// Stack pointer.
        uint8_t sp;

        /// Index register.
        uint16_t i;

        /// Delay timer register.
        std::atomic<uint8_t> dt;

        /// Sound timer register.
        std::atomic<uint8_t> st;

        CPU();
        ~CPU();

        /// Load code into the CPU's memory.
        ///
        /// \param code Pointer to the code to be loaded.
        /// \param length Length of the code to be loaded in bytes.
        void load_code(uint8_t *code, int length);

        /// Load code into the CPU's memory.
        ///
        /// \param path Path to a file from which to load the code.
        ///
        /// \return Whether the load was successful.
        bool load_code_from_file(const char *path);

        /// Execute the next instruction
        void step();

        /// Push a value onto the stack.
        ///
        /// \param val Value to be pushed.
        void push(uint8_t val);

        /// Pop a value off the stack.
        ///
        /// \return The popped value.
        uint8_t pop();

        /// Tick the timers.
        ///
        /// Should be called at a frequency of 60 Hz.
        void tick_timers();

        /// Set the key state of a given key.
        ///
        /// Should be called whenever a key is pressed or released.
        ///
        /// \param key Which key's state has changed. (0x0 - 0xF)
        /// \param down Whether the key was pressed (true) or released (false).
        void set_key_down(uint8_t key, bool down);

        /// Returns whether the given key is currently being pressed.
        ///
        /// \return True if the key is being pressed.
        bool is_key_down(uint8_t key);

        /// Initial value of the program counter.
        static const uint16_t INITIAL_PC = 0x200;

        /// Offset at which the font is loaded.
        static const uint16_t FONT_OFFSET = 0;
};
