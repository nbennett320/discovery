#include "catch.hpp"

#include "../common.h"
#include "../util.h"
#include "../arm_7tdmi.h"

TEST_CASE("branch_exchange") {
    arm_7tdmi arm;

    // will use condition C set, V clear for the instruction condition
    arm.set_condition_code_flag(C, 1);

    // BXHI 9 (BEX cond=HI Rn=9)
    arm_instruction i1 = 0b10000001001011111111111100011001;

    arm.registers.r9 = 0xbeefbeef;

    arm.execute(i1);

    REQUIRE(arm.registers.r15 == 0xbeefbeef); // contents of r9 moved to r15
    REQUIRE(arm.get_mode() == THUMB); // because Rn[0] is odd

    arm.registers.r6 = 0xabcde;

    // BXHI 6 (BEX cond=HI Rn=6)
    arm_instruction i2 = 0b10000001001011111111111100010110;
    arm.execute(i2);
    REQUIRE(arm.registers.r15 == 0xabcde); // contents of r6 moved to r15
    REQUIRE(arm.get_mode() == ARM); // because Rn[0] is even

    // unrecognized condition (Rn = 15)
    arm_instruction i3 = 0b10000001001011111111111100011111;
    arm.execute(i3);
    REQUIRE(arm.registers.r15 == 0xabcde); // contents of r15 unchanged
    REQUIRE(arm.get_state() == UND); // because Rn was 15
    REQUIRE(arm.get_mode() == ARM); // mode unchanged
}

TEST_CASE("multiply") {
    arm_7tdmi arm;

    arm.set_condition_code_flag(Z, 1);

    // 0b0000000000asRdxxRnxxRsxx1001Rmxx
    // r3 = r1 * r2 => rd = rm * rs
    arm_instruction i1 = 0b00000000000000110000001010010111;
    arm.execute(i1);
    REQUIRE(arm.registers.r3 == 14);

    // r4 = r1 * r2 + 2
    arm_instruction i2 = 0b00000000001001000010001010010111;
    arm.execute(i2);
    REQUIRE(arm.registers.r4 == 16);
}

TEST_CASE("branch_link") {
    arm_7tdmi arm1;

    // 1110 101 0 000000000000000000000000 
    arm_instruction i1 = 0b11101010000000000000000000000101;

    // branch offset 20
    arm1.execute(i1);
    REQUIRE(arm1.registers.r15 == 20);

    arm_7tdmi arm2;
    arm2.registers.r15 = 100;
    arm_instruction i2 = 0b11101010111111111111111111110110; // offset should expand to -40 in 2s compliment

    // branch offset -40
    arm2.execute(i2);
    REQUIRE(arm2.registers.r15 == 60);
}

