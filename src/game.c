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
        initial_state->duckling_direction[i] = -1;
        initial_state->duckling_holds_bread[i] = false;
    }

    initial_state->player_last_duckling = -1;

    for(int i = 0; i < MAX_BREAD_COUNT; i++){

        // A bread with position -1, -1 is also non-existant or already collected
        initial_state->bread_x[i] = -1;
        initial_state->bread_y[i] = -1;
    }

    initial_state->map_width = 20;
    initial_state->map_height = 12;

    initial_state->duckling_x[0] = 4;
    initial_state->duckling_y[0] = 2;
    initial_state->duckling_x[1] = 4;
    initial_state->duckling_y[1] = 4;
    initial_state->duckling_x[2] = 2;
    initial_state->duckling_y[2] = 4;

    initial_state->bread_x[0] = 6;
    initial_state->bread_y[0] = 6;

    initial_state->previous_state = NULL;

    return initial_state;
}

void handle_move(State* current_state, int player_move){

    State* previous_state = (State*)malloc(sizeof(State));
    *previous_state = *current_state;
    current_state->previous_state = previous_state;

    int direction_array[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

    // Perform player action if called for
    if(player_move >= PLAYER_MOVE_UP && player_move <= PLAYER_MOVE_LEFT){

        // Move player
        int direction_array_index = player_move - PLAYER_MOVE_UP;

        int dest_x = current_state->player_x + direction_array[direction_array_index][0];
        int dest_y = current_state->player_y + direction_array[direction_array_index][1];

        bool move_allowed = !square_occupied(current_state, dest_x, dest_y);
        bool added_duckling = false;
        // If the square is occupied but it is occupied by a duck which is not in the list, then the move is still allowed
        if(!move_allowed){

            for(int i = 0; i < MAX_DUCK_COUNT; i++){

                if(current_state->duckling_x[i] == dest_x && current_state->duckling_y[i] == dest_y && current_state->duckling_follows[i] == i){

                    // Since player collides with idle duckling, add it to the list
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

                    current_state->player_last_duckling = i;

                    added_duckling = true;
                    move_allowed = true;
                    break;
                }
            }
        }

        if(move_allowed){

            current_state->player_x += direction_array[direction_array_index][0];
            current_state->player_y += direction_array[direction_array_index][1];

            // Now update all the ducklings in the list
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

    }else if(player_move >= PLAYER_WADDLE_UP && player_move <= PLAYER_WADDLE_LEFT){

        // Send duckling waddling
        // It is assumed that there is a duckling available because the input code should ensure this
        int direction_array_index = player_move - PLAYER_WADDLE_UP;

        // First check if the destination square of our waddler is available
        int dest_x = current_state->player_x + direction_array[direction_array_index][0];
        int dest_y = current_state->player_y + direction_array[direction_array_index][1];
        if(!square_occupied(current_state, dest_x, dest_y)){

            // Now since we currently have the head duckling's position saved, let's update each duckling up one
            int previous_index = -1;
            int current_index = current_state->player_last_duckling;
            while(current_index != -1){

                int follows_index = current_state->duckling_follows[current_index];
                if(follows_index == -1){

                    if(previous_index != -1){

                        // If there is another duckling in the list, make him point to the player after the waddling begins
                        current_state->duckling_follows[previous_index] = -1;

                    }else{

                        // If there are no other ducklings, than the player last index should be -1
                        current_state->player_last_duckling = -1;
                    }

                    current_state->duckling_x[current_index] = current_state->player_x;
                    current_state->duckling_y[current_index] = current_state->player_y;
                    current_state->duckling_follows[current_index] = current_index;
                    current_state->duckling_direction[current_index] = direction_array_index;

                    // This is the last duckling so call this to exit the loop
                    current_index = -1;

                }else{

                    current_state->duckling_x[current_index] = current_state->duckling_x[follows_index];
                    current_state->duckling_y[current_index] = current_state->duckling_y[follows_index];

                    previous_index = current_index;
                    current_index = follows_index;
                }
            }
        }
    }

    // If the ducklings are waddling, waddle them along
    for(int i = 0; i < MAX_DUCK_COUNT; i++){

        if(current_state->duckling_x[i] != -1 && current_state->duckling_follows[i] == i && current_state->duckling_direction[i] != -1){

            current_state->duckling_x[i] += direction_array[current_state->duckling_direction[i]][0];
            current_state->duckling_y[i] += direction_array[current_state->duckling_direction[i]][1];
        }
    }

    // Check and handle the ducklings for invalid movement
    for(int i = 0; i < MAX_DUCK_COUNT; i++){

        if(current_state->duckling_x[i] != -1 && current_state->duckling_follows[i] == i && current_state->duckling_direction[i] != -1){

            if(!square_in_bounds(current_state, current_state->duckling_x[i], current_state->duckling_y[i])){

                // Turn the direction to the opposite way. Think of this as moving the duckling direction 90 degrees clockwise twice,
                // using modulus to wrap around if we go above 360 degrees
                current_state->duckling_direction[i] = (current_state->duckling_direction[i] + 2) % 4;

                // And we have them move once in their opposite direction now that they've turned around
                current_state->duckling_x[i] += (2 * direction_array[current_state->duckling_direction[i]][0]);
                current_state->duckling_y[i] += (2 * direction_array[current_state->duckling_direction[i]][1]);

            }else if(current_state->duckling_x[i] == current_state->player_x && current_state->duckling_y[i] == current_state->player_y){

                // Duckling bumped into player, so undo its movement
                current_state->duckling_x[i] = previous_state->duckling_x[i];
                current_state->duckling_y[i] = previous_state->duckling_y[i];

            }else{

                // Does duckling bump into another of his kind?
                for(int j = 0; j < MAX_DUCK_COUNT; j++){

                    if(i == j || current_state->duckling_x[j] == -1){

                        continue;
                    }

                    if(current_state->duckling_x[i] == current_state->duckling_x[j] && current_state->duckling_y[i] == current_state->duckling_y[j]){

                        // Turn this duckling around
                        current_state->duckling_direction[i] = (current_state->duckling_direction[i] + 2) % 4;
                        current_state->duckling_x[i] += (2 * direction_array[current_state->duckling_direction[i]][0]);
                        current_state->duckling_y[i] += (2 * direction_array[current_state->duckling_direction[i]][1]);

                        // If the duckling we collided into is also a waddler, turn him around too
                        if(current_state->duckling_follows[j] == j && current_state->duckling_direction[j] != j){

                            current_state->duckling_direction[j] = (current_state->duckling_direction[j] + 2) % 4;
                            current_state->duckling_x[j] += (2 * direction_array[current_state->duckling_direction[j]][0]);
                            current_state->duckling_y[j] += (2 * direction_array[current_state->duckling_direction[j]][1]);
                        }
                    }
                }
            }
        }
    }

    // Check if a duckling has picked up bread
    for(int i = 0; i < MAX_DUCK_COUNT; i++){

        if(!current_state->duckling_holds_bread[i]){

            for(int j = 0; j < MAX_BREAD_COUNT; j++){

                if(current_state->bread_x[j] == current_state->duckling_x[i] && current_state->bread_y[j] == current_state->duckling_y[i]){

                    current_state->bread_x[j] = -1;
                    current_state->duckling_holds_bread[i] = true;
                }
            }
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

bool square_occupied(State* current_state, int square_x, int square_y){

    if(square_x == current_state->player_x && square_y == current_state->player_y){

        return true;
    }

    for(int i = 0; i < MAX_DUCK_COUNT; i++){

        if(square_x == current_state->duckling_x[i] && square_y == current_state->duckling_y[i]){

            return true;
        }
    }

    return false;
}

bool square_in_bounds(State* current_state, int square_x, int square_y){

    return square_x >= 0 && square_x < current_state->map_width && square_y >= 0 && square_y < current_state->map_height;
}

int get_ducklist_length(State* current_state){

    int length = 0;
    for(int i = 0; i < MAX_DUCK_COUNT; i++){

        if(current_state->duckling_x[i] != -1 && current_state->duckling_follows[i] != i){

            length++;
        }
    }

    return length;
}
