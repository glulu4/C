// dont need extern but

typedef unsigned short color_t;
#define MAKE_COLOR(r,g,b) ( ( (r & 0x1f) << 11) | (( g&0x3f) << 5) | (b&0x1f) )
 
extern void init_graphics();
extern void exit_graphics(); 
extern char getkey();
extern void sleep_ms(long ms);
extern void clear_screen();
extern void draw_pixel(int x, int y, color_t color);
extern void draw_rect(int x1, int y1, int width, int height, color_t c);
extern void draw_text(int x, int y, const char *text, color_t  c);
extern void draw_circle(int x, int y, int r, color_t c); 

