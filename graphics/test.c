#include "graphics.h"

typedef unsigned short color_t;

void exit_graphics();
void init_graphics();
void draw_pixel(int x, int y, color_t color);
void sleep_ms(long ms);
void clear_screen();
void draw_text(int x, int y, const char *text, color_t  c);
void draw_circle(int x, int y, int r, color_t c);

int main()
{
    clear_screen();
    color_t color = MAKE_COLOR(31,63,31);
    init_graphics();
    int i;
    char key;

    draw_text(75, 75, "A big white rectangle will appear in 5 seconds\0", MAKE_COLOR(0,0,27) );
    sleep_ms(5000);
    draw_rect( 100, 100, 350, 100, color );

    draw_text(90, 150, "A circle will appear in green in 2 seconds\0", MAKE_COLOR(0,63,0) );
    sleep_ms(2000);
    draw_circle( 350, 300, 50, MAKE_COLOR(0,63,0));

    draw_text(75, 250, "wait for it... btw press p to exit\0", MAKE_COLOR(25,20,20) );
    sleep_ms(1000);
    draw_text(300, 300, "HELLO WORLD\0", MAKE_COLOR(31,0,0) );

    key = getkey();
    if ( key == 'p'){
        clear_screen();
        exit_graphics();
    }
       
    
    
    
    return 0;
}