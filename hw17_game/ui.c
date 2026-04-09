#include "ui.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>

ALLEGRO_COLOR get_tile_color(int value)
{
    switch (value)
    {
        case 2:    return al_map_rgb(238, 228, 218);
        case 4:    return al_map_rgb(237, 224, 200);
        case 8:    return al_map_rgb(242, 177, 121);
        case 16:   return al_map_rgb(245, 149, 99);
        case 32:   return al_map_rgb(246, 124, 95);
        case 64:   return al_map_rgb(246, 94, 59);
        case 128:  return al_map_rgb(237, 207, 114);
        case 256:  return al_map_rgb(237, 207, 99);
        case 512:  return al_map_rgb(237, 207, 82);
        case 1024: return al_map_rgb(237, 207, 64);
        case 2048: return al_map_rgb(237, 207, 48);
        default:   return al_map_rgb(60, 58, 50);
    }
}

ALLEGRO_COLOR get_tile_text_color(int value)
{
    if (value <= 4)
        return al_map_rgb(119, 110, 101);
    return al_map_rgb(255, 255, 255);
}

void draw_menu(ALLEGRO_FONT *font, ALLEGRO_BITMAP *bg, int w, int h, int selected)
{
    al_draw_scaled_bitmap(bg, 0, 0, al_get_bitmap_width(bg), al_get_bitmap_height(bg), 0, 0, w, h, 0);

    const char *items[] = {"ИГРАТЬ", "РЕКОРДЫ", "ВЫХОД"};
    for (int i = 0; i < 3; i++)
    {
        ALLEGRO_COLOR col = (i == selected) ? al_map_rgb(255, 100, 0) : al_map_rgb(119, 110, 101);
        al_draw_text(font, col, w / 2, 250 + i * 60, ALLEGRO_ALIGN_CENTER, items[i]);
    }
}

void draw_game(ALLEGRO_FONT *font, Game *g)
{
    al_clear_to_color(al_map_rgb(250, 248, 239));
    char buf[32];
    sprintf(buf, "SCORE: %d", g->score);
    al_draw_text(font, al_map_rgb(119, 110, 101), 20, 30, 0, buf);

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
                sprintf(buf, "%d", value);
                al_draw_text(font, text_col, x + 50, y + 35, ALLEGRO_ALIGN_CENTER, buf);
            }
            else
            {
                al_draw_filled_rounded_rectangle(x, y, x + 100, y + 100, 5, 5, al_map_rgb(204, 192, 179));
            }
        }
    }
}

void draw_leaderboard(ALLEGRO_FONT *font, ALLEGRO_BITMAP *bg, int w, int h, Record *list)
{
    al_draw_scaled_bitmap(bg, 0, 0, al_get_bitmap_width(bg), al_get_bitmap_height(bg), 0, 0, w, h, 0);

    al_draw_text(font, al_map_rgb(80, 70, 60), w / 2, 50, ALLEGRO_ALIGN_CENTER, "TOP 10");
    for (int i = 0; i < MAX_RECORDS; i++)
    {
        char buf[64];
        sprintf(buf, "%d. %s - %d", i + 1, list[i].name, list[i].score);
        al_draw_text(font, al_map_rgb(60, 55, 50), w / 2, 100 + i * 35, ALLEGRO_ALIGN_CENTER, buf);
    }
}

void draw_name_input(ALLEGRO_FONT *font, ALLEGRO_BITMAP *bg, int w, int h, const char *buf, int pos, int score, bool blink_on)
{
    al_draw_scaled_bitmap(bg, 0, 0, al_get_bitmap_width(bg), al_get_bitmap_height(bg), 0, 0, w, h, 0);

    char text[64];
    sprintf(text, "SCORE: %d", score);
    al_draw_text(font, al_map_rgb(119, 110, 101), w / 2, 80, ALLEGRO_ALIGN_CENTER, text);
    al_draw_text(font, al_map_rgb(119, 110, 101), w / 2, 130, ALLEGRO_ALIGN_CENTER, "ENTER YOUR NAME:");

    al_draw_text(font, al_map_rgb(150, 140, 130), w / 2, 200, ALLEGRO_ALIGN_CENTER, "A-Z/SPACE: type letters");
    al_draw_text(font, al_map_rgb(150, 140, 130), w / 2, 235, ALLEGRO_ALIGN_CENTER, "BACKSPACE: erase, ENTER: save");
    al_draw_text(font, al_map_rgb(150, 140, 130), w / 2, 270, ALLEGRO_ALIGN_CENTER, "LEFT/RIGHT: move cursor");

    al_draw_rounded_rectangle(w / 2 - 110, 310, w / 2 + 110, 350, 5, 5, al_map_rgb(119, 110, 101), 2);

    int display_len = 10;
    for (int i = 0; i < display_len; i++)
    {
        char ch[2] = {buf[i] ? buf[i] : '_', '\0'};
        float x = w / 2 - 100 + i * 22;

        if (i == pos && blink_on)
        {
            al_draw_filled_rectangle(x - 2, 313, x + 16, 347, al_map_rgba(255, 100, 0, 80));
        }

        ALLEGRO_COLOR col = (i == pos) ? al_map_rgb(255, 100, 0) : al_map_rgb(50, 50, 50);
        al_draw_text(font, col, x, 318, 0, ch);
    }
}