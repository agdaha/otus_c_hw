#include "ui.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>

ALLEGRO_COLOR get_tile_color(int value)
{
    switch (value)
    {
    case 2:    return C_2;
    case 4:    return C_4;
    case 8:    return C_8;
    case 16:   return C_16;
    case 32:   return C_32;
    case 64:   return C_64;
    case 128:  return C_128;
    case 256:  return C_256;
    case 512:  return C_512;
    case 1024: return C_1024;
    case 2048: return C_2048;
    default:   return C_DEF;
    }
}

ALLEGRO_COLOR get_tile_text_color(int value)
{
    if (value <= 4)
        return C_TXTDK;
    return C_TXTLT;
}

void draw_menu(UiRes res, UiDim dim, int selected)
{
    al_draw_scaled_bitmap(res.bg, 0, 0, al_get_bitmap_width(res.bg), al_get_bitmap_height(res.bg), 0, 0, dim.w, dim.h, 0);

    const char *items[] = {"ИГРАТЬ", "РЕКОРДЫ", "ВЫХОД"};
    for (int i = 0; i < 3; i++)
    {
        ALLEGRO_COLOR col = (i == selected) ? C_ACCENT : C_TXTDK;
        al_draw_text(res.font, col, dim.w / 2, 250 + i * 60, ALLEGRO_ALIGN_CENTER, items[i]);
    }
}

void draw_game(UiRes res, Game *g)
{
    al_clear_to_color(C_BG);
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE: %d", g->score);
    al_draw_text(res.font, C_TXTDK, 20, 30, 0, buf);

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            float x = 50 + c * 110;
            float y = 150 + r * 110;
            int value = g->board[r][c];

            if (value > 0)
            {
                ALLEGRO_COLOR bg_col = get_tile_color(value);
                ALLEGRO_COLOR text_col = get_tile_text_color(value);
                al_draw_filled_rounded_rectangle(x, y, x + 100, y + 100, 5, 5, bg_col);
                snprintf(buf, sizeof(buf), "%d", value);
                al_draw_text(res.font, text_col, x + 50, y + 35, ALLEGRO_ALIGN_CENTER, buf);
            }
            else
            {
                al_draw_filled_rounded_rectangle(x, y, x + 100, y + 100, 5, 5, C_EMPTY);
            }
        }
    }
}

void draw_leaderboard(UiRes res, UiDim dim, Record *list)
{
    al_draw_scaled_bitmap(res.bg, 0, 0, al_get_bitmap_width(res.bg), al_get_bitmap_height(res.bg), 0, 0, dim.w, dim.h, 0);

    al_draw_text(res.font, C_HEAD, dim.w / 2, 50, ALLEGRO_ALIGN_CENTER, "TOP 10");
    for (int i = 0; i < MAX_RECORDS; i++)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d. %s - %d", i + 1, list[i].name, list[i].score);
        al_draw_text(res.font, C_ROW, dim.w / 2, 100 + i * 35, ALLEGRO_ALIGN_CENTER, buf);
    }
}

void draw_name_input(UiRes res, UiDim dim, const char *buf, int pos, int score, bool blink_on)
{
    al_draw_scaled_bitmap(res.bg, 0, 0, al_get_bitmap_width(res.bg), al_get_bitmap_height(res.bg), 0, 0, dim.w, dim.h, 0);

    char text[64];
    snprintf(text, sizeof(text), "SCORE: %d", score);
    al_draw_text(res.font, C_TXTDK, dim.w / 2, 80, ALLEGRO_ALIGN_CENTER, text);
    al_draw_text(res.font, C_TXTDK, dim.w / 2, 130, ALLEGRO_ALIGN_CENTER, "ENTER YOUR NAME:");

    al_draw_text(res.font, C_HINT, dim.w / 2, 200, ALLEGRO_ALIGN_CENTER, "A-Z/SPACE: type letters");
    al_draw_text(res.font, C_HINT, dim.w / 2, 235, ALLEGRO_ALIGN_CENTER, "BACKSPACE: erase, ENTER: save");
    al_draw_text(res.font, C_HINT, dim.w / 2, 270, ALLEGRO_ALIGN_CENTER, "LEFT/RIGHT: move cursor");

    al_draw_rounded_rectangle(dim.w / 2 - 110, 310, dim.w / 2 + 110, 350, 5, 5, C_BRDR, 2);

    int display_len = 10;
    for (int i = 0; i < display_len; i++)
    {
        char ch[2] = {buf[i] ? buf[i] : '_', '\0'};
        float x = dim.w / 2 - 100 + i * 22;

        if (i == pos && blink_on)
        {
            al_draw_filled_rectangle(x - 2, 313, x + 16, 347, C_CRSR);
        }

        ALLEGRO_COLOR col = (i == pos) ? C_ACCENT : C_CHRLT;
        al_draw_text(res.font, col, x, 318, 0, ch);
    }
}