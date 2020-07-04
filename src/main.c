#include "game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

void render_state(SDL_Renderer* renderer, State* current_state);
void render_text(SDL_Renderer* renderer, TTF_Font* font, char* text, SDL_Color color, int x, int y);

int main(){

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    TTF_Font* font_small = NULL;

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

                font_small = TTF_OpenFont("./res/helvetica.ttf", 10);

                if(font_small == NULL){

                    printf("Unable to load font_small! %s\n", TTF_GetError());
                    init_successful = false;
                }

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
    // double delta_time = 0;
    int frames = 0;
    int fps = 0;
    bool running = true;

    State* current_state = get_initial_state();

    while(running){

        int player_move = NOTHING;

        // Poll events
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0){

            if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){

                running = false;

            }else if(e.type == SDL_KEYDOWN){

                int key = e.key.keysym.sym;
                if(key == SDLK_UP || key == SDLK_w){

                    player_move = PLAYER_MOVE_UP;

                }else if(key == SDLK_RIGHT || key == SDLK_d){

                    player_move = PLAYER_MOVE_RIGHT;

                }else if(key == SDLK_DOWN || key == SDLK_s){

                    player_move = PLAYER_MOVE_DOWN;

                }else if(key == SDLK_LEFT || key == SDLK_a){

                    player_move = PLAYER_MOVE_LEFT;

                }else if(key == SDLK_z){

                    player_move = PLAYER_MOVE_UNDO;
                }
            }
        }

        // Update
        if(player_move == PLAYER_MOVE_UNDO){

            current_state = undo_move(current_state);

        }else if(player_move != NOTHING){

            handle_move(current_state, player_move);
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_state(renderer, current_state);

        char fps_text[10];
        sprintf(fps_text, "FPS: %i", fps);
        render_text(renderer, font_small, fps_text, (SDL_Color){.r = 255, .g = 255, .b = 255, .a = 255}, 0, 0);

        SDL_RenderPresent(renderer);
        frames++;

        current_time = SDL_GetTicks();

        if(current_time - second_before_time >= SECOND){

            fps = frames;
            frames = 0;
            second_before_time += SECOND;
        }

        // delta_time = (current_time - frame_before_time) / (double)FRAME_TIME;

        if(current_time - frame_before_time < FRAME_TIME){

            unsigned long delay_time = FRAME_TIME - (current_time - frame_before_time);
            SDL_Delay(delay_time);
        }

        frame_before_time = SDL_GetTicks();
    }

    // Quit SDL
    TTF_CloseFont(font_small);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

void render_state(SDL_Renderer* renderer, State* current_state){

    SDL_Rect player_rect;
    player_rect.x = current_state->player_x * PLAYER_WIDTH;
    player_rect.y = current_state->player_y * PLAYER_HEIGHT;
    player_rect.w = PLAYER_WIDTH;
    player_rect.h = PLAYER_HEIGHT;

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &player_rect);
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, char* text, SDL_Color color, int x, int y){

    SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, color);

    if(text_surface == NULL){

        printf("Unable to render text to surface! SDL Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    if(text_texture == NULL){

        printf("Unable to create texture! SDL Error: %s\n", SDL_GetError());
        return;
    }

    int draw_x = x;
    int draw_y = y;

    if(draw_x == -1){

        draw_x = (SCREEN_WIDTH / 2) - (text_surface->w / 2);
    }
    if(draw_y == -1){

        draw_y = (SCREEN_HEIGHT / 2) - (text_surface->h / 2);
    }

    SDL_Rect source_rect = (SDL_Rect){.x = 0, .y = 0, .w = text_surface->w, .h = text_surface->h};
    SDL_Rect dest_rect = (SDL_Rect){.x = draw_x, .y = draw_y, .w = text_surface->w, .h = text_surface->h};
    SDL_RenderCopy(renderer, text_texture, &source_rect, &dest_rect);

    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}
