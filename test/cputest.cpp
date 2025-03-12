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

void performTest(GameBoy *gb, SingleTest test) {
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

    gb->cpu.setState(startingState);

    for (const auto &memVal : test.initial.ram) {
        gb->mmu.memoryWrite(std::get<0>(memVal), std::get<1>(memVal));
    }

    gb->cpu.step();

    CPUState actualState = gb->cpu.getState();
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
        const auto value = gb->mmu.memoryRead(std::get<0>(memVal));

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

void testOpcode(GameBoy *gb, const std::string &filename) {
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
        performTest(gb, test);
    }
}

TEST_CASE_METHOD(GameBoy, "CPU 8-bit arithmetic and logic instructions") {
    initForTests();

    SECTION("ADC: Add with Carry") {
        constexpr std::string tests[] {
            "88", "89", "8A", "8B", "8C", "8D", "8E", "8F", "CE"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("ADD: Add") {
        constexpr std::string tests[] {
            "80", "81", "82", "83", "84", "85", "86", "87", "C6"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("AND: Logical AND") {
        constexpr std::string tests[] {
            "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "E6"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("CP: Compare") {
        constexpr std::string tests[] {
            "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF", "FE"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("DEC: Decrement") {
        constexpr std::string tests[] {
            "05", "0D", "15", "1D", "25", "2D", "35", "3D"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("INC: Increment") {
        constexpr std::string tests[] {
            "04", "0C", "14", "1C", "24", "2C", "34", "3C"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("OR: Logical OR") {
        constexpr std::string tests[] {
            "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "F6"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("SBC: Subtract with Carry") {
        constexpr std::string tests[] {
            "98", "99", "9A", "9B", "9C", "9D", "9E", "9F", "DE"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("SUB: Subtract") {
        constexpr std::string tests[] {
            "90", "91", "92", "93", "94", "95", "96", "97", "D6"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("XOR: Exclusive OR") {
        constexpr std::string tests[] {
            "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF", "EE"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }
}

TEST_CASE_METHOD(GameBoy, "CPU 16-bit arithmetic instructions") {
    initForTests();

    SECTION("ADD: Add") {
        constexpr std::string tests[] {
            "09", "19", "29"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("DEC: Decrement") {
        constexpr std::string tests[] {
            "0B", "1B", "2B", "3B"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("INC: Increment") {
        constexpr std::string tests[] {
            "03", "13", "23", "33"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }
}

TEST_CASE_METHOD(GameBoy, "CPU bit operation instructions") {
    initForTests();

    SECTION("BIT: Check bit") {
        constexpr std::string tests[] {
            "CB 40", "CB 41", "CB 42", "CB 43", "CB 44", "CB 45", "CB 46", "CB 47",
            "CB 48", "CB 49", "CB 4A", "CB 4B", "CB 4C", "CB 4D", "CB 4E", "CB 4F",
            "CB 50", "CB 51", "CB 52", "CB 53", "CB 54", "CB 55", "CB 56", "CB 57",
            "CB 58", "CB 59", "CB 5A", "CB 5B", "CB 5C", "CB 5D", "CB 5E", "CB 5F",
            "CB 60", "CB 61", "CB 62", "CB 63", "CB 64", "CB 65", "CB 66", "CB 67",
            "CB 68", "CB 69", "CB 6A", "CB 6B", "CB 6C", "CB 6D", "CB 6E", "CB 6F",
            "CB 70", "CB 71", "CB 72", "CB 73", "CB 74", "CB 75", "CB 76", "CB 77",
            "CB 78", "CB 79", "CB 7A", "CB 7B", "CB 7C", "CB 7D", "CB 7E", "CB 7F",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("RES: Reset bit") {
        constexpr std::string tests[] {
            "CB 80", "CB 81", "CB 82", "CB 83", "CB 84", "CB 85", "CB 86", "CB 87",
            "CB 88", "CB 89", "CB 8A", "CB 8B", "CB 8C", "CB 8D", "CB 8E", "CB 8F",
            "CB 90", "CB 91", "CB 92", "CB 93", "CB 94", "CB 95", "CB 96", "CB 97",
            "CB 98", "CB 99", "CB 9A", "CB 9B", "CB 9C", "CB 9D", "CB 9E", "CB 9F",
            "CB A0", "CB A1", "CB A2", "CB A3", "CB A4", "CB A5", "CB A6", "CB A7",
            "CB A8", "CB A9", "CB AA", "CB AB", "CB AC", "CB AD", "CB AE", "CB AF",
            "CB B0", "CB B1", "CB B2", "CB B3", "CB B4", "CB B5", "CB B6", "CB B7",
            "CB B8", "CB B9", "CB BA", "CB BB", "CB BC", "CB BD", "CB BE", "CB BF",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("SET: Set bit") {
        constexpr std::string tests[] {
            "CB C0", "CB C1", "CB C2", "CB C3", "CB C4", "CB C5", "CB C6", "CB C7",
            "CB C8", "CB C9", "CB CA", "CB CB", "CB CC", "CB CD", "CB CE", "CB CF",
            "CB D0", "CB D1", "CB D2", "CB D3", "CB D4", "CB D5", "CB D6", "CB D7",
            "CB D8", "CB D9", "CB DA", "CB DB", "CB DC", "CB DD", "CB DE", "CB DF",
            "CB E0", "CB E1", "CB E2", "CB E3", "CB E4", "CB E5", "CB E6", "CB E7",
            "CB E8", "CB E9", "CB EA", "CB EB", "CB EC", "CB ED", "CB EE", "CB EF",
            "CB F0", "CB F1", "CB F2", "CB F3", "CB F4", "CB F5", "CB F6", "CB F7",
            "CB F8", "CB F9", "CB FA", "CB FB", "CB FC", "CB FD", "CB FE", "CB FF",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("SWAP: Swap nibbles") {
        constexpr std::string tests[] {
            "CB 30", "CB 31", "CB 32", "CB 33", "CB 34", "CB 35", "CB 36", "CB 37",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }
}

TEST_CASE_METHOD(GameBoy, "CPU bit shift instructions") {
    initForTests();

    SECTION("RL: Rotate left") {
        constexpr std::string tests[] {
            "CB 10", "CB 11", "CB 12", "CB 13", "CB 14", "CB 15", "CB 16", "CB 17",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("RLA: Rotate left accumulator") {
        testOpcode(this, "17");
    }

    SECTION("RLC: Rotate left with carry") {
        constexpr std::string tests[] {
            "CB 00", "CB 01", "CB 02", "CB 03", "CB 04", "CB 05", "CB 06", "CB 07",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("RLCA: Rotate left accumulator with carry") {
        testOpcode(this, "07");
    }

    SECTION("RR: Rotate right") {
        constexpr std::string tests[] {
            "CB 18", "CB 19", "CB 1A", "CB 1B", "CB 1C", "CB 1D", "CB 1E", "CB 1F",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("RRA: Rotate right accumulator") {
        testOpcode(this, "1F");
    }

    SECTION("RRC: Rotate right with carry") {
        constexpr std::string tests[] {
            "CB 08", "CB 09", "CB 0A", "CB 0B", "CB 0C", "CB 0D", "CB 0E", "CB 0F",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("RRCA: Rotate right accumulator with carry") {
        testOpcode(this, "0F");
    }

    SECTION("SLA: Shift left arithmetically") {
        constexpr std::string tests[] {
            "CB 20", "CB 21", "CB 22", "CB 23", "CB 24", "CB 25", "CB 26", "CB 27",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("SRA: Shift right arithmetically") {
        constexpr std::string tests[] {
            "CB 28", "CB 29", "CB 2A", "CB 2B", "CB 2C", "CB 2D", "CB 2E", "CB 2F",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("SRL: Shift right logically") {
        constexpr std::string tests[] {
            "CB 38", "CB 39", "CB 3A", "CB 3B", "CB 3C", "CB 3D", "CB 3E", "CB 3F",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }
}

TEST_CASE_METHOD(GameBoy, "CPU load instructions") {
    initForTests();

    SECTION("LD r8: Load into 8-bit register") {
        constexpr std::string tests[] {
            "06", "0E", "16", "1E", "26", "2E", "3E",
            "40", "41", "42", "43", "44", "45", "46", "47",
            "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
            "50", "51", "52", "53", "54", "55", "56", "57",
            "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
            "60", "61", "62", "63", "64", "65", "66", "67",
            "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
            "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("LD r16: Load into 16-bit register") {
        constexpr std::string tests[] {
            "01", "11", "21", "31"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("LD HL: Load into memory at address HL") {
        constexpr std::string tests[] {
            "36", "70", "71", "72", "73", "74", "75", "77"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("LD n16 A: Load A into memory at address n16") {
        constexpr std::string tests[] {
            "02", "12", "EA"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("LD A n16: Load memory at address n16 at A") {
        constexpr std::string tests[] {
            "0A", "1A", "FA"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("LDH u8 A: Load A into memory at address $FF00 + u8") {
        constexpr std::string tests[] {
            "E0", "E2"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("LDH A u8: Load memory at address $FF00 + u8 into A") {
        constexpr std::string tests[] {
            "F0", "F2"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("LD HLI A: Load A into memory at address HL and increment HL") {
        testOpcode(this, "22");
    }

    SECTION("LD HLD A: Load A into memory at address HL and decrement HL") {
        testOpcode(this, "32");
    }

    SECTION("LD A HLI: Load memory at address HL into A and increment HL") {
        testOpcode(this, "2A");
    }

    SECTION("LD A HLD: Load memory at address HL into A and decrement HL") {
        testOpcode(this, "3A");
    }
}

TEST_CASE_METHOD(GameBoy, "CPU jump and subroutine instructions") {
    initForTests();

    SECTION("CALL: Call") {
        constexpr std::string tests[] {
            "C4", "CC", "CD", "D4", "DC"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("JP: Jump") {
        constexpr std::string tests[] {
            "C2", "C3", "CA", "D2", "DA", "E9"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("JR: Jump relative") {
        constexpr std::string tests[] {
            "18", "20", "28", "30", "38"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("RET: Return") {
        constexpr std::string tests[] {
            "C0", "C8", "C9", "D0", "D8"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("RETI: Return and enable interrupts") {
        testOpcode(this, "D9");
    }

    SECTION("RST: Call vector") {
        constexpr std::string tests[] {
            "C7", "CF", "D7", "DF", "E7", "EF", "F7", "FF"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }
}

TEST_CASE_METHOD(GameBoy, "CPU stack operation instructions") {
    initForTests();

    SECTION("ADD: Add") {
        constexpr std::string tests[] {
            "39", "E8"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("DEC: Decrement") {
        testOpcode(this, "3B");
    }

    SECTION("INC: Increment") {
        testOpcode(this, "33");
    }

    SECTION("LD: Load") {
        constexpr std::string tests[] {
            "08", "31", "F8", "F9"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("POP: Pop from stack") {
        constexpr std::string tests[] {
            "C1", "D1", "E1", "F1"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }

    SECTION("PUSH: Push to stack") {
        constexpr std::string tests[] {
            "C5", "D5", "E5", "F5"
        };

        for (const std::string &test : tests) {
            testOpcode(this, test);
        }
    }
}

TEST_CASE_METHOD(GameBoy, "CPU miscellaneous instructions") {
    initForTests();

    SECTION("CCF: Complement carry flag") {
        testOpcode(this, "3F");
    }

    SECTION("CPL: Complement accumulator") {
        testOpcode(this, "2F");
    }

    SECTION("DAA: Decimal adjust accumulator") {
        testOpcode(this, "27");
    }

    SECTION("DI: Disable interrupts") {
        testOpcode(this, "F3");
    }

    SECTION("EI: Enable interrupts") {
        testOpcode(this, "FB");
    }

    SECTION("HALT: Halt") {
        testOpcode(this, "76");
    }

    SECTION("NOP: No operation") {
        testOpcode(this, "00");
    }

    SECTION("SCF: Set carry flag") {
        testOpcode(this, "37");
    }

    SECTION("STOP: Stop") {
        testOpcode(this, "10");
    }
}