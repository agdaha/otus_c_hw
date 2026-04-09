#include "game_logic.h"
#include <stdlib.h>
#include <string.h>

// Сдвиг, объединение и повторный сдвиг одной линии из 4 элементов
static bool process_line(int *a, int *b, int *c, int *d, int *score)
{
    int line[4] = {*a, *b, *c, *d};
    int temp[4] = {0};
    int pos = 0;
    bool changed = false;

    for (int i = 0; i < 4; i++)
    {
        if (line[i] != 0)
            temp[pos++] = line[i];
    }

    for (int i = 0; i < 3; i++)
    {
        if (temp[i] != 0 && temp[i] == temp[i + 1])
        {
            temp[i] *= 2;
            *score += temp[i];
            temp[i + 1] = 0;
        }
    }

    int final_l[4] = {0};
    pos = 0;
    for (int i = 0; i < 4; i++)
    {
        if (temp[i] != 0)
            final_l[pos++] = temp[i];
    }

    if (memcmp(line, final_l, sizeof(int) * 4) != 0)
    {
        changed = true;
        *a = final_l[0];
        *b = final_l[1];
        *c = final_l[2];
        *d = final_l[3];
    }
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
        g->board[empty[idx][0]][empty[idx][1]] = (rand() % 10 == 0) ? 4 : 2;
    }
}

bool move_up(Game *g)
{
    bool changed = false;
    for (int i = 0; i < 4; i++)
        if (process_line(&g->board[0][i], &g->board[1][i], &g->board[2][i], &g->board[3][i], &g->score))
            changed = true;
    return changed;
}

bool move_down(Game *g)
{
    bool changed = false;
    for (int i = 0; i < 4; i++)
        if (process_line(&g->board[3][i], &g->board[2][i], &g->board[1][i], &g->board[0][i], &g->score))
            changed = true;
    return changed;
}

bool move_left(Game *g)
{
    bool changed = false;
    for (int i = 0; i < 4; i++)
        if (process_line(&g->board[i][0], &g->board[i][1], &g->board[i][2], &g->board[i][3], &g->score))
            changed = true;
    return changed;
}

bool move_right(Game *g)
{
    bool changed = false;
    for (int i = 0; i < 4; i++)
        if (process_line(&g->board[i][3], &g->board[i][2], &g->board[i][1], &g->board[i][0], &g->score))
            changed = true;
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