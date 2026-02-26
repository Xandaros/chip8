#include <catch2/catch_test_macros.hpp>

#include "display.h"

TEST_CASE("Draw byte", "[display]") {
    Display display = Display();

    display.clear();
    SECTION("Simple draw") {
        bool flag = display.draw_byte(0, 0, 0xFF);

        auto vram = display.get_vram();
        CHECK(vram[0] == 1);
        CHECK(vram[1] == 1);
        CHECK(vram[2] == 1);
        CHECK(vram[3] == 1);
        CHECK(vram[4] == 1);
        CHECK(vram[5] == 1);
        CHECK(vram[6] == 1);
        CHECK(vram[7] == 1);
        REQUIRE(vram[8] == 0);

        REQUIRE(flag == false);
    }

    SECTION("Simple draw over existing data") {
        display.draw_byte(0, 0, 0xFF);
        bool flag = display.draw_byte(0, 0, 0xFF);

        auto vram = display.get_vram();
        CHECK(vram[0] == 0);
        CHECK(vram[1] == 0);
        CHECK(vram[2] == 0);
        CHECK(vram[3] == 0);
        CHECK(vram[4] == 0);
        CHECK(vram[5] == 0);
        CHECK(vram[6] == 0);
        CHECK(vram[7] == 0);
        REQUIRE(vram[8] == 0);

        REQUIRE(flag == true);
    }

    SECTION("Simple draw at non-zero position") {
        display.draw_byte(5, 5, 0xFF);

        auto vram = display.get_vram();
        CHECK(vram[5 * Display::WIDTH + 5] == 1);
        CHECK(vram[5 * Display::WIDTH + 6] == 1);
        CHECK(vram[5 * Display::WIDTH + 7] == 1);
        CHECK(vram[5 * Display::WIDTH + 8] == 1);
        CHECK(vram[5 * Display::WIDTH + 9] == 1);
        CHECK(vram[5 * Display::WIDTH + 10] == 1);
        CHECK(vram[5 * Display::WIDTH + 11] == 1);
        CHECK(vram[5 * Display::WIDTH + 12] == 1);
        REQUIRE(vram[5 * Display::WIDTH + 13] == 0);
    }

    SECTION("Pattern draw") {
        display.draw_byte(0, 0, 0b01101001);

        auto vram = display.get_vram();
        CHECK(vram[0] == 0);
        CHECK(vram[1] == 1);
        CHECK(vram[2] == 1);
        CHECK(vram[3] == 0);
        CHECK(vram[4] == 1);
        CHECK(vram[5] == 0);
        CHECK(vram[6] == 0);
        CHECK(vram[7] == 1);
        REQUIRE(vram[8] == 0);
    }

    SECTION("Draw over edge") {
        display.draw_byte(Display::WIDTH - 3, 0, 0xFF);

        auto vram = display.get_vram();
        CHECK(vram[Display::WIDTH - 3] == 1);
        CHECK(vram[Display::WIDTH - 2] == 1);
        CHECK(vram[Display::WIDTH - 1] == 1);
        CHECK(vram[0] == 1);
        CHECK(vram[1] == 1);
        CHECK(vram[2] == 1);
        CHECK(vram[3] == 1);
        CHECK(vram[4] == 1);
    }
}
