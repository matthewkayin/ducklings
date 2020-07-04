#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>


int main(){

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    int SCREEN_WIDTH = 640;
    int SCREEN_HEIGHT = 360;

    // Init SDL
    bool init_successful = true;

    if(SDL_Init(SDL_INIT_VIDEO) < 0){

        printf("Unable to initialize SDL!\n");
        init_successful = false;

    }else{

        window = SDL_CreateWindow("Duckline", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

        int img_flags = IMG_INIT_PNG;

        if(!(IMG_Init(img_flags) & img_flags)){

            printf("Unable to initialize SDL_image! SDL Error: %s\n", IMG_GetError());
            init_successful = false;

        }else{

            if(TTF_Init() == -1){

                printf("Unable to initialize SDL_ttf! SDL Error: %s\n", TTF_GetError());
                init_successful = false;

            }else{

                if(!window || !renderer){

                    printf("Unable to initialize engine!\n");
                    init_successful = false;
                }
            }
        }
    }

    if(!init_successful){

        printf("Exiting...\n");
        return 0;
    }

    // Start game loop
    const unsigned long SECOND = 1000;
    const unsigned long TARGET_FPS = 60;
    const unsigned long FRAME_TIME = (unsigned long)(SECOND / (double)TARGET_FPS);
    unsigned long second_before_time = SDL_GetTicks();
    unsigned long frame_before_time = second_before_time;
    unsigned long current_time;
    double delta_time = 0;
    int frames = 0;
    int fps = 0;
    bool running = true;

    while(running){

        // Poll events
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0){

            if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){

                running = false;
            }
        }

        // Update

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect rect;
        rect.x = 10;
        rect.y = 10;
        rect.w = 32;
        rect.h = 32;
        SDL_RenderFillRect(renderer, &rect);

        SDL_RenderPresent(renderer);
        frames++;

        current_time = SDL_GetTicks();

        if(current_time - second_before_time >= SECOND){

            fps = frames;
            frames = 0;
            second_before_time += SECOND;
            printf("FPS: %i\n", fps);
        }

        delta_time = (current_time - frame_before_time) / (double)FRAME_TIME;

        if(current_time - frame_before_time < FRAME_TIME){

            unsigned long delay_time = FRAME_TIME - (current_time - frame_before_time);
            SDL_Delay(delay_time);
        }

        frame_before_time = SDL_GetTicks();
    }

    // Quit SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
