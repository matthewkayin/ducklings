#include "game.h"

State* get_initial_state(){

    State* initial_state = (State*)malloc(sizeof(State));
    
    initial_state->victory = 0;
    initial_state->required_bread = 3;

    initial_state->player_x = 2;
    initial_state->player_y = 2;

    initial_state->player_last_duckling = -1;
    initial_state->player_bread_count = 0;

    for(int i = 0; i < MAX_DUCK_COUNT; i++){

        // A duckling with position -1, -1 is interpreted to be non-existant
        initial_state->duckling_x[i] = -1;
        initial_state->duckling_y[i] = -1;
        initial_state->duckling_follows[i] = i;
        initial_state->duckling_direction[i] = -1;
        initial_state->duckling_holds_bread[i] = false;
    }

    for(int i = 0; i < MAX_BREAD_COUNT; i++){

        // A bread with position -1, -1 is also non-existant or already collected
        initial_state->bread_x[i] = -1;
        initial_state->bread_y[i] = -1;
    }

    for(int i = 0; i < MAX_GOOSE_COUNT; i++){

        // As with all the other coords, -1 indicates non-existant
        initial_state->goose_x[i] = -1;
        initial_state->goose_y[i] = -1;
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
    initial_state->bread_x[1] = 7;
    initial_state->bread_y[1] = 6;
    initial_state->bread_x[2] = 8;
    initial_state->bread_y[2] = 6;

    initial_state->goose_x[0] = 19;
    initial_state->goose_y[0] = 8;

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

        bool move_allowed = !square_occupied(current_state, dest_x, dest_y) && square_in_bounds(current_state, dest_x, dest_y);
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

                    // If the duckling we added has bread, collect it
                    if(current_state->duckling_holds_bread[i]){

                        current_state->player_bread_count++;
                        current_state->duckling_holds_bread[i] = false;
                    }

                    added_duckling = true;
                    move_allowed = true;
                    break;
                }
            }
        }

        if(move_allowed){

            current_state->player_x += direction_array[direction_array_index][0];
            current_state->player_y += direction_array[direction_array_index][1];

            // Check if the player touched a bread
            for(int i = 0; i < MAX_BREAD_COUNT; i++){

                if(current_state->player_x == current_state->bread_x[i] && current_state->player_y == current_state->bread_y[i]){

                    current_state->player_bread_count++;
                    current_state->bread_x[i] = -1;
                    break;
                }
            }

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

        if(current_state->duckling_x[i] == -1 && current_state->duckling_y[i] == -1){

            continue;
        }

        if(current_state->duckling_follows[i] == i && current_state->duckling_direction[i] != -1){

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

    bool goose_got_bread = false;
    for(int i = 0; i < MAX_GOOSE_COUNT; i++){

        if(current_state->goose_x[i] != -1){

            goose_pathfind(current_state, i);
            
            // Once goose has moved, check if they got any bread
            for(int j = 0; j < MAX_BREAD_COUNT; j++){

                if(current_state->goose_x[i] == current_state->bread_x[j] && current_state->goose_y[i] == current_state->bread_y[j]){

                    current_state->bread_x[j] = -1;
                    goose_got_bread = true;
                    break;
                }
            }
        }
    }

    if(goose_got_bread){

        current_state->victory = -1;

    }else if(current_state->player_bread_count >= current_state->required_bread){

        current_state->victory = 1;
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

    for(int i = 0; i < MAX_GOOSE_COUNT; i++){

        if(square_x == current_state->goose_x[i] && square_y == current_state->goose_y[i]){

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

void goose_pathfind(State* current_state, int goose_index){

    typedef struct {

        int direction;
        int path_length;
        int x;
        int y;
        int score;
    } Node;

    int direction_vector[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

    // First let's find the goal
    int nearest_bread = -1;
    int nearest_bread_distance = -1;

    for(int i = 0; i < MAX_BREAD_COUNT; i++){

        if(current_state->bread_x[i] == -1){

            continue;
        }

        int bread_dist = abs(current_state->goose_x[goose_index] - current_state->bread_x[i]) + abs(current_state->goose_y[goose_index] - current_state->bread_y[i]);
        if(nearest_bread == -1){

            nearest_bread = i;
            nearest_bread_distance = bread_dist;

        }else if(bread_dist < nearest_bread_distance){

            nearest_bread = i;
            nearest_bread_distance = bread_dist;
        }
    }

    if(nearest_bread == -1){

        // Don't chase after non-existance bread
        return;
    }

    int goal_x = current_state->bread_x[nearest_bread];
    int goal_y = current_state->bread_y[nearest_bread];

    int frontier_capacity = 16;
    int frontier_size = 0;
    Node* frontier = (Node*)malloc(frontier_capacity * sizeof(Node));
    int explored_capacity = 16;
    int explored_size = 0;
    Node* explored = (Node*)malloc(explored_capacity * sizeof(Node));

    frontier[0] = (Node){ .direction = -1, .path_length = 0, .x = current_state->goose_x[goose_index], .y = current_state->goose_y[goose_index], .score = nearest_bread_distance };
    frontier_size++;

    while(true){

        // Check that the frontier isn't empty
        if(frontier_size == 0){

            printf("Pathfinding failed!\n");
            break;
        }

        // Find the smallest node in the frontier
        int smallest_index = 0;
        for(int i = 1; i < frontier_size; i++){

            if(frontier[i].score < frontier[smallest_index].score){

                smallest_index = i;
            }
        }

        // Remove it from the frontier
        Node smallest = frontier[smallest_index];
        for(int i = smallest_index + 1; i < frontier_size; i++){

            frontier[i - 1] = frontier[i];
        }
        frontier_size--;

        // Check if it's the solution
        if(smallest.x == goal_x && smallest.y == goal_y){

            // If it is, move goose one step along that path then exit
            current_state->goose_x[goose_index] += direction_vector[smallest.direction][0];
            current_state->goose_y[goose_index] += direction_vector[smallest.direction][1];
            break;
        }

        // Add it to explored
        if(explored_size == explored_capacity){

            explored_capacity *= 2;
            explored = (Node*)realloc(explored, explored_capacity * sizeof(Node));
        }
        explored_size++;
        explored[explored_size - 1] = smallest;

        // Expand out all possible paths based on the one we've chosen
        for(int direction = 0; direction < 4; direction++){

            int child_x = smallest.x + direction_vector[direction][0];
            int child_y = smallest.y + direction_vector[direction][1];

            // If the path leads to an invalid square, ignore it
            if(square_occupied(current_state, child_x, child_y) || !square_in_bounds(current_state, child_x, child_y)){

                continue;
            }

            // Create the child node
            int first_direction = smallest.direction;
            if(first_direction == -1){

                first_direction = direction;
            }
            int path_length = smallest.path_length + 1;
            int score = path_length + abs(child_x - goal_x) + abs(child_y - goal_y);
            Node child = (Node){ .direction = first_direction, .path_length = path_length, .x = child_x, .y = child_y, .score = score };

            // Ignore this child if in explored
            bool child_in_explored = false;
            for(int i = 0; i < explored_size; i++){

                if(child.x == explored[i].x && child.y == explored[i].y){

                    child_in_explored = true;
                    break;
                }
            }
            if(child_in_explored){

                continue;
            }

            // Ignore this child if in frontier
            bool child_in_frontier = false;
            int frontier_index = -1;
            for(int i = 0; i < frontier_size; i++){

                if(child.x == frontier[i].x && child.y == frontier[i].y){

                    child_in_frontier = true;
                    frontier_index = i;
                    break;
                }
            }
            if(child_in_frontier){

                // If the child is in frontier but with a smaller cost, replace the frontier version with the child
                if(child.score < frontier[frontier_index].score){

                    frontier[frontier_index] = child;
                }
                continue;
            }

            // Finally if child is neither in frontier nor explored, add it to the frontier
            if(frontier_size == frontier_capacity){

                frontier_capacity *= 2;
                frontier = (Node*)realloc(frontier, frontier_capacity * sizeof(Node));
            }
            frontier_size++;
            frontier[frontier_size - 1] = child;
        }
    }

    free(frontier);
    free(explored);
}
