#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>

#include <core/gameboy.h>
#include <core/cpu.h>

namespace fs = std::filesystem;

typedef std::tuple<int, int, std::string> Cycle;

struct SM83State {
    uint8_t a, b, c, d, e, f, h, l {};
    uint16_t pc {};
    uint16_t sp {};
    uint8_t ime {};
    uint8_t ie {};
    std::vector<std::tuple<uint16_t, uint8_t>> ram {};
};

struct SingleTest {
    std::string name {};
    SM83State initial {};
    SM83State final {};
    std::vector<Cycle> cycles {};
};

void performTest(SingleTest test) {
    CPUState startingState {
        .a = test.initial.a,
        .b = test.initial.b,
        .c = test.initial.c,
        .d = test.initial.d,
        .e = test.initial.e,
        .f = test.initial.f,
        .h = test.initial.h,
        .l = test.initial.l,
        .sp = test.initial.sp,
        .pc = test.initial.pc,
    };

    GameBoy gb;
    gb.initForTests();
    gb.cpu.setState(startingState);

    for (const auto &memVal : test.initial.ram) {
        gb.mmu.memoryWrite(std::get<0>(memVal), std::get<1>(memVal));
    }

    gb.cpu.step();

    CPUState actualState = gb.cpu.getState();
    CPUState expectedState {
        .a = test.final.a,
        .b = test.final.b,
        .c = test.final.c,
        .d = test.final.d,
        .e = test.final.e,
        .f = test.final.f,
        .h = test.final.h,
        .l = test.final.l,
        .sp = test.final.sp,
        .pc = test.final.pc,
    };

    std::ostringstream actualMem {};
    std::ostringstream expectedMem {};

    bool ramMatch = true;
    for (const auto &memVal : test.final.ram) {
        const auto value = gb.mmu.memoryRead(std::get<0>(memVal));

        actualMem << "\tAddr: " << +std::get<0>(memVal) << " Val: " << +value << '\n';
        expectedMem << "\tAddr: " << +std::get<0>(memVal) << " Val: " << +std::get<1>(memVal) << '\n';

        if (value != std::get<1>(memVal)) {
            ramMatch = false;
            break;
        }
    }

    INFO("Test Name: $" + test.name);
    INFO("Expected CPU State:\n\t" + expectedState.toString() + "\nActual CPU State:\n\t" + actualState.toString());
    INFO("Expected Memory State:\n" + expectedMem.str() + "Actual Memory State:\n" + actualMem.str());

    REQUIRE(actualState == expectedState);
    REQUIRE(ramMatch);
}

void testOpcode(const std::string &filename) {
    fs::path filepath = "cputestdata" / fs::path(filename + ".json");

    std::ifstream file(filepath);
    if (!file) { SKIP("Failed to open test data file."); }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string jsonString(size, ' ');

    file.read(jsonString.data(), size);

    std::vector<SingleTest> testData;
    const auto error = glz::read_json<std::vector<SingleTest>>(testData, jsonString);
    if (error) SKIP("Failed to parse JSON test data.");

    for (const auto &test : testData) {
        performTest(test);
    }
}

TEST_CASE("CPU 8-bit arithmetic and logic instructions") {
    SECTION("ADC: Add with Carry") {
        constexpr std::string tests[] {
            "88", "89", "8A", "8B", "8C", "8D", "8E", "8F", "CE"
        };

        for (const std::string &test : tests) {
            testOpcode(test);
        }
    }

    SECTION("ADD: Add") {

    }

    SECTION("AND: Logical AND") {

    }

    SECTION("CP: Compare") {

    }

    SECTION("DEC: Decrement") {

    }

    SECTION("INC: Increment") {

    }

    SECTION("OR: Logical OR") {

    }

    SECTION("SBC: Subtract with Carry") {

    }

    SECTION("SBC: Subtract") {

    }

    SECTION("XOR: Exclusive OR") {

    }
}

TEST_CASE("CPU 16-bit arithmetic instructions") {

}

TEST_CASE("CPU bit operation instructions") {

}

TEST_CASE("CPU bit shift instructions") {

}

TEST_CASE("CPU load instructions") {

}

TEST_CASE("CPU jump and subroutine instructions") {

}

TEST_CASE("CPU stack operation instructions") {

}

TEST_CASE("CPU miscellaneous instructions") {

}