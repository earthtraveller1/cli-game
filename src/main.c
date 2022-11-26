#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

char** framebuffer;
int32_t framebuffer_width, framebuffer_height;

bool running = true;

int8_t x = 0, y = 0;

void allocate_framebuffer(uint32_t width, uint32_t height)
{
    width--;
    height--;
    
    framebuffer = malloc(height * sizeof(char*));
    for (char** column = framebuffer; column < framebuffer + height; column++)
    {
        *column = malloc(width * sizeof(char*));
    }
    
    framebuffer_width = width;
    framebuffer_height = height;
}

void renderer_begin()
{
    for (char** row = framebuffer; row < framebuffer + framebuffer_height; row++)
    {
        for (char* ch = *row; ch < *row + framebuffer_width; ch++)
        {
            *ch = ' ';
        }
    }
}

void renderer_draw_rectangle(int8_t width, int8_t height, int8_t x, int8_t y, char color)
{
    int8_t left = x, top = y;
    int8_t right = x + width, bottom = y + height;
    
    for (int8_t i = top; i < bottom; i++)
    {
        for (int8_t j = left; j < right; j++)
        {
            /* All coordinates outside of the range simply gets clipped. */
            if (j < framebuffer_width && i < framebuffer_height && j > 0 && i > 0)
            {
                framebuffer[i][j] = color;
            }
        }
    }
}

void renderer_end()
{
    printf("\033[H");
    
    for (char** row = framebuffer; row < framebuffer + framebuffer_height; row++)
    {
        fwrite(*row, 1, framebuffer_width, stdout);
        
        putchar('\n');
    }
}

void handle_event(char event)
{
    wait_for_mutex();
    
    switch (event)
    {
        case 'w':
            y -= 1;
            break;
        case 'a':
            x -= 1;
            break;
        case 's':
            y += 1;
            break;
        case 'd':
            x += 1;
            break;
        case '\033':
            running = false;
            break;
    }
    
    release_mutex();
}

void getch_loop()
{
    while (running)
    {
        handle_event(get_single_char());
    }
}

int main()
{
    uint32_t terminal_width, terminal_height;
    get_terminal_size(&terminal_width, &terminal_height);
    allocate_framebuffer(terminal_width, terminal_height);
    create_global_mutex();
    start_getch_loop(getch_loop);
    
    while (running)
    {
        renderer_begin();
        
        wait_for_mutex();
        
        renderer_draw_rectangle(10, 10, 0 + x, 0 + y, '#');
        renderer_draw_rectangle(10, 10, 50 + x, 10 + y, '#');
        
        release_mutex();
        
        renderer_end();
    }
    
    join_getch_loop();
    
    return 0;
}