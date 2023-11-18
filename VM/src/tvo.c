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

void TVO_DrawRect(int x, int y, int w, int h, SDL_Renderer* renderer) {
    for (size_t cy = y; cy < y + h; y++) {
        for (size_t cx = x; cx < x + w; x++) {
            SDL_RenderDrawPoint(renderer, cx, cy);
        }
    }
}

void TVO_Render(SDL_Renderer* renderer, CPU* cpu) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    TVO_DrawRect(cpu->ports[2] * 8, (cpu->ports[3] * 16) - 3, 8, 3, renderer);
    //SDL_SetRenderDrawColor(renderer, (argb & 0xFF), ((argb >> 8) & 0xFF), ((argb >> 16) & 0xFF), 255);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            TVO_WriteChar(cpu->memory[0x707f + (x + (y * 80))], x * 8, y * 16, 0x0, renderer);
        }
    }
    TVO_DrawRect(cpu->ports[2] * 8, (cpu->ports[3] * 16) - 3, 8, 3, renderer);
}
