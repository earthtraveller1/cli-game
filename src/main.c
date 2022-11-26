#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include <Windows.h>
#include <conio.h>
#include <process.h>

char** framebuffer;
int32_t framebuffer_width, framebuffer_height;

bool running = true;

HANDLE global_mutex;
int8_t x = 0, y = 0;

#ifdef _WIN32
    #define get_single_char _getch
#else
    #define get_single_char getchar
#endif

void get_terminal_size(uint32_t* width, uint32_t* height)
{
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);
    
    *width = console_info.srWindow.Right - console_info.srWindow.Left + 1;
    *height = console_info.srWindow.Bottom - console_info.srWindow.Top + 1;
}

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

#ifdef _WIN32
void create_global_mutex()
{
    global_mutex = CreateMutexW(NULL, FALSE, NULL);
}

void wait_for_mutex()
{
    WaitForSingleObject(global_mutex, INFINITE);
}

void release_mutex()
{
    ReleaseMutex(global_mutex);
}
#endif

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

#ifdef _WIN32
struct getch_loop
{
    HANDLE thread;
};

void start_getch_loop(struct getch_loop* self)
{
    self->thread = (HANDLE)_beginthread((_beginthread_proc_type)getch_loop, 64, NULL);
}

void wait_for_getch_loop_to_end(struct getch_loop* self)
{
    WaitForSingleObject(self->thread, INFINITE);
}
#endif

int main()
{
    uint32_t terminal_width, terminal_height;
    get_terminal_size(&terminal_width, &terminal_height);
    
    allocate_framebuffer(terminal_width, terminal_height);
    
    struct getch_loop getch_loop;
    start_getch_loop(&getch_loop);
    
    while (running)
    {
        renderer_begin();
        
        wait_for_mutex();
        
        renderer_draw_rectangle(10, 10, 0 + x, 0 + y, '#');
        renderer_draw_rectangle(10, 10, 50 + x, 10 + y, '#');
        
        release_mutex();
        
        renderer_end();
    }
    
    wait_for_getch_loop_to_end(&getch_loop);
    
    return 0;
}