#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define NOTHING 0
#define PLAYER_MOVE_UP 1
#define PLAYER_MOVE_RIGHT 2
#define PLAYER_MOVE_DOWN 3
#define PLAYER_MOVE_LEFT 4
#define PLAYER_MOVE_UNDO 5
#define PLAYER_WADDLE_UP 6
#define PLAYER_WADDLE_RIGHT 7
#define PLAYER_WADDLE_DOWN 8
#define PLAYER_WADDLE_LEFT 9
#define PLAYER_MOVE_WAIT 10
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define MAX_DUCK_COUNT 16
#define MAX_BREAD_COUNT 16
#define MAX_GOOSE_COUNT 16

typedef struct State{

    int victory;
    int required_bread;

    int player_x;
    int player_y;
    int player_direction;
    int player_last_duckling;
    int player_bread_count;

    int duckling_x[MAX_DUCK_COUNT];
    int duckling_y[MAX_DUCK_COUNT];
    int duckling_follows[MAX_DUCK_COUNT]; // self-index is noone, -1 is player, 0-15 is other duckling
    int duckling_direction[MAX_DUCK_COUNT]; // -1 is stand still, 0-3 are up right down left
    bool duckling_waddles[MAX_DUCK_COUNT];
    bool duckling_holds_bread[MAX_DUCK_COUNT];

    int bread_x[MAX_BREAD_COUNT];
    int bread_y[MAX_BREAD_COUNT];

    int goose_x[MAX_GOOSE_COUNT];
    int goose_y[MAX_GOOSE_COUNT];
    int goose_direction[MAX_GOOSE_COUNT];

    int map_width;
    int map_height;

    struct State* previous_state;
} State;

State* get_empty_state();
void handle_move(State* current_state, int player_move);
State* undo_move(State* current_state);
bool square_occupied(State* current_state, int square_x, int square_y);
bool square_in_bounds(State* current_state, int square_x, int square_y);
int get_ducklist_length(State* current_state);
void goose_pathfind(State* current_state, int goose_index);

void editor_erase_at(State* current_state, int square_x, int square_y);
void editor_save_puzzle(State* current_state, char* filename);
State* get_from_file(char* filename);
int get_duckling_count(State* current_state);
int get_bread_count(State* current_state);
int get_goose_count(State* current_state);

#endif
