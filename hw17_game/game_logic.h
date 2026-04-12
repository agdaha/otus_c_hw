#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>

#define GRID_SIZE 4

typedef struct
{
    int board[GRID_SIZE][GRID_SIZE];
    int score;
    bool game_over;
} Game;

void init_game(Game *g);

void add_random_tile(Game *g);

bool move_up(Game *g);

bool move_down(Game *g);

bool move_left(Game *g);

bool move_right(Game *g);

bool can_move(Game *g);

#endif