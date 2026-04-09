#ifndef UI_H
#define UI_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "game_logic.h"
#include "records.h"

void draw_menu(ALLEGRO_FONT *font, ALLEGRO_BITMAP *bg, int w, int h, int selected);

void draw_game(ALLEGRO_FONT *font, Game *g);

void draw_leaderboard(ALLEGRO_FONT *font, ALLEGRO_BITMAP *bg, int w, int h, Record *list);

void draw_name_input(ALLEGRO_FONT *font, ALLEGRO_BITMAP *bg, int w, int h, const char *buf, int pos, int score, bool blink_on);

#endif