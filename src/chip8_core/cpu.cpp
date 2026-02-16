#include "cpu.h"
#include <cstdlib>

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
    } else if ((instruction & 0xF000) == 0x3000) {
        // SE Vx, yy - Skip next instruction if Vx == yy
        uint8_t reg = (instruction & 0x0F00) >> 8;
        uint8_t value = instruction & 0x00FF;

        if (this->registers[reg] == value) {
            this->pc += 2;
        }
    } else if ((instruction & 0xF000) == 0x4000) {
        // SNE Vx, yy - Skip next instruction if Vx != yy
        uint8_t reg = (instruction & 0x0F00) >> 8;
        uint8_t value = instruction & 0x00FF;

        if (this->registers[reg] != value) {
            this->pc += 2;
        }
    } else if ((instruction & 0xF00F) == 0x5000) {
        // SNE Vx, Vy - Skip next instruction if Vx == yy
        uint8_t reg1 = (instruction & 0x0F00) >> 8;
        uint8_t reg2 = (instruction & 0x00F0) >> 4;

        if (this->registers[reg1] == this->registers[reg2]) {
            this->pc += 2;
        }
    } else if ((instruction & 0xF000) == 0x6000) {
        // LD Vx, yy - Load immediate to register
        uint8_t reg = (instruction & 0x0F00) >> 8;
        uint8_t value = instruction & 0x00FF;

        this->registers[reg] = value;
    } else if ((instruction & 0xF000) == 0x7000) {
        // ADD Vx, yy - Add immediate to register
        uint8_t reg = (instruction & 0x0F00) >> 8;
        uint8_t value = instruction & 0x00FF;

        this->registers[reg] += value;
    } else if ((instruction & 0xF00F) == 0x8000) {
        // LD Vx, Vy - Set Vx = Vy
        uint8_t reg1 = (instruction & 0x0F00) >> 8;
        uint8_t reg2 = (instruction & 0x00F0) >> 4;

        this->registers[reg1] = this->registers[reg2];
    } else if ((instruction & 0xF00F) == 0x8001) {
        // OR Vx, Vy - Set Vx = Vx | Vy
        uint8_t reg1 = (instruction & 0x0F00) >> 8;
        uint8_t reg2 = (instruction & 0x00F0) >> 4;

        this->registers[reg1] |= this->registers[reg2];
    } else if ((instruction & 0xF00F) == 0x8002) {
        // AND Vx, Vy - Set Vx = Vx & Vy
        uint8_t reg1 = (instruction & 0x0F00) >> 8;
        uint8_t reg2 = (instruction & 0x00F0) >> 4;

        this->registers[reg1] &= this->registers[reg2];
    } else if ((instruction & 0xF00F) == 0x8003) {
        // XOR Vx, Vy - Set Vx = Vx ^ Vy
        uint8_t reg1 = (instruction & 0x0F00) >> 8;
        uint8_t reg2 = (instruction & 0x00F0) >> 4;

        this->registers[reg1] ^= this->registers[reg2];
    } else if ((instruction & 0xF00F) == 0x8004) {
        // ADD Vx, Vy - Set Vx = Vx + Vy; Set Vf if carry
        uint8_t reg1 = (instruction & 0x0F00) >> 8;
        uint8_t reg2 = (instruction & 0x00F0) >> 4;

        uint16_t result = this->registers[reg1] + this->registers[reg2];

        this->registers[reg1] = result & 0xFF;

        if (result > 0xFF) {
            this->registers[15] = 1;
        } else {
            this->registers[15] = 0;
        }
    } else if ((instruction & 0xF00F) == 0x8005) {
        // SUB Vx, Vy - Set Vx = Vx - Vy; Set Vf if Vx > Vy
        uint8_t reg1 = (instruction & 0x0F00) >> 8;
        uint8_t reg2 = (instruction & 0x00F0) >> 4;

        this->registers[reg1] = this->registers[reg1] - this->registers[reg2];

        if (this->registers[reg1] > this->registers[reg2]) {
            this->registers[15] = 1;
        } else {
            this->registers[15] = 0;
        }
    } else if ((instruction & 0xF00F) == 0x8006) {
        // SHR Vx{, Vy} - Set Vx = Vx >> 1; Set Vf to least significant bit of Vx

        // Apparently most implementations ignore Vy in this instruction
        // The original set Vx = Vy before the shift
        uint8_t reg1 = (instruction & 0x0F00) >> 8;

        this->registers[reg1] = this->registers[reg1] >> 1;

        this->registers[15] = this->registers[reg1] & 0x01;
    } else if ((instruction & 0xF00F) == 0x8007) {
        // SUBN Vx, Vy - Set Vx = Vy - Vx; Set Vf if Vy > Vx
        uint8_t reg1 = (instruction & 0x0F00) >> 8;
        uint8_t reg2 = (instruction & 0x00F0) >> 4;

        this->registers[reg1] = this->registers[reg2] - this->registers[reg1];

        if (this->registers[reg2] > this->registers[reg1]) {
            this->registers[15] = 1;
        } else {
            this->registers[15] = 0;
        }
    } else if ((instruction & 0xF00F) == 0x800E) {
        // SHL Vx{, Vy} - Set Vx = Vx << 1; Set Vf to most significant bit of Vx

        // Apparently most implementations ignore Vy in this instruction
        // The original set Vx = Vy before the shift
        uint8_t reg1 = (instruction & 0x0F00) >> 8;

        this->registers[reg1] = this->registers[reg1] << 1;

        this->registers[15] = (this->registers[reg1] & 0x80) >> 7;
    } else if ((instruction & 0xF00F) == 0x9000) {
        // SNE Vx, Vy - Skip next instruction if Vx != Vy
        uint8_t reg1 = (instruction & 0x0F00) >> 8;
        uint8_t reg2 = (instruction & 0x00F0) >> 4;

        if (this->registers[reg1] != this->registers[reg2]) {
            this->pc += 2;
        }
    } else if ((instruction & 0xF000) == 0xA000) {
        // LD I, nnn - Load immediate to I
        uint16_t value = instruction & 0x0FFF;

        this->i = value;
    } else if ((instruction & 0xF000) == 0xB000) {
        // JP V0, nnn - Jump to address (nnn + V0)
        uint16_t value = instruction & 0x0FFF;

        this->pc = value + this->registers[0];
    } else if ((instruction & 0xF000) == 0xC000) {
        // RND Vx, nn - Set Vx to a random byte ANDed with nn
        uint8_t reg = (instruction & 0x0F00) >> 8;
        uint16_t mask = instruction & 0x00FF;

        // Randomness slightly biased towards lower numbers due to the modulo
        // This is acceptable
        this->pc = (std::rand() % 256) & mask;
    } else if ((instruction & 0xF000) == 0xD000) {
        // DRW Vx, Vy, n - Draw n bytes of sprite at I to x, y
        uint8_t x_reg = (instruction & 0x0F00) >> 8;
        uint8_t y_reg = (instruction & 0x00F0) >> 4;
        uint8_t n = instruction & 0x000F;

        uint8_t x = this->registers[x_reg];
        uint8_t y = this->registers[y_reg];

        for (int i = 0; i < n; ++i) {
            uint8_t data = this->memory[this->i + i];
            this->display->draw_byte(x, (y + i) % Display::HEIGHT, data);
        }
    } else if ((instruction & 0xF0FF) == 0xF01E) {
        // ADD I, Vx - Add Vx to I
        uint8_t reg = (instruction & 0x0F00) >> 8;

        this->i += this->registers[reg];
    } else if ((instruction & 0xF0FF) == 0xF029) {
        // LD F, Vx - Set I to the address of font character Vx
        //
        // Documentation does not say what happens if Vx > 15 - here we just consider the lower 4 bits

        uint8_t reg = (instruction & 0x0F00) >> 8;
        uint8_t character = this->registers[reg] & 0x0F;

        this->i = CPU::FONT_OFFSET + 5 * character;
    } else if ((instruction & 0xF0FF) == 0xF033) {
        // LD B, Vx - Set memory locations at I, I+1, and I+2 to the BCD representation of Vx

        uint8_t reg = (instruction & 0x0F00) >> 8;

        this->memory[this->i] = (this->registers[reg] / 100) % 10;
        this->memory[this->i + 1] = (this->registers[reg] / 10) % 10;
        this->memory[this->i + 2] = this->registers[reg] % 10;
    } else if ((instruction & 0xF0FF) == 0xF055) {
        // LD [I], Vx - Store registers V0 through Vx to memory starting at address I

        uint8_t max_reg = (instruction & 0x0F00) >> 8;

        for (int i = 0; i <= max_reg; ++i) {
            this->memory[this->i + i] = this->registers[i];
        }
    } else if ((instruction & 0xF0FF) == 0xF065) {
        // LD Vx, [I] - Load registers V0 through Vx from memory starting at address I

        uint8_t max_reg = (instruction & 0x0F00) >> 8;

        for (int i = 0; i <= max_reg; ++i) {
            this->registers[i] = this->memory[this->i + i];
        }
    }

    this->pc += 2;
}
