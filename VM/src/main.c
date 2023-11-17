#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "cpu.h"
#include "tvo.h"

const int SW = 640;
const int SH = 400;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
CPU cpu;
bool running = true;
SDL_Event e;
bool keyDowned = false;

void* SDL_Update() {
    while (running) {
        if (CPU_Execute(&cpu, 3)) {
            printf("CPU Error %d!\n", cpu.status);
        }
        TVO_Render(renderer, &cpu);
        SDL_RenderPresent(renderer);
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    running = false;
                    exit(0);
                    break;
                case SDL_KEYUP:
                    cpu.ports[1] = 0;
                    cpu.ports[0] = 0;
                    keyDowned = false;
                    break;
                case SDL_KEYDOWN:
                    if (!keyDowned) {
                        cpu.ports[0] = 1;
                        if (e.key.keysym.sym >= 33 && e.key.keysym.sym <= 126) {
                            cpu.ports[1] = (char)e.key.keysym.sym;
                        }
                    }
                    keyDowned = true;
                    break;
                
            }
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Error: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_CreateWindowAndRenderer(SW, SH, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    if (window == NULL) {
        printf("Could not initialise main window! '%s'\n", SDL_GetError());
        return 1;
    }

    FILE *fptr;
    uint16_t rom[0x7e80] = {};
    if ((fptr = fopen(argv[1], "rb")) == NULL){
       printf("Error opening rom.\n");
       return 1;
    }

    fread(&rom, sizeof(rom), 1, fptr);
    fclose(fptr);
    
    CPU_Init(&cpu, rom);

    SDL_Update();

    return 0;
}
