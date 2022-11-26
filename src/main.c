#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "platform.h"

char** framebuffer;
int32_t framebuffer_width, framebuffer_height;

bool running = true;

double x = 20.0, y = 10.0;
double delta_time = 0.0;

clock_t timer_start = 0;

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
    
    /*switch (event)
    {
        /* Do stuff here. /
    }*/
    
    release_mutex();
}

void getch_loop()
{
    while (running)
    {
        handle_event(get_single_char());
    }
}

void start_timer()
{
    timer_start = clock();
}

double get_time()
{
    return (clock() - timer_start) / 1000.0;
}

int main()
{
    uint32_t terminal_width, terminal_height;
    get_terminal_size(&terminal_width, &terminal_height);
    allocate_framebuffer(terminal_width, terminal_height);
    create_global_mutex();
    start_getch_loop(getch_loop);
    
    start_timer();
    
    bool travelling_left = false;
    
    while (running)
    {
        double start_time = get_time();
        
        if (x > 100.0)
        {
            travelling_left = true;
        }
        else if (x < 10.0)
        {
            travelling_left = false;
        }
        
        if (travelling_left)
        {
            x -= 10.0 * delta_time;
        }
        else 
        {
            x += 10.0 * delta_time;
        }
        
        renderer_begin();
        
        wait_for_mutex();
        
        renderer_draw_rectangle(10, 10, (int8_t)x, (int8_t)y, '#');
        
        release_mutex();
        
        renderer_end();
        
        double end_time = get_time();
        delta_time = end_time - start_time;
    }
    
    join_getch_loop();
    
    return 0;
}