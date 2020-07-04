#include "game.h"

State* get_initial_state(){

    State* initial_state = (State*)malloc(sizeof(State));

    initial_state->player_x = 2;
    initial_state->player_y = 2;

    initial_state->previous_state = NULL;

    return initial_state;
}

void handle_move(State* current_state, int player_move){

    State* previous_state = (State*)malloc(sizeof(State));
    *previous_state = *current_state;
    current_state->previous_state = previous_state;

    if(player_move == PLAYER_MOVE_UP){

        current_state->player_y--;

    }else if(player_move == PLAYER_MOVE_RIGHT){

        current_state->player_x++;

    }else if(player_move == PLAYER_MOVE_DOWN){

        current_state->player_y++;

    }else if(player_move == PLAYER_MOVE_LEFT){

        current_state->player_x--;
    }
}

State* undo_move(State* current_state){

    if(current_state->previous_state == NULL){

        return current_state;
    }

    State* discarded_state = current_state;
    State* previous_state = current_state->previous_state;
    free(discarded_state);

    return previous_state;
}
