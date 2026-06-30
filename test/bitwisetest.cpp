#include <catch2/catch_test_macros.hpp>

#include "common/bits.h"
#include "common/types.h"

TEST_CASE("Half carry for addition is properly calculated") {
    SECTION("8-bit addition") {
        u8 a {}, b {};

        a = 0xF, b = 0xF;
        REQUIRE(true == bits::checkHalfCarryAdd(a, b));
        a = 0x1, b = 0x1;
        REQUIRE(false == bits::checkHalfCarryAdd(a, b));
        a = 0x1, b = 0xF;
        REQUIRE(true == bits::checkHalfCarryAdd(a, b));
        a = 0xF, b = 0x1;
        REQUIRE(true == bits::checkHalfCarryAdd(a, b));
    }

    SECTION("16-bit addition") {
        u16 a {}, b {};

        a = 0xFFF, b = 0xFFF;
        REQUIRE(true == bits::checkHalfCarryAdd(a, b));
        a = 0x100, b = 0x100;
        REQUIRE(false == bits::checkHalfCarryAdd(a, b));
        a = 0x100, b = 0xF00;
        REQUIRE(true == bits::checkHalfCarryAdd(a, b));
        a = 0xF00, b = 0x100;
        REQUIRE(true == bits::checkHalfCarryAdd(a, b));
    }
}

TEST_CASE("Half carry for subtraction is properly calculated") {
    SECTION("8-bit subtraction") {
        u8 a {}, b {};

        a = 0xF, b = 0xF;
        REQUIRE(false == bits::checkHalfCarrySub(a, b));
        a = 0x1, b = 0x1;
        REQUIRE(false == bits::checkHalfCarrySub(a, b));
        a = 0x1, b = 0xF;
        REQUIRE(true == bits::checkHalfCarrySub(a, b));
        a = 0xF, b = 0x1;
        REQUIRE(false == bits::checkHalfCarrySub(a, b));
    }

    SECTION("16-bit subtraction") {
        u16 a {}, b {};

        a = 0xFFF, b = 0xFFF;
        REQUIRE(false == bits::checkHalfCarrySub(a, b));
        a = 0x100, b = 0x100;
        REQUIRE(false == bits::checkHalfCarrySub(a, b));
        a = 0x100, b = 0xF00;
        REQUIRE(true == bits::checkHalfCarrySub(a, b));
        a = 0xF00, b = 0x100;
        REQUIRE(false == bits::checkHalfCarrySub(a, b));
    }
}

TEST_CASE("Individual bit is returned") {
    REQUIRE(0b1 == bits::getBitInByte(0b1011'1001, 7));
    REQUIRE(0b0 == bits::getBitInByte(0b1011'1001, 6));
    REQUIRE(0b1 == bits::getBitInByte(0b1011'1001, 5));
    REQUIRE(0b1 == bits::getBitInByte(0b1011'1001, 4));
    REQUIRE(0b1 == bits::getBitInByte(0b1011'1001, 3));
    REQUIRE(0b0 == bits::getBitInByte(0b1011'1001, 2));
    REQUIRE(0b0 == bits::getBitInByte(0b1011'1001, 1));
    REQUIRE(0b1 == bits::getBitInByte(0b1011'1001, 0));
}

TEST_CASE("Individual bit is modified") {
    REQUIRE(0b1010'1011 == bits::modifyBitInByte(0b1010'1011, 7, 1));
    REQUIRE(0b0010'1011 == bits::modifyBitInByte(0b1010'1011, 7, 0));
    REQUIRE(0b1010'1111 == bits::modifyBitInByte(0b1010'1011, 2, 1));
    REQUIRE(0b1010'1011 == bits::modifyBitInByte(0b1010'1011, 4, 0));
}

TEST_CASE("First nibble is correctly returned") {
    REQUIRE(0xF == bits::getFirstNibble(0xF0));
    REQUIRE(0xF == bits::getFirstNibble(0xFF));
    REQUIRE(0x1 == bits::getFirstNibble(0x12));
    REQUIRE(0x2 == bits::getFirstNibble(0x21));
    REQUIRE(0x0 == bits::getFirstNibble(0x00));
}

TEST_CASE("Second nibble is correctly returned") {
    REQUIRE(0x0 == bits::getSecondNibble(0xF0));
    REQUIRE(0xF == bits::getSecondNibble(0xFF));
    REQUIRE(0x2 == bits::getSecondNibble(0x12));
    REQUIRE(0x1 == bits::getSecondNibble(0x21));
    REQUIRE(0x0 == bits::getSecondNibble(0x00));
}

TEST_CASE("Nibbles are swapped correctly") {
    REQUIRE(0xF3 == bits::swapNibbles(0x3F));
    REQUIRE(0x3F == bits::swapNibbles(0xF3));
    REQUIRE(0xFF == bits::swapNibbles(0xFF));
    REQUIRE(0x12 == bits::swapNibbles(0x21));
}

TEST_CASE("Bytes are concatenated correctly") {
    REQUIRE(0xFF0F == bits::concatBytes(0x0F, 0xFF));
    REQUIRE(0x32EC == bits::concatBytes(0xEC, 0x32));
    REQUIRE(0x9FA1 == bits::concatBytes(0xA1, 0x9F));
    REQUIRE(0x0001 == bits::concatBytes(0x01, 0x00));
}
