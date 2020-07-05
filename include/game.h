#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <stdio.h>
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
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define MAX_DUCK_COUNT 16

typedef struct State{

    int player_x;
    int player_y;
    int player_last_duckling;
    int duckling_x[MAX_DUCK_COUNT];
    int duckling_y[MAX_DUCK_COUNT];
    int duckling_follows[MAX_DUCK_COUNT]; // self-index is noone, -1 is player, -2 is waddle, 0-15 is other duckling
    int duckling_direction[MAX_DUCK_COUNT]; // -1 is stand still, 0-3 are up right down left
    struct State* previous_state;
} State;

State* get_initial_state();
void handle_move(State* current_state, int player_move);
State* undo_move(State* current_state);
bool square_occupied(State* current_state, int square_x, int square_y);
int get_ducklist_length(State* current_state);

#endif
