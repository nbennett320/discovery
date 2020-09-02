#include "gpu.h"
#include "obj_attr.h"
#include <ctime>

u8 five_bits_to_eight(u8);

GPU::GPU() {
    mem = NULL;
    stat = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Could not initialize GPU" << "\n";
        exit(2);
    }

    window = SDL_CreateWindow("discovery", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (window == NULL) {
        std::cerr << "Could not create window" << "\n";
        exit(2);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);

    lcd_clock = 0;
    current_scanline = 0;

    reset();

    old_clock = clock();
}

GPU::~GPU() {
    std::cout << "GPU:: Shutdown\n";
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
}

void GPU::reset() {
    // paint the screen black
    SDL_Rect rect{8, 8, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderPresent(renderer);
}

// 1 clock cycle of the gpu
void GPU::clock_gpu() {
    lcd_clock++;

    // 4 cycles per pixel
    if (lcd_clock % 4 == 0)
        stat->current_scanline_pixel++;

    // finished hDraw
    if (stat->current_scanline_pixel == SCREEN_WIDTH) {
        stat->in_hBlank = true;
    }

    // completed a scanline
    if (lcd_clock % SCANLINE_CYCLES == 0) {
        stat->current_scanline++;
        if (stat->current_scanline == NUM_SCANLINES)
            stat->current_scanline = 0;

        // go back into hDraw
        stat->in_hBlank = false;

        // reset current scanline pixels
        stat->current_scanline_pixel = 0;
    }

    // finished vDraw
    if (stat->current_scanline == SCREEN_HEIGHT) {
        stat->in_vBlank = true;
    }

    // completed a refresh
    if (lcd_clock == REFRESH_CYCLES) {
        lcd_clock = 0; // restart lcd_clock
        stat->current_scanline = 0;
        stat->current_scanline_pixel = 0;
        stat->in_hBlank = false;
        stat->in_vBlank = false;
        draw();
    }
}

void GPU::draw() {
    // if (!stat->needs_refresh) return;

    //std::cout << "Executing graphics mode: " << (mem->read_u32(REG_DISPCNT) & 0x7) << "\n";
    switch (mem->read_u32_unprotected(REG_DISPCNT) & 0x7) { // bits 0-2 represent video mode
        case 0: draw_mode0(); break;
        case 3: draw_mode3(); break;
        case 4: draw_mode4(); break;
        default: 
            std::cerr << "Error: unknown video mode" << "\n";
            break;
    }
    // double duration;
    // clock_t new_time = std::clock();
    // duration = ( new_time - old_clock ) / (double) CLOCKS_PER_SEC;
    // std::cout << "Refresh took: " << duration << "\n";
    // old_clock = new_time;
    // stat->needs_refresh = false;
}

// video mode 0 - sprite mode
void GPU::draw_mode0() {
    //std::cout << "REG: " << mem->read_u16_unprotected(REG_DISPCNT) << "\n";
    u32 starting_address;
    u32 *pixels = new u32[SCREEN_WIDTH * SCREEN_HEIGHT];
    u32 current_pixel; // in mode 3 each pixel uses 2 bytes
    u8 palette_index;
    u8 r;
    u8 g;
    u8 b;
    u8 alpha = 255;

    for (int i = 0; i < 128; i++) {
        obj_attr attr = get_attr(i);
        //std::cout << (attr0 >> 0xD) << "\n";
        std::cout << (attr.attr_2.attr.) << "\n";
    }

    // starting_address = LOWER_SPRITE_BLOCK + (64 * 28);
    // for (int i = 0; i < 64; i++) {
    //     palette_index = mem->read_u8_unprotected(starting_address + i);
    //     current_pixel = mem->read_u32_unprotected(TILE_PALETTE + (palette_index * sizeof(u16)));
    //     r = five_bits_to_eight(current_pixel & 0b11111);
    //     g = five_bits_to_eight((current_pixel >> 5) & 0b11111);
    //     b = five_bits_to_eight((current_pixel >> 10) & 0b11111);

    //     // add current pixel in argb format to pixel array
    //     pixels[(i/8 * SCREEN_WIDTH) + (i % 8)] = alpha;
    //     pixels[(i/8 * SCREEN_WIDTH) + (i % 8)] <<= 8;
    //     pixels[(i/8 * SCREEN_WIDTH) + (i % 8)] |= r;
    //     pixels[(i/8 * SCREEN_WIDTH) + (i % 8)] <<= 8;
    //     pixels[(i/8 * SCREEN_WIDTH) + (i % 8)] |= g;
    //     pixels[(i/8 * SCREEN_WIDTH) + (i % 8)] <<= 8;
    //     pixels[(i/8 * SCREEN_WIDTH) + (i % 8)] |= b;
    // }
    // for (int x = 0; x < 8; x++) {
    //     for (int i = 0; i < 8; ++i) {
    //         starting_address = LOWER_SPRITE_BLOCK + (x * i);
    //         for (int y = 0; y < 64; y++) {
    //             palette_index = mem->read_u8_unprotected(starting_address + y);
    //             current_pixel = mem->read_u32_unprotected(TILE_PALETTE + (palette_index * sizeof(u16)));
    //             r = five_bits_to_eight(current_pixel & 0b11111);
    //             g = five_bits_to_eight((current_pixel >> 5) & 0b11111);
    //             b = five_bits_to_eight((current_pixel >> 10) & 0b11111);

    //             // add current pixel in argb format to pixel array
    //             pixels[x*SCREEN_WIDTH + y*SCREEN_HEIGHT + i] = alpha;
    //             pixels[x*SCREEN_WIDTH + y*SCREEN_HEIGHT + i] <<= 8;
    //             pixels[x*SCREEN_WIDTH + y*SCREEN_HEIGHT + i] |= r;
    //             pixels[x*SCREEN_WIDTH + y*SCREEN_HEIGHT + i] <<= 8;
    //             pixels[x*SCREEN_WIDTH + y*SCREEN_HEIGHT + i] |= g;
    //             pixels[x*SCREEN_WIDTH + y*SCREEN_HEIGHT + i] <<= 8;
    //             pixels[x*SCREEN_WIDTH + y*SCREEN_HEIGHT + i] |= b;
    //         }
    //     }
    // }
    

    //for (int i = 64; i < SCREEN_HEIGHT * SCREEN_HEIGHT; ++i) pixels[i] = 0;

    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(u32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    delete[] pixels;
}

// video mode 3 - bitmap mode
void GPU::draw_mode3() {
    u16 current_pixel; // in mode 3 each pixel uses 2 bytes
    u8 r;
    u8 g;
    u8 b;
    u8 alpha = 255;
    u32 *pixels = new u32[SCREEN_WIDTH * SCREEN_HEIGHT]; // array representing each pixel on the screen

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        current_pixel = mem->read_u16_unprotected(MEM_VRAM_START + (2 * i)); // multiply i * 2 b/c each pixel is 2 bytes
        r = five_bits_to_eight(current_pixel & 0b11111);
        g = five_bits_to_eight((current_pixel >> 5) & 0b11111);
        b = five_bits_to_eight((current_pixel >> 10) & 0b11111);

        // add current pixel in argb format to pixel array
        pixels[i] = alpha;
        pixels[i] <<= 8;
        pixels[i] |= r;
        pixels[i] <<= 8;
        pixels[i] |= g;
        pixels[i] <<= 8;
        pixels[i] |= b;
    }   

    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(u32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    delete[] pixels;
}

// video mode 4 - bitmap mode
void GPU::draw_mode4() {
    u8 pallette_index; // in mode 4 each pixel uses 1 byte 
    u32 current_pixel; // the color located at pallette_ram[pallette_index]
    u8 r;
    u8 g;
    u8 b;
    u8 alpha = 0;
    u32 *pixels = new u32[SCREEN_WIDTH * SCREEN_HEIGHT]; // array representing each pixel on the screen

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        pallette_index = mem->read_u8_unprotected(MEM_VRAM_START + i);
        current_pixel = mem->read_u32_unprotected(MEM_PALETTE_RAM_START + (pallette_index * sizeof(u16)));
        r = five_bits_to_eight(current_pixel & 0b11111);
        g = five_bits_to_eight((current_pixel >> 5) & 0b11111);
        b = five_bits_to_eight((current_pixel >> 10) & 0b11111);

        // add current pixel in argb format to pixel array
        pixels[i] = alpha;
        pixels[i] <<= 8;
        pixels[i] |= r;
        pixels[i] <<= 8;
        pixels[i] |= g;
        pixels[i] <<= 8;
        pixels[i] |= b;
    }   

    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(u32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    delete[] pixels;
}

// fills an obj_attr struct from OAM from the given index (0-127)
obj_attr GPU::get_attr(int index) {
    obj_attr attr;
    obj_attr._zero = mem->read_u16(MEM_OAM_START + index + 0);
    obj_attr._one = mem->read_u16(MEM_OAM_START + index + 2);
    obj_attr._two = mem->read_u16(MEM_OAM_START + index + 4);
    return attr;
}

// given a range of 0-31 return a range of 0-255
inline u8 five_bits_to_eight (u8 u5) {
    return u5 * 255 / 32;
}