#include "cpu.h"

#include <SDL3/SDL.h>

CPU::CPU() {
    this->pc = CPU::INITIAL_PC;
    this->sp = 0;

    this->display = new Display();

    std::copy(CPU::FONT.begin(), CPU::FONT.end(), this->memory.begin() + CPU::FONT_OFFSET);
}

CPU::~CPU() {
    delete this->display;
}

void CPU::push(uint8_t val) {
    this->memory[0x1FF - this->sp] = val;
    this->sp += 1;
}

uint8_t CPU::pop() {
    this->sp -= 1;
    return this->memory[0x1FF - this->sp];
}

void CPU::load_code(uint8_t *code, int length) {
    std::copy(code, code + length, this->memory.begin() + 0x200);
}

void CPU::step() {
    uint16_t instruction = this->memory[this->pc] << 8;
    instruction |= this->memory[this->pc + 1];

    SDL_Log("Instruction: %x", instruction);
    SDL_Log("PC: %x", this->pc);

    if ((instruction & 0xF000) == 0x1000) {
        // JP - jump to address
        uint16_t addr = instruction & 0x0FFF;
        SDL_Log("Jumping to address %x", addr);
        this->pc = addr;

        return; // Prevent PC increment
    } else if (instruction == 0x00E0) {
        // CLS - clear screen
        this->display->clear();
    }

    this->pc += 2;
}
