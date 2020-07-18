#include <iostream>
#include <fstream>
#include "discovery.h"
#include "cpu/arm_7tdmi.h"
#include "memory/memory.h"

void run_asm(char *name) {
    arm_7tdmi arm;
    arm.mem.load_rom(name);
    arm_instruction i;
    while (true) {
        i = arm.mem.read_u32(arm.registers.r15);
        std::cout << i << "\n";
        arm.execute(i);
        if (i == 0) arm.registers.r15 += 4;
    }
}

int main(int argc, char **argv) {
    std::cout << "Gameboy emulator!" << "\n";
    //run_asm(argv[1]);
    discovery emulator;
    while (true) {
        emulator.gpu.draw_pixel(400, 300);
    }
    return 0;
}
