#ifndef UI_H
#define UI_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "game_logic.h"
#include "records.h"

// Цвета плиток
static const ALLEGRO_COLOR C_2    = { .r = 238/255.0f, .g = 228/255.0f, .b = 218/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_4    = { .r = 237/255.0f, .g = 224/255.0f, .b = 200/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_8    = { .r = 242/255.0f, .g = 177/255.0f, .b = 121/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_16   = { .r = 245/255.0f, .g = 149/255.0f, .b =  99/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_32   = { .r = 246/255.0f, .g = 124/255.0f, .b =  95/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_64   = { .r = 246/255.0f, .g =  94/255.0f, .b =  59/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_128  = { .r = 237/255.0f, .g = 207/255.0f, .b = 114/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_256  = { .r = 237/255.0f, .g = 207/255.0f, .b =  99/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_512  = { .r = 237/255.0f, .g = 207/255.0f, .b =  82/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_1024 = { .r = 237/255.0f, .g = 207/255.0f, .b =  64/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_2048 = { .r = 237/255.0f, .g = 207/255.0f, .b =  48/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_DEF  = { .r =  60/255.0f, .g =  58/255.0f, .b =  50/255.0f, .a = 1.0f };

// Цвета текста
static const ALLEGRO_COLOR C_TXTDK = { .r = 119/255.0f, .g = 110/255.0f, .b = 101/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_TXTLT = { .r = 255/255.0f, .g = 255/255.0f, .b = 255/255.0f, .a = 1.0f };

// Остальные цвета 
static const ALLEGRO_COLOR C_ACCENT= { .r = 255/255.0f, .g = 100/255.0f, .b =   0/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_BG    = { .r = 250/255.0f, .g = 248/255.0f, .b = 239/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_EMPTY = { .r = 204/255.0f, .g = 192/255.0f, .b = 179/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_HEAD  = { .r =  80/255.0f, .g =  70/255.0f, .b =  60/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_ROW   = { .r =  60/255.0f, .g =  55/255.0f, .b =  50/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_HINT  = { .r = 150/255.0f, .g = 140/255.0f, .b = 130/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_BRDR  = { .r = 119/255.0f, .g = 110/255.0f, .b = 101/255.0f, .a = 1.0f };
static const ALLEGRO_COLOR C_CRSR  = { .r = 255/255.0f, .g = 100/255.0f, .b =   0/255.0f, .a =  80/255.0f };
static const ALLEGRO_COLOR C_CHRLT = { .r =  50/255.0f, .g =  50/255.0f, .b =  50/255.0f, .a = 1.0f };

typedef struct
{
    ALLEGRO_FONT *font;
    ALLEGRO_BITMAP *bg;
} UiRes;

typedef struct
{
    int w;
    int h;
} UiDim;

void draw_menu(UiRes res, UiDim dim, int selected);

void draw_game(UiRes res, Game *g);

void draw_leaderboard(UiRes res, UiDim dim, Record *list);

void draw_name_input(UiRes res, UiDim dim, const char *buf, int pos, int score, bool blink_on);

#endif