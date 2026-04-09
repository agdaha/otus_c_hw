#include <stdio.h>
#include <time.h>
#include "game_logic.h"
#include "records.h"
#include "ui.h"

#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

// Размеры игрового окна
#define WIDTH 520
#define HEIGHT 650

// Перечисление состояний игрового автомата
typedef enum
{
    ST_MENU,
    ST_GAME,
    ST_TOPS,
    ST_NAME_INPUT,
    ST_EXIT
} State;

int main(void)
{
    srand(time(NULL));

    if (!al_init())
        return -1;
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_image_addon();

    ALLEGRO_DISPLAY *disp = al_create_display(WIDTH, HEIGHT);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);

    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    ALLEGRO_FONT *font = al_load_ttf_font("./assets/Dusha V5.ttf", 32, 0);
    if (!font)
        font = al_create_builtin_font();

    ALLEGRO_BITMAP *bg = al_load_bitmap("./assets/bg.png");
    if (!bg)
        bg = al_create_bitmap(WIDTH, HEIGHT);

    Game game;
    Record leaders[MAX_RECORDS];
    load_records(leaders);

    char name_buf[MAX_NAME_LEN];
    int name_pos = 0;
    int final_score = 0;
    ALLEGRO_TIMER *blink_timer = al_create_timer(1.0 / 2);
    bool blink_on = false;
    al_register_event_source(queue, al_get_timer_event_source(blink_timer));

    State state = ST_MENU;
    int menu_sel = 0;
    bool redraw = true;

    int win_w = WIDTH, win_h = HEIGHT;

    al_start_timer(timer);
    while (state != ST_EXIT)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            redraw = true;
            if (ev.timer.source == blink_timer)
                blink_on = !blink_on;
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            state = ST_EXIT;
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                state = ST_MENU;

            if (state == ST_MENU)
            {
                if (ev.keyboard.keycode == ALLEGRO_KEY_UP)
                    menu_sel = (menu_sel + 2) % 3;
                if (ev.keyboard.keycode == ALLEGRO_KEY_DOWN)
                    menu_sel = (menu_sel + 1) % 3;
                if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    if (menu_sel == 0)
                    {
                        init_game(&game);
                        state = ST_GAME;
                    }
                    else if (menu_sel == 1)
                        state = ST_TOPS;
                    else
                        state = ST_EXIT;
                }
            }
            else if (state == ST_GAME)
            {
                bool moved = false;
                if (ev.keyboard.keycode == ALLEGRO_KEY_UP)
                    moved = move_up(&game);
                if (ev.keyboard.keycode == ALLEGRO_KEY_DOWN)
                    moved = move_down(&game);
                if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT)
                    moved = move_left(&game);
                if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                    moved = move_right(&game);

                if (moved)
                {
                    add_random_tile(&game);
                    if (!can_move(&game))
                    {
                        final_score = game.score;
                        memset(name_buf, 0, sizeof(name_buf));
                        name_pos = 0;
                        al_start_timer(blink_timer);
                        state = ST_NAME_INPUT;
                    }
                }
            }
            else if (state == ST_NAME_INPUT)
            {
                if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    al_stop_timer(blink_timer);
                    if (name_buf[0] == 0 || name_buf[0] == '_')
                        add_record(leaders, "Player", final_score);
                    else
                        add_record(leaders, name_buf, final_score);
                    state = ST_TOPS;
                }
                else if (ev.keyboard.keycode == ALLEGRO_KEY_LEFT)
                {
                    if (name_pos > 0)
                        name_pos--;
                }
                else if (ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                {
                    if (name_pos < MAX_NAME_LEN - 2)
                        name_pos++;
                }
                else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
                {
                    if (name_pos > 0)
                    {
                        name_pos--;
                        name_buf[name_pos] = 0;
                    }
                }
                else if (ev.keyboard.keycode >= ALLEGRO_KEY_A && ev.keyboard.keycode <= ALLEGRO_KEY_Z)
                {
                    if (name_pos < MAX_NAME_LEN - 2)
                    {
                        char ch = (char)('A' + (ev.keyboard.keycode - ALLEGRO_KEY_A));
                        name_buf[name_pos] = ch;
                        name_pos++;
                    }
                }
                else if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE)
                {
                    if (name_pos < MAX_NAME_LEN - 2)
                    {
                        name_buf[name_pos] = ' ';
                        name_pos++;
                    }
                }
            }
        }

        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(250, 248, 239));
            if (state == ST_MENU)
                draw_menu(font, bg, win_w, win_h, menu_sel);
            else if (state == ST_GAME)
                draw_game(font, &game);
            else if (state == ST_TOPS)
                draw_leaderboard(font, bg, win_w, win_h, leaders);
            else if (state == ST_NAME_INPUT)
                draw_name_input(font, bg, win_w, win_h, name_buf, name_pos, final_score, blink_on);
            al_flip_display();
            redraw = false;
        }
    }

    al_destroy_bitmap(bg);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_timer(blink_timer);
    al_destroy_event_queue(queue);
    return 0;
}