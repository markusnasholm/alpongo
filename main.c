#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#define FPS 60.0

#define SCREEN_W 1200
#define SCREEN_H 900

#define R_SPACING 30
#define BALL_SIZE 10
#define BALL_MAX_dy 12

#define KEY_SEEN     1
#define KEY_RELEASED 2

typedef struct racket {
    int x,y, w,h;
    int dx,dy;
    bool human;
} racket;

typedef struct ball {
    int x,y;
    int dx,dy;
} ball;

void must_init(bool test, const char *description)
{
    if(test) return;

    printf("couldn't initialize %s\n", description);
    exit(1);
}

int bmove(ball* b, racket* ra[2])
{
    if (b->y < 0 || b->y + BALL_SIZE > SCREEN_H) {
        b->dy *= -1;
    }
    if (b->x + BALL_SIZE < 0) {
        return 2;
    }
    if (b->x > SCREEN_W) {
        return 1;
    }

    for (int i = 0; i < 2; i++) {
        int x1 = ra[i]->x;
        int y1 = ra[i]->y;
        int x2 = ra[i]->x + ra[i]->w;
        int y2 = ra[i]->y + ra[i]->h;
        int m = ra[i]->y + (ra[i]->h / 2);

        int bx1 = b->x;
        int by1 = b->y;
        int bx2 = b->x + BALL_SIZE;
        int by2 = b->y + BALL_SIZE;

        if ( (bx1 >= x1 && bx1 <= x2 && by1 >= y1 && by1 <= y2)
                || (bx2 >= x1 && bx2 <= x2 && by1 >= y1 && by1 <= y2) 
                || (bx1 >= x1 && bx1 <= x2 && by2 >= y1 && by2 <= y2) 
                || (bx2 >= x1 && bx2 <= x2 && by1 >= y2 && by2 <= y2) 
                ) {
            if (b->dx < 0)
                b->x = x2;
            if (b->dx > 0)
                b->x = x1 - BALL_SIZE;
            b->dx *= -1;
            // needs work:
            b->dy += ((b->y - m) / 20) + (ra[i]->dy / 2);
        }
    }
    
    if (b->dy > BALL_MAX_dy)
        b->dy = BALL_MAX_dy;
    if (b->dy < -BALL_MAX_dy)
        b->dy = -BALL_MAX_dy;

    b->x += b->dx;
    b->y += b->dy;
    return 0;
}

void rAI(racket* r, ball b)
{
    int m = r->y + (r->h / 2);
    int r4th = r->h / 4;
    if (b.y > m+r4th && r->dy <= 10)
        r->dy += 1;
    else if (b.y < m-r4th && r->dy >= -10)
        r->dy -= 1;
    else if (b.dy >= -6 && b.dy <= 6)
        r->dy = b.dy;
}

void rmove(racket* ra[2])
{
    for (int i = 0; i < 2; i++) {
        if (ra[i]->y + (ra[i]->h / 2) < 0) {
            ra[i]->y = 0 - (ra[i]->h / 2);
            ra[i]->dy = 0;
        }
        else if (ra[i]->y + (ra[i]->h / 2) > SCREEN_H) {
            ra[i]->y = SCREEN_H - (ra[i]->h / 2);
            ra[i]->dy = 0;
        }
        ra[i]->y += ra[i]->dy;
    }
}

int main()
{
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

    ALLEGRO_DISPLAY* disp = al_create_display(SCREEN_W, SCREEN_H);
    must_init(disp, "display");

    ALLEGRO_FONT* font = al_create_builtin_font();
    must_init(font, "font");

    must_init(al_init_primitives_addon(), "primitives");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;
    
    // key handling init
    unsigned char key[ALLEGRO_KEY_MAX];
    memset(key, 0, sizeof(key));

    racket r0 = {0,0, 10, SCREEN_H/6, 0,0, true};
    r0.x = R_SPACING;
    r0.y = (SCREEN_H / 2) - (r0.h / 2);

    racket r1 = {0,0, 10, SCREEN_H/6, 0,0, false};
    r1.x = SCREEN_W - (r1.w + R_SPACING);
    r1.y = (SCREEN_H / 2) - (r1.h / 2);

    ball b = {(SCREEN_W / 2) - (BALL_SIZE / 2),(SCREEN_H / 2) - (BALL_SIZE / 2),-6,0};

    racket* rarr[2];
    rarr[0] = &r0;
    rarr[1] = &r1;

    al_start_timer(timer);
    while (true) {
        al_wait_for_event(queue, &event);

        switch(event.type)
        {
            case ALLEGRO_EVENT_TIMER:
                if (r1.human) {
                    if (key[ALLEGRO_KEY_UP]) {
                        r1.dy -= 1;
                    }
                    else if (key[ALLEGRO_KEY_DOWN]) {
                        r1.dy += 1;
                    }
                    else {
                        r1.dy = r1.dy * 0.95;
                    }
                }
                else
                    rAI(&r1, b);

                if (r0.human) {
                    if(key[ALLEGRO_KEY_W]) {
                        r0.dy -= 1;
                    }
                    else if (key[ALLEGRO_KEY_S]) {
                        r0.dy += 1;
                    }
                    else {
                        r0.dy = r0.dy * 0.95;
                    }
                }
                else
                    rAI(&r1, b);

                for(int i = 0; i < ALLEGRO_KEY_MAX; i++)
                    key[i] &= KEY_SEEN;

                rmove(rarr);
                if (bmove(&b, rarr))
                    done = true;

                redraw = true;
                break;

            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                done = true;
                break;

            case ALLEGRO_EVENT_KEY_DOWN:
                key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
                break;
            case ALLEGRO_EVENT_KEY_UP:
                key[event.keyboard.keycode] &= KEY_RELEASED;
                break;

        }

        if(done)
            break;

        if(redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            for (int i = 0; i < 2; i++) {
                int x1 = rarr[i]->x;
                int y1 = rarr[i]->y;
                int x2 = rarr[i]->x + rarr[i]->w;
                int y2 = rarr[i]->y + rarr[i]->h;

                al_draw_textf(font, al_map_rgb(255, 0, 0), 0, i * 11, 0, "%.1d %.1d %.1d %.1d", x1, y1, x2, y2);
                al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(255,255,255));
            }
            al_draw_textf(font, al_map_rgb(255, 0, 0), 0, SCREEN_H - 11, 0, "%.1d %.1d", b.dx, b.dy);
            al_draw_filled_rectangle(b.x, b.y, b.x + BALL_SIZE, b.y + BALL_SIZE, al_map_rgb(0,255,255));

            al_flip_display();

            redraw = false;
        }
    }

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}
