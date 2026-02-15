#include <catch2/catch_test_macros.hpp>

#include "cpu.h"

TEST_CASE("push/pop", "[cpu][memory]") {
    CPU *cpu = new CPU();

    cpu->push(5);
    cpu->push(7);
    cpu->push(3);
    cpu->push(0);
    cpu->push(255);

    REQUIRE(cpu->pop() == 255);
    REQUIRE(cpu->pop() == 0);
    REQUIRE(cpu->pop() == 3);
    REQUIRE(cpu->pop() == 7);
    REQUIRE(cpu->pop() == 5);
}
