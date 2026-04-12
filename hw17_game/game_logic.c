#include "game_logic.h"
#include <stdlib.h>
#include <string.h>

// Сдвиг, объединение и повторный сдвиг одной линии
static bool process_line(int *line, int size, int *score)
{
    int *temp = calloc(size, sizeof(int));
    if (!temp) return false;

    int pos = 0;
    bool changed = false;

    for (int i = 0; i < size; i++)
    {
        if (line[i] != 0)
            temp[pos++] = line[i];
    }

    for (int i = 0; i < size - 1; i++)
    {
        if (temp[i] != 0 && temp[i] == temp[i + 1])
        {
            temp[i] *= 2;
            *score += temp[i];
            temp[i + 1] = 0;
        }
    }

    int *final_l = calloc(size, sizeof(int));
    if (!final_l) {
        free(temp);
        return false;
    }
    pos = 0;
    for (int i = 0; i < size; i++)
    {
        if (temp[i] != 0)
            final_l[pos++] = temp[i];
    }

    if (memcmp(line, final_l, sizeof(int) * size) != 0)
    {
        changed = true;
        memcpy(line, final_l, sizeof(int) * size);
    }

    free(temp);
    free(final_l);
    return changed;
}

void init_game(Game *g)
{
    memset(g->board, 0, sizeof(g->board));
    g->score = 0;
    add_random_tile(g);
    add_random_tile(g);
}

void add_random_tile(Game *g)
{
    int empty[16][2], count = 0;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            if (g->board[r][c] == 0)
            {
                empty[count][0] = r;
                empty[count][1] = c;
                count++;
            }

    if (count > 0)
    {
        int idx = rand() % count;
        int row = empty[idx][0];
        int col = empty[idx][1];
        int rnd_val =  (rand() % 10 == 0) ? 4 : 2;
        g->board[row][col] = rnd_val;
    }
}

bool move_up(Game *g)
{
    bool changed = false;
    int line[GRID_SIZE];

    for (int col = 0; col < GRID_SIZE; col++)
    {
        for (int row = 0; row < GRID_SIZE; row++)
            line[row] = g->board[row][col];

        if (process_line(line, GRID_SIZE, &g->score))
            changed = true;

        for (int row = 0; row < GRID_SIZE; row++)
            g->board[row][col] = line[row];
    }
    return changed;
}

bool move_down(Game *g)
{
    bool changed = false;
    int line[GRID_SIZE];

    for (int col = 0; col < GRID_SIZE; col++)
    {
        for (int row = 0; row < GRID_SIZE; row++)
            line[row] = g->board[GRID_SIZE - 1 - row][col];

        if (process_line(line, GRID_SIZE, &g->score))
            changed = true;

        for (int row = 0; row < GRID_SIZE; row++)
            g->board[GRID_SIZE - 1 - row][col] = line[row];
    }
    return changed;
}

bool move_left(Game *g)
{
    bool changed = false;
    int line[GRID_SIZE];

    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
            line[col] = g->board[row][col];

        if (process_line(line, GRID_SIZE, &g->score))
            changed = true;

        for (int col = 0; col < GRID_SIZE; col++)
            g->board[row][col] = line[col];
    }
    return changed;
}

bool move_right(Game *g)
{
    bool changed = false;
    int line[GRID_SIZE];

    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
            line[col] = g->board[row][GRID_SIZE - 1 - col];

        if (process_line(line, GRID_SIZE, &g->score))
            changed = true;

        for (int col = 0; col < GRID_SIZE; col++)
            g->board[row][GRID_SIZE - 1 - col] = line[col];
    }
    return changed;
}

bool can_move(Game *g)
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (g->board[r][c] == 0)
                return true;
            if (c < 3 && g->board[r][c] == g->board[r][c + 1])
                return true;
            if (r < 3 && g->board[r][c] == g->board[r + 1][c])
                return true;
        }
    }
    return false;
}