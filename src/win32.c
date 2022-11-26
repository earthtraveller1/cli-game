#include <Windows.h>
#include <process.h>
#include <conio.h>

#include "platform.h"

static HANDLE global_mutex;

static HANDLE getch_thread;

int get_single_char()
{
    return _getch();
}

void get_terminal_size(uint32_t* width, uint32_t* height)
{
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);
    
    *width = console_info.srWindow.Right - console_info.srWindow.Left + 1;
    *height = console_info.srWindow.Bottom - console_info.srWindow.Top + 1;
}

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

void start_getch_loop(getch_loop_proc proc)
{
    getch_thread = (HANDLE)_beginthread((_beginthread_proc_type)proc, 64, NULL);
}

void join_getch_loop()
{
    WaitForSingleObject(getch_thread, INFINITE);
}

void sleep(uint64_t milliseconds)
{
    Sleep(milliseconds);
}
