#include "game.h"

State* get_initial_state(){

    State* initial_state = (State*)malloc(sizeof(State));

    initial_state->player_x = 2;
    initial_state->player_y = 2;

    for(int i = 0; i < MAX_DUCK_COUNT; i++){

        // A duckling with position -1, -1 is interpreted to be non-existant
        initial_state->duckling_x[i] = -1;
        initial_state->duckling_y[i] = -1;
        initial_state->duckling_follows[i] = i;
    }

    initial_state->player_last_duckling = -1;

    initial_state->duckling_x[0] = 4;
    initial_state->duckling_y[0] = 2;
    initial_state->duckling_x[1] = 4;
    initial_state->duckling_y[1] = 4;
    initial_state->duckling_x[2] = 2;
    initial_state->duckling_y[2] = 4;

    initial_state->previous_state = NULL;

    return initial_state;
}

void handle_move(State* current_state, int player_move){

    State* previous_state = (State*)malloc(sizeof(State));
    *previous_state = *current_state;
    current_state->previous_state = previous_state;

    // Move player if called for
    bool player_moved = false;
    if(player_move == PLAYER_MOVE_UP){

        current_state->player_y--;
        player_moved = true;

    }else if(player_move == PLAYER_MOVE_RIGHT){

        current_state->player_x++;
        player_moved = true;

    }else if(player_move == PLAYER_MOVE_DOWN){

        current_state->player_y++;
        player_moved = true;

    }else if(player_move == PLAYER_MOVE_LEFT){

        current_state->player_x--;
        player_moved = true;
    }

    bool added_duckling = false;

    // Check player and duckling movement
    for(int i = 0; i < MAX_DUCK_COUNT; i++){

        if(current_state->player_x == current_state->duckling_x[i] && current_state->player_y == current_state->duckling_y[i]){

            // Undo player movement if player bumps into duckling in list
            if(current_state->duckling_follows[i] != i){

                current_state->player_x = previous_state->player_x;
                current_state->player_y = previous_state->player_y;
                player_moved = false;

            }else{

                // Otherwise add the duckling to the list
                int last_duckling = current_state->player_last_duckling;
                if(last_duckling == -1){

                    current_state->duckling_follows[i] = -1;
                    current_state->duckling_x[i] = previous_state->player_x;
                    current_state->duckling_y[i] = previous_state->player_y;

                }else{

                    current_state->duckling_follows[i] = last_duckling;
                    current_state->duckling_x[i] = previous_state->duckling_x[last_duckling];
                    current_state->duckling_y[i] = previous_state->duckling_y[last_duckling];
                }

                added_duckling = true;
                current_state->player_last_duckling = i;
            }
        }
    }

    // If player moved, move all the ducklings
    if(player_moved){ 

        int current_index = current_state->player_last_duckling;
        bool skip = added_duckling;
        while(current_index != -1){

            if(skip){

                skip = false;

            }else{

                int follows_index = current_state->duckling_follows[current_index];
                int follow_x = 0;
                int follow_y = 0;
                if(follows_index == -1){

                    follow_x = previous_state->player_x;
                    follow_y = previous_state->player_y;

                }else{

                    follow_x = current_state->duckling_x[follows_index];
                    follow_y = current_state->duckling_y[follows_index];
                }
                current_state->duckling_x[current_index] = follow_x;
                current_state->duckling_y[current_index] = follow_y;
            }

            current_index = current_state->duckling_follows[current_index];
        }
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
