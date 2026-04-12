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
} AppState;

// Структура, содержащая всё состояние игры
typedef struct
{
    AppState state;
    bool redraw;

    int menu_sel;

    Game game;
    int final_score;

    // Переменные диалога ввода имени
    char name_buf[MAX_NAME_LEN];
    int name_pos;
    bool blink_on;
    ALLEGRO_TIMER *blink_timer;

    // Таблица рекордов
    Record leaders[MAX_RECORDS];

    // Ресурсы Allegro
    ALLEGRO_DISPLAY *display;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_TIMER *timer;
    ALLEGRO_FONT *font;
    ALLEGRO_BITMAP *bg;

    int win_w;
    int win_h;
} GameState;


static void handle_menu_event(ALLEGRO_EVENT *ev, GameState *gs)
{
    if (ev->keyboard.keycode == ALLEGRO_KEY_UP)
        gs->menu_sel = (gs->menu_sel + 2) % 3;
    else if (ev->keyboard.keycode == ALLEGRO_KEY_DOWN)
        gs->menu_sel = (gs->menu_sel + 1) % 3;
    else if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER)
    {
        if (gs->menu_sel == 0)
        {
            init_game(&gs->game);
            gs->state = ST_GAME;
        }
        else if (gs->menu_sel == 1)
            gs->state = ST_TOPS;
        else
            gs->state = ST_EXIT;
    }
}

static void handle_game_event(ALLEGRO_EVENT *ev, GameState *gs)
{
    bool moved = false;
    if (ev->keyboard.keycode == ALLEGRO_KEY_UP)
        moved = move_up(&gs->game);
    else if (ev->keyboard.keycode == ALLEGRO_KEY_DOWN)
        moved = move_down(&gs->game);
    else if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)
        moved = move_left(&gs->game);
    else if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)
        moved = move_right(&gs->game);

    if (moved)
    {
        add_random_tile(&gs->game);
        if (!can_move(&gs->game))
        {
            gs->final_score = gs->game.score;
            memset(gs->name_buf, 0, MAX_NAME_LEN);
            gs->name_pos = 0;
            gs->blink_on = false;
            al_start_timer(gs->blink_timer);
            gs->state = ST_NAME_INPUT;
        }
    }
}

static void handle_name_input_event(ALLEGRO_EVENT *ev, GameState *gs)
{
    if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER)
    {
        al_stop_timer(gs->blink_timer);
        if (gs->name_buf[0] == 0 || gs->name_buf[0] == '_')
            add_record(gs->leaders, "Player", gs->final_score);
        else
            add_record(gs->leaders, gs->name_buf, gs->final_score);
        gs->state = ST_TOPS;
    }
    else if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)
    {
        if (gs->name_pos > 0)
            (gs->name_pos)--;
    }
    else if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)
    {
        if (gs->name_pos < MAX_NAME_LEN - 2)
            (gs->name_pos)++;
    }
    else if (ev->keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
    {
        if (gs->name_pos > 0)
        {
            (gs->name_pos)--;
            gs->name_buf[gs->name_pos] = 0;
        }
    }
    else if (ev->keyboard.keycode >= ALLEGRO_KEY_A && ev->keyboard.keycode <= ALLEGRO_KEY_Z)
    {
        if (gs->name_pos < MAX_NAME_LEN - 2)
        {
            char ch = (char)('A' + (ev->keyboard.keycode - ALLEGRO_KEY_A));
            gs->name_buf[gs->name_pos] = ch;
            (gs->name_pos)++;
        }
    }
    else if (ev->keyboard.keycode == ALLEGRO_KEY_SPACE)
    {
        if (gs->name_pos < MAX_NAME_LEN - 2)
        {
            gs->name_buf[gs->name_pos] = ' ';
            (gs->name_pos)++;
        }
    }
}

static void handle_event(ALLEGRO_EVENT *ev, GameState *gs)
{
    if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
    {
        gs->state = ST_MENU;
        return;
    }

    switch (gs->state)
    {
    case ST_MENU:
        handle_menu_event(ev, gs);
        break;
    case ST_GAME:
        handle_game_event(ev, gs);
        break;
    case ST_NAME_INPUT:
        handle_name_input_event(ev, gs);
        break;
    case ST_TOPS:
    case ST_EXIT:
        break;
    }
}

static void handle_timer_event(ALLEGRO_EVENT *ev, GameState *gs)
{
    if (ev->timer.source == gs->blink_timer)
        gs->blink_on = !gs->blink_on;
}

static void process_events(GameState *gs)
{
    ALLEGRO_EVENT ev;
    al_wait_for_event(gs->queue, &ev);

    if (ev.type == ALLEGRO_EVENT_TIMER)
    {
        gs->redraw = true;
        handle_timer_event(&ev, gs);
    }
    else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
    {
        gs->state = ST_EXIT;
    }
    else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
    {
        handle_event(&ev, gs);
    }
}

static void render(GameState *gs)
{
    UiRes res = {gs->font, gs->bg};
    UiDim dim = {gs->win_w, gs->win_h};

    switch (gs->state)
    {
    case ST_MENU:
        draw_menu(res, dim, gs->menu_sel);
        break;
    case ST_GAME:
        draw_game(res, &gs->game);
        break;
    case ST_TOPS:
        draw_leaderboard(res, dim, gs->leaders);
        break;
    case ST_NAME_INPUT:
        draw_name_input(res, dim, gs->name_buf, gs->name_pos, gs->final_score, gs->blink_on);
        break;
    case ST_EXIT:
        break;
    }
}

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

    GameState gs = {0};

    gs.display = al_create_display(WIDTH, HEIGHT);
    gs.queue = al_create_event_queue();
    gs.timer = al_create_timer(1.0 / 60);
    gs.blink_timer = al_create_timer(1.0 / 2);

    al_register_event_source(gs.queue, al_get_display_event_source(gs.display));
    al_register_event_source(gs.queue, al_get_keyboard_event_source());
    al_register_event_source(gs.queue, al_get_timer_event_source(gs.timer));
    al_register_event_source(gs.queue, al_get_timer_event_source(gs.blink_timer));

    gs.font = al_load_ttf_font("./assets/Dusha V5.ttf", 32, 0);
    if (!gs.font)
        gs.font = al_create_builtin_font();

    gs.bg = al_load_bitmap("./assets/bg.png");
    if (!gs.bg)
        gs.bg = al_create_bitmap(WIDTH, HEIGHT);

    load_records(gs.leaders);

    gs.win_w = WIDTH;
    gs.win_h = HEIGHT;
    gs.state = ST_MENU;
    gs.redraw = true;

    al_start_timer(gs.timer);
    while (gs.state != ST_EXIT)
    {
        process_events(&gs);

        if (gs.redraw && al_is_event_queue_empty(gs.queue))
        {
            al_clear_to_color(al_map_rgb(250, 248, 239));
            render(&gs);
            al_flip_display();
            gs.redraw = false;
        }
    }

    al_destroy_bitmap(gs.bg);
    al_destroy_font(gs.font);
    al_destroy_display(gs.display);
    al_destroy_timer(gs.timer);
    al_destroy_timer(gs.blink_timer);
    al_destroy_event_queue(gs.queue);
    return 0;
}