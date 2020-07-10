#include "game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>

#define GAMESTATE_EXIT 0
#define GAMESTATE_MENU 1
#define GAMESTATE_GAME 2
#define GAMESTATE_EDIT 3
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360


int menu_loop(SDL_Renderer* renderer, char* filename);
int game_loop(SDL_Renderer* renderer, char* filename);
int edit_loop(SDL_Renderer* renderer, char* filename);

char** generate_puzzle_list(int* puzzle_count);
void render_state(SDL_Renderer* renderer, State* current_state);
void render_text(SDL_Renderer* renderer, TTF_Font* font, char* text, SDL_Color color, int x, int y);

int main(){

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

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

    int gamestate = GAMESTATE_MENU;
    char* filename = (char*)malloc(255 * sizeof(char));
    while(gamestate != GAMESTATE_EXIT){

        if(gamestate == GAMESTATE_MENU){

            gamestate = menu_loop(renderer, filename);

        }else if(gamestate == GAMESTATE_GAME){

            gamestate = game_loop(renderer, filename);

        }else if(gamestate == GAMESTATE_EDIT){

            gamestate = edit_loop(renderer, filename);
        }
    }
    free(filename);

    // Quit SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

int menu_loop(SDL_Renderer* renderer, char* filename){

    TTF_Font* font_small = TTF_OpenFont("./res/notosans.ttf", 10);
    TTF_Font* font_med = TTF_OpenFont("./res/notosans.ttf", 18);

    if(font_small == NULL){

        printf("Unable to load font_small! %s\n", TTF_GetError());
        return GAMESTATE_EXIT;
    }

    if(font_med == NULL){

        printf("Unable to load font_med! %s\n", TTF_GetError());
        return GAMESTATE_EXIT;
    }

    // Start game loop
    const unsigned long SECOND = 1000;
    const unsigned long TARGET_FPS = 60;
    const unsigned long FRAME_TIME = (unsigned long)(SECOND / (double)TARGET_FPS);
    unsigned long second_before_time = SDL_GetTicks();
    unsigned long frame_before_time = second_before_time;
    unsigned long current_time;
    int frames = 0;
    int fps = 0;
    bool running = true;
    int return_state;

    int menubox_width = 80;
    int menubox_height = 40;
    SDL_Rect playbox = (SDL_Rect){ .x = (SCREEN_WIDTH / 2) - (menubox_width / 2), .y = 100 + (18 / 2) - (menubox_height / 2), .w = menubox_width, .h = menubox_height };
    SDL_Rect editbox = (SDL_Rect){ .x = (SCREEN_WIDTH / 2) - (menubox_width / 2), .y = 150 + (18 / 2) - (menubox_height / 2), .w = menubox_width, .h = menubox_height };
    char play_text[10] = "Play";
    char edit_text[10] = "Edit";

    int menu_state = 0;
    int menu_index = 0;

    char** puzzle_files = NULL;
    int puzzle_count = 0;

    char new_puzzle_name[50];
    new_puzzle_name[0] = '\0';
    int cursor_index = 0;

    while(running){

        // Poll events
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0){

            if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){

                return_state = GAMESTATE_EXIT;
                running = false;

            }else if(e.type == SDL_KEYDOWN){

                int key = e.key.keysym.sym;
                if(key == SDLK_UP){

                    menu_index--;
                    int menu_wrap_point = 1;
                    if(menu_state == 1){

                        menu_wrap_point = puzzle_count - 1;

                    }else if(menu_state == 2){

                        menu_wrap_point = puzzle_count;
                    }
                    if(menu_index < 0){

                        menu_index = menu_wrap_point;
                    }

                }else if(key == SDLK_DOWN){

                    menu_index++;
                    int menu_wrap_point = 1;
                    if(menu_state == 1){

                        menu_wrap_point = puzzle_count - 1;

                    }else if(menu_state == 2){

                        menu_wrap_point = puzzle_count;
                    }
                    if(menu_index > menu_wrap_point){

                        menu_index = 0;
                    }

                }else if(key == SDLK_RETURN){

                    if(menu_state == 0){

                        if(menu_index == 0){

                            menu_state = 1;

                        }else if(menu_index == 1){

                            menu_state = 2;
                        }
                        puzzle_files = generate_puzzle_list(&puzzle_count);
                        menu_index = 0;

                    }else if(menu_state == 1){

                        strncpy(filename, puzzle_files[menu_index], strlen(puzzle_files[menu_index]));
                        return_state = GAMESTATE_GAME;
                        running = false;

                    }else if(menu_state == 2){

                        if(menu_index == 0){

                            menu_state = 3;
                        }

                    }else if(menu_state == 3){

                        if(cursor_index != 0){

                            sprintf(filename, "%s.duck", new_puzzle_name);
                            menu_state = 4;
                            return_state = GAMESTATE_EDIT;
                            running = false;
                        }
                    }

                }else if((key >= SDLK_a && key <= SDLK_z) || (key >= SDLK_0 && key <= SDLK_9)){ 

                    if(menu_state == 3){

                        if(cursor_index < 48){

                            new_puzzle_name[cursor_index] = key;
                            new_puzzle_name[cursor_index + 1] = '\0';
                            cursor_index++;
                        }
                    }

                }else if(key == SDLK_BACKSPACE){

                    if(menu_state == 3){

                        new_puzzle_name[cursor_index] = '\0';
                        cursor_index--;
                        if(cursor_index < 0){

                            cursor_index = 0;
                        }
                    }

                }else if(key == SDLK_TAB){

                    menu_state = 0;
                    menu_index = 0;
                }
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if(menu_state == 0){

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            if(menu_index == 0){

                SDL_RenderFillRect(renderer, &playbox);
                render_text(renderer, font_med, play_text, (SDL_Color){ .r = 0, .g = 0, .b = 0, .a = 255 }, -1, 100);
                SDL_RenderDrawRect(renderer, &editbox);
                render_text(renderer, font_med, edit_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, -1, 150);

            }else{

                SDL_RenderDrawRect(renderer, &playbox);
                render_text(renderer, font_med, play_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, -1, 100);
                SDL_RenderFillRect(renderer, &editbox);
                render_text(renderer, font_med, edit_text, (SDL_Color){ .r = 0, .g = 0, .b = 0, .a = 255 }, -1, 150);
            }

        }else if(menu_state == 1){

            if(puzzle_count == 0){

                render_text(renderer, font_med, "No puzzles found!", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 20, 20);

            }else{

                if(puzzle_files == NULL){

                    printf("Error! Cannot render null file array!\n");
                    return 0;
                }
                for(int i = 0; i < puzzle_count; i++){

                    char puzzle_string[70];
                    if(menu_index == i){

                        sprintf(puzzle_string, "> %s", puzzle_files[i]);

                    }else{

                        strncpy(puzzle_string, puzzle_files[i], 64);
                    }
                    render_text(renderer, font_med, puzzle_string, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 20, 20 + (20 * i));
                }
            }

        }else if(menu_state == 2){

            char* create_puzzle_text;
            if(menu_index == 0){

                create_puzzle_text = "> New Puzzle";

            }else{

                create_puzzle_text = "New Puzzle";
            }
            render_text(renderer, font_med, create_puzzle_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 20, 20);
            if(puzzle_count != 0 && puzzle_files != NULL){

                for(int i = 0; i < puzzle_count; i++){

                    char puzzle_string[70];
                    if(menu_index == i + 1){

                        sprintf(puzzle_string, "> %s", puzzle_files[i]);

                    }else{

                        strncpy(puzzle_string, puzzle_files[i], 64);
                    }
                    render_text(renderer, font_med, puzzle_string, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 20, 40 + (20 * i));
                }
            }

        }else if(menu_state == 3){

            char new_puzzle_text[64];
            sprintf(new_puzzle_text, "%s.duck", new_puzzle_name);
            render_text(renderer, font_med, new_puzzle_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, -1, -1);
        }

        char fps_text[10];
        sprintf(fps_text, "FPS: %i", fps);
        render_text(renderer, font_small, fps_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 0);

        SDL_RenderPresent(renderer);
        frames++;

        current_time = SDL_GetTicks();

        if(current_time - second_before_time >= SECOND){

            fps = frames;
            frames = 0;
            second_before_time += SECOND;
        }

        if(current_time - frame_before_time < FRAME_TIME){

            unsigned long delay_time = FRAME_TIME - (current_time - frame_before_time);
            SDL_Delay(delay_time);
        }

        frame_before_time = SDL_GetTicks();
    }

    // Close fonts
    TTF_CloseFont(font_small);
    TTF_CloseFont(font_med);

    return return_state;
}

char** generate_puzzle_list(int* puzzle_count){

    char** puzzles = NULL;
    *puzzle_count = 0;
    int puzzle_index = 0;

    DIR* dir = opendir("./puzzles/");
    struct dirent* ent;
    if(dir != NULL){

        while((ent = readdir(dir)) != NULL){

            char* dot_pointer = (char*)memchr(ent->d_name, '.', strlen(ent->d_name));
            if(dot_pointer != NULL){

                int dot_index = dot_pointer - ent->d_name;
                if(dot_index == strlen(ent->d_name) - 5){

                    char extension[5];
                    memcpy(extension, &(ent->d_name[dot_index + 1]), 4);
                    extension[4] = '\0';
                    if(strcmp(extension, "duck") == 0){

                        (*puzzle_count)++;
                    }
                }
            }
        }
        closedir(dir);

    }else{

        printf("Unable to open puzzles folder!");
        return NULL;
    }

    if(puzzle_count == 0){

        return NULL;
    }
    puzzles = (char**)malloc((*puzzle_count) * sizeof(char*));

    dir = opendir("./puzzles/");
    if(dir != NULL){

        while((ent = readdir(dir)) != NULL){

            char* dot_pointer = (char*)memchr(ent->d_name, '.', strlen(ent->d_name));
            if(dot_pointer != NULL){

                int dot_index = dot_pointer - ent->d_name;
                if(dot_index == strlen(ent->d_name) - 5){

                    char extension[5];
                    memcpy(extension, &(ent->d_name[dot_index + 1]), 4);
                    extension[4] = '\0';
                    if(strcmp(extension, "duck") == 0){

                        puzzles[puzzle_index] = (char*)malloc(64 * sizeof(char));
                        strncpy(puzzles[puzzle_index], ent->d_name, 64);
                        puzzle_index++;
                    }
                }
            }
        }
        closedir(dir);

    }else{

        printf("Unable to open puzzles folder!");
        free(puzzles);
        puzzles = NULL;
        return NULL;
    }

    return puzzles;
}

int game_loop(SDL_Renderer* renderer, char* filename){

    TTF_Font* font_small = TTF_OpenFont("./res/notosans.ttf", 10);
    TTF_Font* font_large = TTF_OpenFont("./res/notosans.ttf", 36);

    if(font_small == NULL){

        printf("Unable to load font_small! %s\n", TTF_GetError());
        return GAMESTATE_EXIT;
    }

    if(font_large == NULL){

        printf("Unable to load font_large! %s\n", TTF_GetError());
        return GAMESTATE_EXIT;
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
    int return_state;

    State* current_state = get_from_file(filename);
    bool awaiting_follow_input = false;

    while(running){

        int player_move = NOTHING;

        // Poll events
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0){

            if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){

                return_state = GAMESTATE_EXIT;
                running = false;

            }else if(e.type == SDL_KEYDOWN){

                int key = e.key.keysym.sym;
                if(key == SDLK_UP || key == SDLK_w){

                    if(awaiting_follow_input){

                        player_move = PLAYER_WADDLE_UP;

                    }else{

                        player_move = PLAYER_MOVE_UP;
                    }

                }else if(key == SDLK_RIGHT || key == SDLK_d){

                    if(awaiting_follow_input){

                        player_move = PLAYER_WADDLE_RIGHT;

                    }else{

                        player_move = PLAYER_MOVE_RIGHT;
                    }

                }else if(key == SDLK_DOWN || key == SDLK_s){

                    if(awaiting_follow_input){

                        player_move = PLAYER_WADDLE_DOWN;

                    }else{

                        player_move = PLAYER_MOVE_DOWN;
                    }

                }else if(key == SDLK_LEFT || key == SDLK_a){

                    if(awaiting_follow_input){

                        player_move = PLAYER_WADDLE_LEFT;

                    }else{

                        player_move = PLAYER_MOVE_LEFT;
                    }

                }else if(key == SDLK_z){

                    player_move = PLAYER_MOVE_UNDO;

                }else if(key == SDLK_LSHIFT){

                    if(!awaiting_follow_input){

                        if(get_ducklist_length(current_state) != 0){

                            awaiting_follow_input = true;
                        }
                    }

                }else if(key == SDLK_SPACE){

                    player_move = PLAYER_MOVE_WAIT;

                }else if(key == SDLK_RETURN){

                    if(current_state->victory == -1){

                        while(current_state->previous_state != NULL){

                            current_state = undo_move(current_state);
                        }

                    }else{

                        return_state = GAMESTATE_MENU;
                        running = false;
                    }

                }else if(key == SDLK_TAB){

                    return_state = GAMESTATE_MENU;
                    running = false;
                }
            }
        }

        if(current_state->victory == 0){

            if(player_move != NOTHING){

                awaiting_follow_input = false;
            }

            // Update
            if(player_move == PLAYER_MOVE_UNDO){

                current_state = undo_move(current_state);

            }else if(player_move != NOTHING){

                handle_move(current_state, player_move);
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_state(renderer, current_state);

        char fps_text[10];
        sprintf(fps_text, "FPS: %i", fps);
        render_text(renderer, font_small, fps_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 0);

        char bread_text[20];
        sprintf(bread_text, "Bread: %i / %i", current_state->player_bread_count, current_state->required_bread);
        render_text(renderer, font_small, bread_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 10);

        if(current_state->victory == 1){

            char victory_text[10] = "Success!";
            render_text(renderer, font_large, victory_text, (SDL_Color){ .r = 0, .g = 255, .b = 0, .a = 255 }, -1, -1);

        }else if(current_state->victory == -1){

            char failure_text[10] = "Failure!";
            render_text(renderer, font_large, failure_text, (SDL_Color){ .r = 255, .g = 0, .b = 0, .a = 255 }, -1, -1);
        }

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

    // Cleanup memory
    while(current_state->previous_state != NULL){

        current_state = undo_move(current_state);
    }
    free(current_state);
    current_state = NULL;

    // Close fonts
    TTF_CloseFont(font_small);
    TTF_CloseFont(font_large);

    return return_state;
}

void render_state(SDL_Renderer* renderer, State* current_state){

    // Render player
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect player_rect = (SDL_Rect){.x = current_state->player_x * TILE_WIDTH, .y = current_state->player_y * TILE_WIDTH, .w = TILE_WIDTH, .h = TILE_HEIGHT};
    SDL_RenderFillRect(renderer, &player_rect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for(int i = 0; i < MAX_DUCK_COUNT; i++){

        if(current_state->duckling_x[i] != -1){

            SDL_Rect duckling_rect = (SDL_Rect){ .x = current_state->duckling_x[i] * TILE_WIDTH, .y = current_state->duckling_y[i] * TILE_HEIGHT, .w = TILE_WIDTH, .h = TILE_HEIGHT };
            SDL_RenderFillRect(renderer, &duckling_rect);
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for(int i = 0; i < MAX_BREAD_COUNT; i++){

        if(current_state->bread_x[i] != -1){

            SDL_Rect bread_rect = (SDL_Rect){ .x = current_state->bread_x[i] * TILE_WIDTH, .y = current_state->bread_y[i] * TILE_HEIGHT, .w = TILE_WIDTH, .h = TILE_HEIGHT };
            SDL_RenderFillRect(renderer, &bread_rect);
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    for(int i = 0; i < MAX_GOOSE_COUNT; i++){

        if(current_state->goose_x[i] != -1){

            SDL_Rect goose_rect = (SDL_Rect){ .x = current_state->goose_x[i] * TILE_WIDTH, .y = current_state->goose_y[i] * TILE_HEIGHT, .w = TILE_WIDTH, .h = TILE_HEIGHT };
            SDL_RenderFillRect(renderer, &goose_rect);
        }
    }
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

int edit_loop(SDL_Renderer* renderer, char* filename){

    TTF_Font* font_small = TTF_OpenFont("./res/notosans.ttf", 10);

    if(font_small == NULL){

        printf("Unable to load font_small! %s\n", TTF_GetError());
        return GAMESTATE_EXIT;
    }

    // Start game loop
    const unsigned long SECOND = 1000;
    const unsigned long TARGET_FPS = 60;
    const unsigned long FRAME_TIME = (unsigned long)(SECOND / (double)TARGET_FPS);
    unsigned long second_before_time = SDL_GetTicks();
    unsigned long frame_before_time = second_before_time;
    unsigned long current_time;
    int frames = 0;
    int fps = 0;
    bool running = true;
    int return_state = GAMESTATE_MENU;

    State* current_state = NULL;

    char filepath[256];
    sprintf(filepath, "./puzzles/%s", filename);
    FILE* file = fopen(filepath, "r");
    if(file){

        current_state = get_empty_state();
        fclose(file);

    }else{

        current_state = get_empty_state();
    }

    const int EDIT_HELP = 0;
    const int EDIT_PLAYER = 1;
    const int EDIT_DUCK = 2;
    const int EDIT_BREAD = 3;
    const int EDIT_GOOSE = 4;
    const int EDIT_ERASE = 5;
    const int EDIT_SAVE = 6;
    int editor_mode = EDIT_HELP;

    int mouse_x = 0;
    int mouse_y = 0;

    while(running){

        // Poll events
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0){

            if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)){

                return_state = GAMESTATE_EXIT;
                running = false;

            }else if(e.type == SDL_KEYDOWN){

                int key = e.key.keysym.sym;
                if(editor_mode == EDIT_SAVE){

                    if(key == SDLK_y){

                        editor_save_puzzle(current_state, filename);
                        running = false;

                    }else if(key == SDLK_n){

                        editor_mode = EDIT_HELP;
                    }
                    continue;
                }

                if(key == SDLK_h){

                    editor_mode = EDIT_HELP;

                }else if(key == SDLK_p){

                    editor_mode = EDIT_PLAYER;

                }else if(key == SDLK_d){

                    editor_mode = EDIT_DUCK;

                }else if(key == SDLK_b){

                    editor_mode = EDIT_BREAD;

                }else if(key == SDLK_g){

                    editor_mode =  EDIT_GOOSE;

                }else if(key == SDLK_e){

                    editor_mode = EDIT_ERASE;

                }else if(key == SDLK_s){

                    editor_mode = EDIT_SAVE;
                }

            }else if(e.type == SDL_MOUSEMOTION){

                int x, y;
                SDL_GetMouseState(&x, &y);
                mouse_x = (int)(x / (double)TILE_WIDTH);
                mouse_y = (int)(y / (double)TILE_HEIGHT);
                if(mouse_x >= current_state->map_width){

                    mouse_x = current_state->map_width - 1;
                }
                if(mouse_y >= current_state->map_height){

                    mouse_y = current_state->map_height - 1;
                }

            }else if(e.type == SDL_MOUSEBUTTONDOWN){

                int x, y;
                SDL_GetMouseState(&x, &y);
                mouse_x = (int)(x / (double)TILE_WIDTH);
                mouse_y = (int)(y / (double)TILE_HEIGHT);
                if(mouse_x >= current_state->map_width){

                    mouse_x = current_state->map_width - 1;
                }
                if(mouse_y >= current_state->map_height){

                    mouse_y = current_state->map_height - 1;
                }

                if(editor_mode == EDIT_PLAYER){

                    if(!square_occupied(current_state, mouse_x, mouse_y)){

                        current_state->player_x = mouse_x;
                        current_state->player_y = mouse_y;
                    }

                }else if(editor_mode == EDIT_DUCK){

                    if(!square_occupied(current_state, mouse_x, mouse_y)){

                        for(int i = 0; i < MAX_DUCK_COUNT; i++){

                            if(current_state->duckling_x[i] == -1){

                                current_state->duckling_x[i] = mouse_x;
                                current_state->duckling_y[i] = mouse_y;
                                break;
                            }
                        }
                    }

                }else if(editor_mode == EDIT_BREAD){

                    if(!square_occupied(current_state, mouse_x, mouse_y)){

                        for(int i = 0; i < MAX_BREAD_COUNT; i++){

                            if(current_state->bread_x[i] == -1){

                                current_state->bread_x[i] = mouse_x;
                                current_state->bread_y[i] = mouse_y;
                                break;
                            }
                        }
                    }

                }else if(editor_mode == EDIT_GOOSE){

                    if(!square_occupied(current_state, mouse_x, mouse_y)){

                        for(int i = 0; i < MAX_GOOSE_COUNT; i++){

                            if(current_state->goose_x[i] == -1){

                                current_state->goose_x[i] = mouse_x;
                                current_state->goose_y[i] = mouse_y;
                                break;
                            }
                        }
                    }

                }else if(editor_mode == EDIT_ERASE){

                    editor_erase_at(current_state, mouse_x, mouse_y);
                }
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_state(renderer, current_state);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect cursor_rect = { .x = mouse_x * TILE_WIDTH, .y = mouse_y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT };
        SDL_RenderFillRect(renderer, &cursor_rect);

        if(editor_mode == EDIT_HELP){

            render_text(renderer, font_small, "Welcome to the editor!", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 0);
            render_text(renderer, font_small, "Press H to open this help prompt", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 10);
            render_text(renderer, font_small, "Press P to place the player", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 20);
            render_text(renderer, font_small, "Press D to place ducklings", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 30);
            render_text(renderer, font_small, "Press G to place geese", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 40);
            render_text(renderer, font_small, "Press B to place bread", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 50);
            render_text(renderer, font_small, "Press E to enter erase mode", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 60);
            render_text(renderer, font_small, "Press S to save", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 70);

        }else if(editor_mode == EDIT_PLAYER){

            render_text(renderer, font_small, "Player edit mode", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 0);

        }else if(editor_mode == EDIT_DUCK){

            char mode_text[128];
            sprintf(mode_text, "Duckling edit mode %i / %i", get_duckling_count(current_state), MAX_DUCK_COUNT);
            render_text(renderer, font_small, mode_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 0);

        }else if(editor_mode == EDIT_BREAD){

            char mode_text[128];
            sprintf(mode_text, "Bread edit mode %i / %i", get_bread_count(current_state), MAX_BREAD_COUNT);
            render_text(renderer, font_small, mode_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 0);

        }else if(editor_mode == EDIT_GOOSE){

            char mode_text[128];
            sprintf(mode_text, "Goose edit mode %i / %i", get_goose_count(current_state), MAX_GOOSE_COUNT);
            render_text(renderer, font_small, mode_text, (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 0);

        }else if(editor_mode == EDIT_ERASE){

            render_text(renderer, font_small, "Erase mode", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 0);

        }else if(editor_mode == EDIT_SAVE){

            render_text(renderer, font_small, "Save and exit? [Y/n]", (SDL_Color){ .r = 255, .g = 255, .b = 255, .a = 255 }, 0, 0);
        }

        SDL_RenderPresent(renderer);
        frames++;

        current_time = SDL_GetTicks();

        if(current_time - second_before_time >= SECOND){

            fps = frames;
            frames = 0;
            second_before_time += SECOND;
        }

        if(current_time - frame_before_time < FRAME_TIME){

            unsigned long delay_time = FRAME_TIME - (current_time - frame_before_time);
            SDL_Delay(delay_time);
        }

        frame_before_time = SDL_GetTicks();
    }

    // Close fonts
    TTF_CloseFont(font_small);

    return return_state;
}
