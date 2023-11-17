#include "tvo.h"

static inline bool bit_address_from_byte(uint32_t to_convert, int to_return)
{
    int mask = 1 << (to_return - 1);
    return ((to_convert & mask) != 0);
}

void TVO_WriteChar(char c, int x, int y, uint32_t argb, SDL_Renderer* renderer) {
    if (c == 0) return;
    uint32_t p = 16 * c;
    for (size_t cy = 0; cy < 16; cy++) {
        for (size_t cx = 0; cx < 8; cx++) {
            if (bit_address_from_byte(font8x16[p + cy], cx + 1)) {
                SDL_RenderDrawPoint(renderer, x + (8 - cx), y + cy);
            }
        }
    }
}

void TVO_Render(SDL_Renderer* renderer, CPU* cpu) {
    //SDL_SetRenderDrawColor(renderer, (argb & 0xFF), ((argb >> 8) & 0xFF), ((argb >> 16) & 0xFF), 255);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //printf("going to render\n");
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            //printf("%c",cpu->memory[0x707f+(x+(y*80))]);
            TVO_WriteChar(cpu->memory[0x707f + (x + (y * 80))], x * 8, y * 16, 0x0, renderer);
        }
    }
}
