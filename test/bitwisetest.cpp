#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include <common/bitwise.h>

TEST_CASE("Half carry for addition is properly calculated") {
    SECTION("8-bit addition") {
        uint8_t a, b;

        a = 0xF, b = 0xF;
        REQUIRE(true == Bitwise::checkHalfCarryAdd(a, b));
        a = 0x1, b = 0x1;
        REQUIRE(false == Bitwise::checkHalfCarryAdd(a, b));
        a = 0x1, b = 0xF;
        REQUIRE(true == Bitwise::checkHalfCarryAdd(a, b));
        a = 0xF, b = 0x1;
        REQUIRE(true == Bitwise::checkHalfCarryAdd(a, b));
    }

    SECTION("16-bit addition") {
        uint16_t a, b;

        a = 0xFFF, b = 0xFFF;
        REQUIRE(true == Bitwise::checkHalfCarryAdd(a, b));
        a = 0x100, b = 0x100;
        REQUIRE(false == Bitwise::checkHalfCarryAdd(a, b));
        a = 0x100, b = 0xF00;
        REQUIRE(true == Bitwise::checkHalfCarryAdd(a, b));
        a = 0xF00, b = 0x100;
        REQUIRE(true == Bitwise::checkHalfCarryAdd(a, b));
    }
}

TEST_CASE("Half carry for subtraction is properly calculated") {
    SECTION("8-bit subtraction") {
        uint8_t a, b;

        a = 0xF, b = 0xF;
        REQUIRE(false == Bitwise::checkHalfCarrySub(a, b));
        a = 0x1, b = 0x1;
        REQUIRE(false == Bitwise::checkHalfCarrySub(a, b));
        a = 0x1, b = 0xF;
        REQUIRE(true == Bitwise::checkHalfCarrySub(a, b));
        a = 0xF, b = 0x1;
        REQUIRE(false == Bitwise::checkHalfCarrySub(a, b));
    }

    SECTION("16-bit subtraction") {
        uint16_t a, b;

        a = 0xFFF, b = 0xFFF;
        REQUIRE(false == Bitwise::checkHalfCarrySub(a, b));
        a = 0x100, b = 0x100;
        REQUIRE(false == Bitwise::checkHalfCarrySub(a, b));
        a = 0x100, b = 0xF00;
        REQUIRE(true == Bitwise::checkHalfCarrySub(a, b));
        a = 0xF00, b = 0x100;
        REQUIRE(false == Bitwise::checkHalfCarrySub(a, b));
    }
}

TEST_CASE("Individual bit is returned") {
    REQUIRE(0b1 == Bitwise::getBitInByte(0b1011'1001, 7));
    REQUIRE(0b0 == Bitwise::getBitInByte(0b1011'1001, 6));
    REQUIRE(0b1 == Bitwise::getBitInByte(0b1011'1001, 5));
    REQUIRE(0b1 == Bitwise::getBitInByte(0b1011'1001, 4));
    REQUIRE(0b1 == Bitwise::getBitInByte(0b1011'1001, 3));
    REQUIRE(0b0 == Bitwise::getBitInByte(0b1011'1001, 2));
    REQUIRE(0b0 == Bitwise::getBitInByte(0b1011'1001, 1));
    REQUIRE(0b1 == Bitwise::getBitInByte(0b1011'1001, 0));
}

TEST_CASE("Individual bit is modified") {
    REQUIRE(0b1010'1011 == Bitwise::modifyBitInByte(0b1010'1011, 7, 1));
    REQUIRE(0b0010'1011 == Bitwise::modifyBitInByte(0b1010'1011, 7, 0));
    REQUIRE(0b1010'1111 == Bitwise::modifyBitInByte(0b1010'1011, 2, 1));
    REQUIRE(0b1010'1011 == Bitwise::modifyBitInByte(0b1010'1011, 4, 0));
}

TEST_CASE("First nibble is correctly returned") {
    REQUIRE(0xF == Bitwise::getFirstNibble(0xF0));
    REQUIRE(0xF == Bitwise::getFirstNibble(0xFF));
    REQUIRE(0x1 == Bitwise::getFirstNibble(0x12));
    REQUIRE(0x2 == Bitwise::getFirstNibble(0x21));
    REQUIRE(0x0 == Bitwise::getFirstNibble(0x00));
}

TEST_CASE("Second nibble is correctly returned") {
    REQUIRE(0x0 == Bitwise::getSecondNibble(0xF0));
    REQUIRE(0xF == Bitwise::getSecondNibble(0xFF));
    REQUIRE(0x2 == Bitwise::getSecondNibble(0x12));
    REQUIRE(0x1 == Bitwise::getSecondNibble(0x21));
    REQUIRE(0x0 == Bitwise::getSecondNibble(0x00));
}

TEST_CASE("Nibbles are swapped correctly") {
    REQUIRE(0xF3 == Bitwise::swapNibbles(0x3F));
    REQUIRE(0x3F == Bitwise::swapNibbles(0xF3));
    REQUIRE(0xFF == Bitwise::swapNibbles(0xFF));
    REQUIRE(0x12 == Bitwise::swapNibbles(0x21));
}

TEST_CASE("Bytes are concatenated correctly") {
    REQUIRE(0xFF0F == Bitwise::concatBytes(0x0F, 0xFF));
    REQUIRE(0x32EC == Bitwise::concatBytes(0xEC, 0x32));
    REQUIRE(0x9FA1 == Bitwise::concatBytes(0xA1, 0x9F));
    REQUIRE(0x0001 == Bitwise::concatBytes(0x01, 0x00));
}