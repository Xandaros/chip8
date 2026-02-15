#include <catch2/catch_test_macros.hpp>

#include "cpu.h"

void step_cpu(CPU *cpu, int num) {
    for (int i = 0; i < num; ++i) {
        cpu->step();
    }
}

TEST_CASE("push/pop", "[cpu][memory]") {
    CPU cpu = CPU();

    cpu.push(5);
    cpu.push(7);
    cpu.push(3);
    cpu.push(0);
    cpu.push(255);

    CHECK(cpu.pop() == 255);
    CHECK(cpu.pop() == 0);
    CHECK(cpu.pop() == 3);
    CHECK(cpu.pop() == 7);
    REQUIRE(cpu.pop() == 5);
}

TEST_CASE("Load register", "[cpu]") {
    CPU cpu = CPU();

    uint8_t code[] = {
        0x60, 0xDE, // LD V0, 0xDE
        0x61, 0xAD, // LD V1, 0xAD
        0x62, 0xBE, // LD V2, 0xBE
        0x63, 0xEF, // LD V3, 0xEF
        0x6F, 0x79, // LD VF, 0x79
    };

    cpu.load_code(code, sizeof(code));
    step_cpu(&cpu, 5);

    CHECK(cpu.registers[0] == 0xDE);
    CHECK(cpu.registers[1] == 0xAD);
    CHECK(cpu.registers[2] == 0xBE);
    CHECK(cpu.registers[3] == 0xEF);
    REQUIRE(cpu.registers[15] == 0x79);
}

TEST_CASE("Call and return", "[cpu]") {
    CPU cpu = CPU();

    uint8_t code[] = {
        0x60, 0x34, // LD V0, 0x34
        0x22, 0x08, // CALL 0x208
        0x60, 0x35, // LD V0, 0x35
        0x60, 0x37, // LD V0, 0x37
        0x60, 0x36, // LD V0, 0x36
        0x00, 0xEE, // RET
    };

    cpu.load_code(code, sizeof(code));

    cpu.step();
    REQUIRE(cpu.registers[0] == 0x34);
    cpu.step();
    REQUIRE(cpu.pc == 0x0208);
    cpu.step();
    REQUIRE(cpu.registers[0] == 0x36);
    cpu.step();
    REQUIRE(cpu.pc == 0x0204);
    cpu.step();
    REQUIRE(cpu.registers[0] == 0x35);
}

TEST_CASE("Sprite drawing", "[cpu][display]") {
    CPU cpu = CPU();

    uint8_t code[] = {
        0xA0, 0x00, // LD I, 0
        0x60, 0x00, // LD V0, 0
        0xD0, 0x05, // DRW V0, V0, 5
    };

    cpu.load_code(code, sizeof(code));
    step_cpu(&cpu, 3);

    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 8; ++x) {
            uint8_t byte = CPU::FONT[y];
            int idx = y * Display::WIDTH + x;

            INFO("x: " << x);
            INFO("y: " << y);
            INFO("idx: " << idx);
            CHECK(cpu.display->vram[idx] == ((byte >> (7 - x)) & 0x01));
        }
    }
}
