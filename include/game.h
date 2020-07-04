#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <stdio.h>

#define NOTHING 0
#define PLAYER_MOVE_UP 1
#define PLAYER_MOVE_RIGHT 2
#define PLAYER_MOVE_DOWN 3
#define PLAYER_MOVE_LEFT 4
#define PLAYER_MOVE_UNDO 5
#define PLAYER_WIDTH 32
#define PLAYER_HEIGHT 32

typedef struct State{

    int player_x;
    int player_y;
    struct State* previous_state;
} State;

State* get_initial_state();
void handle_move(State* current_state, int player_move);
State* undo_move(State* current_state);

#endif
