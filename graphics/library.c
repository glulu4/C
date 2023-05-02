#include "graphics.h"
#include "iso_font.h"
#include <sys/mman.h> // for mmap() and munmap
#include <sys/types.h>
#include <sys/ioctl.h>// for ioctl
#include <linux/fb.h> // for FBIOGET_VSCREENINFO
#include <fcntl.h> // for open
#include <unistd.h> // for close( fd )
#include <stdlib.h> // for exit()
#include <termios.h> // for termios struct
#include <sys/select.h> // for select syscall 
#include <time.h> // for nanosleep syscall



static int fd;
static color_t * mapped_file;
static size_t mapSize;
static struct fb_var_screeninfo virtualRes; 
static struct fb_fix_screeninfo bitDepth;

void draw_circle_points(int x, int y, int x_pos, int y_pos, color_t c);


void init_graphics()
{
    int res1,res2;

    // file path, read only
    fd = open( "/dev/fb0", O_RDWR); // frame buffer is the screen, address of screen in memory  

    if ( fd == -1 ){
        write(1, "File descriptor went wrong with open()...", 42);
        exit(1); // 1 is for failure, 0 for good
    }

    // // getting screen resolution and storing it in the structs
    res1 = ioctl(fd, FBIOGET_VSCREENINFO, &virtualRes);
    res2 = ioctl(fd, FBIOGET_FSCREENINFO, &bitDepth);

    if ( res1 == -1 || res2 == -1){
        write(1,"Getting the screen resolution failed...",40 );
        exit(1);
    }

    mapSize = virtualRes.yres_virtual * bitDepth.line_length; 

    // null tells os to chose address, how much memory to get, write permission ( PROT_READ maybe ), 
    // shared with os, the file descriptor, 0--> the offset = 

    mapped_file = mmap(NULL, mapSize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0); // array of pixels 

    if ( mapped_file == MAP_FAILED ){
        write(1, "File mapping went wrong with mmap()...", 39);
        exit(1); // 1 is for failure, 0 for good
    }

    // disable any key presses
    struct termios tSettings;
    if ( ioctl(STDIN_FILENO, TCGETS, &tSettings) == -1 ){  // gets terminal settings
        write(1, "Getting the terminal settings failed with ioctl()...", 53);
        exit(1); 
    }

    tSettings.c_lflag &= ~ICANON; // disables ICANON bit in c_lflag bitfield
    tSettings.c_lflag &= ~ECHO; // disabling ECHO

    if ( ioctl(STDIN_FILENO, TCSETS, &tSettings) == -1 ){ // sets terminal settings
        write(1, "Setting the terminal settings failed with ioctl()...", 53);
        exit(1); 
    } 
}

void exit_graphics()
{
    struct termios tSettings;
    if (ioctl(STDIN_FILENO, TCGETS, &tSettings) == -1 ){ // gettoing terminal settings
        write(1,"Getting terminal settings failed in exit_graphics()... ", 56);
        exit(1);
    }


    tSettings.c_lflag |= (ECHO | ICANON);

    if (ioctl(STDIN_FILENO, TCSETS, &tSettings) == -1 ){ // setting terminal settings
        write(1,"Setting terminal settings back failed in exit_graphics()... ", 61);
        exit(1);
    }


    if (close(fd) == -1 ){ // closes file descripter / framebuffer
        write(1,"Closing the framebuffer failed", 31);
        exit(1);
    } 
    // deletes mapping
    if (munmap(mapped_file, mapSize ) == -1 ){
        write(1,"Deleting the mapping failed", 28);
        exit(1);
    }

}

char getkey()
{
    char keyPressed;
    fd_set rfds;
    struct timeval tv;
    

    /* Watch stdin (fd 0) to see when it has input. */

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    /* Wait up to five seconds. */

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    keyPressed = select(1, &rfds, NULL, NULL, &tv);
    if ( keyPressed == -1 ){
        write(1,"Select() syscall failed in getKey()... ", 40 );
        exit(1);
    }

    if ( read(STDIN_FILENO, &keyPressed, sizeof(keyPressed))  == -1 ){
        write(1,"Reading keystrokes failed... ", 30 );
        exit(1);
    } 
    return keyPressed;

}

void draw_pixel(int x, int y, color_t color)
{
    // bitDepth.line_length returns number of bytes 
    // each pixel is 2 bytes
    
    //int offset = (y * bitDepth.line_length)/sizeof(color_t) + x;
    void * screen = mapped_file;
    screen += y*bitDepth.line_length; // we convert y*bitDepth.line_length, which is in bytes, to a void* 
    color_t * pixels = screen; // now that its a void pointer, we can do color_t arithmatic on it
    *(pixels + x ) = color; // adding x to a color_t scales it to adding x amount of color_t's
    //mapped_file[offset] = color;
}

void sleep_ms(long ms)
{
    struct timespec sleepTime;
    sleepTime.tv_sec = ms / 1000;
    sleepTime.tv_nsec = (ms % 1000) * 1000000;
    if (nanosleep(&sleepTime, NULL) == -1 ){
        write(1,"nanosleep() failed", 19 );
        exit(1);
    }
}

void clear_screen()
{
    write(1, "\033[2J", 4);
}


void draw_rect(int x1, int y1, int width, int height, color_t c)
{
    int i;
    int j;

    // draws horizntil lines
    for ( i = 0; i < width; i++ ){
        draw_pixel(x1 + i, y1, c);
        draw_pixel(x1 + i, y1+height, c);
    }

    // draws vertical lines
    for ( j = 0; j < height; j++ ){
        draw_pixel(x1, y1 + j, c);
        draw_pixel(x1 + width, y1 + j, c);
    }

}

void draw_text(int x, int y, const char * text, color_t c)
{
    int updatedX = x;
    int updatedY = y;
    char currLetter; 
    int i= 0;
    int j= 0;
    int k= 0;


    // for every letter
    while ( *(text + i) != '\0' ){
        currLetter = *(text + i);

        char currByte;
        // for each of the 16 one byte values in font file, at the letter
        updatedY = y; // reset y after every letyter
        for ( j= 0; j < 16; j++){

            currByte = iso_font[ (currLetter*16) + j ];
            // for each of the bits in each value
            updatedX = x + i*8; // reset x and move it right
            for ( k = 0; k < 8; k++ ){

                // if its a one
                if ( currByte & 0x01 ){
                    draw_pixel(updatedX, updatedY, c);
                }
                currByte = currByte >> 1;
                updatedX++;
            }


            updatedY++;
        }
        i++;
    }

    // loop through text
    // for each letter
        // store ascii value in a varibel defined
        // loop thorugh the inexes from asciiVal*16 -> asciiVal*16 + 15
        // those 16 values are 1 byte each, so we have to each each bit and if its a one,
        // if its a one, draw a pixel at the updated x and y,  
}

void draw_circle(int x, int y, int r, color_t c) {
    // source of this is chatGBT. 
    int x_pos = 0;
    int y_pos = r;
    int p = 1 - r;

    draw_circle_points(x, y, x_pos, y_pos, c);

    while (x_pos < y_pos) {
        x_pos++;
        if (p < 0) {
            p += 2 * x_pos + 1;
        } else {
            y_pos--;
            p += 2 * (x_pos - y_pos) + 1;
        }
        draw_circle_points(x, y, x_pos, y_pos, c);
    }
}

void draw_circle_points(int x, int y, int x_pos, int y_pos, color_t c) {
    draw_pixel(x + x_pos, y + y_pos, c);
    draw_pixel(x - x_pos, y + y_pos, c);
    draw_pixel(x + x_pos, y - y_pos, c);
    draw_pixel(x - x_pos, y - y_pos, c);
    draw_pixel(x + y_pos, y + x_pos, c);
    draw_pixel(x - y_pos, y + x_pos, c);
    draw_pixel(x + y_pos, y - x_pos, c);
    draw_pixel(x - y_pos, y - x_pos, c);
}




