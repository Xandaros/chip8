#include "cpu.h"

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

    if (instruction == 0x00E0) {
        // CLS - clear screen
        this->display->clear();
    } else if (instruction == 0x00EE) {
        // RET - return from subroutine
        uint16_t addr = this->pop();
        addr |= this->pop() << 8;

        this->pc = addr;

        return; // Prevent PC increment
    } else if ((instruction & 0xF000) == 0x1000) {
        // JP - jump to address
        uint16_t addr = instruction & 0x0FFF;
        this->pc = addr;

        return; // Prevent PC increment
    } else if ((instruction & 0xF000) == 0x2000) {
        // CALL - call a subroutine
        uint16_t addr = instruction & 0x0FFF;

        this->pc += 2;

        this->push(this->pc >> 8);
        this->push(this->pc & 0x00FF);

        this->pc = addr;

        return; // Prevent PC increment
    } else if ((instruction & 0xF000) == 0x6000) {
        uint8_t reg = (instruction & 0x0F00) >> 8;
        uint8_t value = instruction & 0x00FF;

        this->registers[reg] = value;
    }

    this->pc += 2;
}
