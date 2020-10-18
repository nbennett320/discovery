/* discovery
 * License: GPLv2
 * See LICENSE.txt for full license text
 * Author: Sam Kravitz
 * 
 * FILE: memory.cpp
 * DATE: July 13, 2020
 * DESCRIPTION: Implementation of memory related functions
 */
#include "memory.h"

#include <fstream>
#include <iostream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

Memory::Memory()
{
    // zero memory
    for (int i = 0; i < MEM_SIZE; i++)
        memory[i] = 0;

    // default cycle accesses for wait statae
    n_cycles = 4;
    s_cycles = 2;

    game_rom = NULL;
    stat = NULL;
}

Memory::~Memory() { }

u32 Memory::read_u32(u32 address)
{
    return (read_u8(address + 3) << 24)
    | (read_u8(address + 2) << 16)
    | (read_u8(address + 1) << 8)
    | read_u8(address);
}

u16 Memory::read_u16(u32 address)
{
    return (read_u8(address + 1) << 8) | read_u8(address);
}

u8 Memory::read_u8(u32 address)
{
    // game rom
    if (address >= MEM_SIZE)
    {

        // rom image 2
        if (address >= 0xC000000)
        {
            address -= 0x4000000;
        }

        // rom image 1
        else if (address >= 0xA000000)
        {
            address -= 0x2000000;
        }

        return game_rom[address - MEM_SIZE];
    }

    // memory mirrors
    // EWRAM
    if (address > MEM_EWRAM_END && address < MEM_IWRAM_START)
    {   
        while (address > MEM_EWRAM_END)
            address -= MEM_EWRAM_SIZE;
    }

    // IWRAM
    else if (address > MEM_IWRAM_END && address < MEM_IO_REG_START)
    {   
        while (address > MEM_IWRAM_END)
            address -= MEM_IWRAM_SIZE;
    }

    // Palette RAM
    else if (address > MEM_PALETTE_RAM_END && address < MEM_VRAM_START)
    {   
        while (address > MEM_PALETTE_RAM_END)
            address -= MEM_PALETTE_RAM_SIZE;
    }

    // VRAM
    else if (address > MEM_VRAM_END && address < MEM_OAM_START)
    {
        //x06010000 - 0x06017FFF is mirrored from 0x06018000 - 0x0601FFFF.
        if (address <= 0x601FFFF)
        {
            address -= 0x8000;
        }

        // otherwise mirrors every 0x20000
        else
        {
            while (address > MEM_VRAM_END)
                address -= MEM_VRAM_SIZE;   
        }
    }

    // OAM
    else if (address > MEM_OAM_END && address < MEM_SIZE)
    {   
        while (address > MEM_OAM_END)
            address -= MEM_OAM_SIZE;
    }

    u8 result = 0;
    switch (address)
    {
        // IO reg
        case REG_DISPSTAT:
            result |= stat->in_vBlank ? 0b1  : 0b0;  // bit 0 set in vblank, clear in vdraw
            result |= stat->in_hBlank ? 0b10 : 0b00; // bit 1 set in hblank, clear in hdraw
            //std::cout << "Polling REG_DISPSTAT " << (int) result << "\n";
            return result;
        case REG_VCOUNT:
            return stat->current_scanline;

        default:
            return memory[address];
    }
}

void Memory::write_u32(u32 address, u32 value)
{
    write_u8(address, value & 0xFF);
    write_u8(address + 1, (value >> 8) & 0xFF);
    write_u8(address + 2, (value >> 16) & 0xFF);
    write_u8(address + 3, (value >> 24) & 0xFF);
}

void Memory::write_u16(u32 address, u16 value)
{
    write_u8(address, value & 0xFF);
    write_u8(address + 1, (value >> 8) & 0xFF);
}

// TODO - add protection against VRAM byte writes
void Memory::write_u8(u32 address, u8 value)
{
    // game rom
    if (address >= MEM_SIZE)
    {
        // rom image 2
        if (address >= 0xC000000)
        {
            address -= 0x4000000;
        }

        // rom image 1
        else if (address >= 0xA000000)
        {
            address -= 0x2000000;
        }

        std::cerr << "Warning: writing to game rom\n";

        game_rom[address - MEM_SIZE] = value;
        return;
    }

    // memory mirrors
    // EWRAM
    if (address > MEM_EWRAM_END && address < MEM_IWRAM_START)
    {
        while (address > MEM_EWRAM_END)
            address -= MEM_EWRAM_SIZE;
    }

    // IWRAM
    else if (address > MEM_IWRAM_END && address < MEM_IO_REG_START)
    {   
        while (address > MEM_IWRAM_END)
            address -= MEM_IWRAM_SIZE;
    }

    // Palette RAM
    else if (address > MEM_PALETTE_RAM_END && address < MEM_VRAM_START)
    {   
        while (address > MEM_PALETTE_RAM_END)
            address -= MEM_PALETTE_RAM_SIZE;
    }

    // VRAM
    else if (address > MEM_VRAM_END && address < MEM_OAM_START)
    {
        //x06010000 - 0x06017FFF is mirrored from 0x06018000 - 0x0601FFFF.
        if (address <= 0x601FFFF)
        {
            address -= 0x8000;
        }

        // otherwise mirrors every 0x20000
        else
        {
            while (address > MEM_VRAM_END)
                address -= MEM_VRAM_SIZE;   
        }
    }

    // OAM
    else if (address > MEM_OAM_END && address < MEM_GAMEPAK_ROM_START)
    {   
        while (address > MEM_OAM_END)
            address -= MEM_OAM_SIZE;
    }

    switch (address)
    {
        // REG_DISPCNT
        case REG_DISPCNT:
            stat->reg_dispcnt.mode                  = value >> 0 & 0x7; // bits 0-2     
            stat->reg_dispcnt.gb                    = value >> 3 & 0x1; // bit 3
            stat->reg_dispcnt.ps                    = value >> 4 & 0x1; // bit 4
            stat->reg_dispcnt.hb                    = value >> 5 & 0x1; // bit 5
            stat->reg_dispcnt.obj_map_mode          = value >> 6 & 0x1; // bit 6
            stat->reg_dispcnt.fb                    = value >> 7 & 0x1; // bit 7    
        break;

        case REG_DISPCNT + 1:
            stat->bg_cnt[0].enabled                 = value >> 0 & 0x1; // bit 8
            stat->bg_cnt[1].enabled                 = value >> 1 & 0x1; // bit 9
            stat->bg_cnt[2].enabled                 = value >> 2 & 0x1; // bit A
            stat->bg_cnt[3].enabled                 = value >> 3 & 0x1; // bit B
            stat->reg_dispcnt.obj_enabled           = value >> 4 & 0x1; // bit C
            stat->reg_dispcnt.win_enabled           = value >> 5 & 0x7; // bits D-F
        break;

        // REG_BG0CNT
        case REG_BG0CNT:
            stat->bg_cnt[0].priority      = value >> 0 & 0x3; // bits 0-1
            stat->bg_cnt[0].cbb           = value >> 2 & 0x3; // bits 2-3
            stat->bg_cnt[0].mosaic        = value >> 6 & 0x1; // bit  6
            stat->bg_cnt[0].color_mode    = value >> 7 & 0x1; // bit  7
        break;

        case REG_BG0CNT + 1:
            stat->bg_cnt[0].sbb           = value >> 0 & 0x1F; // bits 8-C
            stat->bg_cnt[0].affine_wrap   = value >> 5 & 0x1;  // bit  D
            stat->bg_cnt[0].size          = value >> 6 & 0x3;  // bits E-F
        break;

        // REG_BG1CNT
        case REG_BG1CNT:
            stat->bg_cnt[1].priority      = value >> 0 & 0x3; // bits 0-1
            stat->bg_cnt[1].cbb           = value >> 2 & 0x3; // bits 2-3
            stat->bg_cnt[1].mosaic        = value >> 6 & 0x1; // bit  6
            stat->bg_cnt[1].color_mode    = value >> 7 & 0x1; // bit  7
        break;

        case REG_BG1CNT + 1:
            stat->bg_cnt[1].sbb           = value >> 0 & 0x1F; // bits 8-C
            stat->bg_cnt[1].affine_wrap   = value >> 5 & 0x1;  // bit  D
            stat->bg_cnt[1].size          = value >> 6 & 0x3;  // bits E-F
        break;

        // REG_BG2CNT
        case REG_BG2CNT:
            stat->bg_cnt[2].priority      = value >> 0 & 0x3; // bits 0-1
            stat->bg_cnt[2].cbb           = value >> 2 & 0x3; // bits 2-3
            stat->bg_cnt[2].mosaic        = value >> 6 & 0x1; // bit  6
            stat->bg_cnt[2].color_mode    = value >> 7 & 0x1; // bit  7
        break;

        case REG_BG2CNT + 1:
            stat->bg_cnt[2].sbb           = value >> 0 & 0x1F; // bits 8-C
            stat->bg_cnt[2].affine_wrap   = value >> 5 & 0x1;  // bit  D
            stat->bg_cnt[2].size          = value >> 6 & 0x3;  // bits E-F
        break;

        // REG_BG3CNT
        case REG_BG3CNT:
            stat->bg_cnt[3].priority      = value >> 0 & 0x3; // bits 0-1
            stat->bg_cnt[3].cbb           = value >> 2 & 0x3; // bits 2-3
            stat->bg_cnt[3].mosaic        = value >> 6 & 0x1; // bit  6
            stat->bg_cnt[3].color_mode    = value >> 7 & 0x1; // bit  7
        break;

        case REG_BG3CNT + 1:
            stat->bg_cnt[3].sbb           = value >> 0 & 0x1F; // bits 8-C
            stat->bg_cnt[3].affine_wrap   = value >> 5 & 0x1;  // bit  D
            stat->bg_cnt[3].size          = value >> 6 & 0x3;  // bits E-F
        break;
        
        // write into waitstate ctl
        case WAITCNT:
            switch(value >> 2 & 0b11) // bits 2-3
            {
                case 0: n_cycles = 4; break;
                case 1: n_cycles = 3; break;
                case 2: n_cycles = 2; break;
                case 3: n_cycles = 8; break;
            }

            switch (value >> 4) // bit 4
            {
                case 0: s_cycles = 2; break;
                case 1: s_cycles = 1; break;
            }
        break;

        default:
            //u8 *normalized_address = get_internal_region(address);
            memory[address] = value;
    }
}

u32 Memory::read_u32_unprotected(u32 address)
{
    return (read_u8_unprotected(address + 3) << 24)
    | (read_u8_unprotected(address + 2) << 16)
    | (read_u8_unprotected(address + 1) << 8)
    | read_u8_unprotected(address);
}

u16 Memory::read_u16_unprotected(u32 address)
{
    return (read_u8_unprotected(address + 1) << 8) | read_u8_unprotected(address);
}

u8 Memory::read_u8_unprotected(u32 address)
{
    return memory[address];
}

void Memory::write_u32_unprotected(u32 address, u32 value)
{
    write_u8_unprotected(address, value & 0xFF);
    write_u8_unprotected(address + 1, (value >> 8) & 0xFF);
    write_u8_unprotected(address + 2, (value >> 16) & 0xFF);
    write_u8_unprotected(address + 3, (value >> 24) & 0xFF);
}

void Memory::write_u16_unprotected(u32 address, u16 value)
{
    write_u8_unprotected(address, value & 0xFF);
    write_u8_unprotected(address + 1, (value >> 8) & 0xFF);
}

// TODO - add protection against VRAM byte writes
void Memory::write_u8_unprotected(u32 address, u8 value)
{
    memory[address] = value;
}

void Memory::load_rom(char *name)
{
    std::ifstream rom(name, std::ios::in | std::ios::binary);

    if (!rom)
        return;
    size_t size = fs::file_size(name);

    if (!rom.good())
    {
        std::cerr << "Bad rom!" << "\n";
        return;
    }

    game_rom = new u8[size]();
    rom.read((char *) game_rom, size);
    rom.close();
}

void Memory::load_bios()
{
    // bios must be called gba_bios.bin
    std::ifstream bios("gba_bios.bin", std::ios::in | std::ios::binary);
    if (!bios)
        return;

    if (!bios.good())
    {
        std::cerr << "Bad bios!" << "\n";
        return;
    }

    bios.read((char *) memory, MEM_BIOS_SIZE);
    bios.close();
}

/*
 * GBA memory can be addressed anywhere from 0x00000000-0xFFFFFFFF, however most of those addresses are unused.
 * given a 4 u8 address, this function will return the address of the
 * internal region the address points to, which saves a boatload of memory.
 */
u8 *Memory::get_internal_region(u32 address)
{
    // if (address <= MEM_BIOS_END)  {
    //     return &memory.bios[address];
    // }
    // else if (address >= MEM_EWRAM_START && address <= MEM_EWRAM_END) {
    //     //std::cout << "WORK ROM ACCESSED!\n";
    //     return &memory.EWRAM[address - MEM_EWRAM_START];
    // }
    // else if (address >= MEM_CHIP_WRAM_START && address <= MEM_CHIP_WRAM_END) return &memory.chip_wram[address - MEM_CHIP_WRAM_START];
    // else if (address >= MEM_IO_REG_START && address <= MEM_IO_REG_END) return &memory.io_reg[address - MEM_IO_REG_START];
    // else if (address >= MEM_PALETTE_RAM_START && address <= MEM_PALETTE_RAM_END) {
    //     //std::cout << "Pallette RAM accessed\n";
    //     return &memory.palette_ram[address - MEM_PALETTE_RAM_START];
    // }
    // else if (address >= MEM_VRAM_START && address <= MEM_VRAM_END) return &memory.vram[address - MEM_VRAM_START];
    // else if (address >= MEM_OAM_START && address <= MEM_OAM_END) {
    //     //std::cout << "Oam accessed\n";
    //     return &memory.oam[address - MEM_OAM_START];
    // }
    // else if (address >= MEM_GAMEPAK_ROM_START && address <= MEM_GAMEPAK_ROM_END) {
    //     //std::cout << "Gamepak ROM ACCESSED!\n";
    //     return &game_rom[address - MEM_GAMEPAK_ROM_START];
    // }
    // else {
    //     std::cerr << "Error: invalid internal address specified: " << std::hex << "0x" << address << "\n";
    //     exit(2);
    // }
    return 0;
}
