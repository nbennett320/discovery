#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <bitset>

#include "../arm_7tdmi.h"
#include "../common.h"
#include "../util.h"
#include "../../memory/memory.h"

TEST_CASE("ConditionFlagTests", "[condition_flag]") {
    arm_7tdmi arm;
    
    // REQUIRE all flags are 0 upon initialization
    REQUIRE(arm.get_condition_code_flag(N) == 0);
    REQUIRE(arm.get_condition_code_flag(Z) == 0);
    REQUIRE(arm.get_condition_code_flag(C) == 0);
    REQUIRE(arm.get_condition_code_flag(V) == 0);

    // set N, C, and V flags to 1
    arm.set_condition_code_flag(N, 1);
    arm.set_condition_code_flag(C, 1);
    arm.set_condition_code_flag(V, 1);

    // REQUIRE N, C, and V flags are now 1 and Z is still 0
    REQUIRE(arm.get_condition_code_flag(N) == 1);
    REQUIRE(arm.get_condition_code_flag(C) == 1);
    REQUIRE(arm.get_condition_code_flag(V) == 1);
    REQUIRE(arm.get_condition_code_flag(Z) == 0);

    // now set Z flag to 1 and REQUIRE it got changed
    arm.set_condition_code_flag(Z, 1);
    REQUIRE(arm.get_condition_code_flag(Z) == 1);

    // set N back to 0 and REQUIRE it got changed
    arm.set_condition_code_flag(N, 0);
    REQUIRE(arm.get_condition_code_flag(N) == 0);
    
    // try setting the code of a non recognized flag (should print some error message)
    arm.set_condition_code_flag((condition_code_flag_t) 9, 1);
    // try setting the code of a non recognized bit (should print some error message)
    arm.set_condition_code_flag(Z, 4);
}

TEST_CASE("InstructionConditionField", "[instruction_condition_field]") {
    arm_instruction z_set                       =      0b00000000000000000000000000000000; // 32 bits, Z Set
    arm_instruction z_clear                     =      0b00010000000000000000000000000000;
    arm_instruction c_set                       =      0b00100000000000000000000000000000;
    arm_instruction c_clear                     =      0b00110000000000000000000000000000;
    arm_instruction n_set                       =      0b01000000000000000000000000000000;
    arm_instruction n_clear                     =      0b01010000000000000000000000000000;
    arm_instruction v_set                       =      0b01100000000000000000000000000000;
    arm_instruction v_clear                     =      0b01110000000000000000000000000000;
    arm_instruction c_set_and_z_clear           =      0b10000000000000000000000000000000;
    arm_instruction c_clear_or_z_set            =      0b10010000000000000000000000000000;
    arm_instruction n_equal_v                   =      0b10100000000000000000000000000000;
    arm_instruction n_not_equal_v               =      0b10110000000000000000000000000000;
    arm_instruction z_clear_and_n_equal_v       =      0b11000000000000000000000000000000;
    arm_instruction z_set_or_n_not_equal_v      =      0b11010000000000000000000000000000;
    arm_instruction always                      =      0b11100000000000000000000000000000;
    arm_instruction noop                        =      0b11110000000000000000000000000000;

    // a couple of fresh cpu instances to test with
    arm_7tdmi z;
    arm_7tdmi c;
    arm_7tdmi n;
    arm_7tdmi v;

    // z 
    REQUIRE(util::condition_met(z_set, z) == false);
    REQUIRE(util::condition_met(z_clear, z) == true);
    z.set_condition_code_flag(Z, 1);
    REQUIRE(util::condition_met(z_set, z) == true);

    // c 
    REQUIRE(util::condition_met(c_set, c) == false);
    REQUIRE(util::condition_met(c_clear, c) == true);
    c.set_condition_code_flag(C, 1);
    REQUIRE(util::condition_met(c_set, c) == true);

    // n 
    REQUIRE(util::condition_met(n_set, n) == false);
    REQUIRE(util::condition_met(n_clear, n) == true);
    n.set_condition_code_flag(N, 1);
    REQUIRE(util::condition_met(n_set, n) == true);

    // v 
    REQUIRE(util::condition_met(v_set, v) == false);
    REQUIRE(util::condition_met(v_clear, v) == true);
    v.set_condition_code_flag(V, 1);
    REQUIRE(util::condition_met(v_set, v) == true);

    // c set and z clear
    REQUIRE(util::condition_met(c_set_and_z_clear, z) == false);
    z.set_condition_code_flag(Z, 0);
    z.set_condition_code_flag(C, 1);
    REQUIRE(util::condition_met(c_set_and_z_clear, z) == true);

    // c clear or z set
    REQUIRE(util::condition_met(c_clear_or_z_set, z) == false);
    z.set_condition_code_flag(Z, 1);
    REQUIRE(util::condition_met(c_clear_or_z_set, z) == true);
    z.set_condition_code_flag(Z, 0);
    z.set_condition_code_flag(C, 0);
    REQUIRE(util::condition_met(c_clear_or_z_set, z) == true);
    z.set_condition_code_flag(Z, 1);
    REQUIRE(util::condition_met(c_clear_or_z_set, z) == true);

    // n equals v
    REQUIRE(util::condition_met(n_equal_v, n) == false);
    REQUIRE(util::condition_met(n_not_equal_v, n) == true);
    n.set_condition_code_flag(V, 1);
    REQUIRE(util::condition_met(n_equal_v, n) == true);
    REQUIRE(util::condition_met(n_not_equal_v, n) == false);

    // z clear and (n equal v)
    REQUIRE(util::condition_met(z_clear_and_n_equal_v, n) == true);
    n.set_condition_code_flag(Z, 1);
    REQUIRE(util::condition_met(z_clear_and_n_equal_v, n) == false);

    // make a new cpu because this is getting confusing
    arm_7tdmi arm;

    // z set or (n not equal v)
    REQUIRE(util::condition_met(z_set_or_n_not_equal_v, arm) == false);
    arm.set_condition_code_flag(Z, 1);
    REQUIRE(util::condition_met(z_set_or_n_not_equal_v, arm) == true);
    arm.set_condition_code_flag(N, 1);
    REQUIRE(util::condition_met(z_set_or_n_not_equal_v, arm) == true);
    arm.set_condition_code_flag(Z, 0);
    REQUIRE(util::condition_met(z_set_or_n_not_equal_v, arm) == true);

    // always
    REQUIRE(util::condition_met(always, arm) == true);
    REQUIRE(util::condition_met(always, z) == true);
    REQUIRE(util::condition_met(always, c) == true);
    REQUIRE(util::condition_met(always, v) == true);
    REQUIRE(util::condition_met(always, n) == true);

    // never
    REQUIRE(util::condition_met(noop, arm) == false);
    REQUIRE(util::condition_met(noop, z) == false);
    REQUIRE(util::condition_met(noop, c) == false);
    REQUIRE(util::condition_met(noop, v) == false);
    REQUIRE(util::condition_met(noop, n) == false);
}

// #TODO - test cases for Instruction Set Format
// TEST_CASE("InstructionSetFormat", "[instruction_set_format]") {
//     arm_instruction i = 0;
    
//     // zero'd instruction is actually a valid data processing instruction
//     REQUIRE(util::get_instruction_format(i) == DP);

//     // see docs/arm_instruction_set_bitfield.png to see which bits should be set for which instruction formats
//     //  (one's digit of instruction) -                    10987654321098765432109876543210
//     arm_instruction dataProcessing               =      0b00000000000000000000000000000000;
//     arm_instruction multiply                     =      0b00000000000000000000000010010000;
//     arm_instruction multiplyLong                 =      0b00000000100000000000000010010000;
//     arm_instruction singleDataSwap               =      0b00000001000000000000000010010000;
//     arm_instruction branchExchange               =      0b00000001001011111111111100010000;
//     arm_instruction hdtRegisterOffset            =      0b00000000000000000000000010010000;
//     arm_instruction hdtImmediateOffset           =      0b00000000010000000000000010010000;
//     arm_instruction singleDataTransfer           =      0b00000100000000000000000000000000;
//     arm_instruction undefined                    =      0b00000110000000000000000000010000;
//     arm_instruction blockDataTransfer            =      0b00001000000000000000000000000000;
//     arm_instruction branch                       =      0b00001010000000000000000000000000;
//     arm_instruction coprocessorDataTransfer      =      0b00001100000000000000000000000000;
//     arm_instruction coprocessorDataOperation     =      0b00001110000000000000000000000000;
//     arm_instruction coprocessorRegisterTransfer  =      0b00001110000000000000000000010000;
//     arm_instruction softwareInterrupt            =      0b00001111000000000000000000000000;

//     // comparing these hypothetical instructions with their truth,
//     // in somewhat random order to make sure order is irrelvant for this function
//     REQUIRE(util::get_instruction_format(hdtImmediateOffset) == HDT_IO);
//     REQUIRE(util::get_instruction_format(branchExchange) == BEX);
//     REQUIRE(util::get_instruction_format(softwareInterrupt) == INT);
//     REQUIRE(util::get_instruction_format(singleDataSwap) == SDS);
//     REQUIRE(util::get_instruction_format(multiply) == MUL);
//     REQUIRE(util::get_instruction_format(coprocessorRegisterTransfer) == CRT);
//     REQUIRE(util::get_instruction_format(undefined) == UNDEF);
//     REQUIRE(util::get_instruction_format(hdtRegisterOffset) == HDT_RO);
//     REQUIRE(util::get_instruction_format(singleDataTransfer) == SDT);
//     REQUIRE(util::get_instruction_format(blockDataTransfer) == BDT);
//     REQUIRE(util::get_instruction_format(coprocessorDataOperation) == CDO);
//     REQUIRE(util::get_instruction_format(coprocessorDataTransfer) ==  CDT);
//     REQUIRE(util::get_instruction_format(branch) == B);
//     REQUIRE(util::get_instruction_format(dataProcessing) == DP);

//     // some random ones that should be false
//     REQUIRE(util::get_instruction_format(hdtRegisterOffset) != BEX);
//     REQUIRE(util::get_instruction_format(singleDataTransfer) != MUL);
//     REQUIRE(util::get_instruction_format(blockDataTransfer) != UNDEF);
//     REQUIRE(util::get_instruction_format(coprocessorDataOperation) != HDT_RO);
//     REQUIRE(util::get_instruction_format(coprocessorDataTransfer) != DP);
//     REQUIRE(util::get_instruction_format(branch) != BEX);
// }

TEST_CASE("InstructionSubset", "[instruction_subset]") {
    arm_instruction a = 0b00000000000000000000000000000000;
    arm_instruction b = 0b00000000000000000000000000001011;
    arm_instruction c = 0b11001000000000000000000000000000;
    arm_instruction d = 0b11000000000000000000110010011011;

    REQUIRE(util::get_instruction_subset(a, 3, 0) == 0b0000);
    REQUIRE(util::get_instruction_subset(b, 3, 0) == 0b1011);
    REQUIRE(util::get_instruction_subset(c, 31, 28) == 0b1100);
    REQUIRE(util::get_instruction_subset(d, 11, 0) == 0b110010011011);
    REQUIRE(util::get_instruction_subset(c, 27, 27) == 0b1);

    // flipped order of operands
    REQUIRE(util::get_instruction_subset(b, 0, 4) == 0b0000);
}

TEST_CASE("MemoryTests", "[mem_tests]") {
    Memory mem;
    word address = 0x1000;
    mem.write_u32(address, 0xABCDEFA0);
    REQUIRE(mem.read_u8(address) == 0xA0);
    REQUIRE(mem.read_u32(address) == 0xABCDEFA0);
}