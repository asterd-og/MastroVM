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
            SDL_DestroyWindow(window);
            SDL_DestroyRenderer(renderer);
            SDL_Quit();
            running = false;
            exit(1);
        }
        TVO_Render(renderer, &cpu);
        SDL_RenderPresent(renderer);
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    SDL_DestroyWindow(window);
                    SDL_DestroyRenderer(renderer);
                    SDL_Quit();
                    running = false;
                    exit(0);
                    break;
                /* 
                    Generally, SDL_KEYUP/SDL_KEYDOWN is used for control sequences, which currently, can be ommited
                    So, for this case, I'm commenting some of the code for future use.

                    For this purpose, using SDL_TEXTINPUT for printable characters.    
                */
                case SDL_KEYUP:
                    cpu.ports[1] = 0;
                    cpu.ports[0] = 0;
                    keyDowned = false;
                    break;
                /* 
                    This code works with spaces now, however, the SDL_TEXTINPUT method also works with capital characters in my testing.

                    If you don't care for capital characters and want your CPU to handle it manually, then comment out the SDL_TEXTINPUT code and uncomment the SDL_KEYDOWN code.
                */
                // case SDL_KEYDOWN:
                //     if (!keyDowned) {
                //         cpu.ports[0] = 1;
                //         if(e.key.keysym.sym >= SDLK_SPACE && e.key.keysym.sym <= SDLK_z) {
                //             cpu.ports[1] = (char)e.key.keysym.sym;
                //         }
                //     }
                //     keyDowned = true;
                //     break;
                case SDL_TEXTINPUT:
                    if (!keyDowned) {
                        cpu.ports[0] = 1;
                        cpu.ports[1] = (char)e.text.text[0];
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
    window = SDL_CreateWindow("MastroVM", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SW, SH, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    if (window == NULL) {
        printf("Could not initialise main window! '%s'\n", SDL_GetError());
        return 1;
    }

    FILE *fptr;
    uint16_t rom[0xbd00] = {};
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
